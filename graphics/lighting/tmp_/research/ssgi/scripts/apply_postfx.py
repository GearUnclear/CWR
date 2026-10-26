"""Applies the SSGI research pass to EngineGL33_PostFX.cpp in the scratch worktree.

Run once: python apply_postfx.py <path to EngineGL33_PostFX.cpp>
The shader sources live in ../glsl/*.glsl (the same files double as override
sources: psPostGi.glsl, psPostGiTemporal.glsl, psPostGiBlur.glsl).
"""
import sys
from pathlib import Path

here = Path(__file__).resolve().parent
glsl = here.parent / "glsl"
p = Path(sys.argv[1])
s = p.read_text(encoding="utf-8")


def rep(old, new, count=1):
    global s
    n = s.count(old)
    assert n == count, (n, old[:80])
    s = s.replace(old, new)


def raw(name):
    body = (glsl / f"{name}.glsl").read_text(encoding="utf-8")
    assert ')"' not in body
    return body


gi_shaders = (
    "// RESEARCH (ssgi track): screen-space ray-traced diffuse GI. See\n"
    "// tmp/research/ssgi/NOTES.md for the model; the sources double as override files.\n"
    'static const char s_psPostGiGLSL[] = R"(' + raw("psPostGi") + ')";\n\n'
    'static const char s_psPostGiTemporalGLSL[] = R"(' + raw("psPostGiTemporal") + ')";\n\n'
    'static const char s_psPostGiBlurGLSL[] = R"(' + raw("psPostGiBlur") + ')";\n\n'
)
anchor = "// Post constants shared with the lab (tests/rendering/lighting_lab/postfx.py)."
rep(anchor, gi_shaders + anchor)

# Composite: GI add.
rep(
    """uniform vec4 debugView;  // x: 1 = show occlusion
out vec4 fragColor;

void main() {
    vec4 c = texture(colorTex, vUV);
    float ao = texture(aoTex, vUV).r;
    if (debugView.x > 0.5) { fragColor = vec4(vec3(ao), 1.0); return; }
    // Take the occluded part of the sky light out of the radiance. c.a is one
    // minus the pixel's sky light share (1 for pre-lit and coverage pixels).
    float share = 1.0 - clamp(c.a, 0.0, 1.0);
    float keep = max(1.0 - share * (1.0 - ao) * compParams.x, 0.0);
    vec3 lin = pow(clamp(c.rgb, 0.0, 1.0), vec3(2.2)) * keep;""",
    """uniform vec4 debugView;  // x: 1 = show occlusion
uniform sampler2D giTex;    // unit 5: RESEARCH ssgi ratio (rgb) + view depth (a), half res
uniform sampler2D giDepth;  // unit 6: full-res depth for the joint-bilateral upsample
uniform vec4 giComp;        // x strength, y fallback share, z debug, w on
uniform vec4 giProj;        // x _33, y _43, z depth scale, w depth bias
out vec4 fragColor;

void main() {
    vec4 c = texture(colorTex, vUV);
    float ao = texture(aoTex, vUV).r;
    if (debugView.x > 0.5) { fragColor = vec4(vec3(ao), 1.0); return; }
    // Take the occluded part of the sky light out of the radiance. c.a is one
    // minus the pixel's sky light share (1 for pre-lit and coverage pixels).
    float share = 1.0 - clamp(c.a, 0.0, 1.0);
    float keep = max(1.0 - share * (1.0 - ao) * compParams.x, 0.0);
    vec3 lin0 = pow(clamp(c.rgb, 0.0, 1.0), vec3(2.2));
    vec3 lin = lin0 * keep;
    vec3 giAdd = vec3(0.0);
    if (giComp.w > 0.5) {
        float d = texture(giDepth, vUV).r;
        if (d < 0.999999) {
            float z = giProj.y / ((d * giProj.z + giProj.w) - giProj.x);
            vec2 hs = vec2(textureSize(giTex, 0));
            vec2 f = vUV * hs - 0.5;
            vec2 i0 = floor(f);
            vec2 fr = f - i0;
            vec3 acc = vec3(0.0);
            float ws = 0.0;
            for (int k = 0; k < 4; ++k) {
                ivec2 o = ivec2(k & 1, k >> 1);
                ivec2 ip = clamp(ivec2(i0) + o, ivec2(0), ivec2(hs) - 1);
                vec4 g = texelFetch(giTex, ip, 0);
                float bw = (o.x == 1 ? fr.x : 1.0 - fr.x) * (o.y == 1 ? fr.y : 1.0 - fr.y);
                float dw = 1.0 / (1e-3 + abs(g.a - z) / (0.02 * z + 0.05));
                float w = (bw + 1e-3) * dw;
                acc += g.rgb * w;
                ws += w;
            }
            vec3 ratio = acc / max(ws, 1e-6);
            float s = share;
            // Coverage and alpha-tested pixels (tree crowns) store alpha 1: no share.
            if (c.a > 0.995) s = giComp.y * (1.0 - smoothstep(150.0, 400.0, z));
            giAdd = s * lin0 * ratio * giComp.x;
            lin = clamp(lin + giAdd, vec3(0.0), vec3(1.0));
        }
        if (giComp.z > 0.5) { fragColor = vec4(pow(clamp(abs(giAdd) * 4.0, 0.0, 1.0), vec3(1.0 / 2.2)), 1.0); return; }
    }""",
)
rep(
    "    if (compParams.w > 0.5 && (keep < 0.9999 || max(glow.r, max(glow.g, glow.b)) > 1e-5)) {",
    "    if (compParams.w > 0.5 && (keep < 0.9999 || max(glow.r, max(glow.g, glow.b)) > 1e-5 ||\n"
    "                               abs(giAdd.r) + abs(giAdd.g) + abs(giAdd.b) > 1e-5)) {",
)

