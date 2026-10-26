// Remastered lighting: screen-space reflections (feature switch ssr,
// INTEGRATION-PLAN.md step B6; the math is Graphics/Remaster/ScreenReflections.hpp,
// the shaders EngineGL33_PostSsrShaders.cpp).
//
// While the switch is on, the world's passes write each draw's surface tag into
// the top bits of the stencil (ApplyPassState, ApplyDepthMode; the projected
// shadows keep the low bits). The post pass then runs two stages:
//   - TagReadout: the tags out of the resolved stencil into an R8 texture (one
//     fullscreen draw per tag bit under a stencil test, added up), and the
//     share of water and wettable ground on a small grid, read back through a
//     ring of pixel-pack buffers a few frames later (never waited on) for the
//     frame gate (UpdateFeatureGates);
//   - Ssr, when the gate is on: the resolved frame into a mip chain, the trace
//     at half resolution over the half-resolution linear depth (a multiply and
//     an add per texel, and the width of its cone at what it reflects), the
//     rough blur (a sideways gather over that width, so a rough reflection
//     has no hard outline), and the composite of the full-resolution frame into
//     _ssr.outTex, which the glow stages and the composite then read instead
//     of the resolved frame (PostFrame::frameColor). The resolved frame itself
//     stays as the world drew it (the TAA particle mask compares it with the
//     opaque copy).
// The stages bind their inputs to kPostUnits, and the sky table to its own
// unit 8, which holds the same table outside the post pass.

#include <PoseidonGL33/EngineGL33.hpp>
#include <PoseidonGL33/EngineGL33_Lighting.hpp>
#include <PoseidonGL33/EngineGL33_PostPass.hpp>
#include <PoseidonGL33/GL33BindCache.hpp>
#include <Poseidon/Core/Global.hpp>
#include <Poseidon/Graphics/Core/GLBlendState.hpp>
#include <Poseidon/Graphics/Core/GLDepthStencilState.hpp>
#include <Poseidon/Graphics/Remaster/FeatureSwitches.hpp>
#include <Poseidon/World/Scene/Scene.hpp>
#include <Poseidon/World/Scene/Camera/Camera.hpp>
#include <Poseidon/World/Terrain/Landscape.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>

namespace rm = Poseidon::remaster;
using namespace Poseidon::gl33post;
using Poseidon::gl33lighting::PostShader;
using Poseidon::gl33lighting::PostShaderName;
using Poseidon::gl33lighting::PostShaderSource;

namespace
{

GLuint CompileSsrShader(GLenum type, const char* src, const char* name)
{
    std::string overrideSource;
    if (LoadShaderOverride(name, overrideSource))
        src = overrideSource.c_str();
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        char log[1024];
        glGetShaderInfoLog(s, sizeof(log), nullptr, log);
        LOG_ERROR(Graphics, "GL33 reflections: shader compile error [{}]: {}", name, log);
        glDeleteShader(s);
        return 0;
    }
    return s;
}

GLuint LinkSsrProgram(GLuint vs, PostShader shader)
{
    GLuint fs = CompileSsrShader(GL_FRAGMENT_SHADER, PostShaderSource(shader), PostShaderName(shader));
    if (!vs || !fs)
    {
        if (fs)
            glDeleteShader(fs);
        return 0;
    }
    GLuint p = glCreateProgram();
    glAttachShader(p, vs);
    glAttachShader(p, fs);
    glLinkProgram(p);
    glDeleteShader(fs);
    GLint ok = 0;
    glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok)
    {
        char log[1024];
        glGetProgramInfoLog(p, sizeof(log), nullptr, log);
        LOG_ERROR(Graphics, "GL33 reflections: program link error [{}]: {}", PostShaderName(shader), log);
        glDeleteProgram(p);
        return 0;
    }
    // A program that reads the rig takes LitConstants from its binding.
    const GLuint block = glGetUniformBlockIndex(p, "LitConstants");
    if (block != GL_INVALID_INDEX)
        glUniformBlockBinding(p, block, Poseidon::gl33lighting::kLitBinding);
    return p;
}

