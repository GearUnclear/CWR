"""CPU shadows of the GL33 uniform blocks and the upload functions that fill them.

Mirrors arma_CWR/engine/PoseidonGL33:
  EngineGL33.hpp        namespace VSConst slots, struct PSConstants slots
  EngineGL33_Shaders.cpp s_vsShadow[280] / s_psShadow[108], BuildFrameState,
                        UploadFrameConstants, UploadVSMaterialConstants,
                        UploadVSTexGenConstants, SetShaderFogEnabled,
                        UpdateShadowMapLitState, EnableNightEye, UploadPSFogColor,
                        UploadVSWorldMatrix (WorldInstances slot 0)
  EngineGL33_State.cpp  SetAlphaTest

Each function takes the shadow arrays explicitly so another renderer can reuse
them with its own extended layout. Buffers are created at least as large as the
C++ arrays AND the linked block size (whichever is bigger).
"""

from __future__ import annotations

from dataclasses import dataclass

import numpy as np

import moderngl

import gl_common as G
from lighting import SunLighting, TLMaterial, vs_material_constants


class VSConst:
    PROJ = 0
    VIEW = 4
    WORLD = 8
    SUN_DIR = 12
    AMBIENT = 13
    DIFFUSE = 14
    EMISSIVE = 15
    FOG_PARAM = 16
    CAM_POS = 17
    SPECULAR = 18
    SPEC_EN = 19
    SUN_EN = 20
    VP_SCALE = 21
    TEX_MAT0 = 24
    TEX_MAT1 = 28
    TEX_CTRL = 32
    LIGHT_COUNT = 33
    LIGHT_POS = 34
    LIGHT_DIFFUSE = 42
    LIGHT_AMBIENT = 50
    LIGHT_DIR = 58
    LIGHT_VP = 66
    MAX_LOCAL_LIGHTS = 8
    N_FLOATS = 280  # static float s_vsShadow[280]


class PSConst:
    FOG_COLOR = 0
    ALPHA_REF = 1
    SHADOW_CTL = 2
    CONST_COLOR = 3
    LIGHT_DIR = 4
    GRASS_COEF1 = 5
    GRASS_COEF2 = 6
    RGB_EYE_COEF = 7
    CASCADE_VP = 8
    CASCADE_SPLITS = 24
    CASCADE_CTL = 25
    CAM_FWD = 26
    N_FLOATS = 108  # static float s_psShadow[108]


# TexGenMode (EngineGL33 enum)
TG_NONE, TG_FIXED, TG_DETAIL, TG_GRASS, TG_WATER = "none", "fixed", "detail", "grass", "water"

IDENTITY16 = np.eye(4, dtype=np.float32).reshape(16)
MAT_TRANS32 = np.diag([32.0, 32.0, 32.0, 1.0]).astype(np.float32).reshape(16)
MAT_TRANS64 = np.diag([64.0, 64.0, 64.0, 1.0]).astype(np.float32).reshape(16)


class ConstantBlock:
    """A float shadow array addressed in vec4 slots (like s_vsShadow / s_psShadow)."""

    def __init__(self, n_floats: int):
        self.data = np.zeros(n_floats, dtype=np.float32)

    def set(self, slot: int, values) -> None:
        v = np.asarray(values, dtype=np.float32).reshape(-1)
        self.data[slot * 4: slot * 4 + v.size] = v

    def get(self, slot: int, n: int = 4) -> np.ndarray:
        return self.data[slot * 4: slot * 4 + n]

    def check_finite(self, label: str) -> None:
        if not np.all(np.isfinite(self.data)):
            bad = np.nonzero(~np.isfinite(self.data))[0]
            raise FloatingPointError(f"{label}: non-finite constants at float offsets {bad[:16].tolist()}")


@dataclass
class FrameState:
    """EngineGL33 FrameState (BuildFrameState)."""

    view: np.ndarray  # float32[16] GfxMatrix
    projection: np.ndarray  # float32[16]
    camera_pos: np.ndarray  # world position (used only for camera-relative world matrices)
    viewport: tuple[int, int, int, int]
    fog_params: np.ndarray  # {start, 1/(end-start), enabled, 0}
    fog_color: np.ndarray  # RGBA
    sun_dir: np.ndarray  # {dir.xyz, 0}
    sun_enabled: bool = True


