"""Research track aa, resume step: add a present-pass GPU timer and a separable
Lanczos2 variant (p = 6, 7) to the scratch build in D:\\Arma_CWA_worktrees\\gfx-aa."""
from pathlib import Path

ROOT = Path(r"D:\Arma_CWA_worktrees\gfx-aa\arma_CWR\engine\PoseidonGL33")


def rep(s, old, new):
    assert old in s, old[:80]
    return s.replace(old, new)


p = ROOT / "EngineGL33_VertexBuffer.cpp"
s = p.read_text(encoding="utf-8")

s = rep(s, '''struct PresentAA
{
    bool tried = false;
    bool ok = false;
    GLuint progDown = 0, progSharpen = 0, vao = 0;
    GLuint resolveTex = 0, resolveFbo = 0;
    GLuint downTex = 0, downFbo = 0;
    int rw = 0, rh = 0, dw = 0, dh = 0;
};
PresentAA s_present;
''', '''struct PresentAA
{
    bool tried = false;
    bool ok = false;
    GLuint progDown = 0, progSharpen = 0, vao = 0;
    GLuint resolveTex = 0, resolveFbo = 0;
    GLuint downTex = 0, downFbo = 0;
    GLuint sepTex = 0, sepFbo = 0; // separable Lanczos: horizontal pass target (window w x frame h, RGBA16F)
    int rw = 0, rh = 0, dw = 0, dh = 0, sw = 0, sh = 0;
};
PresentAA s_present;

// GPU time of ResolveSSAAToDefault (stock blits or the research pass), from a
// private ring of timestamp pairs read back late, never waited for. Shown as
// "present" at the end of the triGpuTimes summary.
struct PresentTimer
{
    static constexpr int kN = 8;
    GLuint q[kN][2] = {};
    bool pending[kN] = {};
    int slot = 0;
    double avg = 0.0;
    int n = 0;
    void Begin()
    {
        if (!q[0][0])
            glGenQueries(kN * 2, &q[0][0]);
        slot = (slot + 1) % kN;
        if (pending[slot])
        {
            GLint a = 0;
            glGetQueryObjectiv(q[slot][1], GL_QUERY_RESULT_AVAILABLE, &a);
            if (a)
            {
                GLuint64 t0 = 0, t1 = 0;
                glGetQueryObjectui64v(q[slot][0], GL_QUERY_RESULT, &t0);
                glGetQueryObjectui64v(q[slot][1], GL_QUERY_RESULT, &t1);
                const double ms = t1 > t0 ? double(t1 - t0) * 1e-6 : 0.0;
                avg = n == 0 ? ms : avg + 0.1 * (ms - avg);
                ++n;
            }
            pending[slot] = false;
        }
        glQueryCounter(q[slot][0], GL_TIMESTAMP);
    }
    void End()
    {
        glQueryCounter(q[slot][1], GL_TIMESTAMP);
        pending[slot] = true;
    }
};
PresentTimer s_presentTimer;

struct PresentTimeScope
{
    bool on;
    explicit PresentTimeScope(bool o) : on(o)
    {
        if (on)
            s_presentTimer.Begin();
    }
    ~PresentTimeScope()
    {
        if (on)
            s_presentTimer.End();
    }
};
''')

s = rep(s, '''void PresentTarget(GLuint& tex, GLuint& fbo, int& tw, int& th, int w, int h)
{''', '''void PresentTarget(GLuint& tex, GLuint& fbo, int& tw, int& th, int w, int h, bool half = false)
{''')

s = rep(s, '''    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);''', '''    if (half)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_HALF_FLOAT, nullptr);
    else
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);''')

s = rep(s, '''    tw = w;
    th = h;
}
} // namespace
''', '''    tw = w;
    th = h;
}
} // namespace

// Read by EngineGL33::GpuTimingSummary / reset by SetGpuTimingEnabled (research track aa).
void AaPresentTimerReset()
{
    s_presentTimer.avg = 0.0;
    s_presentTimer.n = 0;
    for (bool& b : s_presentTimer.pending)
        b = false;
}
bool AaPresentTimerMs(float& ms)
{
    ms = static_cast<float>(s_presentTimer.avg);
    return s_presentTimer.n > 0;
}
''')

s = rep(s, '''    if (_sdlWindow)
        SDL_GetWindowSizeInPixels(_sdlWindow, &winW, &winH);

    // RESEARCH PROTOTYPE (track aa): remastered present pass, see above.''', '''    if (_sdlWindow)
        SDL_GetWindowSizeInPixels(_sdlWindow, &winW, &winH);
    PresentTimeScope presentTime(_gpuTiming && _glContext);

    // RESEARCH PROTOTYPE (track aa): remastered present pass, see above.''')

s = rep(s, '''            const float sharp = p == 4 ? 1.0f : (p == 5 ? 0.25f : 0.5f);
            const bool sharpen = p == 2 || p == 3 || p == 4 || p == 5;
            PresentTarget(pr.resolveTex, pr.resolveFbo, pr.rw, pr.rh, _ssaaW, _ssaaH);
            if (sharpen)
                PresentTarget(pr.downTex, pr.downFbo, pr.dw, pr.dh, winW, winH);''', '''            const float sharp = p == 4 ? 1.0f : (p == 5 ? 0.25f : 0.5f);
            const bool sharpen = p == 2 || p == 3 || p == 4 || p == 5 || p == 6;
            const bool separable = p == 6 || p == 7; // two 1D Lanczos2 passes instead of the 2D kernel
            PresentTarget(pr.resolveTex, pr.resolveFbo, pr.rw, pr.rh, _ssaaW, _ssaaH);
            if (sharpen)
                PresentTarget(pr.downTex, pr.downFbo, pr.dw, pr.dh, winW, winH);
            if (separable)
                PresentTarget(pr.sepTex, pr.sepFbo, pr.sw, pr.sh, winW, _ssaaH, /*half*/ true);''')

