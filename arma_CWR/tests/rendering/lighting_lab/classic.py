"""Renderer for the CURRENT engine pipeline ("classic").

Draws the synthetic scene through the engine's real GLSL, extracted from
EngineGL33_Shaders.cpp (working tree or any git revision):

  sky + sun     lab pre-lit VS (stand-in for the CPU TL path, see PRELIT_VS) + s_psNormalGLSL
  terrain       s_vsTransformGLSL + s_psDetailGLSL, detail texture on unit 1, TGDetail texgen
  horizont      s_vsTransformGLSL + s_psNormalGLSL, far-plane haze band (Landscape::DrawHorizont)
  objects       s_vsTransformGLSL + s_psNormalGLSL
  foliage       same, alpha test ref 0xC0 (PassId::Cutout), no alpha-to-coverage
  final         s_vsGammaGLSL + s_psGammaGLSL, invGamma = 1 / gamma

into an RGBA8 colour target (the engine's default framebuffer is 8-bit, so lit
colour is clamped on write as well as in the VS), with the std140 blocks packed
slot-for-slot as EngineGL33 does (engine_state.py).

Renderer interface (used by run_lab.py):
  configure(source=None, shader_override_dir=None, nan_check=True)
  render(ctx, scene, preset, width, height) -> PIL.Image   (metadata in image.info["lab"])
"""

from __future__ import annotations

import numpy as np
from PIL import Image

import moderngl

import engine_state as E
import gl_common as G
import glsl_extract
from lighting import compute_lighting, quantize8, resolve_material, sky_vertex_colors, sun_object_colors
from scene import HORIZON_MATERIAL, horizon_polygon, sun_billboards

NAME = "classic"

# Lab-only vertex shader for CPU-lit geometry (sky dome, sun/halo). The engine
# lights these vertices on the CPU (TLVertexTable::DoSkyLighting / sun-object
# lighting), projects them on the CPU and draws them with s_vsScreenGLSL. The
# lab keeps the CPU colour computation (lighting.sky_vertex_colors /
# sun_object_colors) but transforms on the GPU with the same proj/view/world
# constants, which avoids re-implementing the CPU clipper. Varyings match
# s_vsScreenGLSL: vSpecColor = 0 and vFogTC = specular.a = 1 (PackedBlack).
PRELIT_VS = """#version 330 core
layout(std140) uniform VSConstants {
    mat4 proj;          // c0-c3
    mat4 view;          // c4-c7
};
layout(std140) uniform WorldInstances {
    mat4 worldArr[256];
};
layout(location = 0) in vec3 pos;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec4 color;
out vec4 vColor;
out vec4 vSpecColor;
out vec2 vUV0;
out vec2 vUV1;
out float vFogTC;
out vec3 vWorldRel;
void main() {
    vec4 worldPos = worldArr[0] * vec4(pos, 1.0);
    gl_Position = proj * (view * worldPos);
    vColor = color;
    vSpecColor = vec4(0.0);
    vUV0 = uv;
    vUV1 = uv;
    vFogTC = 1.0;
    vWorldRel = worldPos.xyz;
}
"""

ALPHA_REF_CUTOUT = 0xC0  # BuildRenderPassDescriptor: IsTransparent -> AlphaMode::Test, ref 0xc0
ALPHA_REF_OPAQUE = 0xC0  # opaque path keeps ref 0xc0 with the test disabled
ALPHA_REF_SUN = 1  # IsAlphaFog path: Test, ref 1
ALPHA_REF_ALPHA = 1  # IsAlpha path (horizont): AlphaBlend, Test, ref 1

_options: dict = {"source": None, "shader_override_dir": None, "nan_check": True}
_cache: dict = {}


def configure(source=None, shader_override_dir=None, nan_check=True, **_kw) -> None:
    """source: C++ shader file path, git rev, or rev:path (None = worktree).
    shader_override_dir: <dir>/<name>.glsl replacements (engine override convention).
    nan_check: also render each frame into an RGBA32F target and count non-finite pixels."""
    new = {"source": source, "shader_override_dir": shader_override_dir, "nan_check": bool(nan_check)}
    if (new["source"], new["shader_override_dir"]) != (_options["source"], _options["shader_override_dir"]):
        _cache.clear()
    _options.update(new)


