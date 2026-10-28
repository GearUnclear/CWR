"""Sun shadow-map depth pass for the lab (both renderers).

The engine fits up to four cascades around the camera (SceneShadowPass.cpp) and
renders the object casters into a GL_DEPTH_COMPONENT24 2D array; the lit
shaders read it through the PSConstants cascade block. The lab fits ONE
orthographic tier around the camera's view, renders the scene's object meshes
(not the terrain: the engine's casters are objects only) and fills the same
PSConstants slots, so the classic shader's darkness-multiply path and the
remastered sun-only path both see a real shadow map.

Projected (stencil) shadows, the classic in-game default, are approximated by
the classic shader's own shadow-map path with its default darkness 0.35: both
darken everything under a caster by a constant factor.
"""

from __future__ import annotations

import numpy as np

import moderngl

import engine_state as E
import rawgl

DEPTH_VS = """#version 330 core
uniform mat4 lightVP;
layout(location = 0) in vec3 pos;
layout(location = 2) in vec2 uv;
out vec2 vUV;
void main() { vUV = uv; gl_Position = lightVP * vec4(pos, 1.0); }
"""

DEPTH_FS = """#version 330 core
uniform sampler2D tex0;
uniform float alphaTest;
in vec2 vUV;
void main() { if (alphaTest > 0.5 && texture(tex0, vUV).a < 0.75) discard; }
"""

RES = 2048
BIAS_BASE = 0.00002  # Engine::ShadowMapTuning::biasBase
FADE_RANGE = 40.0  # Engine::ShadowMapTuning::fadeRange
DARKNESS = 0.35  # Engine::ShadowMapTuning::darkness


class ShadowPass:
    def __init__(self, ctx: moderngl.Context, scene):
        self.ctx = ctx
        self.prog = ctx.program(vertex_shader=DEPTH_VS, fragment_shader=DEPTH_FS)
        self.prog["tex0"].value = 0
        # GL_DEPTH_COMPONENT24 2D array, one layer (moderngl has no depth arrays)
        self.depth_tex = rawgl.depth_array(RES, 1)
        self.fbo = ctx.detect_framebuffer(rawgl.depth_layer_fbo(self.depth_tex, 0))
        self.casters = []
        for m in scene.meshes:
            if m.shader == "detail":
                continue  # terrain does not cast in the engine
            data = np.hstack([m.positions, m.uvs]).astype(np.float32)
            vbo = ctx.buffer(np.ascontiguousarray(data).tobytes())
            ibo = ctx.buffer(np.ascontiguousarray(m.indices, dtype=np.uint32).tobytes())
            vao = ctx.vertex_array(self.prog, [(vbo, "3f 2f", "pos", "uv")], index_buffer=ibo,
                                   index_element_size=4)
            self.casters.append((m, vao))

    def render(self, camera, cam_pos: np.ndarray, sun_dir: np.ndarray, textures: dict, reach: float = 220.0):
        """Renders the casters; returns (lightVP16 camera-relative column-major, eye far)."""
        _, _, fwd = camera.basis()
        center = cam_pos + fwd * (reach * 0.45)
        d = np.asarray(sun_dir, dtype=np.float64)
        d /= np.linalg.norm(d)
        up = np.array([0.0, 1.0, 0.0]) if abs(d[1]) < 0.95 else np.array([1.0, 0.0, 0.0])
        x = np.cross(up, d)
        x /= np.linalg.norm(x)
        y = np.cross(d, x)
        half = reach * 0.6
        depth = 600.0
        eye = center - d * (depth * 0.5)
        # light view-projection, world -> clip, depth 0..1 (ZERO_TO_ONE)
        view = np.eye(4)
        view[0, :3], view[1, :3], view[2, :3] = x, y, d
        view[:3, 3] = -view[:3, :3] @ eye
        proj = np.eye(4)
        proj[0, 0] = 1.0 / half
        proj[1, 1] = 1.0 / half
        proj[2, 2] = 1.0 / depth
        proj[2, 3] = 0.0
        vp_world = proj @ view
        # the lit shaders feed camera-relative positions: fold in +cam_pos
        to_world = np.eye(4)
        to_world[:3, 3] = cam_pos
        vp_rel = vp_world @ to_world

        ctx = self.ctx
        self.fbo.use()
        ctx.viewport = (0, 0, RES, RES)
        self.fbo.clear(depth=1.0)
        ctx.enable(moderngl.DEPTH_TEST)
        ctx.depth_func = "<="
        ctx.disable(moderngl.BLEND)
        self.prog["lightVP"].write(vp_world.T.astype(np.float32).tobytes())
        for m, vao in self.casters:
            cut = m.pass_id == "cutout"
            if cut:
                ctx.disable(moderngl.CULL_FACE)
            else:
                ctx.enable(moderngl.CULL_FACE)  # front-face culling does the acne work
                ctx.front_face = "cw"
                ctx.cull_face = "front"
            self.prog["alphaTest"].value = 1.0 if cut else 0.0
            textures[m.texture].use(location=0)
            vao.render(moderngl.TRIANGLES)
        ctx.cull_face = "back"
        return vp_rel.T.astype(np.float32).reshape(16), reach

    def bind(self, unit: int = 2) -> None:
        rawgl.bind_texture(unit, rawgl.GL_TEXTURE_2D_ARRAY, self.depth_tex)

    def upload_ps(self, ps: E.ConstantBlock, vp16: np.ndarray, eye_far: float, cam_fwd: np.ndarray,
                  darkness: float = DARKNESS) -> None:
        """UpdateShadowMapLitState with one active frustum tier."""
        ps.set(E.PSConst.SHADOW_CTL, [1.0, 0.0, darkness, 1.0 / RES])
        ps.set(E.PSConst.CASCADE_VP, vp16)
        ps.set(E.PSConst.CASCADE_SPLITS, [eye_far, 0, 0, 0])
        ps.set(E.PSConst.CASCADE_CTL, [1.0, FADE_RANGE, BIAS_BASE, 0.0])
        f = np.asarray(cam_fwd, dtype=np.float64)
        f /= np.linalg.norm(f)
        ps.set(E.PSConst.CAM_FWD, [f[0], f[1], f[2], 0.0])
