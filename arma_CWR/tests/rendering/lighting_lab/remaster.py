"""Renderer for the REMASTERED lighting pipeline.

Same scene, passes and std140 VS/PS blocks as classic.py, plus:
  - the remastered program set from EngineGL33_LightingShaders.cpp
    (s_vs*RemasterGLSL, s_psRemasterCommon + s_ps*RemasterMain);
  - the LitConstants block (binding 3) filled like EngineGL33's remastered
    uploads, with the light rig computed by the engine's own C++ (native.py);
  - a sun shadow map (shadowpass.py) read through a comparison sampler;
  - the terrain horizon map (engine C++ builder) and the cloud noise (engine
    C++ generator) on units 3/4/5.
The post pass (AO/bloom) is applied when the engine's post shaders exist.

Renderer interface: configure(...), render(ctx, scene, preset, width, height).
"""

from __future__ import annotations

import numpy as np
from PIL import Image

import moderngl

import classic as C
import engine_state as E
import gl_common as G
import glsl_extract
import native
import rawgl
from lighting import compute_lighting, quantize8, resolve_material, sky_vertex_colors, sun_object_colors
from scene import HORIZON_MATERIAL, sun_billboards
from postfx import PostFX
from shadowpass import ShadowPass

NAME = "remaster"
REMASTER_FILE = "arma_CWR/engine/PoseidonGL33/EngineGL33_LightingShaders.cpp"
BINDING_LIT = 3
UNIT_HORIZON, UNIT_HEIGHT, UNIT_CLOUD = 3, 4, 5

# Lab stand-in for VSScreen draws (sky, sun): pre-lit colour, no normal.
PRELIT_VS = C.PRELIT_VS.replace("out vec3 vWorldRel;", "out vec3 vWorldRel;\nout vec3 vNormalW;\nflat out float vLitMode;") \
    .replace("vWorldRel = worldPos.xyz;", "vWorldRel = worldPos.xyz;\n    vNormalW = vec3(0.0, 1.0, 0.0);\n    vLitMode = 0.0;")


class Lit:
    SUN_DIR, SUN, ZENITH, HORIZON, GROUND, MOON_DIR, MOON, CAM_POS = range(8)
    MAT_DIFFUSE, MAT_AMBIENT, MAT_FORCED, MAT_EMISSIVE, MAT_SPECULAR = 8, 9, 10, 11, 12
    TERRAIN, TERRAIN_SEL0, TERRAIN_SEL1, TERRAIN_SUN, TERRAIN_LIFT = 13, 14, 15, 16, 17
    CLOUD, CLOUD_OFFSET, SHADOW_DELTA, FLAGS, LIGHT_COUNT = 18, 19, 20, 21, 22
    HORIZON_SUN, FOG_SUN, FOG_CAP = 23, 24, 25
    COUNT = 58


# VSConst::SlotCrown / SlotCrownScale (the remastered VSTransform reads them)
VS_SLOT_CROWN, VS_SLOT_CROWN_SCALE = 22, 23


_options: dict = {"source": None, "remaster_source": None, "shader_override_dir": None, "nan_check": True,
                  "tuning": None, "shadows": True, "debug_view": 0, "post": True}
_cache: dict = {}

# horizon map: world window [ORIGIN, ORIGIN + N*CELL] mapped to the engine's map origin 0
MAP_ORIGIN = np.array([-1200.0, -1200.0])
MAP_CELL = 12.5
MAP_N = 256


def configure(source=None, shader_override_dir=None, nan_check=True, tuning=None, remaster_source=None,
              shadows=True, debug_view=0, post=True, **_kw) -> None:
    if source is not None and remaster_source is None:
        # The renderer also runs the working tree's post shaders, lighting
        # constants and Graphics/Remaster C++ (native.py); a revision of the
        # GLSL alone would pair old shaders with new everything else.
        raise ValueError("the remaster renderer renders the working tree only; to compare an older revision, "
                         "check it out (git worktree) and run the lab there")
    new = {"source": source, "remaster_source": remaster_source or (f"{source}:{REMASTER_FILE}" if source else None),
           "shader_override_dir": shader_override_dir, "nan_check": bool(nan_check), "tuning": tuning,
           "shadows": shadows, "debug_view": int(debug_view), "post": bool(post)}
    if (new["source"], new["remaster_source"], new["shader_override_dir"]) != (
            _options["source"], _options["remaster_source"], _options["shader_override_dir"]):
        _cache.clear()
    _options.update(new)