class _Resources:
    def __init__(self, ctx: moderngl.Context, scene, source, override_dir):
        self.ctx = ctx
        self.scene = scene
        self.source = source
        self.override_dir = override_dir
        self.shaders = glsl_extract.load_shaders(source)
        self.overridden = glsl_extract.apply_override_dir(self.shaders, override_dir)
        need = ["s_vsTransformGLSL", "s_psNormalGLSL", "s_psDetailGLSL", "s_vsGammaGLSL", "s_psGammaGLSL"]
        missing = [n for n in need if n not in self.shaders]
        if missing:
            raise KeyError(f"shader source {glsl_extract.describe_source(source)} lacks {missing}")
        sh = self.shaders
        self.prog = {
            "normal": G.create_program(ctx, sh["s_vsTransformGLSL"], sh["s_psNormalGLSL"], "vsTransform+psNormal"),
            "detail": G.create_program(ctx, sh["s_vsTransformGLSL"], sh["s_psDetailGLSL"], "vsTransform+psDetail"),
            "prelit": G.create_program(ctx, PRELIT_VS, sh["s_psNormalGLSL"], "prelit+psNormal"),
            "gamma": G.create_program(ctx, sh["s_vsGammaGLSL"], sh["s_psGammaGLSL"], "gamma"),
        }
        vs_bytes = max(G.block_size(p, "VSConstants") for p in self.prog.values())
        ps_bytes = max(G.block_size(p, "PSConstants") for p in self.prog.values())
        self.block_sizes = {"VSConstants": vs_bytes, "PSConstants": ps_bytes,
                            "WorldInstances": G.block_size(self.prog["normal"], "WorldInstances")}
        self.ubo = E.UniformBuffers(ctx, min_vs_bytes=vs_bytes, min_ps_bytes=ps_bytes)
        self.textures = {k: G.upload_texture(ctx, v["rgba"], v["clamp_u"], v["clamp_v"])
                         for k, v in scene.textures.items() if not k.startswith("_")}
        self.sky_textures: dict = {}
        # unit 2 shadowMap: a 1x1x1 array texture so the sampler is complete
        self.dummy_array = ctx.texture_array((1, 1, 1), 1, np.ones(1, dtype=np.float32).tobytes(), dtype="f4")
        self.meshes = []
        for m in scene.meshes:
            data, names = G.interleave(m)
            vbo = ctx.buffer(data.tobytes())
            ibo = ctx.buffer(np.ascontiguousarray(m.indices, dtype=np.uint32).tobytes())
            prog = self.prog["detail" if m.shader == "detail" else "normal"]
            self.meshes.append((m, G.make_vao(ctx, prog, vbo, ibo, names)))
        sky = scene.sky
        self.sky_vbo = ctx.buffer(reserve=sky.positions.shape[0] * 12 * 4, dynamic=True)
        self.sky_ibo = ctx.buffer(np.ascontiguousarray(sky.indices, dtype=np.uint32).tobytes())
        self.sky_vao = G.make_vao(ctx, self.prog["prelit"], self.sky_vbo, self.sky_ibo, ["pos", "normal", "uv", "color"])
        self.sun_vbo = ctx.buffer(reserve=8 * 12 * 4, dynamic=True)
        self.sun_ibo = ctx.buffer(np.array([0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7], dtype=np.uint32).tobytes())
        self.sun_vao = G.make_vao(ctx, self.prog["prelit"], self.sun_vbo, self.sun_ibo, ["pos", "normal", "uv", "color"])
        self.hor_vbo = ctx.buffer(reserve=8 * 8 * 4, dynamic=True)
        self.hor_ibo = ctx.buffer(reserve=4 * 3 * 4, dynamic=True)
        self.hor_vao = G.make_vao(ctx, self.prog["normal"], self.hor_vbo, self.hor_ibo, ["pos", "normal", "uv"])
        self.gamma_vao = ctx.vertex_array(self.prog["gamma"], [])
        self.targets: dict = {}

    def sky_texture(self, sky_through: float):
        key = round(float(sky_through), 4)
        if key not in self.sky_textures:
            self.sky_textures[key] = G.upload_texture(self.ctx, self.scene.sky_texture(sky_through), clamp_u=False,
                                                      clamp_v=True)
        return self.sky_textures[key]

    def target(self, w: int, h: int, dtype: str = "f1"):
        """Scene target: colour texture (RGBA8 like the default framebuffer, or
        RGBA32F for the NaN check) + 24-bit depth."""
        key = (w, h, dtype)
        if key not in self.targets:
            ctx = self.ctx
            color = ctx.texture((w, h), 4, dtype=dtype)
            color.filter = (moderngl.NEAREST, moderngl.NEAREST)  # ApplyGammaPass: GL_NEAREST, clamp
            color.repeat_x = color.repeat_y = False
            depth = ctx.depth_renderbuffer((w, h))
            fbo = ctx.framebuffer(color_attachments=[color], depth_attachment=depth)
            self.targets[key] = (fbo, color)
        return self.targets[key]

    def output_target(self, w: int, h: int):
        key = (w, h, "out")
        if key not in self.targets:
            rb = self.ctx.renderbuffer((w, h), 4)
            self.targets[key] = (self.ctx.framebuffer(color_attachments=[rb]), rb)
        return self.targets[key]


