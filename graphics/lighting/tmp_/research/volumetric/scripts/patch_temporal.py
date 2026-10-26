"""Adds the RESEARCH temporal accumulation of the volumetric scattering to EngineGL33_PostFX.cpp."""
import sys

p = sys.argv[1]
s = open(p, encoding='utf-8').read()


def rep(old, new, count=1):
    global s
    n = s.count(old)
    assert n == count, (old[:90], n)
    s = s.replace(old, new)


# --- shader
rep("""static const char s_psPostCompositeGLSL[] = R"(#version 330 core""",
    """// RESEARCH: temporal accumulation of the jittered scattering. Reprojects the
// pixel's world point (camera-relative, from the view depth in .a) into the
// previous frame, rejects history that left the screen or belongs to another
// surface, clips it to the current 3x3 neighbourhood (mean +- k sigma), and
// blends 10 % of the new frame in.
static const char s_psPostVolTemporalGLSL[] = R"(#version 330 core
in vec2 vUV;
uniform sampler2D curTex;
uniform sampler2D histTex;
uniform mat4 viewRot;
uniform mat4 prevViewRot;
uniform vec4 projInfo;   // z = 1/_11, w = 1/_22 (current)
uniform vec4 prevProj;   // x = _11, y = _22 (previous)
uniform vec4 camDelta;   // xyz current camera - previous camera, w = 1 history valid
uniform vec4 worldRect;
uniform vec4 halfTexel;  // xy 1/size, z blend, w clip sigma
out vec4 fragColor;
void main() {
    vec4 cur = texture(curTex, vUV);
    vec3 m1 = vec3(0.0);
    vec3 m2 = vec3(0.0);
    for (int j = -1; j <= 1; ++j) {
        for (int i = -1; i <= 1; ++i) {
            vec3 s = texture(curTex, vUV + vec2(i, j) * halfTexel.xy).rgb;
            m1 += s;
            m2 += s * s;
        }
    }
    m1 /= 9.0;
    vec3 sigma = sqrt(max(m2 / 9.0 - m1 * m1, vec3(0.0)));
    float z = min(cur.a, 1e4);
    vec2 ndc = (vUV - worldRect.xy) / worldRect.zw * 2.0 - 1.0;
    vec3 rel = inverse(mat3(viewRot)) * (vec3(ndc.x * projInfo.z, ndc.y * projInfo.w, 1.0) * z);
    vec3 pv = mat3(prevViewRot) * (rel + camDelta.xyz);
    float a = 1.0;
    if (camDelta.w > 0.5 && pv.z > 0.01) {
        vec2 pndc = vec2(pv.x * prevProj.x, pv.y * prevProj.y) / pv.z;
        vec2 puv = worldRect.xy + (pndc * 0.5 + 0.5) * worldRect.zw;
        if (puv.x > 0.0 && puv.x < 1.0 && puv.y > 0.0 && puv.y < 1.0) {
            vec4 h = texture(histTex, puv);
            bool sameSurface = (cur.a >= 9e4 && h.a >= 9e4) || abs(h.a - pv.z) < 0.1 * pv.z + 0.5;
            if (sameSurface) {
                vec3 lo = m1 - halfTexel.w * sigma;
                vec3 hi = m1 + halfTexel.w * sigma;
                vec3 hc = clamp(h.rgb, lo, hi);
                fragColor = vec4(mix(hc, cur.rgb, halfTexel.z), cur.a);
                return;
            }
        }
    }
    fragColor = vec4(m1 * 0.5 + cur.rgb * 0.5, cur.a); // no history: a little spatial help
}
)";

static const char s_psPostCompositeGLSL[] = R"(#version 330 core""")

# --- targets / programs
rep("""    delFbo(p.volFbo[0]);
    delFbo(p.volFbo[1]);
""", """    delFbo(p.volFbo[0]);
    delFbo(p.volFbo[1]);
    delFbo(p.volHistFbo[0]);
    delFbo(p.volHistFbo[1]);
""")
rep("""    delTex(p.volTex[0]);
    delTex(p.volTex[1]);
""", """    delTex(p.volTex[0]);
    delTex(p.volTex[1]);
    delTex(p.volHist[0]);
    delTex(p.volHist[1]);
    p.volHistValid = false;
""")
rep("""&p.progVol,
          &p.progVolBlur})""", """&p.progVol,
          &p.progVolBlur, &p.progVolTemporal})""")