def _remaster_shaders(source, override_dir=None):
    """The remastered GLSL, with the engine's override pieces applied
    (psRemasterCommon.glsl, psNormalRemasterMain.glsl, vsTransformRemaster.glsl...)."""
    path = source if source is not None else glsl_extract.ENGINE_DIR.parent / REMASTER_FILE
    shaders = glsl_extract.load_shaders(path)
    glsl_extract.apply_override_dir(shaders, override_dir)
    return shaders


class _Resources(C._Resources):
    def __init__(self, ctx, scene):
        super().__init__(ctx, scene, _options["source"], _options["shader_override_dir"])
        rs = _remaster_shaders(_options["remaster_source"], _options["shader_override_dir"])
        self.rshaders = rs
        common = rs["s_psRemasterCommon"]
        vs = rs["s_vsTransformRemasterGLSL"]
        self.rprog = {
            "normal": G.create_program(ctx, vs, common + rs["s_psNormalRemasterMain"], "vsTransformR+psNormalR"),
            "detail": G.create_program(ctx, vs, common + rs["s_psDetailRemasterMain"], "vsTransformR+psDetailR"),
            "prelit": G.create_program(ctx, PRELIT_VS, common + rs["s_psNormalRemasterMain"], "prelit+psNormalR"),
        }
        for p in self.rprog.values():
            if "LitConstants" in p:
                p["LitConstants"].binding = BINDING_LIT
            for name, unit in (("shadowMap", 2), ("terrainHorizon", UNIT_HORIZON), ("terrainHeight", UNIT_HEIGHT),
                               ("cloudNoise", UNIT_CLOUD)):
                rawgl.set_sampler_unit(p.glo, name, unit)
        self.lit = E.ConstantBlock(Lit.COUNT * 4)
        lit_bytes = max(G.block_size(p, "LitConstants") for p in self.rprog.values())
        self.lit_buf = ctx.buffer(reserve=max(self.lit.data.nbytes, lit_bytes), dynamic=True)
        self.rmeshes = []
        for m, _vao in self.meshes:
            data, names = G.interleave(m)
            vbo = ctx.buffer(data.tobytes())
            ibo = ctx.buffer(np.ascontiguousarray(m.indices, dtype=np.uint32).tobytes())
            prog = self.rprog["detail" if m.shader == "detail" else "normal"]
            self.rmeshes.append((m, G.make_vao(ctx, prog, vbo, ibo, names)))
        self.rsky_vao = G.make_vao(ctx, self.rprog["prelit"], self.sky_vbo, self.sky_ibo, ["pos", "normal", "uv", "color"])
        self.rsun_vao = G.make_vao(ctx, self.rprog["prelit"], self.sun_vbo, self.sun_ibo, ["pos", "normal", "uv", "color"])
        self.rhor_vao = G.make_vao(ctx, self.rprog["normal"], self.hor_vbo, self.hor_ibo, ["pos", "normal", "uv"])
        self.shadow = ShadowPass(ctx, scene)
        self.compare_sampler = rawgl.compare_sampler()
        # terrain horizon map from the scene heightfield (engine builder)
        xs = MAP_ORIGIN[0] + np.arange(MAP_N) * MAP_CELL
        zs = MAP_ORIGIN[1] + np.arange(MAP_N) * MAP_CELL
        X, Z = np.meshgrid(xs, zs)
        heights = scene.heightfield(X, Z).astype(np.float32)
        # ground colour per vertex = the average colour of its landscape texture
        # (TerrainLighting.cpp GroundColors: Texture::GetColor)
        means = {k: scene.textures[k]["rgba"][..., :3].reshape(-1, 3).mean(0) for k in ("grass", "dirt")}
        names = scene.ground_texture(X, Z)
        ground = np.zeros((MAP_N, MAP_N, 3), dtype=np.float32)
        for k, m in means.items():
            ground[names == k] = m
        hor, sky, hgt, tint = native.build_horizon(heights, MAP_CELL, ground)
        self.horizon_tex = rawgl.rgba16_array(np.ascontiguousarray(hor).tobytes(), MAP_N, MAP_N, 4)
        hs = np.concatenate([hgt[..., None], sky.astype(np.float32)[..., None] / 255.0, tint], axis=-1).astype(np.float32)
        self.height_tex = ctx.texture((MAP_N, MAP_N), 4, np.ascontiguousarray(hs).tobytes(), dtype="f4")
        self.height_tex.filter = (moderngl.LINEAR, moderngl.LINEAR)
        self.height_tex.repeat_x = self.height_tex.repeat_y = False
        self.post = PostFX(ctx, override_dir=_options["shader_override_dir"])
        self.scene_targets = {}
        noise = native.cloud_noise(256, 1)
        self.cloud_tex = ctx.texture((256, 256), 1, np.ascontiguousarray(noise).tobytes())
        self.cloud_tex.filter = (moderngl.LINEAR, moderngl.LINEAR)
        self.cloud_tex.repeat_x = self.cloud_tex.repeat_y = True