GLuint SsrTexture(GLenum internalFormat, GLenum format, GLenum type, int w, int h, bool linear)
{
    GLuint t = 0;
    glGenTextures(1, &t);
    glBindTexture(GL_TEXTURE_2D, t);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, type, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, linear ? GL_LINEAR : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, linear ? GL_LINEAR : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    return t;
}

GLuint SsrFbo(GLuint tex)
{
    GLuint f = 0;
    glGenFramebuffers(1, &f);
    glBindFramebuffer(GL_FRAMEBUFFER, f);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    return f;
}

// The mip levels of the reflections' blur source: enough for the widest cone
// (a wet road's 0.1 rad at a few metres spans some 60 pixels).
constexpr int kSsrMaxMipLevels = 7;

} // namespace

bool EngineGL33::SsrTagging() const
{
    return _remasterPrograms && _worldPassOpen && !_portraitTargetSize &&
           rm::FeatureOn(_lighting.features, rm::Feature::Ssr);
}

void EngineGL33::DestroySsrTargets()
{
    SsrTargets& s = _ssr;
    auto delTex = [](unsigned int& t)
    {
        if (t)
        {
            GL33Bind::OnTexDeleted(t);
            glDeleteTextures(1, &t);
        }
        t = 0;
    };
    auto delFbo = [](unsigned int& f)
    {
        if (f)
            glDeleteFramebuffers(1, &f);
        f = 0;
    };
    delFbo(s.tagFbo);
    delFbo(s.shareFbo);
    delFbo(s.mipFbo);
    delFbo(s.traceFbo);
    delFbo(s.blurFbo);
    delFbo(s.outFbo);
    delTex(s.tagTex);
    delTex(s.shareTex);
    delTex(s.mipTex);
    delTex(s.traceTex);
    delTex(s.spreadTex);
    delTex(s.blurTex);
    delTex(s.outTex);
    for (int i = 0; i < SsrTargets::kRing; ++i)
    {
        if (s.fence[i])
            glDeleteSync(s.fence[i]);
        s.fence[i] = nullptr;
        if (s.pbo[i])
            glDeleteBuffers(1, &s.pbo[i]);
        s.pbo[i] = 0;
    }
    s.ringNext = 0;
    s.w = s.h = 0;
}

void EngineGL33::DestroySsrPrograms()
{
    SsrTargets& s = _ssr;
    for (unsigned int* prog : {&s.progTag, &s.progShare, &s.progTrace, &s.progBlur, &s.progComposite})
    {
        if (*prog)
            glDeleteProgram(*prog);
        *prog = 0;
    }
    s.programsTried = false;
    s.unavailable = false;
}