def _resources(ctx, scene) -> _Resources:
    key = (id(ctx), id(scene), _options["source"], _options["shader_override_dir"])
    if key not in _cache:
        _cache.clear()
        _cache[key] = _Resources(ctx, scene, _options["source"], _options["shader_override_dir"])
    return _cache[key]


def set_pipeline(ctx, depth: bool, cull: bool, blend: bool, depth_write: bool = True) -> None:
    """Subset of EngineGL33::ApplyPipeline the lab needs."""
    if ctx.fbo is not None:
        ctx.fbo.depth_mask = depth_write  # DepthMode::ReadOnly -> glDepthMask(GL_FALSE)
    if depth:
        ctx.enable(moderngl.DEPTH_TEST)
        ctx.depth_func = "<="  # GLDepthStencilState: GL_LEQUAL
    else:
        ctx.disable(moderngl.DEPTH_TEST)
    if cull:
        ctx.enable(moderngl.CULL_FACE)
        ctx.front_face = "cw"  # D3D convention, BeginPass: FrontFaceCW + cull Back
        ctx.cull_face = "back"
    else:
        ctx.disable(moderngl.CULL_FACE)
    if blend:
        ctx.enable(moderngl.BLEND)
        ctx.blend_func = moderngl.SRC_ALPHA, moderngl.ONE_MINUS_SRC_ALPHA
    else:
        ctx.disable(moderngl.BLEND)