def _scene_target(R, w, h):
    """Frame target: RGBA8 colour (1 - sky light share in alpha) + depth texture, and
    the RGBA8 output the post pass composites into."""
    key = (w, h)
    if key not in R.scene_targets:
        ctx = R.ctx
        color = ctx.texture((w, h), 4)
        color.filter = (moderngl.LINEAR, moderngl.LINEAR)
        color.repeat_x = color.repeat_y = False
        depth = ctx.depth_texture((w, h))
        depth.compare_func = ""
        depth.filter = (moderngl.NEAREST, moderngl.NEAREST)
        depth.repeat_x = depth.repeat_y = False
        fbo = ctx.framebuffer(color_attachments=[color], depth_attachment=depth)
        out = ctx.texture((w, h), 4)
        out.filter = (moderngl.NEAREST, moderngl.NEAREST)
        out.repeat_x = out.repeat_y = False
        out_fbo = ctx.framebuffer(color_attachments=[out])
        R.scene_targets[key] = (fbo, color, depth, out_fbo, out)
    return R.scene_targets[key]


def _resources(ctx, scene) -> _Resources:
    key = (id(ctx), id(scene), _options["source"], _options["remaster_source"], _options["shader_override_dir"])
    if key not in _cache:
        _cache.clear()
        _cache[key] = _Resources(ctx, scene)
    return _cache[key]


def _tuning() -> dict:
    t = native.default_tuning()
    if _options["tuning"]:
        t.update(_options["tuning"])
    return t


def _rig(sun, lp) -> dict:
    to_sun = -np.asarray(sun.direction, dtype=np.float64)
    return native.compute_rig(to_sun, [0.0, -1.0, 0.0], 0.0, sun.diffuse, sun.ambient, sun.fog_color, lp.sky_through,
                              sun.night_effect, True, _tuning())


LIGHTING_FILE = "arma_CWR/engine/PoseidonGL33/EngineGL33_Lighting.cpp"
_engine_k: dict = {}


def engine_constants() -> dict:
    """The `constexpr float kName = value;` constants of EngineGL33_Lighting.cpp."""
    if not _engine_k:
        import re
        text = (glsl_extract.ENGINE_DIR.parent / LIGHTING_FILE).read_text(encoding="utf-8")
        for m in re.finditer(r"constexpr float (k\w+) = ([-0-9.]+)f( / [-0-9.]+f)?;", text):
            v = float(m.group(2))
            if m.group(3):
                v /= float(m.group(3)[3:-1])
            _engine_k[m.group(1)] = v
    return _engine_k