# Targets.
rep(
    """    delTex(p.shaftTex);
    p.w = p.h = 0;""",
    """    delTex(p.shaftTex);
    for (int i = 0; i < 2; ++i)
    {
        delFbo(p.giFbo[i]);
        delFbo(p.giHistFbo[i]);
        delTex(p.giTex[i]);
        delTex(p.giHist[i]);
    }
    p.giHistValid = false;
    p.w = p.h = 0;""",
)
rep(
    "         {&p.progAo, &p.progAoBlur, &p.progBloomDown, &p.progBloomUp, &p.progShafts, &p.progComposite})",
    "         {&p.progAo, &p.progAoBlur, &p.progBloomDown, &p.progBloomUp, &p.progShafts, &p.progComposite, &p.progGi,\n"
    "          &p.progGiTemporal, &p.progGiBlur})",
)
rep(
    """    p.vao = 0;
    p.programsTried = false;""",
    """    p.vao = 0;
    if (p.giMipSampler)
        glDeleteSamplers(1, &p.giMipSampler);
    p.giMipSampler = 0;
    p.programsTried = false;""",
)
rep(
    """        p.progComposite = LinkPostProgram(vs, s_psPostCompositeGLSL, "psPostComposite");
        if (vs)""",
    """        p.progComposite = LinkPostProgram(vs, s_psPostCompositeGLSL, "psPostComposite");
        p.progGi = LinkPostProgram(vs, s_psPostGiGLSL, "psPostGi");
        p.progGiTemporal = LinkPostProgram(vs, s_psPostGiTemporalGLSL, "psPostGiTemporal");
        p.progGiBlur = LinkPostProgram(vs, s_psPostGiBlurGLSL, "psPostGiBlur");
        LOG_INFO(Graphics, "GL33 post: ssgi programs gi={} temporal={} blur={}", p.progGi, p.progGiTemporal,
                 p.progGiBlur);
        glGenSamplers(1, &p.giMipSampler);
        glSamplerParameteri(p.giMipSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glSamplerParameteri(p.giMipSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glSamplerParameteri(p.giMipSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glSamplerParameteri(p.giMipSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        if (vs)""",
)
rep(
    """    p.shaftTex = MakeTexture(GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, hw, hh, true);
    int bw = hw, bh = hh;""",
    """    p.shaftTex = MakeTexture(GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, hw, hh, true);
    // RESEARCH (ssgi): the colour gets a mip chain for cone-footprint hit fetches
    // (sampled only through the GI pass's trilinear sampler object).
    glBindTexture(GL_TEXTURE_2D, p.colorTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 6);
    for (int i = 0; i < 2; ++i)
    {
        p.giTex[i] = MakeTexture(GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, hw, hh, true);
        p.giHist[i] = MakeTexture(GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, hw, hh, true);
    }
    int bw = hw, bh = hh;""",
)
rep(
    """    p.shaftFbo = MakeFbo(GL_COLOR_ATTACHMENT0, p.shaftTex);
    check();""",
    """    p.shaftFbo = MakeFbo(GL_COLOR_ATTACHMENT0, p.shaftTex);
    check();
    for (int i = 0; i < 2; ++i)
    {
        p.giFbo[i] = MakeFbo(GL_COLOR_ATTACHMENT0, p.giTex[i]);
        check();
        p.giHistFbo[i] = MakeFbo(GL_COLOR_ATTACHMENT0, p.giHist[i]);
        check();
    }
    p.giHistValid = false;""",
)
rep(
    """    if (_lighting.ambientOcclusion <= 0.0f && _lighting.bloom <= 0.0f && _lighting.sunShafts <= 0.0f)
        return; // nothing to add: leave the frame as drawn""",
    """    if (_lighting.ambientOcclusion <= 0.0f && _lighting.bloom <= 0.0f && _lighting.sunShafts <= 0.0f &&
        _lighting.gi <= 0.0f)
        return; // nothing to add: leave the frame as drawn""",
)

