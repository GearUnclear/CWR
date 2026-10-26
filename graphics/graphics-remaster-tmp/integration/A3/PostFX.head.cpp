// Remastered lighting: post pass at the world/HUD boundary.
//
// World::Draw calls Engine::EndWorldScene() after the 3D scene and before the
// map, HUD and dialogs. When the remastered lighting and its post effects are
// on, this resolves the frame target (colour + depth), then:
//   1. ambient occlusion (Alchemy/SAO style) at half resolution from the depth
//      buffer, blurred with a depth-aware separable filter;
//   2. a five-level bloom chain from the bright parts of the image;
//   3. sun shafts: a radial blur of the visible sky toward the sun;
//   4. a composite back into the frame target. Occlusion darkens only the sky
//      light: the remastered shaders store one minus each opaque pixel's sky
//      light share in the colour alpha (blended draws keep the destination
//      alpha), so a sunlit wall next to a corner keeps its sunlight and pre-lit
//      or alpha-to-coverage pixels (alpha 1) are left alone. Bloom and shafts
//      add in linear light with a soft screen-style add that can never clip,
//      and leave the frame exactly as it was where they are zero (the frame is
//      already tonemapped: no second curve). The shafts are light scattered
//      by the air in front of each pixel, so they grow with its distance
//      (LightRig ShaftInscatter): a nearby vehicle is not veiled. A dither
//      hides banding.
// The HUD then draws on top, untouched.
//
// The shaders live in EngineGL33_PostShaders.cpp, which has no GL code, so the
// unit tests compile the sources the engine builds (PostShaderSource in
// EngineGL33_Lighting.hpp).
//
// All GL state the passes touch is restored or the bind caches invalidated;
// texture unit 1 is never used (its sampler object belongs to ApplyPipeline).

#include <PoseidonGL33/EngineGL33.hpp>
#include <PoseidonGL33/EngineGL33_Lighting.hpp>
#include <PoseidonGL33/GL33BindCache.hpp>
#include <Poseidon/Graphics/Core/GLBlendState.hpp>
#include <Poseidon/Graphics/Core/GLDepthStencilState.hpp>
#include <Poseidon/Graphics/Core/GLPipelineState.hpp>
#include <Poseidon/Core/Global.hpp>
#include <Poseidon/World/Scene/Scene.hpp>
#include <Poseidon/World/Scene/Camera/Camera.hpp>
#include <Poseidon/World/Terrain/Landscape.hpp>

#include <algorithm>
#include <cmath>
#include <string>

namespace
{

// Post constants shared with the lab (tests/rendering/lighting_lab/postfx.py).
constexpr float kAoRadius = 1.2f;      // metres
constexpr float kAoFadeStart = 120.0f; // metres: AO fades out with distance
constexpr float kAoFadeEnd = 250.0f;
constexpr float kBloomThreshold = 0.82f; // linear peak where bloom starts
constexpr float kBloomKnee = 0.12f;

GLuint CompilePostShader(GLenum type, const char* src, const char* name)
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
        LOG_ERROR(Graphics, "GL33 post: shader compile error [{}]: {}", name, log);
        glDeleteShader(s);
        return 0;
    }
    return s;
}

GLuint LinkPostProgram(GLuint vs, const char* fsSrc, const char* name)
{
    GLuint fs = CompilePostShader(GL_FRAGMENT_SHADER, fsSrc, name);
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
        LOG_ERROR(Graphics, "GL33 post: program link error [{}]: {}", name, log);
        glDeleteProgram(p);
        return 0;
    }
    return p;
}

GLuint MakeTexture(GLenum internalFormat, GLenum format, GLenum type, int w, int h, bool linear)
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

GLuint MakeFbo(GLenum attachment, GLuint tex)
{
    GLuint f = 0;
    glGenFramebuffers(1, &f);
    glBindFramebuffer(GL_FRAMEBUFFER, f);
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, tex, 0);
    return f;
}

} // namespace

void EngineGL33::DestroyPostTargets()
{
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
    PostTargets& p = _post;
    delFbo(p.colorFbo);
    delFbo(p.depthFbo);
    delFbo(p.aoFbo[0]);
    delFbo(p.aoFbo[1]);
    delFbo(p.shaftFbo);
    for (int i = 0; i < PostTargets::kBloomLevels; ++i)
    {
        delFbo(p.bloomFbo[i]);
        delTex(p.bloomTex[i]);
    }
    delTex(p.colorTex);
    delTex(p.depthTex);
    delTex(p.aoTex[0]);
    delTex(p.aoTex[1]);
    delTex(p.shaftTex);
    p.w = p.h = 0;
}