s = rep(s, '''            // Downsample (into the window, or into downTex for sharpening).
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, sharpen ? pr.downFbo : 0);
            glViewport(0, 0, winW, winH);
            glUseProgram(pr.progDown);
            glUniform1i(glGetUniformLocation(pr.progDown, "src"), 0);
            glUniform4f(glGetUniformLocation(pr.progDown, "srcSize"), float(_ssaaW), float(_ssaaH), 1.0f / _ssaaW,
                        1.0f / _ssaaH);
            glUniform4f(glGetUniformLocation(pr.progDown, "dstSize"), float(winW), float(winH), 1.0f / winW,
                        1.0f / winH);
            glUniform4f(glGetUniformLocation(pr.progDown, "sel"), float(p), sharp, 0.0f, 0.0f);
            glBindTexture(GL_TEXTURE_2D, pr.resolveTex);
            glDrawArrays(GL_TRIANGLES, 0, 3);''', '''            glUseProgram(pr.progDown);
            glUniform1i(glGetUniformLocation(pr.progDown, "src"), 0);
            if (separable)
            {
                // Horizontal pass: frame (ssaaW x ssaaH) -> sepTex (winW x ssaaH).
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, pr.sepFbo);
                glViewport(0, 0, winW, _ssaaH);
                glUniform4f(glGetUniformLocation(pr.progDown, "srcSize"), float(_ssaaW), float(_ssaaH),
                            1.0f / _ssaaW, 1.0f / _ssaaH);
                glUniform4f(glGetUniformLocation(pr.progDown, "dstSize"), float(winW), float(_ssaaH), 1.0f / winW,
                            1.0f / _ssaaH);
                glUniform4f(glGetUniformLocation(pr.progDown, "sel"), float(p), sharp, 1.0f, 0.0f);
                glBindTexture(GL_TEXTURE_2D, pr.resolveTex);
                glDrawArrays(GL_TRIANGLES, 0, 3);
                // Vertical pass: sepTex -> window (or downTex for sharpening).
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, sharpen ? pr.downFbo : 0);
                glViewport(0, 0, winW, winH);
                glUniform4f(glGetUniformLocation(pr.progDown, "srcSize"), float(winW), float(_ssaaH), 1.0f / winW,
                            1.0f / _ssaaH);
                glUniform4f(glGetUniformLocation(pr.progDown, "dstSize"), float(winW), float(winH), 1.0f / winW,
                            1.0f / winH);
                glUniform4f(glGetUniformLocation(pr.progDown, "sel"), float(p), sharp, 2.0f, 0.0f);
                glBindTexture(GL_TEXTURE_2D, pr.sepTex);
                glDrawArrays(GL_TRIANGLES, 0, 3);
            }
            else
            {
                // Downsample (into the window, or into downTex for sharpening).
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, sharpen ? pr.downFbo : 0);
                glViewport(0, 0, winW, winH);
                glUniform4f(glGetUniformLocation(pr.progDown, "srcSize"), float(_ssaaW), float(_ssaaH),
                            1.0f / _ssaaW, 1.0f / _ssaaH);
                glUniform4f(glGetUniformLocation(pr.progDown, "dstSize"), float(winW), float(winH), 1.0f / winW,
                            1.0f / winH);
                glUniform4f(glGetUniformLocation(pr.progDown, "sel"), float(p), sharp, 0.0f, 0.0f);
                glBindTexture(GL_TEXTURE_2D, pr.resolveTex);
                glDrawArrays(GL_TRIANGLES, 0, 3);
            }''')

s = rep(s, '''//   p = 5  Lanczos2 + RCAS 0.25
''', '''//   p = 5  Lanczos2 + RCAS 0.25
//   p = 6  separable Lanczos2 (two 1D passes) + RCAS 0.5
//   p = 7  separable Lanczos2
''')
p.write_text(s, encoding="utf-8", newline="")

p = ROOT / "EngineGL33_GpuTimer.cpp"
s = p.read_text(encoding="utf-8")
s = rep(s, '''using Poseidon::gpu::kMarkCount;
using Poseidon::gpu::Mark;
''', '''using Poseidon::gpu::kMarkCount;
using Poseidon::gpu::Mark;

// RESEARCH PROTOTYPE (track aa): present-pass timer, EngineGL33_VertexBuffer.cpp.
void AaPresentTimerReset();
bool AaPresentTimerMs(float& ms);
''')
s = rep(s, '''    _gpuTimings.Reset();
    if (!enabled)''', '''    _gpuTimings.Reset();
    AaPresentTimerReset();
    if (!enabled)''')
s = rep(s, '''    if (!_gpuTiming)
        return "gpu: timing off";
    return _gpuTimings.Summary();''', '''    if (!_gpuTiming)
        return "gpu: timing off";
    std::string out = _gpuTimings.Summary();
    float presentMs = 0.0f;
    if (AaPresentTimerMs(presentMs))
    {
        char buf[48];
        std::snprintf(buf, sizeof(buf), " | present %.3f", presentMs);
        out += buf;
    }
    return out;''')
s = rep(s, '#include <cstring>', '#include <cstdio>\n#include <cstring>')
p.write_text(s, encoding="utf-8", newline="")
print("patched")