# The GI passes: after the AO blur, before the AoEnd mark.
rep(
    """        drawTo(p.aoFbo[0], hw, hh);
        GpuMark(Poseidon::gpu::Mark::AoEnd);
    }""",
    """        drawTo(p.aoFbo[0], hw, hh);
    }

    // RESEARCH (ssgi): trace, temporal accumulation, spatial blur. Its GPU time
    // lands in the "ao" section (it runs before the AoEnd mark).
    const bool giOn = worldDepth && _lighting.gi > 0.0f && p.progGi && p.progGiTemporal && p.progGiBlur && GScene &&
                      GScene->GetCamera() && GScene->MainLight();
    if (giOn)
    {
        Camera* cam = GScene->GetCamera();
        LightSun* sun = GScene->MainLight();
        const Matrix4 inv = cam->InverseScaled();
        const Vector3 upV = inv.Rotate(Vector3(0, 1, 0));
        const Vector3 sunV = inv.Rotate(-sun->SunDirection());
        Vector3 ax = cam->DirectionAside(), ay = cam->DirectionUp(), az = cam->Direction();
        ax.Normalize();
        ay.Normalize();
        az.Normalize();
        const Vector3 pos = cam->Position();

        // Colour mips for the hit fetches.
        glActiveTexture(GL_TEXTURE0 + 2);
        glBindTexture(GL_TEXTURE_2D, p.colorTex);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindSampler(2, p.giMipSampler);

        const bool temporal = _lighting.giTemporal > 0.5f;
        if (temporal)
            ++p.giFrame;
        glUseProgram(p.progGi);
        u1i(p.progGi, "depthTex", 0);
        u1i(p.progGi, "colorTex", 2);
        u4f(p.progGi, "projInfo", proj._33, proj._43, 1.0f / proj._11, 1.0f / proj._22);
        u4f(p.progGi, "depthMap", depthScale, depthBias, 0.0f, 0.0f);
        u4f(p.progGi, "screen", static_cast<float>(w), static_cast<float>(h), 1.0f / w, 1.0f / h);
        u4f(p.progGi, "worldRect", rectX, rectY, rectW, rectH);
        u4f(p.progGi, "giParams", _lighting.giRadius, static_cast<float>(p.giFrame % 64u), _lighting.giMode, 0.0f);
        u4f(p.progGi, "giParams2", _lighting.giRays, _lighting.giSteps, _giRig[0][3], _giRig[1][3]);
        u4f(p.progGi, "upView", upV.X(), upV.Y(), upV.Z(), 0.0f);
        u4f(p.progGi, "sunView", sunV.X(), sunV.Y(), sunV.Z(), 0.0f);
        u4f(p.progGi, "rigZenith", _giRig[0][0], _giRig[0][1], _giRig[0][2], 0.0f);
        u4f(p.progGi, "rigHorizon", _giRig[1][0], _giRig[1][1], _giRig[1][2], 0.0f);
        u4f(p.progGi, "rigHorizonSun", _giRig[2][0], _giRig[2][1], _giRig[2][2], 0.0f);
        u4f(p.progGi, "rigGround", _giRig[3][0], _giRig[3][1], _giRig[3][2], 0.0f);
        bindTex(0, p.depthTex);
        drawTo(p.giFbo[0], hw, hh);
        glBindSampler(2, 0);

        GLuint blurInput = p.giTex[0];
        if (temporal)
        {
            const int prev = p.giHistCur;
            const int cur = 1 - prev;
            glUseProgram(p.progGiTemporal);
            u1i(p.progGiTemporal, "curTex", 0);
            u1i(p.progGiTemporal, "histTex", 2);
            u4f(p.progGiTemporal, "projInfo", proj._33, proj._43, 1.0f / proj._11, 1.0f / proj._22);
            u4f(p.progGiTemporal, "prevProj", 0.0f, 0.0f, p.giPrevProj[0], p.giPrevProj[1]);
            u4f(p.progGiTemporal, "worldRect", rectX, rectY, rectW, rectH);
            u4f(p.progGiTemporal, "curX", ax.X(), ax.Y(), ax.Z(), 0.0f);
            u4f(p.progGiTemporal, "curY", ay.X(), ay.Y(), ay.Z(), 0.0f);
            u4f(p.progGiTemporal, "curZ", az.X(), az.Y(), az.Z(), 0.0f);
            u4f(p.progGiTemporal, "prevX", p.giPrevAxes[0], p.giPrevAxes[1], p.giPrevAxes[2], 0.0f);
            u4f(p.progGiTemporal, "prevY", p.giPrevAxes[3], p.giPrevAxes[4], p.giPrevAxes[5], 0.0f);
            u4f(p.progGiTemporal, "prevZ", p.giPrevAxes[6], p.giPrevAxes[7], p.giPrevAxes[8], 0.0f);
            u4f(p.progGiTemporal, "dPos", pos.X() - p.giPrevPos[0], pos.Y() - p.giPrevPos[1], pos.Z() - p.giPrevPos[2],
                0.0f);
            u4f(p.progGiTemporal, "tParams", _lighting.giTemporal > 1.5f ? 0.04f : 0.1f, p.giHistValid ? 1.0f : 0.0f,
                0.0f, 0.0f);
            bindTex(0, p.giTex[0]);
            bindTex(2, p.giHist[prev]);
            drawTo(p.giHistFbo[cur], hw, hh);
            p.giHistCur = cur;
            p.giHistValid = true;
            blurInput = p.giHist[cur];
        }
        else
        {
            p.giHistValid = false;
        }
        glUseProgram(p.progGiBlur);
        u1i(p.progGiBlur, "giSrc", 0);
        bindTex(0, blurInput);
        u2f(p.progGiBlur, "stepUV", 1.0f / hw, 0.0f);
        drawTo(p.giFbo[1], hw, hh);
        bindTex(0, p.giTex[1]);
        u2f(p.progGiBlur, "stepUV", 0.0f, 1.0f / hh);
        drawTo(p.giFbo[0], hw, hh);

        p.giPrevPos[0] = pos.X();
        p.giPrevPos[1] = pos.Y();
        p.giPrevPos[2] = pos.Z();
        const float axes[9] = {ax.X(), ax.Y(), ax.Z(), ay.X(), ay.Y(), ay.Z(), az.X(), az.Y(), az.Z()};
        for (int i = 0; i < 9; ++i)
            p.giPrevAxes[i] = axes[i];
        p.giPrevProj[0] = 1.0f / proj._11;
        p.giPrevProj[1] = 1.0f / proj._22;
    }
    else
    {
        p.giHistValid = false;
    }
    if (aoStrength > 0.0f || giOn)
        GpuMark(Poseidon::gpu::Mark::AoEnd);""",
)