bool EngineGL33::EnsureSsrResources(int w, int h)
{
    SsrTargets& s = _ssr;
    const PostTargets& p = _post;
    if (!s.programsTried)
    {
        s.programsTried = true;
        GLuint vs = CompileSsrShader(GL_VERTEX_SHADER, PostShaderSource(PostShader::Vertex),
                                     PostShaderName(PostShader::Vertex));
        s.progTag = LinkSsrProgram(vs, PostShader::SsrTag);
        s.progShare = LinkSsrProgram(vs, PostShader::SsrShare);
        s.progTrace = LinkSsrProgram(vs, PostShader::SsrTrace);
        s.progBlur = LinkSsrProgram(vs, PostShader::SsrBlur);
        s.progComposite = LinkSsrProgram(vs, PostShader::SsrComposite);
        if (vs)
            glDeleteShader(vs);
        if (!s.progTag || !s.progShare || !s.progTrace || !s.progBlur || !s.progComposite)
        {
            LOG_ERROR(Graphics, "GL33 reflections: programs failed to build; screen-space reflections disabled");
            s.unavailable = true;
        }
    }
    if (s.unavailable || !p.depthTex)
        return false;
    if (s.w == w && s.h == h && s.tagTex)
        return true;

    DestroySsrTargets();
    const int hw = (w + 1) / 2;
    const int hh = (h + 1) / 2;
    GL33Bind::ActiveUnit(kUploadUnit - GL_TEXTURE0);
    s.tagTex = SsrTexture(GL_R8, GL_RED, GL_UNSIGNED_BYTE, w, h, false);
    // RGBA8 and read back as GL_RGBA bytes: the format the drivers copy without a conversion.
    s.shareTex = SsrTexture(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, rm::kSsrShareW, rm::kSsrShareH, false);
    s.traceTex = SsrTexture(GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, hw, hh, false);
    s.spreadTex = SsrTexture(GL_R16F, GL_RED, GL_HALF_FLOAT, hw, hh, false);
    s.blurTex = SsrTexture(GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, hw, hh, false);
    s.outTex = SsrTexture(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, w, h, true);
    // The blur source: the frame and its mips, trilinear.
    int levels = 1;
    for (int m = std::max(w, h); m > 1 && levels < kSsrMaxMipLevels; m >>= 1)
        ++levels;
    s.mipLevels = levels;
    glGenTextures(1, &s.mipTex);
    glBindTexture(GL_TEXTURE_2D, s.mipTex);
    for (int l = 0, lw = w, lh = h; l < levels; ++l, lw = std::max(1, lw / 2), lh = std::max(1, lh / 2))
        glTexImage2D(GL_TEXTURE_2D, l, GL_RGBA8, lw, lh, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, levels - 1);
    glBindTexture(GL_TEXTURE_2D, 0);
    GL33Bind::Invalidate();

    bool complete = true;
    auto check = [&complete]()
    {
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            complete = false;
    };
    // The readout draws under a stencil test against the resolved stencil.
    s.tagFbo = SsrFbo(s.tagTex);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, p.depthTex, 0);
    check();
    s.shareFbo = SsrFbo(s.shareTex);
    check();
    s.mipFbo = SsrFbo(s.mipTex);
    check();
    // The trace writes its result and its spread (two targets).
    s.traceFbo = SsrFbo(s.traceTex);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, s.spreadTex, 0);
    const GLenum traceTargets[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, traceTargets);
    check();
    s.blurFbo = SsrFbo(s.blurTex);
    check();
    s.outFbo = SsrFbo(s.outTex);
    check();
    const GLsizeiptr shareBytes = static_cast<GLsizeiptr>(rm::kSsrShareW) * rm::kSsrShareH * 4;
    glGenBuffers(SsrTargets::kRing, s.pbo);
    for (unsigned int buf : s.pbo)
    {
        glBindBuffer(GL_PIXEL_PACK_BUFFER, buf);
        glBufferData(GL_PIXEL_PACK_BUFFER, shareBytes, nullptr, GL_STREAM_READ);
    }
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    BindFrameRenderTarget();
    if (!complete)
    {
        LOG_WARN(Graphics, "GL33 reflections: targets {}x{} incomplete; screen-space reflections disabled", w, h);
        DestroySsrTargets();
        s.unavailable = true;
        return false;
    }
    s.w = w;
    s.h = h;
    LOG_INFO(Graphics, "GL33 reflections: targets {}x{} (trace {}x{}, {} blur levels)", w, h, hw, hh, levels);
    return true;
}

void EngineGL33::ReadSsrShares()
{
    SsrTargets& s = _ssr;
    // Every measurement the GPU has finished, oldest first (never waited on):
    // the next slot to write holds the oldest.
    for (int k = 0; k < SsrTargets::kRing; ++k)
    {
        const int i = (s.ringNext + k) % SsrTargets::kRing;
        if (!s.fence[i])
            continue;
        const GLenum r = glClientWaitSync(s.fence[i], 0, 0);
        if (r != GL_ALREADY_SIGNALED && r != GL_CONDITION_SATISFIED)
            continue;
        std::uint8_t cells[rm::kSsrShareW * rm::kSsrShareH * 4];
        glBindBuffer(GL_PIXEL_PACK_BUFFER, s.pbo[i]);
        glGetBufferSubData(GL_PIXEL_PACK_BUFFER, 0, sizeof(cells), cells);
        float water = 0.0f, ground = 0.0f;
        for (int c = 0; c < rm::kSsrShareW * rm::kSsrShareH; ++c)
        {
            water += cells[c * 4] / 255.0f;
            ground += cells[c * 4 + 1] / 255.0f;
        }
        const float n = static_cast<float>(rm::kSsrShareW * rm::kSsrShareH);
        s.waterShare = water / n;
        s.groundShare = ground / n;
        s.shareFrame = s.pboFrame[i];
        s.shareValid = true;
        glDeleteSync(s.fence[i]);
        s.fence[i] = nullptr;
    }
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    // This frame's grid into the next slot; a measurement still in flight
    // there after a whole ring is dropped.
    const int slot = s.ringNext;
    if (s.fence[slot])
    {
        glDeleteSync(s.fence[slot]);
        s.fence[slot] = nullptr;
    }
    glBindFramebuffer(GL_READ_FRAMEBUFFER, s.shareFbo);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, s.pbo[slot]);
    glReadPixels(0, 0, rm::kSsrShareW, rm::kSsrShareH, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    s.fence[slot] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    s.pboFrame[slot] = GetFrameCounter();
    s.ringNext = (slot + 1) % SsrTargets::kRing;
}

