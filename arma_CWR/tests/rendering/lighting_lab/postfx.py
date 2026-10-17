"""The remastered post pass (EngineGL33_PostFX.cpp) for the lab.

Runs the engine's own post GLSL (s_vsPostGLSL, s_psPost*GLSL), extracted from
the C++ source, over the lab's scene colour (display space, ambient share in
alpha) and depth texture, with the same pass order and uniforms as
EngineGL33::EndWorldScene: half-res AO + two blur passes, a five-level bloom
chain, sun shafts, composite with dither.
"""

from __future__ import annotations

import numpy as np

import moderngl

import glsl_extract

POST_FILE = "arma_CWR/engine/PoseidonGL33/EngineGL33_PostFX.cpp"
BLOOM_LEVELS = 5


def _constants(path) -> dict:
    """The `constexpr float kName = value;` post constants of the C++ source."""
    import re
    text = open(path, encoding="utf-8").read()
    return {m.group(1): float(m.group(2)) for m in re.finditer(r"constexpr float (k\w+) = ([-0-9.]+)f;", text)}


class PostFX:
    def __init__(self, ctx: moderngl.Context, source=None):
        self.ctx = ctx
        path = source or (glsl_extract.ENGINE_DIR.parent / POST_FILE)
        sh = glsl_extract.load_shaders(path)
        self.k = _constants(path)
        vs = sh["s_vsPostGLSL"]

        def prog(name):
            p = ctx.program(vertex_shader=vs, fragment_shader=sh[name])
            return p

        self.p_ao = prog("s_psPostAoGLSL")
        self.p_blur = prog("s_psPostAoBlurGLSL")
        self.p_down = prog("s_psPostBloomDownGLSL")
        self.p_up = prog("s_psPostBloomUpGLSL")
        self.p_shafts = prog("s_psPostShaftsGLSL")
        self.p_comp = prog("s_psPostCompositeGLSL")
        self.vaos = {id(p): ctx.vertex_array(p, []) for p in
                     (self.p_ao, self.p_blur, self.p_down, self.p_up, self.p_shafts, self.p_comp)}
        self.size = None
        self.white = ctx.texture((1, 1), 4, bytes([255, 255, 255, 255]))
        self.black = ctx.texture((1, 1), 4, bytes([0, 0, 0, 0]))

    def _targets(self, w, h):
        if self.size == (w, h):
            return
        ctx = self.ctx
        hw, hh = (w + 1) // 2, (h + 1) // 2

        def tex(size, comps, dtype):
            t = ctx.texture(size, comps, dtype=dtype)
            t.filter = (moderngl.LINEAR, moderngl.LINEAR)
            t.repeat_x = t.repeat_y = False
            return t

        self.ao = [tex((hw, hh), 2, "f2") for _ in range(2)]
        self.ao_fbo = [ctx.framebuffer(color_attachments=[t]) for t in self.ao]
        self.shaft = tex((hw, hh), 4, "f2")
        self.shaft_fbo = ctx.framebuffer(color_attachments=[self.shaft])
        self.bloom, self.bloom_fbo, self.bloom_size = [], [], []
        bw, bh = hw, hh
        for _ in range(BLOOM_LEVELS):
            bw, bh = max(bw, 1), max(bh, 1)
            t = tex((bw, bh), 4, "f2")
            self.bloom.append(t)
            self.bloom_fbo.append(ctx.framebuffer(color_attachments=[t]))
            self.bloom_size.append((bw, bh))
            bw, bh = (bw + 1) // 2, (bh + 1) // 2
        self.size = (w, h)

    @staticmethod
    def _set(prog, name, value):
        if name in prog:
            prog[name].value = value

    def _draw(self, prog, fbo, size):
        fbo.use()
        self.ctx.viewport = (0, 0, *size)
        self.vaos[id(prog)].render(moderngl.TRIANGLES, vertices=3)

    def run(self, color_tex, depth_tex, out_fbo, w, h, proj16, tuning, sun_uv=None, shaft_weight=0.0,
            sun_rgb=(1.0, 1.0, 1.0), debug_ao=False):
        """proj16: GfxMatrix row-major (_11.._44). Writes the composite to out_fbo."""
        ctx = self.ctx
        self._targets(w, h)
        hw, hh = (w + 1) // 2, (h + 1) // 2
        ctx.disable(moderngl.DEPTH_TEST | moderngl.CULL_FACE | moderngl.BLEND)
        m = np.asarray(proj16, dtype=np.float64).reshape(4, 4)
        p11, p22, p33, p43 = m[0, 0], m[1, 1], m[2, 2], m[3, 2]
        ao_strength = tuning["ambientOcclusion"]
        bloom_strength = tuning["bloom"]

        if ao_strength > 0:
            p = self.p_ao
            depth_tex.use(location=0)
            self._set(p, "depthTex", 0)
            self._set(p, "projInfo", (p33, p43, 1.0 / p11, 1.0 / p22))
            self._set(p, "depthMap", (1.0, 0.0, 0.0, 0.0))
            k = self.k
            self._set(p, "aoParams", (k["kAoRadius"], 1.0, k["kAoFadeStart"], k["kAoFadeEnd"]))
            self._set(p, "screen", (float(w), float(h), 1.0 / w, 1.0 / h))
            self._set(p, "worldRect", (0.0, 0.0, 1.0, 1.0))
            self._draw(p, self.ao_fbo[0], (hw, hh))
            p = self.p_blur
            self._set(p, "aoTex", 0)
            self.ao[0].use(location=0)
            self._set(p, "stepUV", (1.0 / hw, 0.0))
            self._draw(p, self.ao_fbo[1], (hw, hh))
            self.ao[1].use(location=0)
            self._set(p, "stepUV", (0.0, 1.0 / hh))
            self._draw(p, self.ao_fbo[0], (hw, hh))

        if bloom_strength > 0:
            p = self.p_down
            self._set(p, "srcTex", 0)
            for i in range(BLOOM_LEVELS):
                src = color_tex if i == 0 else self.bloom[i - 1]
                sw, sh = (w, h) if i == 0 else self.bloom_size[i - 1]
                src.use(location=0)
                self._set(p, "srcTexel", (1.0 / sw, 1.0 / sh))
                self._set(p, "bloomParams", (self.k["kBloomThreshold"], self.k["kBloomKnee"], 1.0 if i == 0 else 0.0, 0.0))
                self._draw(p, self.bloom_fbo[i], self.bloom_size[i])
            p = self.p_up
            self._set(p, "srcTex", 0)
            ctx.enable(moderngl.BLEND)
            ctx.blend_func = moderngl.SRC_ALPHA, moderngl.ONE
            for i in range(BLOOM_LEVELS - 1, 0, -1):
                self.bloom[i].use(location=0)
                bw, bh = self.bloom_size[i]
                self._set(p, "srcTexel", (1.0 / bw, 1.0 / bh))
                self._draw(p, self.bloom_fbo[i - 1], self.bloom_size[i - 1])
            ctx.disable(moderngl.BLEND)

        if shaft_weight > 0.001 and sun_uv is not None:
            p = self.p_shafts
            depth_tex.use(location=0)
            color_tex.use(location=2)
            self._set(p, "depthTex", 0)
            self._set(p, "colorTex", 2)
            self._set(p, "sunScreen", (sun_uv[0], sun_uv[1], shaft_weight, w / float(h)))
            self._draw(p, self.shaft_fbo, (hw, hh))

        p = self.p_comp
        color_tex.use(location=0)
        (self.ao[0] if ao_strength > 0 else self.white).use(location=2)
        (self.bloom[0] if bloom_strength > 0 else self.black).use(location=3)
        (self.shaft if shaft_weight > 0.001 else self.black).use(location=4)
        self._set(p, "colorTex", 0)
        self._set(p, "aoTex", 2)
        self._set(p, "bloomTex", 3)
        self._set(p, "shaftTex", 4)
        self._set(p, "compParams", (ao_strength, bloom_strength * 0.25, tuning["sunShafts"] * 0.6, 1.0))
        lum = max(1e-4, 0.2126 * sun_rgb[0] + 0.7152 * sun_rgb[1] + 0.0722 * sun_rgb[2])
        self._set(p, "shaftColor", (sun_rgb[0] / lum, sun_rgb[1] / lum, sun_rgb[2] / lum, 1.0))
        self._set(p, "debugView", (1.0 if debug_ao else 0.0, 0.0, 0.0, 0.0))
        self._draw(p, out_fbo, (w, h))
