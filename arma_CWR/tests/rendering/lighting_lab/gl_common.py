"""Renderer-independent GL plumbing for the lighting lab (moderngl).

- headless context creation (EGL, llvmpipe) + glClipControl via ctypes
- D3D-style matrices exactly as the engine builds them (ConvertMatrix,
  ConvertProjectionMatrix, Camera::Adjust) and uploads them (row-major
  GfxMatrix memory read by std140 as column-major)
- texture upload (RGBA8, mipmapped, wrap per texture, anisotropy off)
- interleaved mesh VAOs that tolerate attributes a program does not use
- program creation with the engine's block/sampler bindings
  (EngineGL33::InitPixelShaders bindProgramSamplersAndBlocks)
"""

from __future__ import annotations

import ctypes
import os
from dataclasses import dataclass

import numpy as np

import moderngl

GL_LOWER_LEFT = 0x8CA1
GL_ZERO_TO_ONE = 0x935F

# std140 block bindings used by EngineGL33
BINDING_VS = 0
BINDING_PS = 1
BINDING_WORLD = 2


def create_context() -> moderngl.Context:
    """Headless GL 3.3+ context. Tries EGL (Mesa llvmpipe), then the default
    standalone backend (works under xvfb-run)."""
    os.environ.setdefault("LIBGL_ALWAYS_SOFTWARE", "1")
    errors = []
    for kwargs in (
        dict(backend="egl", require=330),
        dict(standalone=True, backend="egl"),
        dict(standalone=True, require=330),
    ):
        try:
            if "standalone" in kwargs:
                return moderngl.create_context(**kwargs)
            return moderngl.create_standalone_context(**kwargs)
        except Exception as e:  # noqa: BLE001 - report all backends' failures
            errors.append(f"{kwargs}: {e}")
    raise RuntimeError("could not create a GL context:\n  " + "\n  ".join(errors) +
                       "\nTry: xvfb-run -a python run_lab.py ...")


_clip_control_state: dict[int, bool] = {}


def _get_proc(name: bytes):
    for lib, getter in (("libEGL.so.1", "eglGetProcAddress"), ("libGL.so.1", "glXGetProcAddressARB")):
        try:
            dll = ctypes.CDLL(lib)
            fn = getattr(dll, getter)
            fn.restype = ctypes.c_void_p
            fn.argtypes = [ctypes.c_char_p]
            p = fn(name)
            if p:
                return p
        except (OSError, AttributeError):
            continue
    return None


def enable_clip_control(ctx: moderngl.Context) -> bool:
    """glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE) as EngineGL33::Init3DState.

    moderngl does not wrap it; call it through the driver's proc address.
    Returns False when unavailable (then depth is mapped -1..1 by GL and the
    near half of the depth range is unused; the image is unaffected).
    """
    key = id(ctx)
    if key in _clip_control_state:
        return _clip_control_state[key]
    ok = False
    if "GL_ARB_clip_control" in ctx.extensions or ctx.version_code >= 450:
        p = _get_proc(b"glClipControl")
        if p:
            ctypes.CFUNCTYPE(None, ctypes.c_uint, ctypes.c_uint)(p)(GL_LOWER_LEFT, GL_ZERO_TO_ONE)
            ok = ctx.error == "GL_NO_ERROR"
    _clip_control_state[key] = ok
    return ok


# ---------------------------------------------------------------------------
# Matrices (engine GfxMatrix layout: D3D row-vector convention, row-major)
# ---------------------------------------------------------------------------


def gfx_matrix(aside, up, direction, position) -> np.ndarray:
    """ConvertMatrix: rows _1x = DirectionAside, _2x = DirectionUp, _3x = Direction,
    _4x = Position; _14 = _24 = _34 = 0, _44 = 1. Returns float32[16] (_11.._44)."""
    m = np.zeros((4, 4), dtype=np.float64)
    m[0, :3] = aside
    m[1, :3] = up
    m[2, :3] = direction
    m[3, :3] = position
    m[3, 3] = 1.0
    return m.astype(np.float32).reshape(16)


def view_matrix(aside, up, forward) -> np.ndarray:
    """BuildFrameState: ConvertMatrix(camera->InverseScaled()) with _41.._43 = 0.

    The inverse of the camera frame R = [aside up forward] (columns) is R^T, so
    its DirectionAside/Up/Direction (columns of R^T) are the x/y/z components
    of the camera axes.
    """
    R = np.stack([aside, up, forward], axis=1)  # columns
    Rt = R.T
    return gfx_matrix(Rt[:, 0], Rt[:, 1], Rt[:, 2], np.zeros(3))


def world_matrix_camera_relative(cam_pos, rotation=None, position=None) -> np.ndarray:
    """Object world matrix with camera-relative translation (the engine renders
    camera-relative; the VS camPos uniform is 0). Mesh data is already in world
    space, so the rotation defaults to identity and the translation is -cam."""
    rot = np.eye(3) if rotation is None else np.asarray(rotation)
    pos = np.zeros(3) if position is None else np.asarray(position)
    return gfx_matrix(rot[:, 0], rot[:, 1], rot[:, 2], pos - np.asarray(cam_pos))


@dataclass
class Frustum:
    c_left: float
    c_top: float
    c_near: float
    c_far: float


def aspect_settings(width: int, height: int) -> tuple[float, float]:
    """AspectRatio::BuildSettingsForRatio -> (leftFOV, topFOV)."""
    k_base_ratio = 4.0 / 3.0
    ratio = width / float(height)
    if ratio >= k_base_ratio:
        return 0.75 * ratio, 0.75
    return 1.0, 1.0 / ratio