def upload_lit_frame(lit: E.ConstantBlock, rig: dict, sun, cam_pos: np.ndarray, tuning: dict, map_ok: bool) -> None:
    """The per-frame LitConstants (EngineGL33::UploadLitFrameConstants)."""
    k = engine_constants()
    to_sun = -np.asarray(sun.direction, dtype=np.float64)
    lit.set(Lit.SUN_DIR, [*to_sun, 1.0])
    lit.set(Lit.SUN, [*rig["sun"], rig["elevation"]])
    lit.set(Lit.ZENITH, [*rig["zenith"], rig["exposure"]])
    lit.set(Lit.HORIZON, [*rig["horizon"], tuning["specular"]])
    lit.set(Lit.HORIZON_SUN, [*rig["horizon_sun"], 0.0])
    # fog glow toward the sun: sky texture * SunSkyColor * accom, capped at the sky texture
    sky_tex = np.asarray(sun.sky_texture_color, dtype=np.float64)[:3]
    glow = sky_tex * np.asarray(sun.sun_sky_color)[:3] * np.asarray(sun.accom)[:3]
    lit.set(Lit.FOG_SUN, [*glow, 1.0])
    lit.set(Lit.FOG_CAP, [*sky_tex, 0.0])
    lit.set(Lit.GROUND, [*rig["ground"], tuning["localLights"]])
    lit.set(Lit.MOON_DIR, [0.0, -1.0, 0.0, 0.0])
    lit.set(Lit.MOON, [*rig["moon"], 0.0])
    lit.set(Lit.CAM_POS, [cam_pos[0] - MAP_ORIGIN[0], cam_pos[1], cam_pos[2] - MAP_ORIGIN[1], 0.0])
    # terrain horizon: interpolate the two stored azimuths around the sun
    az = np.degrees(np.arctan2(to_sun[0], to_sun[2])) % 360.0
    f = az / 360.0 * 16.0
    i0 = int(np.floor(f)) % 16
    i1 = (i0 + 1) % 16
    w = f - np.floor(f)
    sel0 = np.zeros(4)
    sel0[i0 % 4] = 1.0
    sel1 = np.zeros(4)
    sel1[i1 % 4] = 1.0
    lit.set(Lit.TERRAIN, [1.0 if map_ok else 0.0, 1.0 / MAP_CELL, 1.0 / MAP_N, tuning["terrainOcclusion"]])
    lit.set(Lit.TERRAIN_SEL0, sel0)
    lit.set(Lit.TERRAIN_SEL1, sel1)
    lit.set(Lit.TERRAIN_SUN, [i0 // 4, i1 // 4, w, k["kTerrainPenumbraDeg"]])
    lit.set(Lit.TERRAIN_LIFT, [3.0, 30.0, tuning["terrainShadows"], 0.0])
    cover = rig["cover"]
    lit.set(Lit.CLOUD, [cover, rig["transmit"], k["kCloudScale"], 1.0 if cover > 0.0 else 0.0])
    lit.set(Lit.CLOUD_OFFSET, [1830.0, 970.0, k["kCloudDeckAltitude"], k["kCloudEdge"]])
    lit.set(Lit.SHADOW_DELTA, [0.0, 0.0, 0.0, k["kShadowFilterTexels"] * tuning["shadowSoftness"]])
    lit.set(Lit.FLAGS, [0.0, 1.0, float(_options["debug_view"]), tuning["saturation"]])
    lit.set(Lit.LIGHT_COUNT, [0.0, 0.0, 0.0, 0.0])


def upload_lit_material(lit: E.ConstantBlock, mat, sun_enabled: bool = True) -> None:
    """Per-draw material (EngineGL33::UploadVSMaterialConstants, remastered part)."""
    lit.set(Lit.MAT_DIFFUSE, [*mat.diffuse[:3], 1.0])
    lit.set(Lit.MAT_AMBIENT, [*mat.ambient[:3], 0.0])
    lit.set(Lit.MAT_FORCED, [*mat.forced_diffuse[:3], 0.0])
    lit.set(Lit.MAT_EMISSIVE, [*mat.emissive[:3], 0.0])
    lit.set(Lit.MAT_SPECULAR, [*mat.specular[:3], float(mat.specular_power) if mat.specular_power > 0 else 0.0])


def draw_scene(ctx, R: _Resources, fbo, preset, sun, frame, width, height, rig, tuning) -> dict:
    ub = R.ubo
    ub.vs.data[:] = 0.0
    ub.ps.data[:] = 0.0
    ub.world.data[:] = 0.0
    E.init_ps_defaults(ub.ps)
    E.enable_night_eye(ub.ps, sun.rgb_eye_coef)
    E.update_shadow_map_lit_state_disabled(ub.ps)
    E.upload_frame_constants(ub.vs, ub.ps, frame)
    E.upload_vs_lights_none(ub.vs)

    # sun shadow map (rendered before the lit pass; the engine uses last frame's)
    shadows = _options["shadows"]
    if shadows:
        vp16, far = R.shadow.render(preset.camera, frame.camera_pos, sun.direction, R.textures)
        _, _, fwd = preset.camera.basis()
        R.shadow.upload_ps(ub.ps, vp16, far, fwd)
    lit = R.lit
    lit.data[:] = 0.0
    upload_lit_frame(lit, rig, sun, frame.camera_pos, tuning, True)
    ub.bind()
    R.lit_buf.bind_to_uniform_block(BINDING_LIT)

    def flush(label, opaque=True):
        # EngineGL33::SetLitOpaque: opaque draws store 1 - the sky light share in alpha
        lit.data[Lit.FLAGS * 4] = 1.0 if opaque else 0.0
        ub.flush(label)
        lit.check_finite(f"LitConstants {label}")
        R.lit_buf.write(lit.data.tobytes())

    fbo.use()
    fbo.depth_mask = True
    ctx.viewport = (0, 0, width, height)
    clear = quantize8(sun.fog_color)
    fbo.clear(float(clear[0]), float(clear[1]), float(clear[2]), 1.0, depth=1.0)
    R.shadow.bind(2)
    rawgl.bind_sampler(2, R.compare_sampler)
    rawgl.bind_texture(UNIT_HORIZON, rawgl.GL_TEXTURE_2D_ARRAY, R.horizon_tex)
    R.height_tex.use(location=UNIT_HEIGHT)
    R.cloud_tex.use(location=UNIT_CLOUD)
    lp = preset.lighting

    # sky (pre-lit, unchanged)
    sky = R.scene.sky
    sky_cols = quantize8(sky_vertex_colors(sun, sky.normals.astype(np.float64)))
    data, _ = G.interleave(sky, sky_cols)
    R.sky_vbo.write(data.tobytes())
    E.set_shader_fog_enabled(ub.vs, frame, False)
    E.upload_vs_texgen_constants(ub.vs, E.TG_NONE)
    E.set_alpha_test(ub.ps, False, C.ALPHA_REF_OPAQUE)
    E.upload_vs_world_matrix(ub.vs, ub.world, G.gfx_matrix(*np.eye(3), np.zeros(3)))
    ub.vs.set(VS_SLOT_CROWN, [0.0, 0.0, 0.0, 0.0])
    flush("sky")
    C.set_pipeline(ctx, depth=False, cull=False, blend=False)
    R.sky_texture(lp.sky_through).use(location=0)
    R.rsky_vao.render(moderngl.TRIANGLES)

    disc_col, halo_col = sun_object_colors(sun)
    _, up, _ = preset.camera.basis()
    quads = sun_billboards(sun.to_sun, up)
    pos = np.concatenate([q[1] for q in quads])
    uvs = np.concatenate([q[2] for q in quads])
    cols = np.concatenate([np.tile(quantize8(halo_col), (4, 1)), np.tile(quantize8(disc_col), (4, 1))])
    nrm = np.tile(-sun.direction, (8, 1))
    R.sun_vbo.write(np.hstack([pos, nrm, uvs, cols]).astype(np.float32).tobytes())
    E.set_alpha_test(ub.ps, True, C.ALPHA_REF_SUN)
    flush("sun", opaque=False)
    C.set_pipeline(ctx, depth=False, cull=False, blend=True)
    ctx.blend_func = moderngl.SRC_ALPHA, moderngl.ONE_MINUS_SRC_ALPHA, moderngl.ZERO, moderngl.ONE
    R.textures["sun_halo"].use(location=0)
    R.rsun_vao.render(moderngl.TRIANGLES, vertices=6, first=0)
    R.textures["sun_disc"].use(location=0)
    R.rsun_vao.render(moderngl.TRIANGLES, vertices=6, first=6)

    E.set_shader_fog_enabled(ub.vs, frame, True)
    world_m = G.world_matrix_camera_relative(frame.camera_pos)
    E.upload_vs_world_matrix(ub.vs, ub.world, world_m)
    groups = [("ground", lambda m: m.pass_id == "opaque" and m.shader == "detail"), ("horizont", None),
              ("objects", lambda m: m.pass_id == "opaque" and m.shader != "detail"),
              ("cutout", lambda m: m.pass_id == "cutout")]
    for group, pred in groups:
        if group == "horizont":
            fr = G.camera_frustum(*frame.viewport[2:], preset.camera.fov, lp.fog_end)
            _, _, fwd = preset.camera.basis()
            from scene import horizon_polygon
            hpos, huvs, tri = horizon_polygon(frame.camera_pos, fwd, fr.c_far, fr.c_left)
            hn = np.tile(-fwd, (hpos.shape[0], 1))
            R.hor_vbo.write(np.hstack([hpos, hn, huvs]).astype(np.float32).tobytes())
            R.hor_ibo.write(np.ascontiguousarray(tri, dtype=np.uint32).tobytes())
            mat = resolve_material(HORIZON_MATERIAL, sun.accom)
            E.upload_vs_material_constants(ub.vs, sun, mat)
            upload_lit_material(lit, mat)
            E.upload_vs_texgen_constants(ub.vs, E.TG_NONE)
            E.set_alpha_test(ub.ps, True, C.ALPHA_REF_ALPHA)
            E.upload_vs_world_matrix(ub.vs, ub.world, G.gfx_matrix(*np.eye(3), np.zeros(3)))
            flush("horizont", opaque=False)
            C.set_pipeline(ctx, depth=True, cull=False, blend=True, depth_write=False)
            ctx.blend_func = moderngl.SRC_ALPHA, moderngl.ONE_MINUS_SRC_ALPHA, moderngl.ZERO, moderngl.ONE
            R.textures["horizon"].use(location=0)
            R.rhor_vao.render(moderngl.TRIANGLES, vertices=tri.size)
            C.set_pipeline(ctx, depth=True, cull=True, blend=False, depth_write=True)
            E.upload_vs_world_matrix(ub.vs, ub.world, world_m)
            continue
        pass_id = "cutout" if group == "cutout" else "opaque"
        for mesh, vao in R.rmeshes:
            if not pred(mesh):
                continue
            mat = resolve_material(mesh.material, sun.accom)
            E.upload_vs_material_constants(ub.vs, sun, mat)
            upload_lit_material(lit, mat)
            # EngineGL33::BeginMeshTL: crown ellipsoid from the shape's bounds
            bmin, bmax = mesh.bounds if mesh.bounds is not None else (mesh.positions.min(0), mesh.positions.max(0))
            cc, ih = native.crown_ellipsoid(bmin, bmax)
            ub.vs.set(VS_SLOT_CROWN, [*cc, 1.0])
            ub.vs.set(VS_SLOT_CROWN_SCALE, [*ih, 0.0])
            if mesh.shader == "detail":
                E.upload_vs_texgen_constants(ub.vs, E.TG_DETAIL)
                R.textures[mesh.detail_texture].use(location=1)
            else:
                E.upload_vs_texgen_constants(ub.vs, E.TG_NONE)
            if pass_id == "cutout":
                E.set_alpha_test(ub.ps, True, C.ALPHA_REF_CUTOUT, a2c=False)
            else:
                E.set_alpha_test(ub.ps, False, C.ALPHA_REF_OPAQUE)
            flush(mesh.name)
            C.set_pipeline(ctx, depth=True, cull=True, blend=False)
            R.textures[mesh.texture].use(location=0)
            vao.render(moderngl.TRIANGLES)
    rawgl.bind_sampler(2, 0)
    return {}


def _sun_screen(preset, sun, frame, lp):
    """EngineGL33::EndWorldScene sun-shaft setup: sun uv and weight."""
    aside, up, fwd = preset.camera.basis()
    to_sun = -np.asarray(sun.direction, dtype=np.float64)
    v = np.array([to_sun @ aside, to_sun @ up, to_sun @ fwd])
    if v[2] <= 0.05 or to_sun[1] <= -0.02:
        return None, 0.0
    m = np.asarray(frame.projection, dtype=np.float64).reshape(4, 4)
    ndc_x, ndc_y = m[0, 0] * v[0] / v[2], m[1, 1] * v[1] / v[2]
    off = max(abs(ndc_x) - 1.0, 0.0) + max(abs(ndc_y) - 1.0, 0.0)
    on_screen = np.exp(-3.0 * off)
    low_sun = 0.55 + 0.45 * (1.0 - min(1.0, to_sun[1] / 0.6))
    weight = on_screen * low_sun * lp.sky_through * (1.0 - sun.night_effect)
    return (ndc_x * 0.5 + 0.5, ndc_y * 0.5 + 0.5), float(weight)


def render(ctx: moderngl.Context, scene, preset, width: int, height: int) -> Image.Image:
    R = _resources(ctx, scene)
    G.enable_clip_control(ctx)
    lp = preset.lighting
    sun = compute_lighting(lp, scene.sky_texture_color(lp.sky_through))
    frustum = G.camera_frustum(width, height, preset.camera.fov, lp.fog_end)
    tuning = _tuning()
    rig = _rig(sun, lp)

    nonfinite = None
    if _options["nan_check"]:
        fbo_f, _ = R.target(width, height, "f4")
        draw_scene(ctx, R, fbo_f, preset, sun, E.build_frame_state(preset.camera, frustum, sun, width, height),
                   width, height, rig, tuning)
        raw = np.frombuffer(fbo_f.read(components=4, dtype="f4"), dtype=np.float32)
        nonfinite = int(np.count_nonzero(~np.isfinite(raw)))

    frame = E.build_frame_state(preset.camera, frustum, sun, width, height)
    fbo, color, depth, out_fbo, out_tex = _scene_target(R, width, height)
    draw_scene(ctx, R, fbo, preset, sun, frame, width, height, rig, tuning)
    final = color
    if _options["post"] and tuning.get("postEffects", 1.0):
        sun_uv, weight = _sun_screen(preset, sun, frame, lp)
        R.post.run(color, depth, out_fbo, width, height, frame.projection, tuning, sun_uv, weight,
                   tuple(rig["sun"]), debug_ao=_options["debug_view"] == 4)
        final = out_tex
    img = C.apply_gamma(ctx, R, final, lp.gamma, width, height)
    err = ctx.error
    if err != "GL_NO_ERROR":
        raise RuntimeError(f"GL error after render: {err}")
    out = Image.fromarray(img, "RGB")
    out.info["lab"] = {"renderer": NAME, "nonfinite_values": nonfinite,
                       "rig": {k: (v.tolist() if hasattr(v, "tolist") else v) for k, v in rig.items()},
                       "tuning": tuning, "lighting": sun.summary()}
    return out