def draw_scene(ctx, R: _Resources, fbo, preset, sun, frame, width: int, height: int) -> dict:
    """Clear + sky + sun + terrain/objects + cutout foliage into `fbo`."""
    ub = R.ubo
    ub.vs.data[:] = 0.0
    ub.ps.data[:] = 0.0
    ub.world.data[:] = 0.0
    E.init_ps_defaults(ub.ps)
    E.enable_night_eye(ub.ps, sun.rgb_eye_coef)
    E.update_shadow_map_lit_state_disabled(ub.ps)
    E.upload_frame_constants(ub.vs, ub.ps, frame)
    E.upload_vs_lights_none(ub.vs)
    ub.bind()

    fbo.use()
    fbo.depth_mask = True  # glClear honours the depth write mask
    ctx.viewport = (0, 0, width, height)
    clear = quantize8(sun.fog_color)  # World.cpp: InitDraw(clear, PackedColor(FogColor()))
    fbo.clear(float(clear[0]), float(clear[1]), float(clear[2]), 1.0, depth=1.0)
    R.dummy_array.use(location=2)
    draws, tris = 0, 0
    lp = preset.lighting

    # ---- sky (PassId::Sky: depth off, fog off, no cull) -------------------
    sky = R.scene.sky
    sky_cols = quantize8(sky_vertex_colors(sun, sky.normals.astype(np.float64)))  # PackedColorRGB
    data, _ = G.interleave(sky, sky_cols)
    R.sky_vbo.write(data.tobytes())
    E.set_shader_fog_enabled(ub.vs, frame, False)
    E.upload_vs_texgen_constants(ub.vs, E.TG_NONE)
    E.set_alpha_test(ub.ps, False, ALPHA_REF_OPAQUE)
    E.upload_vs_world_matrix(ub.vs, ub.world, G.gfx_matrix(*np.eye(3), np.zeros(3)))  # sky follows the camera
    ub.flush("sky")
    set_pipeline(ctx, depth=False, cull=False, blend=False)
    R.sky_texture(lp.sky_through).use(location=0)
    R.sky_vao.render(moderngl.TRIANGLES)
    draws += 1
    tris += sky.n_tris

    # ---- sun halo + disc (IsAlphaFog: alpha blend, test ref 1, NoZBuf, fog off)
    disc_col, halo_col = sun_object_colors(sun)
    _, up, _ = preset.camera.basis()
    quads = sun_billboards(sun.to_sun, up)
    pos = np.concatenate([q[1] for q in quads])
    uvs = np.concatenate([q[2] for q in quads])
    cols = np.concatenate([np.tile(quantize8(halo_col), (4, 1)), np.tile(quantize8(disc_col), (4, 1))])
    nrm = np.tile(-sun.direction, (8, 1))
    R.sun_vbo.write(np.hstack([pos, nrm, uvs, cols]).astype(np.float32).tobytes())
    E.set_alpha_test(ub.ps, True, ALPHA_REF_SUN)
    ub.flush("sun")
    set_pipeline(ctx, depth=False, cull=False, blend=True)
    R.textures["sun_halo"].use(location=0)
    R.sun_vao.render(moderngl.TRIANGLES, vertices=6, first=0)
    R.textures["sun_disc"].use(location=0)
    R.sun_vao.render(moderngl.TRIANGLES, vertices=6, first=6)
    draws += 2
    tris += 4

    # ---- world: ground, horizont, opaque objects, then cutout -------------
    # (Landscape::DrawRect order: DrawGround, DrawWater, DrawHorizont,
    #  DrawObjectsAndShadowsPass1; the cutout pass after the opaque one)
    E.set_shader_fog_enabled(ub.vs, frame, True)
    world_m = G.world_matrix_camera_relative(frame.camera_pos)
    E.upload_vs_world_matrix(ub.vs, ub.world, world_m)
    material_log = {}
    groups = [("ground", lambda m: m.pass_id == "opaque" and m.shader == "detail"), ("horizont", None),
              ("objects", lambda m: m.pass_id == "opaque" and m.shader != "detail"),
              ("cutout", lambda m: m.pass_id == "cutout")]
    for group, pred in groups:
        if group == "horizont":
            n_tris = _draw_horizont(ctx, R, ub, preset, sun, frame, world_m, material_log)
            draws += 1
            tris += n_tris
            continue
        pass_id = "cutout" if group == "cutout" else "opaque"
        for mesh, vao in R.meshes:
            if not pred(mesh):
                continue
            mat = resolve_material(mesh.material, sun.accom)
            consts = E.upload_vs_material_constants(ub.vs, sun, mat)
            material_log.setdefault(mesh.name, {k: [round(float(x), 4) for x in v] for k, v in consts.items()})
            if mesh.shader == "detail":
                E.upload_vs_texgen_constants(ub.vs, E.TG_DETAIL)
                R.textures[mesh.detail_texture].use(location=1)
            else:
                E.upload_vs_texgen_constants(ub.vs, E.TG_NONE)
            if pass_id == "cutout":
                E.set_alpha_test(ub.ps, True, ALPHA_REF_CUTOUT, a2c=False)
            else:
                E.set_alpha_test(ub.ps, False, ALPHA_REF_OPAQUE)
            ub.flush(mesh.name)
            set_pipeline(ctx, depth=True, cull=True, blend=False)
            R.textures[mesh.texture].use(location=0)
            vao.render(moderngl.TRIANGLES)
            draws += 1
            tris += mesh.n_tris
    return {"draws": draws, "triangles": tris, "materials": material_log}