rep(
    """    u4f(p.progComposite, "debugView", _lighting.debugView == 4 ? 1.0f : 0.0f, 0.0f, 0.0f, 0.0f);
    drawTo(_ssaaFbo, w, h);""",
    """    u4f(p.progComposite, "debugView", _lighting.debugView == 4 ? 1.0f : 0.0f, 0.0f, 0.0f, 0.0f);
    u1i(p.progComposite, "giTex", 5);
    u1i(p.progComposite, "giDepth", 6);
    bindTex(5, giOn ? p.giTex[0] : p.blackTex);
    bindTex(6, p.depthTex);
    u4f(p.progComposite, "giComp", _lighting.gi, _lighting.giFallbackShare, _lighting.giDebug, giOn ? 1.0f : 0.0f);
    u4f(p.progComposite, "giProj", proj._33, proj._43, depthScale, depthBias);
    drawTo(_ssaaFbo, w, h);""",
)
rep(
    """    for (int unit : {0, 2, 3, 4, 5})
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, 0);
    }""",
    """    for (int unit : {0, 2, 3, 4, 5, 6})
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, 0);
    }""",
)
rep(
    """    for (int unit : {0, 2, 3, 4, 5})
        glBindSampler(static_cast<GLuint>(unit), 0);""",
    """    for (int unit : {0, 2, 3, 4, 5, 6})
        glBindSampler(static_cast<GLuint>(unit), 0);""",
)
p.write_text(s, encoding="utf-8", newline="")
print("patched", p)