def build_frame_state(camera, frustum: G.Frustum, sun: SunLighting, width: int, height: int) -> FrameState:
    aside, up, fwd = camera.basis()
    fs, fe = sun.preset.fog_start, sun.preset.fog_end
    inv = 1.0 / (fe - fs) if fe > fs else 0.0
    return FrameState(
        view=G.view_matrix(aside, up, fwd),
        projection=G.projection_matrix(frustum),
        camera_pos=np.asarray(camera.position, dtype=np.float64),
        viewport=(0, 0, width, height),
        fog_params=np.array([fs, inv, 1.0, 0.0], dtype=np.float32),
        fog_color=np.array([sun.fog_color[0], sun.fog_color[1], sun.fog_color[2], 1.0], dtype=np.float32),
        sun_dir=np.array([*sun.direction, 0.0], dtype=np.float32),
        sun_enabled=True,
    )


# --- VS uploads --------------------------------------------------------------


def upload_frame_constants(vs: ConstantBlock, ps: ConstantBlock, frame: FrameState) -> None:
    """EngineGL33::UploadFrameConstants (+ UploadPSFogColor)."""
    vs.set(VSConst.PROJ, frame.projection)
    vs.set(VSConst.VIEW, frame.view)
    vs.set(VSConst.SUN_DIR, frame.sun_dir)
    vs.set(VSConst.SUN_EN, [1.0 if frame.sun_enabled else 0.0, 0, 0, 0])
    vs.set(VSConst.FOG_PARAM, frame.fog_params)
    vs.set(VSConst.CAM_POS, [0, 0, 0, 0])  # camera-relative rendering
    vs.set(VSConst.TEX_CTRL, [0, 0, 0, 0])
    upload_ps_fog_color(ps, frame.fog_color)


def set_shader_fog_enabled(vs: ConstantBlock, frame: FrameState, enabled: bool) -> None:
    """EngineGL33::SetShaderFogEnabled."""
    frame.fog_params[2] = 1.0 if enabled else 0.0
    vs.set(VSConst.FOG_PARAM, frame.fog_params)


def upload_vs_material_constants(vs: ConstantBlock, sun: SunLighting, mat: TLMaterial) -> dict:
    """EngineGL33::UploadVSMaterialConstants."""
    c = vs_material_constants(sun, mat)
    vs.set(VSConst.AMBIENT, c["ambient"])
    vs.set(VSConst.DIFFUSE, c["diffuse"])
    vs.set(VSConst.EMISSIVE, c["emissive"])
    vs.set(VSConst.SPECULAR, c["specular"])
    vs.set(VSConst.SPEC_EN, c["specEn"])
    return c


def upload_vs_lights_none(vs: ConstantBlock) -> None:
    """EngineGL33::UploadVSLights with no local lights (daytime: nightEffect 0)."""
    vs.set(VSConst.LIGHT_COUNT, [0, 0, 0, 0])


def upload_vs_texgen_constants(vs: ConstantBlock, mode: str, time_s: float = 0.0) -> None:
    """EngineGL33::UploadVSTexGenConstants."""
    if mode in (TG_FIXED, TG_NONE):
        vs.set(VSConst.TEX_CTRL, [0, 0, 0, 0])
    elif mode in (TG_DETAIL, TG_GRASS):
        vs.set(VSConst.TEX_CTRL, [0, 1, 0, 0])
        vs.set(VSConst.TEX_MAT1, MAT_TRANS32)
    elif mode == TG_WATER:
        move = IDENTITY16.copy()
        zoom = MAT_TRANS64.copy()
        mw1 = np.sin(time_s * 0.04)
        mw2 = np.fmod(time_s * 0.3 + np.sin(time_s * 0.5) * 0.5, 2.0)
        move[8], move[9] = mw1 * 0.5, mw1
        zoom[8], zoom[9] = mw2 * 0.5, mw2
        vs.set(VSConst.TEX_CTRL, [1, 1, 0, 0])
        vs.set(VSConst.TEX_MAT0, move)
        vs.set(VSConst.TEX_MAT1, zoom)
    else:
        raise ValueError(mode)