def camera_frustum(width: int, height: int, fov: float, far: float) -> Frustum:
    """World.cpp: cNear = clamp(0.067 / fov, 0.07, 0.2); far = fog max range;
    Camera::SetPerspectiveForView: cLeft = fov * leftFOV, cTop = fov * topFOV."""
    left_fov, top_fov = aspect_settings(width, height)
    c_near = min(max(0.067 / fov, 0.07), 0.2)
    return Frustum(fov * left_fov, fov * top_fov, c_near, far)


def projection_matrix(fr: Frustum) -> np.ndarray:
    """Camera::Adjust (_projectionNormal) + ConvertProjectionMatrix (zBias 0).

    x' = x / cLeft, y' = y / cTop, z' = q z - q cNear, w' = z; q = ccFar / (ccFar - cNear),
    ccFar = max(500, cFar * 1.01). Depth range 0..1 (glClipControl ZERO_TO_ONE).
    """
    cc_far = max(500.0, fr.c_far * 1.01)
    q = cc_far / (cc_far - fr.c_near)
    m = np.zeros((4, 4), dtype=np.float64)
    m[0, 0] = 1.0 / fr.c_left
    m[1, 1] = 1.0 / fr.c_top
    m[2, 2] = q
    m[2, 3] = 1.0
    m[3, 2] = -q * fr.c_near
    return m.astype(np.float32).reshape(16)


def gfx_to_numpy_column(m16: np.ndarray) -> np.ndarray:
    """GLSL view of a GfxMatrix (column-vector 4x4): the transpose of the D3D matrix."""
    return np.asarray(m16, dtype=np.float64).reshape(4, 4).T


# ---------------------------------------------------------------------------
# Resources
# ---------------------------------------------------------------------------


def upload_texture(ctx: moderngl.Context, rgba: np.ndarray, clamp_u=False, clamp_v=False) -> moderngl.Texture:
    """RGBA8 texture, full mip chain, trilinear, wrap/clamp per axis, anisotropy 1.

    (The engine's sampler objects use 16x anisotropy; the lab keeps it off for
    llvmpipe speed and determinism.)
    """
    if rgba.dtype != np.uint8:
        rgba = np.clip(np.round(rgba * 255.0), 0, 255).astype(np.uint8)
    h, w = rgba.shape[:2]
    tex = ctx.texture((w, h), 4, np.ascontiguousarray(rgba).tobytes())
    tex.build_mipmaps()
    tex.filter = (moderngl.LINEAR_MIPMAP_LINEAR, moderngl.LINEAR)
    tex.repeat_x = not clamp_u
    tex.repeat_y = not clamp_v
    tex.anisotropy = 1.0
    return tex


def create_program(ctx: moderngl.Context, vs: str, fs: str, name: str = "") -> moderngl.Program:
    try:
        prog = ctx.program(vertex_shader=vs, fragment_shader=fs)
    except moderngl.Error as e:
        raise RuntimeError(f"shader program {name!r} failed to build:\n{e}") from e
    bind_program_blocks_and_samplers(prog)
    return prog


def bind_program_blocks_and_samplers(prog: moderngl.Program) -> None:
    """EngineGL33::InitPixelShaders bindProgramSamplersAndBlocks."""
    for block, binding in (("WorldInstances", BINDING_WORLD), ("VSConstants", BINDING_VS), ("PSConstants", BINDING_PS)):
        if block in prog:
            prog[block].binding = binding
    for sampler, unit in (("tex0", 0), ("tex1", 1), ("shadowMap", 2)):
        if sampler in prog:
            prog[sampler].value = unit


def block_size(prog: moderngl.Program, name: str) -> int:
    return prog[name].size if name in prog else 0


VERTEX_ATTRS = (("pos", "3f"), ("normal", "3f"), ("uv", "2f"), ("color", "4f"))


def interleave(mesh, colors: np.ndarray | None = None) -> tuple[np.ndarray, list[str]]:
    cols = [mesh.positions, mesh.normals, mesh.uvs]
    names = ["pos", "normal", "uv"]
    if colors is not None:
        cols.append(np.asarray(colors, dtype=np.float32))
        names.append("color")
    data = np.hstack([np.asarray(c, dtype=np.float32) for c in cols])
    return np.ascontiguousarray(data), names


def make_vao(ctx: moderngl.Context, prog: moderngl.Program, vbo: moderngl.Buffer, ibo: moderngl.Buffer | None,
             names: list[str]) -> moderngl.VertexArray:
    """VAO over an interleaved buffer; attributes the program lacks become padding."""
    fmt_parts, used = [], []
    sizes = dict(VERTEX_ATTRS)
    for n in names:
        f = sizes[n]
        if n in prog:
            fmt_parts.append(f)
            used.append(n)
        else:
            fmt_parts.append(f"{int(f[0]) * 4}x")
    content = [(vbo, " ".join(fmt_parts), *used)]
    if ibo is None:
        return ctx.vertex_array(prog, content)
    return ctx.vertex_array(prog, content, index_buffer=ibo, index_element_size=4)


def read_fbo_image(fbo: moderngl.Framebuffer, width: int, height: int):
    """Read RGBA8 colour attachment 0 as a top-down numpy array (H, W, 3)."""
    raw = fbo.read(components=3, alignment=1)
    img = np.frombuffer(raw, dtype=np.uint8).reshape(height, width, 3)
    return img[::-1].copy()  # GL rows are bottom-up (lower-left origin)