void EngineGL33::DestroyPostPrograms()
{
    PostTargets& p = _post;
    for (unsigned int* prog :
         {&p.progAo, &p.progAoBlur, &p.progBloomDown, &p.progBloomUp, &p.progShafts, &p.progComposite})
    {
        if (*prog)
            glDeleteProgram(*prog);
        *prog = 0;
    }
    if (p.vao)
    {
        GL33Bind::OnVaoDeleted(p.vao);
        glDeleteVertexArrays(1, &p.vao);
    }
    p.vao = 0;
    p.programsTried = false;
}

bool EngineGL33::EnsurePostResources(int w, int h)
{
    PostTargets& p = _post;
    if (!p.programsTried)
    {
        p.programsTried = true;
        using Poseidon::gl33lighting::PostShader;
        using Poseidon::gl33lighting::PostShaderName;
        using Poseidon::gl33lighting::PostShaderSource;
        GLuint vs = CompilePostShader(GL_VERTEX_SHADER, PostShaderSource(PostShader::Vertex),
                                      PostShaderName(PostShader::Vertex));
        auto link = [vs](PostShader s) { return LinkPostProgram(vs, PostShaderSource(s), PostShaderName(s)); };
        p.progAo = link(PostShader::Ao);
        p.progAoBlur = link(PostShader::AoBlur);
        p.progBloomDown = link(PostShader::BloomDown);
        p.progBloomUp = link(PostShader::BloomUp);
        p.progShafts = link(PostShader::Shafts);
        p.progComposite = link(PostShader::Composite);
        if (vs)
            glDeleteShader(vs);
        glGenVertexArrays(1, &p.vao);
        if (!p.progAo || !p.progAoBlur || !p.progBloomDown || !p.progBloomUp || !p.progShafts || !p.progComposite)
        {
            LOG_ERROR(Graphics, "GL33 post: programs failed to build; remastered post effects disabled");
            p.unavailable = true;
        }
    }
    if (p.unavailable)
        return false;
    if (p.w == w && p.h == h && p.colorTex)
        return true;

    DestroyPostTargets();
    const int hw = (w + 1) / 2;
    const int hh = (h + 1) / 2;
    GL33Bind::ActiveUnit(kUploadUnit - GL_TEXTURE0);
    p.colorTex = MakeTexture(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, w, h, true);
    p.depthTex = MakeTexture(GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, w, h, false);
    p.aoTex[0] = MakeTexture(GL_RG16F, GL_RG, GL_HALF_FLOAT, hw, hh, true);
    p.aoTex[1] = MakeTexture(GL_RG16F, GL_RG, GL_HALF_FLOAT, hw, hh, true);
    p.shaftTex = MakeTexture(GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, hw, hh, true);
    int bw = hw, bh = hh;
    for (int i = 0; i < PostTargets::kBloomLevels; ++i)
    {
        p.bloomW[i] = bw = bw > 1 ? bw : 1;
        p.bloomH[i] = bh = bh > 1 ? bh : 1;
        p.bloomTex[i] = MakeTexture(GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, bw, bh, true);
        bw = (bw + 1) / 2;
        bh = (bh + 1) / 2;
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    GL33Bind::Invalidate();

    bool complete = true;
    auto check = [&complete]()
    {
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            complete = false;
    };
    p.colorFbo = MakeFbo(GL_COLOR_ATTACHMENT0, p.colorTex);
    check();
    p.depthFbo = MakeFbo(GL_DEPTH_STENCIL_ATTACHMENT, p.depthTex);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    check();
    for (int i = 0; i < 2; ++i)
    {
        p.aoFbo[i] = MakeFbo(GL_COLOR_ATTACHMENT0, p.aoTex[i]);
        check();
    }
    p.shaftFbo = MakeFbo(GL_COLOR_ATTACHMENT0, p.shaftTex);
    check();
    for (int i = 0; i < PostTargets::kBloomLevels; ++i)
    {
        p.bloomFbo[i] = MakeFbo(GL_COLOR_ATTACHMENT0, p.bloomTex[i]);
        check();
    }
    BindFrameRenderTarget();
    if (!complete)
    {
        LOG_WARN(Graphics, "GL33 post: targets {}x{} incomplete; remastered post effects disabled", w, h);
        DestroyPostTargets();
        p.unavailable = true;
        return false;
    }
    p.w = w;
    p.h = h;
    LOG_INFO(Graphics, "GL33 post: targets {}x{} (AO/shafts {}x{})", w, h, hw, hh);
    return true;
}

void EngineGL33::EndWorldScene()
{
    // Only the frame that actually drew the world through the remastered
    // shaders has a sky light share in its alpha and a world depth buffer.
    const bool worldDrawn = _worldPassOpen;
    // Queued world draws commit with the remastered programs before the HUD
    // and dialogs switch back to the classic set (and before the world's GPU
    // timestamp, so the world section holds all of them).
    if (_remasterPrograms || (_gpuTiming && worldDrawn))
        FlushAndFreeAllQueues(_queueNo);
    if (worldDrawn)
        GpuMark(Poseidon::gpu::Mark::WorldEnd);
    _worldPassOpen = false;
    _worldEffectsOn = false;
    _worldSceneEnded = true;
    UpdateRemasterPrograms();
    if (_lightingActive)
        UpdateShadowMapLitState(); // disarms the shadow test for the HUD and dialogs
    if (!worldDrawn || !_glContext || !_lightingActive || !_lighting.postEffects || !SSAAActive() ||
        _portraitTargetSize)
        return;
    if (_lighting.ambientOcclusion <= 0.0f && _lighting.bloom <= 0.0f && _lighting.sunShafts <= 0.0f)
        return; // nothing to add: leave the frame as drawn

    FlushAndFreeAllQueues(_queueNo);
    int w = 0, h = 0;
    RenderTargetSize(w, h);
    if (w <= 0 || h <= 0 || !EnsurePostResources(w, h))
        return;
    PostTargets& p = _post;

    // Resolve the multisampled frame target: colour (with the ambient share in
    // alpha) and depth.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, _ssaaFbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, p.colorFbo);
    glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, p.depthFbo);
    glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);

    // Common state for the fullscreen passes.
    Poseidon::render::depthstencil::Disabled(/*hasStencil*/ false);
    glDisable(GL_STENCIL_TEST);
    Poseidon::render::blend::Opaque();
    glDisable(GL_CULL_FACE);
    glDisable(GL_SCISSOR_TEST);
    Poseidon::render::pipeline::DisableDepthClamp();
    glBindVertexArray(p.vao);
    for (int unit : {0, 2, 3, 4, 5})
        glBindSampler(static_cast<GLuint>(unit), 0);

    auto bindTex = [](int unit, GLuint tex)
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, tex);
    };
    auto drawTo = [](GLuint fbo, int vw, int vh)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glViewport(0, 0, vw, vh);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    };
    auto u1i = [](GLuint prog, const char* name, int v)
    {
        const GLint loc = glGetUniformLocation(prog, name);
        if (loc >= 0)
            glUniform1i(loc, v);
    };
    auto u4f = [](GLuint prog, const char* name, float a, float b, float c, float d)
    {
        const GLint loc = glGetUniformLocation(prog, name);
        if (loc >= 0)
            glUniform4f(loc, a, b, c, d);
    };
    auto u2f = [](GLuint prog, const char* name, float a, float b)
    {
        const GLint loc = glGetUniformLocation(prog, name);
        if (loc >= 0)
            glUniform2f(loc, a, b);
    };

    const int hw = (w + 1) / 2;
    const int hh = (h + 1) / 2;
    // A vehicle interior clears the world's depth and draws itself with its own
    // near plane (Transport::Draw): no world depth to reconstruct from, so no
    // occlusion and no depth-tested shafts this frame. Bloom does not need it.
    const bool worldDepth = !_worldDepthCleared;
    const float aoStrength = worldDepth ? _lighting.ambientOcclusion : 0.0f;
    // A screen overlay drawn inside the world pass (gunner optics mask) is in
    // the frame already: glow would grey its black surround.
    const bool glow = !_worldOverlayDrawn;
    const float bloomStrength = glow ? _lighting.bloom : 0.0f;

    // 1. Ambient occlusion + blur.
    const GfxMatrix& proj = _frameState.projection;
    // The 3D scene may fill only a sub-rectangle (aspect pillarbox); uv <-> NDC
    // goes through that rectangle.
    const Poseidon::AspectSettings& as = _aspectSettings;
    const bool fullRect =
        as.worldLeft <= 0.0f && as.worldTop <= 0.0f && as.worldRight >= 1.0f && as.worldBottom >= 1.0f;
    const float rectX = fullRect ? 0.0f : as.worldLeft;
    const float rectY = fullRect ? 0.0f : 1.0f - as.worldBottom;
    const float rectW = fullRect ? 1.0f : as.worldRight - as.worldLeft;
    const float rectH = fullRect ? 1.0f : as.worldBottom - as.worldTop;
    const float depthScale = GLAD_GL_ARB_clip_control ? 1.0f : 2.0f;
    const float depthBias = GLAD_GL_ARB_clip_control ? 0.0f : -1.0f;
    if (aoStrength > 0.0f)
    {
        glUseProgram(p.progAo);
        u1i(p.progAo, "depthTex", 0);
        u4f(p.progAo, "projInfo", proj._33, proj._43, 1.0f / proj._11, 1.0f / proj._22);
        u4f(p.progAo, "depthMap", depthScale, depthBias, 0.0f, 0.0f);
        u4f(p.progAo, "aoParams", kAoRadius, 1.0f, kAoFadeStart, kAoFadeEnd);
        u4f(p.progAo, "screen", static_cast<float>(w), static_cast<float>(h), 1.0f / w, 1.0f / h);
        u4f(p.progAo, "worldRect", rectX, rectY, rectW, rectH);
        bindTex(0, p.depthTex);
        drawTo(p.aoFbo[0], hw, hh);
        glUseProgram(p.progAoBlur);
        u1i(p.progAoBlur, "aoTex", 0);
        bindTex(0, p.aoTex[0]);
        u2f(p.progAoBlur, "stepUV", 1.0f / hw, 0.0f);
        drawTo(p.aoFbo[1], hw, hh);
        bindTex(0, p.aoTex[1]);
        u2f(p.progAoBlur, "stepUV", 0.0f, 1.0f / hh);
        drawTo(p.aoFbo[0], hw, hh);
        GpuMark(Poseidon::gpu::Mark::AoEnd);
    }

    // 2. Bloom chain.
    if (bloomStrength > 0.0f)
    {
        glUseProgram(p.progBloomDown);
        u1i(p.progBloomDown, "srcTex", 0);
        for (int i = 0; i < PostTargets::kBloomLevels; ++i)
        {
            const bool first = i == 0;
            const GLuint src = first ? p.colorTex : p.bloomTex[i - 1];
            const int sw = first ? w : p.bloomW[i - 1];
            const int sh = first ? h : p.bloomH[i - 1];
            bindTex(0, src);
            u2f(p.progBloomDown, "srcTexel", 1.0f / sw, 1.0f / sh);
            u4f(p.progBloomDown, "bloomParams", kBloomThreshold, kBloomKnee, first ? 1.0f : 0.0f, 0.0f);
            drawTo(p.bloomFbo[i], p.bloomW[i], p.bloomH[i]);
        }
        glUseProgram(p.progBloomUp);
        u1i(p.progBloomUp, "srcTex", 0);
        Poseidon::render::blend::Additive(); // (SRC_ALPHA, ONE) with alpha 1 = add
        for (int i = PostTargets::kBloomLevels - 1; i > 0; --i)
        {
            bindTex(0, p.bloomTex[i]);
            u2f(p.progBloomUp, "srcTexel", 1.0f / p.bloomW[i], 1.0f / p.bloomH[i]);
            drawTo(p.bloomFbo[i - 1], p.bloomW[i - 1], p.bloomH[i - 1]);
        }
        Poseidon::render::blend::Opaque();
        GpuMark(Poseidon::gpu::Mark::BloomEnd);
    }

    // 3. Sun shafts, when the sun is in front of the camera and above the horizon.
    float shaftWeight = 0.0f;
    float sunU = 0.5f, sunV = 0.5f;
    Color shaftTint(1, 1, 1, 1);
    if (worldDepth && glow && _lighting.sunShafts > 0.0f && GScene)
    {
        LightSun* sun = GScene->MainLight();
        Camera* cam = GScene->GetCamera();
        const Vector3 toSun = -sun->SunDirection();
        // Camera space: InverseScaled maps world directions into view space.
        const Vector3 v = cam->InverseScaled().Rotate(toSun);
        if (v.Z() > 0.05f && toSun.Y() > -0.02f)
        {
            const float ndcX = proj._11 * v.X() / v.Z();
            const float ndcY = proj._22 * v.Y() / v.Z();
            sunU = rectX + (ndcX * 0.5f + 0.5f) * rectW;
            sunV = rectY + (ndcY * 0.5f + 0.5f) * rectH;
            const float offX = std::fabs(ndcX) > 1.0f ? std::fabs(ndcX) - 1.0f : 0.0f;
            const float offY = std::fabs(ndcY) > 1.0f ? std::fabs(ndcY) - 1.0f : 0.0f;
            const float onScreen = std::exp(-3.0f * (offX + offY));
            const float lowSun = 0.55f + 0.45f * (1.0f - std::min(1.0f, toSun.Y() / 0.6f));
            Landscape* land = GScene->GetLandscape();
            const float clear = land ? land->SkyThrough() : 1.0f;
            shaftWeight = onScreen * lowSun * clear * (1.0f - sun->NightEffect());
            shaftTint = _lightRigSun;
        }
    }
    if (shaftWeight > 0.001f)
    {
        glUseProgram(p.progShafts);
        u1i(p.progShafts, "depthTex", 0);
        u1i(p.progShafts, "colorTex", 2);
        bindTex(0, p.depthTex);
        bindTex(2, p.colorTex);
        u4f(p.progShafts, "sunScreen", sunU, sunV, shaftWeight, static_cast<float>(w) / static_cast<float>(h));
        drawTo(p.shaftFbo, hw, hh);
        GpuMark(Poseidon::gpu::Mark::ShaftsEnd);
    }

    // 4. Composite into the frame target (every sample gets the resolved value).
    glUseProgram(p.progComposite);
    u1i(p.progComposite, "colorTex", 0);
    u1i(p.progComposite, "aoTex", 2);
    u1i(p.progComposite, "bloomTex", 3);
    u1i(p.progComposite, "shaftTex", 4);
    bindTex(0, p.colorTex);
    bindTex(2, aoStrength > 0.0f ? p.aoTex[0] : p.whiteTex);
    bindTex(3, bloomStrength > 0.0f ? p.bloomTex[0] : p.blackTex);
    bindTex(4, shaftWeight > 0.001f ? p.shaftTex : p.blackTex);
    u4f(p.progComposite, "compParams", aoStrength, bloomStrength * 0.25f, _lighting.sunShafts * 0.6f, 1.0f);
    const float lum = std::max(1e-4f, 0.2126f * shaftTint.R() + 0.7152f * shaftTint.G() + 0.0722f * shaftTint.B());
    u4f(p.progComposite, "shaftColor", shaftTint.R() / lum, shaftTint.G() / lum, shaftTint.B() / lum, 1.0f);
    // The shafts keep only what the air in front of each pixel scatters; the
    // depth is read only where there is a shaft term.
    u1i(p.progComposite, "depthTex", 5);
    bindTex(5, p.depthTex);
    u4f(p.progComposite, "projInfo", proj._33, proj._43, 1.0f / proj._11, 1.0f / proj._22);
    const float scatterLength = Poseidon::remaster::ShaftScatterLength(GScene ? GScene->GetFogMaxRange() : 0.0f);
    u4f(p.progComposite, "depthMap", depthScale, depthBias, 1.0f / scatterLength, 0.0f);
    u4f(p.progComposite, "worldRect", rectX, rectY, rectW, rectH);
    u4f(p.progComposite, "debugView", _lighting.debugView == 4 ? 1.0f : 0.0f, 0.0f, 0.0f, 0.0f);
    drawTo(_ssaaFbo, w, h);
    GpuMark(Poseidon::gpu::Mark::CompositeEnd);

    // Hand the GL state back: frame target bound with its full viewport, unit 0
    // sampler restored, caches invalidated so the next draw re-applies state.
    for (int unit : {0, 2, 3, 4, 5})
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glActiveTexture(GL_TEXTURE0);
    glBindSampler(0, _samplerObjects[0]);
    glBindVertexArray(0);
    glUseProgram(0);
    BindFrameRenderTarget();
    _pixelShaderSel = PSNone;
    GL33Bind::Invalidate();
    InvalidatePipelineCache();
}