def upload_vs_world_matrix(vs: ConstantBlock, world: ConstantBlock, m16: np.ndarray) -> None:
    """EngineGL33::UploadVSWorldMatrix: VSConstants.world (padding) + WorldInstances[0]."""
    vs.set(VSConst.WORLD, m16)
    world.set(0, m16)


# --- PS uploads --------------------------------------------------------------


def upload_ps_fog_color(ps: ConstantBlock, fog_color) -> None:
    c = np.asarray(fog_color, dtype=np.float32)
    ps.set(PSConst.FOG_COLOR, [c[0], c[1], c[2], 1.0])


def set_alpha_test(ps: ConstantBlock, enable: bool, ref_byte: int, a2c: bool = False, flat_debug: bool = False) -> None:
    """EngineGL33::SetAlphaTest: {ref/255, enabled, alphaToCoverage, flatDebug}."""
    ps.set(PSConst.ALPHA_REF, [ref_byte / 255.0, 1.0 if enable else 0.0, 1.0 if a2c else 0.0, 1.0 if flat_debug else 0.0])


def init_ps_defaults(ps: ConstantBlock) -> None:
    """InitPixelShaders: PSConstants{} defaults — constColor white (else black
    output), rgbEyeCoef {0,0,0,1}."""
    ps.set(PSConst.CONST_COLOR, [1, 1, 1, 1])
    ps.set(PSConst.RGB_EYE_COEF, [0, 0, 0, 1])


def update_shadow_map_lit_state_disabled(ps: ConstantBlock) -> None:
    """UpdateShadowMapLitState with shadow maps off: ctl {0,0,1,0}, splits 0,
    cascadeCtl 0, camFwd {0,0,1,0}."""
    ps.set(PSConst.SHADOW_CTL, [0.0, 0.0, 1.0, 0.0])
    ps.set(PSConst.CASCADE_SPLITS, [0, 0, 0, 0])
    ps.set(PSConst.CASCADE_CTL, [0, 0, 0, 0])
    ps.set(PSConst.CAM_FWD, [0, 0, 1, 0])


def enable_night_eye(ps: ConstantBlock, rgb_eye_coef) -> None:
    ps.set(PSConst.RGB_EYE_COEF, rgb_eye_coef)


# --- GPU side ----------------------------------------------------------------


class UniformBuffers:
    """GL buffers for the three blocks, bound to bindings 0/1/2 like
    InitVertexShaders / InitPixelShaders."""

    def __init__(self, ctx: moderngl.Context, vs_floats: int = VSConst.N_FLOATS, ps_floats: int = PSConst.N_FLOATS,
                 min_vs_bytes: int = 0, min_ps_bytes: int = 0):
        self.vs = ConstantBlock(max(vs_floats, (min_vs_bytes + 3) // 4))
        self.ps = ConstantBlock(max(ps_floats, (min_ps_bytes + 3) // 4))
        self.world = ConstantBlock(256 * 16)
        self.vs_buf = ctx.buffer(reserve=self.vs.data.nbytes, dynamic=True)
        self.ps_buf = ctx.buffer(reserve=self.ps.data.nbytes, dynamic=True)
        self.world_buf = ctx.buffer(reserve=self.world.data.nbytes, dynamic=True)
        self.bind()

    def bind(self) -> None:
        self.vs_buf.bind_to_uniform_block(G.BINDING_VS)
        self.ps_buf.bind_to_uniform_block(G.BINDING_PS)
        self.world_buf.bind_to_uniform_block(G.BINDING_WORLD)

    def flush(self, label: str = "") -> None:
        """FlushVSConstants / FlushPSConstants / world slot 0, with a finiteness check."""
        self.vs.check_finite(f"VSConstants {label}")
        self.ps.check_finite(f"PSConstants {label}")
        self.vs_buf.write(self.vs.data.tobytes())
        self.ps_buf.write(self.ps.data.tobytes())
        self.world_buf.write(self.world.data[:16].tobytes(), offset=0)

    def release(self) -> None:
        for b in (self.vs_buf, self.ps_buf, self.world_buf):
            b.release()