def _draw_horizont(ctx, R, ub, preset, sun, frame, world_m, material_log) -> int:
    """Landscape::DrawHorizont: ObjectPlain with MSInShadow vertices
    (CreateMaterialConstant(accom, 0): ambient only), spec IsAlpha | NoZWrite |
    ClampV -> alpha blend, test ref 1, depth read-only, fog on, drawn
    ClipAll & ~ClipBack (no back-face cull)."""
    fr = G.camera_frustum(*frame.viewport[2:], preset.camera.fov, preset.lighting.fog_end)
    _, _, fwd = preset.camera.basis()
    pos, uvs, tri = horizon_polygon(frame.camera_pos, fwd, fr.c_far, fr.c_left)
    nrm = np.tile(-fwd, (pos.shape[0], 1))
    R.hor_vbo.write(np.hstack([pos, nrm, uvs]).astype(np.float32).tobytes())
    R.hor_ibo.write(np.ascontiguousarray(tri, dtype=np.uint32).tobytes())
    mat = resolve_material(HORIZON_MATERIAL, sun.accom)
    consts = E.upload_vs_material_constants(ub.vs, sun, mat)
    material_log.setdefault("horizont", {k: [round(float(x), 4) for x in v] for k, v in consts.items()})
    E.upload_vs_texgen_constants(ub.vs, E.TG_NONE)
    E.set_alpha_test(ub.ps, True, ALPHA_REF_ALPHA)
    E.upload_vs_world_matrix(ub.vs, ub.world, G.gfx_matrix(*np.eye(3), np.zeros(3)))  # camera-relative already
    ub.flush("horizont")
    set_pipeline(ctx, depth=True, cull=False, blend=True, depth_write=False)
    R.textures["horizon"].use(location=0)
    R.hor_vao.render(moderngl.TRIANGLES, vertices=tri.size)
    set_pipeline(ctx, depth=True, cull=True, blend=False, depth_write=True)
    E.upload_vs_world_matrix(ub.vs, ub.world, world_m)
    return int(tri.shape[0])


def apply_gamma(ctx, R: _Resources, scene_color, gamma: float, width: int, height: int) -> np.ndarray:
    """EngineGL33::ApplyGammaPass (skipped when gamma is ~1, like the engine)."""
    if 0.999 < gamma < 1.001:
        fbo, _ = R.target(width, height)
        return G.read_fbo_image(fbo, width, height)
    out_fbo, _ = R.output_target(width, height)
    out_fbo.use()
    ctx.viewport = (0, 0, width, height)
    set_pipeline(ctx, depth=False, cull=False, blend=False)
    scene_color.use(location=0)
    gp = R.prog["gamma"]
    gp["frame"].value = 0
    gp["invGamma"].value = 1.0 / gamma
    R.gamma_vao.render(moderngl.TRIANGLES, vertices=3)
    return G.read_fbo_image(out_fbo, width, height)


def render(ctx: moderngl.Context, scene, preset, width: int, height: int) -> Image.Image:
    R = _resources(ctx, scene)
    clip_ok = G.enable_clip_control(ctx)
    lp = preset.lighting
    sun = compute_lighting(lp, scene.sky_texture_color(lp.sky_through))
    frustum = G.camera_frustum(width, height, preset.camera.fov, lp.fog_end)

    nonfinite = None
    if _options["nan_check"]:
        fbo_f, col_f = R.target(width, height, "f4")
        draw_scene(ctx, R, fbo_f, preset, sun, E.build_frame_state(preset.camera, frustum, sun, width, height),
                   width, height)
        raw = np.frombuffer(fbo_f.read(components=4, dtype="f4"), dtype=np.float32)
        nonfinite = int(np.count_nonzero(~np.isfinite(raw)))

    frame = E.build_frame_state(preset.camera, frustum, sun, width, height)
    fbo, color = R.target(width, height)
    stats = draw_scene(ctx, R, fbo, preset, sun, frame, width, height)
    img = apply_gamma(ctx, R, color, lp.gamma, width, height)
    err = ctx.error
    if err != "GL_NO_ERROR":
        raise RuntimeError(f"GL error after render: {err}")

    out = Image.fromarray(img, "RGB")
    out.info["lab"] = {
        "renderer": NAME,
        "shader_source": glsl_extract.describe_source(R.source),
        "shader_overrides": R.overridden,
        "clip_control": clip_ok,
        "nonfinite_values": nonfinite,
        "draws": stats["draws"],
        "triangles": stats["triangles"],
        "block_sizes": R.block_sizes,
        "ubo_bytes": {"VSConstants": R.ubo.vs.data.nbytes, "PSConstants": R.ubo.ps.data.nbytes},
        "lighting": sun.summary(),
        "materials": stats["materials"],
        "frustum": {"cLeft": frustum.c_left, "cTop": frustum.c_top, "cNear": frustum.c_near, "cFar": frustum.c_far},
    }
    return out