void EngineGL33::PostTagReadout(const PostFrame& f)
{
    if (!EnsureSsrResources(f.w, f.h))
        return;
    const SsrTargets& s = _ssr;
    // The tags: zero, then each tag bit added where the stencil has it.
    glUseProgram(s.progTag);
    Set4(s.progTag, "tagValue", 0.0f, 0.0f, 0.0f, 1.0f);
    DrawTo(s.tagFbo, f.w, f.h);
    Poseidon::render::blend::Additive(); // (SRC_ALPHA, ONE) with alpha 1 = add
    for (const std::uint8_t bit : {rm::ssrtag::kWater, rm::ssrtag::kTerrain, rm::ssrtag::kCover})
    {
        Poseidon::render::depthstencil::StencilTestEqual(bit, bit);
        Set4(s.progTag, "tagValue", static_cast<float>(bit) / 255.0f, 0.0f, 0.0f, 1.0f);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
    Poseidon::render::depthstencil::StencilOffFullMask();
    Poseidon::render::blend::Opaque();
    // The screen shares for the gate.
    glUseProgram(s.progShare);
    SetInt(s.progShare, "tagTex", 0);
    BindTex(0, s.tagTex);
    Set4(s.progShare, "grid", static_cast<float>(f.w), static_cast<float>(f.h), static_cast<float>(rm::kSsrShareW),
         static_cast<float>(rm::kSsrShareH));
    DrawTo(s.shareFbo, rm::kSsrShareW, rm::kSsrShareH);
    ReadSsrShares();
}

void EngineGL33::PostSsr(PostFrame& f)
{
    if (!EnsureSsrResources(f.w, f.h) || !GScene || !GScene->GetCamera())
        return;
    const PostTargets& p = _post;
    const SsrTargets& s = _ssr;
    // 1. The resolved frame and its mips: the reflections' blur source.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, p.colorFbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, s.mipFbo);
    glBlitFramebuffer(0, 0, f.w, f.h, 0, 0, f.w, f.h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    BindTex(3, s.mipTex);
    glGenerateMipmap(GL_TEXTURE_2D);

    // 2. The trace, at half resolution.
    Camera* cam = GScene->GetCamera();
    const Matrix4 inv = cam->InverseScaled();
    const Vector3 ex = inv.Rotate(Vector3(1, 0, 0));
    const Vector3 ey = inv.Rotate(Vector3(0, 1, 0));
    const Vector3 ez = inv.Rotate(Vector3(0, 0, 1));
    // Column-major: column j is the view-space image of world axis j.
    const float w2v[9] = {ex.X(), ex.Y(), ex.Z(), ey.X(), ey.Y(), ey.Z(), ez.X(), ez.Y(), ez.Z()};
    // Its inverse (the camera's rotation may carry a scale).
    float v2w[9] = {1, 0, 0, 0, 1, 0, 0, 0, 1};
    {
        auto at = [&w2v](int r, int c) { return w2v[c * 3 + r]; };
        const float det = at(0, 0) * (at(1, 1) * at(2, 2) - at(1, 2) * at(2, 1)) -
                          at(0, 1) * (at(1, 0) * at(2, 2) - at(1, 2) * at(2, 0)) +
                          at(0, 2) * (at(1, 0) * at(2, 1) - at(1, 1) * at(2, 0));
        if (std::fabs(det) > 1e-12f)
        {
            const float id = 1.0f / det;
            auto set = [&v2w](int r, int c, float v) { v2w[c * 3 + r] = v; };
            set(0, 0, (at(1, 1) * at(2, 2) - at(1, 2) * at(2, 1)) * id);
            set(0, 1, (at(0, 2) * at(2, 1) - at(0, 1) * at(2, 2)) * id);
            set(0, 2, (at(0, 1) * at(1, 2) - at(0, 2) * at(1, 1)) * id);
            set(1, 0, (at(1, 2) * at(2, 0) - at(1, 0) * at(2, 2)) * id);
            set(1, 1, (at(0, 0) * at(2, 2) - at(0, 2) * at(2, 0)) * id);
            set(1, 2, (at(0, 2) * at(1, 0) - at(0, 0) * at(1, 2)) * id);
            set(2, 0, (at(1, 0) * at(2, 1) - at(1, 1) * at(2, 0)) * id);
            set(2, 1, (at(0, 1) * at(2, 0) - at(0, 0) * at(2, 1)) * id);
            set(2, 2, (at(0, 0) * at(1, 1) - at(0, 1) * at(1, 0)) * id);
        }
    }
    Landscape* land = GScene->GetLandscape();
    const Vector3 cp = cam->Position();
    const GfxMatrix& proj = _frameState.projection;
    float farZ = std::fabs(1.0f - proj._33) > 1e-6f ? proj._43 / (1.0f - proj._33) : 5000.0f;
    if (!(farZ > 1.0f && farZ < 1e6f))
        farZ = 5000.0f;
    // The ground's wetness: the knob when it forces one, else the weather's
    // (the wet weather's state while its gate is on, else the rain). With the
    // wet weather on, the forward pass darkens the wet ground and its puddles
    // (wetPuddle, which the trace shares) set where the mirrors are.
    const bool wetWeather =
        _featureGates.computed && _featureGates.frame == GetFrameCounter() && _featureGates[rm::Feature::WetWeather].on;
    const float weatherWet = wetWeather ? _wetFrame.surface : (land ? land->GetRainDensity() : 0.0f);
    float wet = _lighting.ssrWet >= 0.0f ? _lighting.ssrWet : weatherWet;
    wet = std::clamp(wet, 0.0f, 1.0f);
    // The sea's reflection replaces the sea program's sky term, so it runs
    // only while that program draws the sea.
    const bool seaProgram = rm::FeatureOn(_lighting.features, rm::Feature::SeaWaves) && _featureGates.computed &&
                            _featureGates.frame == GetFrameCounter() && _featureGates[rm::Feature::SeaWaves].on;
    const int debug = _lighting.debugView;
    const GLuint tr = s.progTrace;
    glUseProgram(tr);
    SetInt(tr, "linDepthTex", 0);
    SetInt(tr, "tagTex", 2);
    SetInt(tr, "mipTex", 3);
    SetInt(tr, "terrainHeight", 4);
    SetInt(tr, "depthTex", 5);
    SetInt(tr, "skyLut", Poseidon::gl33lighting::kSkyLutUnit);
    SetInt(tr, "groundField", Poseidon::gl33lighting::kGroundFieldUnit);
    SetInt(tr, "tex0", 0);
    BindTex(0, p.linDepthTex);
    BindTex(2, s.tagTex);
    BindTex(3, s.mipTex);
    BindTex(4, _terrainHeightTex ? _terrainHeightTex : _dummyHeightTex);
    BindTex(5, p.depthTex);
    // The ground field, the land's classes the forward pass used (the world's
    // own binding of that unit, so it stays bound after the stage).
    BindTex(Poseidon::gl33lighting::kGroundFieldUnit, GroundFieldTexture());
    // (The raw binds above left the bind cache's active unit stale: without
    // this the sky table could land on unit 5, the depth's; look director.)
    GL33Bind::Invalidate();
    BindSkyTable();
    Set4(tr, "projInfo", f.projInfo[0], f.projInfo[1], f.projInfo[2], f.projInfo[3]);
    Set4(tr, "depthMap", f.depthScale, f.depthBias, 0.0f, 0.0f);
    Set4(tr, "worldRect", f.rect[0], f.rect[1], f.rect[2], f.rect[3]);
    Set4(tr, "screen", static_cast<float>(f.w), static_cast<float>(f.h), static_cast<float>(f.hw),
         static_cast<float>(f.hh));
    const GLint w2vLoc = glGetUniformLocation(tr, "w2v");
    if (w2vLoc >= 0)
        glUniformMatrix3fv(w2vLoc, 1, GL_FALSE, w2v);
    const GLint v2wLoc = glGetUniformLocation(tr, "v2w");
    if (v2wLoc >= 0)
        glUniformMatrix3fv(v2wLoc, 1, GL_FALSE, v2w);
    Set4(tr, "camWorld", cp.X(), cp.Y(), cp.Z(), land ? land->GetSeaLevel() : 0.0f);
    // The world's fog as BeginPass copied it (the surfaces the trace changes
    // draw with it), not the frame state's fog flag, which is the last draw's.
    Set4(tr, "fogParam", _ssrWorldFog[0], _ssrWorldFog[1], 0.0f, 0.0f);
    Set4(tr, "fogColor", _frameState.fogColor[0], _frameState.fogColor[1], _frameState.fogColor[2], 1.0f);
    Set4(tr, "ssrParams", _lighting.ssrWater, wet, Glob.time.toFloat(), static_cast<float>(s.mipLevels));
    Set4(tr, "ssrFrame", f.noiseShift, debug == rm::kDebugViewSsrReflection ? 1.0f : 0.0f, farZ,
         seaProgram ? 1.0f : 0.0f);
    Set4(tr, "ssrWetMode", wetWeather ? 1.0f : 0.0f, wetWeather ? _wetFrame.puddles : 0.0f, 0.0f, 0.0f);
    DrawTo(s.traceFbo, f.hw, f.hh);

    // 3. The rough blur: the trace gathered sideways over each texel's spread.
    const GLuint bl = s.progBlur;
    glUseProgram(bl);
    SetInt(bl, "traceTex", 0);
    SetInt(bl, "spreadTex", 2);
    SetInt(bl, "linDepthTex", 3);
    BindTex(0, s.traceTex);
    BindTex(2, s.spreadTex);
    BindTex(3, p.linDepthTex);
    Set4(bl, "screen", static_cast<float>(f.w), static_cast<float>(f.h), static_cast<float>(f.hw),
         static_cast<float>(f.hh));
    DrawTo(s.blurFbo, f.hw, f.hh);

    // 4. The reflections on the full-resolution frame.
    const GLuint co = s.progComposite;
    glUseProgram(co);
    SetInt(co, "colorTex", 0);
    SetInt(co, "tagTex", 2);
    SetInt(co, "traceTex", 3);
    SetInt(co, "linDepthTex", 4);
    SetInt(co, "depthTex", 5);
    BindTex(0, p.colorTex);
    BindTex(2, s.tagTex);
    BindTex(3, s.blurTex);
    BindTex(4, p.linDepthTex);
    BindTex(5, p.depthTex);
    Set4(co, "projInfo", f.projInfo[0], f.projInfo[1], f.projInfo[2], f.projInfo[3]);
    Set4(co, "depthMap", f.depthScale, f.depthBias, 0.0f, 0.0f);
    Set4(co, "worldRect", f.rect[0], f.rect[1], f.rect[2], f.rect[3]);
    Set4(co, "screen", static_cast<float>(f.w), static_cast<float>(f.h), static_cast<float>(f.hw),
         static_cast<float>(f.hh));
    Set4(co, "debugView", debug == rm::kDebugViewSsrTags ? 1.0f : 0.0f,
         debug == rm::kDebugViewSsrReflection ? 1.0f : 0.0f, 0.0f, 0.0f);
    Set4(co, "ssrCamera", f.camera ? 1.0f : 0.0f, 0.0f, 0.0f, 0.0f);
    DrawTo(s.outFbo, f.w, f.h);
    f.frameColor = s.outTex;
}