rep("""        p.progVolBlur = LinkPostProgram(vs, s_psPostVolBlurGLSL, "psPostVolBlur");
""", """        p.progVolBlur = LinkPostProgram(vs, s_psPostVolBlurGLSL, "psPostVolBlur");
        p.progVolTemporal = LinkPostProgram(vs, s_psPostVolTemporalGLSL, "psPostVolTemporal");
""")
rep("""    p.volTex[1] = MakeTexture(GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, hw, hh, true);
""", """    p.volTex[1] = MakeTexture(GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, hw, hh, true);
    p.volHist[0] = MakeTexture(GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, hw, hh, true);
    p.volHist[1] = MakeTexture(GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, hw, hh, true);
""")
rep("""        p.volFbo[i] = MakeFbo(GL_COLOR_ATTACHMENT0, p.volTex[i]);
        check();
""", """        p.volFbo[i] = MakeFbo(GL_COLOR_ATTACHMENT0, p.volTex[i]);
        check();
        p.volHistFbo[i] = MakeFbo(GL_COLOR_ATTACHMENT0, p.volHist[i]);
        check();
""")
# per-frame jitter
rep("""        u4f(prog, "volParams3", _sunEnabled ? 1.0f - night : 0.0f, _lighting.volLocal, static_cast<float>(nLights),
            0.0f);""", """        const bool temporal = _lighting.volTemporal > 0.5f && p.progVolTemporal;
        p.volFrame++;
        const float frameJitter = temporal ? std::fmod(static_cast<float>(p.volFrame) * 0.61803399f, 1.0f) : 0.0f;
        u4f(prog, "volParams3", _sunEnabled ? 1.0f - night : 0.0f, _lighting.volLocal, static_cast<float>(nLights),
            frameJitter);""")
# replace the blur block with temporal + optional blur
rep("""        glUseProgram(p.progVolBlur);
        u1i(p.progVolBlur, "volTex", 0);
        bindTex(0, p.volTex[0]);
        u2f(p.progVolBlur, "stepUV", 1.0f / hw, 0.0f);
        drawTo(p.volFbo[1], hw, hh);
        bindTex(0, p.volTex[1]);
        u2f(p.progVolBlur, "stepUV", 0.0f, 1.0f / hh);
        drawTo(p.volFbo[0], hw, hh);
        GpuMark(Poseidon::gpu::Mark::ShaftsEnd);""", """        GLuint volResult = p.volTex[0];
        if (temporal)
        {
            const int cur = p.volHistIdx ^ 1;
            const GLuint tp = p.progVolTemporal;
            glUseProgram(tp);
            u1i(tp, "curTex", 0);
            u1i(tp, "histTex", 2);
            bindTex(0, p.volTex[0]);
            bindTex(2, p.volHist[p.volHistIdx]);
            const GLint lv = glGetUniformLocation(tp, "viewRot");
            if (lv >= 0)
                glUniformMatrix4fv(lv, 1, GL_FALSE, &_frameState.view._11);
            const GLint lpv = glGetUniformLocation(tp, "prevViewRot");
            if (lpv >= 0)
                glUniformMatrix4fv(lpv, 1, GL_FALSE, p.volPrevView);
            u4f(tp, "projInfo", proj._33, proj._43, 1.0f / proj._11, 1.0f / proj._22);
            u4f(tp, "prevProj", p.volPrevProj[0], p.volPrevProj[1], 0.0f, 0.0f);
            u4f(tp, "camDelta", camPos.X() - p.volPrevCam[0], camPos.Y() - p.volPrevCam[1],
                camPos.Z() - p.volPrevCam[2], p.volHistValid ? 1.0f : 0.0f);
            u4f(tp, "worldRect", rectX, rectY, rectW, rectH);
            u4f(tp, "halfTexel", 1.0f / hw, 1.0f / hh, 0.1f, 2.0f);
            drawTo(p.volHistFbo[cur], hw, hh);
            p.volHistIdx = cur;
            p.volHistValid = true;
            volResult = p.volHist[cur];
        }
        else
        {
            p.volHistValid = false;
        }
        std::memcpy(p.volPrevView, &_frameState.view._11, sizeof(p.volPrevView));
        p.volPrevProj[0] = proj._11;
        p.volPrevProj[1] = proj._22;
        p.volPrevCam[0] = camPos.X();
        p.volPrevCam[1] = camPos.Y();
        p.volPrevCam[2] = camPos.Z();
        if (_lighting.volBlur > 0.0f)
        {
            glUseProgram(p.progVolBlur);
            u1i(p.progVolBlur, "volTex", 0);
            bindTex(0, volResult);
            u2f(p.progVolBlur, "stepUV", _lighting.volBlur / hw, 0.0f);
            drawTo(p.volFbo[1], hw, hh);
            bindTex(0, p.volTex[1]);
            u2f(p.progVolBlur, "stepUV", 0.0f, _lighting.volBlur / hh);
            drawTo(p.volFbo[0], hw, hh);
            volResult = p.volTex[0];
        }
        p.volResultTex = volResult;
        GpuMark(Poseidon::gpu::Mark::ShaftsEnd);""")
rep("""    bindTex(4, volumetric ? p.volTex[0] : (shaftWeight > 0.001f ? p.shaftTex : p.blackTex));""",
    """    bindTex(4, volumetric ? p.volResultTex : (shaftWeight > 0.001f ? p.shaftTex : p.blackTex));""")
rep("""    const bool volumetric = worldDepth && glow && _lighting.volumetric > 0.0f && GScene && GScene->GetCamera() &&
                            p.progVol && p.progVolBlur;""",
    """    const bool volumetric = worldDepth && glow && _lighting.volumetric > 0.0f && GScene && GScene->GetCamera() &&
                            p.progVol && p.progVolBlur;
    if (!volumetric)
        p.volHistValid = false;""")
if '#include <cstring>' not in s:
    rep('#include <cmath>\n', '#include <cmath>\n#include <cstring>\n')
open(p, 'w', encoding='utf-8', newline='').write(s)
print('patched temporal')
