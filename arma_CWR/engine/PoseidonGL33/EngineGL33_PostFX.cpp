// Remastered lighting: post pass at the world/HUD boundary.
//
// World::Draw calls Engine::EndWorldScene() after the 3D scene and before the
// map, HUD and dialogs. When the remastered lighting and its post effects are
// on, it runs a fixed list of stages (Poseidon::remaster::PostStage,
// Graphics/Remaster/PostStages.hpp) on one PostFrame. PlanPostStages decides
// from the knobs and the frame which stages run; each stage that runs ends
// with its own GPU mark. The stages with a pass today, in order:
//   - Resolve: the multisampled frame target (colour with the ambient share
//     in alpha, depth and stencil) into single-sample textures;
//   - LinearDepth: half-resolution view depth in metres, for the stages that
//     trace or upsample at half resolution. Only the linear depth debug view
//     (debug=5) reads it so far, so it runs only then;
//   - Ao: ambient occlusion (Alchemy/SAO style) at half resolution from the
//     depth buffer, blurred with a depth-aware separable filter;
//   - Air (EngineGL33_PostAir.cpp, B8): sun beams, headlight cones, the aerial
//     haze and the valley mist, marched at half resolution on the linear depth
//     and applied at full resolution into airOutTex, which the bloom and the
//     composite then read in place of the resolved colour. Its beams replace
//     the shafts;
//   - Bloom: a five-level bloom chain from the bright parts of the image;
//   - Shafts: a radial blur of the visible sky toward the sun;
//   - Composite: back into the frame target. Occlusion darkens only the sky
//     light: the remastered shaders store one minus each opaque pixel's sky
//     light share in the colour alpha (blended draws keep the destination
//     alpha), so a sunlit wall next to a corner keeps its sunlight and pre-lit
//     or alpha-to-coverage pixels (alpha 1) are left alone. Bloom and shafts
//     add in linear light with a soft screen-style add that can never clip,
//     and leave the frame exactly as it was where they are zero (the frame is
//     already tonemapped: no second curve). The shafts are light scattered
//     by the air in front of each pixel, so they grow with its distance
//     (LightRig ShaftInscatter): a nearby vehicle is not veiled. A dither
//     hides banding;
//   - Temporal and Sharpen (EngineGL33_PostTemporal.cpp): the TAA resolve of
//     the jittered frame with its history, then a bounded sharpen back into
//     the frame target; with TAA off, Sharpen alone is the luma sharpen
//     (contrast-adaptive, brightness only, B11). When Sharpen runs the
//     composite draws into the TAA input instead of the frame target;
//   - DebugView: a diagnostic view of a post buffer over the frame.
// The other stages of the list (tag readout, reflections, combat scatter) land
// with their features. The HUD then draws on top, untouched.
//
// The shaders live in EngineGL33_PostShaders.cpp, which has no GL code, so the
// unit tests compile the sources the engine builds (PostShaderSource in
// EngineGL33_Lighting.hpp).
//
// All GL state the stages touch is restored or the bind caches invalidated;
// the stages bind their inputs to kPostUnits (EngineGL33_Lighting.hpp), never
// to texture unit 1 (its sampler object belongs to ApplyPipeline).

#include <PoseidonGL33/EngineGL33.hpp>
#include <PoseidonGL33/EngineGL33_Lighting.hpp>
#include <PoseidonGL33/EngineGL33_PostPass.hpp>
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

namespace rm = Poseidon::remaster;
// The helpers of the fullscreen stages (EngineGL33_PostPass.hpp).
using namespace Poseidon::gl33post;

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
    delFbo(p.linDepthFbo);
    delFbo(p.taaInFbo);
    delFbo(p.histFbo[0]);
    delFbo(p.histFbo[1]);
    delFbo(p.opaqueFbo);
    delFbo(p.airMarchFbo);
    delFbo(p.airFbo[0]);
    delFbo(p.airFbo[1]);
    delFbo(p.airMeanFbo);
    delFbo(p.airOutFbo);
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
    delTex(p.linDepthTex);
    delTex(p.taaInTex);
    delTex(p.histTex[0]);
    delTex(p.histTex[1]);
    delTex(p.opaqueTex);
    delTex(p.airTex[0]);
    delTex(p.airTex[1]);
    delTex(p.airMistTex);
    delTex(p.airMeanTex);
    delTex(p.airOutTex);
    p.w = p.h = 0;
    _taa.history.exists = false; // the TAA history went with its targets
}

void EngineGL33::DestroyPostPrograms()
{
    PostTargets& p = _post;
    for (unsigned int* prog : {&p.progAo, &p.progAoBlur, &p.progBloomDown, &p.progBloomUp, &p.progShafts,
                               &p.progComposite, &p.progLinearDepth, &p.progDebugDepth, &p.progTaa, &p.progTaaSharpen,
                               &p.progLumaSharpen, &p.progAirMarch, &p.progAirBlur, &p.progAirMean, &p.progAirApply})
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
    p.airUnavailable = false;
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
        p.progLinearDepth = link(PostShader::LinearDepth);
        p.progDebugDepth = link(PostShader::DebugDepth);
        p.progTaa = link(PostShader::Taa);
        p.progTaaSharpen = link(PostShader::TaaSharpen);
        p.progLumaSharpen = link(PostShader::LumaSharpen);
        // The air pass is optional: if its programs fail, it never runs and
        // the rest of the post pass does.
        p.progAirMarch = link(PostShader::AirMarch);
        p.progAirBlur = link(PostShader::AirBlur);
        p.progAirMean = link(PostShader::AirMean);
        p.progAirApply = link(PostShader::AirApply);
        if (vs)
            glDeleteShader(vs);
        // The march and the apply read the rig (LitConstants, binding 3).
        for (GLuint prog : {p.progAirMarch, p.progAirApply})
        {
            const GLuint block = prog ? glGetUniformBlockIndex(prog, "LitConstants") : GL_INVALID_INDEX;
            if (block != GL_INVALID_INDEX)
                glUniformBlockBinding(prog, block, Poseidon::gl33lighting::kLitBinding);
        }
        if (!p.progAirMarch || !p.progAirBlur || !p.progAirMean || !p.progAirApply)
        {
            LOG_ERROR(Graphics, "GL33 post: air pass programs failed to build; the air pass is off");
            p.airUnavailable = true;
        }
        glGenVertexArrays(1, &p.vao);
        if (!p.progAo || !p.progAoBlur || !p.progBloomDown || !p.progBloomUp || !p.progShafts || !p.progComposite ||
            !p.progLinearDepth || !p.progDebugDepth || !p.progTaa || !p.progTaaSharpen || !p.progLumaSharpen)
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
    p.linDepthTex = MakeTexture(GL_R32F, GL_RED, GL_FLOAT, hw, hh, false);
    // TAA: the composite's output and the opaque world copy are read texel by
    // texel; the history is read through bilinear taps (Catmull-Rom).
    p.taaInTex = MakeTexture(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, w, h, false);
    p.histTex[0] = MakeTexture(GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, w, h, true);
    p.histTex[1] = MakeTexture(GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, w, h, true);
    p.opaqueTex = MakeTexture(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, w, h, false);
    // The air pass: the march's two targets and the blur's ping-pong (filtered:
    // the blur and the mean tap between texels, the second target holds the
    // view depth they match), the mean, and the frame with the air applied
    // (filtered as colorTex is, for the bloom's taps).
    p.airTex[0] = MakeTexture(GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, hw, hh, true);
    p.airTex[1] = MakeTexture(GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, hw, hh, true);
    p.airMistTex = MakeTexture(GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, hw, hh, true);
    p.airMeanTex = MakeTexture(GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, hw, hh, false);
    p.airOutTex = MakeTexture(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, w, h, true);
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
    p.linDepthFbo = MakeFbo(GL_COLOR_ATTACHMENT0, p.linDepthTex);
    check();
    p.taaInFbo = MakeFbo(GL_COLOR_ATTACHMENT0, p.taaInTex);
    check();
    for (int i = 0; i < 2; ++i)
    {
        p.histFbo[i] = MakeFbo(GL_COLOR_ATTACHMENT0, p.histTex[i]);
        check();
    }
    p.opaqueFbo = MakeFbo(GL_COLOR_ATTACHMENT0, p.opaqueTex);
    check();
    p.airMarchFbo = MakeFbo(GL_COLOR_ATTACHMENT0, p.airTex[0]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, p.airMistTex, 0);
    const GLenum airBuffers[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, airBuffers);
    check();
    for (int i = 0; i < 2; ++i)
    {
        p.airFbo[i] = MakeFbo(GL_COLOR_ATTACHMENT0, p.airTex[i]);
        check();
    }
    p.airMeanFbo = MakeFbo(GL_COLOR_ATTACHMENT0, p.airMeanTex);
    check();
    p.airOutFbo = MakeFbo(GL_COLOR_ATTACHMENT0, p.airOutTex);
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
    LOG_INFO(Graphics, "GL33 post: targets {}x{} (AO, shafts, air and linear depth {}x{}; TAA history half float)", w,
             h, hw, hh);
    return true;
}

void EngineGL33::PostFrameGeometry(PostFrame& f) const
{
    const GfxMatrix& proj = _frameState.projection;
    f.projInfo[0] = proj._33;
    f.projInfo[1] = proj._43;
    f.projInfo[2] = 1.0f / proj._11;
    f.projInfo[3] = 1.0f / proj._22;
    // The 3D scene may fill only a sub-rectangle (aspect pillarbox); uv <-> NDC
    // goes through that rectangle.
    const Poseidon::AspectSettings& as = _aspectSettings;
    const bool fullRect =
        as.worldLeft <= 0.0f && as.worldTop <= 0.0f && as.worldRight >= 1.0f && as.worldBottom >= 1.0f;
    f.rect[0] = fullRect ? 0.0f : as.worldLeft;
    f.rect[1] = fullRect ? 0.0f : 1.0f - as.worldBottom;
    f.rect[2] = fullRect ? 1.0f : as.worldRight - as.worldLeft;
    f.rect[3] = fullRect ? 1.0f : as.worldBottom - as.worldTop;
    f.depthScale = GLAD_GL_ARB_clip_control ? 1.0f : 2.0f;
    f.depthBias = GLAD_GL_ARB_clip_control ? 0.0f : -1.0f;
}

void EngineGL33::PostFrameSun(PostFrame& f) const
{
    // The sun's place and weight on screen for the shafts, when it is in front
    // of the camera and above the horizon.
    LightSun* sun = GScene->MainLight();
    Camera* cam = GScene->GetCamera();
    const Vector3 toSun = -sun->SunDirection();
    // Camera space: InverseScaled maps world directions into view space.
    const Vector3 v = cam->InverseScaled().Rotate(toSun);
    if (v.Z() > 0.05f && toSun.Y() > -0.02f)
    {
        const GfxMatrix& proj = _frameState.projection;
        const float ndcX = proj._11 * v.X() / v.Z();
        const float ndcY = proj._22 * v.Y() / v.Z();
        f.sunU = f.rect[0] + (ndcX * 0.5f + 0.5f) * f.rect[2];
        f.sunV = f.rect[1] + (ndcY * 0.5f + 0.5f) * f.rect[3];
        const float offX = std::fabs(ndcX) > 1.0f ? std::fabs(ndcX) - 1.0f : 0.0f;
        const float offY = std::fabs(ndcY) > 1.0f ? std::fabs(ndcY) - 1.0f : 0.0f;
        const float onScreen = std::exp(-3.0f * (offX + offY));
        const float lowSun = 0.55f + 0.45f * (1.0f - std::min(1.0f, toSun.Y() / 0.6f));
        Landscape* land = GScene->GetLandscape();
        const float clear = land ? land->SkyThrough() : 1.0f;
        f.shaftWeight = onScreen * lowSun * clear * (1.0f - sun->NightEffect());
        f.shaftTint = _lightRigSun;
    }
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
    // Passes after the world (portraits, 3D previews) light each draw from its
    // own list.
    _lightTilesActive = false;
    UpdateRemasterPrograms();
    if (_lightingActive)
        UpdateShadowMapLitState(); // disarms the shadow test for the HUD and dialogs
    // The HUD, dialogs and everything else after the world draw without the
    // TAA jitter and texture LOD bias (EngineGL33_PostTemporal.cpp).
    const bool taaFrame = _taa.frameOn && _taa.frame == GetFrameCounter();
    _taa.frameOn = false;
    SetTaaJitterState({}, 0.0f);
    if (!worldDrawn || !_glContext || !_lightingActive || !_lighting.postEffects || !SSAAActive() ||
        _portraitTargetSize)
        return;

    // This frame's stages. A vehicle interior clears the world's depth and
    // draws itself with its own near plane (Transport::Draw): no world depth to
    // reconstruct from, so no occlusion and no depth-tested shafts this frame.
    // A screen overlay drawn inside the world pass (gunner optics mask) is in
    // the frame already: glow would grey its black surround.
    PostFrame f;
    PostFrameGeometry(f);
    rm::PostFrameInputs in;
    in.ambientOcclusion = _lighting.ambientOcclusion;
    in.bloom = _lighting.bloom;
    in.sunShafts = _lighting.sunShafts;
    in.debugView = _lighting.debugView;
    in.worldDepth = !_worldDepthCleared;
    in.glow = !_worldOverlayDrawn;
    if (in.worldDepth && in.glow && in.sunShafts > 0.0f && GScene)
        PostFrameSun(f);
    in.shaftWeight = f.shaftWeight;
    // The world's passes were drawn with the TAA jitter (TaaWantedThisFrame).
    in.temporal = taaFrame;
    // With TAA off this frame, the luma sharpen when its switch and gate are on.
    in.lumaSharpen = !taaFrame && _featureGates.computed && _featureGates.frame == GetFrameCounter() &&
                     _featureGates[rm::Feature::Sharpen].on && _lighting.sharpenStrength > 0.0f;
    // The air pass, when one of its parts is on this frame (UpdateAirMedium,
    // with the gates).
    const bool airFrame = _airFrame == GetFrameCounter() && !_post.airUnavailable;
    in.air = airFrame && (_airParts.haze || _airParts.valleyMist || _airParts.beams);
    in.airBeams = airFrame && _airParts.beams;
    f.plan = rm::PlanPostStages(in);
    if (f.plan.Empty())
        return; // nothing to add: leave the frame as drawn
    // Under TAA the AO and shaft patterns turn every frame (golden-ratio
    // steps) and the history averages them; without it they stay fixed.
    if (f.plan.Runs(rm::PostStage::Temporal))
        f.noiseShift = static_cast<float>(std::fmod(0.6180339887 * static_cast<double>(_taa.sequence), 1.0));

    FlushAndFreeAllQueues(_queueNo);
    RenderTargetSize(f.w, f.h);
    if (f.w <= 0 || f.h <= 0 || !EnsurePostResources(f.w, f.h))
        return;
    f.hw = (f.w + 1) / 2;
    f.hh = (f.h + 1) / 2;

    // The stages in list order, each followed by its GPU mark. The resolve
    // blits under the state the world left, as it always has; the fullscreen
    // stages after it share one state.
    for (int i = 0; i < rm::kPostStageCount; ++i)
    {
        const auto stage = static_cast<rm::PostStage>(i);
        if (!f.plan.Runs(stage))
            continue;
        RunPostStage(stage, f);
        GpuMark(rm::GetPostStageInfo(stage).endMark);
        if (stage == rm::PostStage::Resolve)
            BeginPostFullscreen();
    }
    GpuMark(Poseidon::gpu::Mark::PostEnd);
    EndPostFullscreen();
}

void EngineGL33::RunPostStage(rm::PostStage stage, const PostFrame& f)
{
    switch (stage)
    {
        case rm::PostStage::Resolve:
            PostResolve(f);
            break;
        case rm::PostStage::LinearDepth:
            PostLinearDepth(f);
            break;
        case rm::PostStage::Ao:
            PostAo(f);
            break;
        case rm::PostStage::Air:
            PostAir(f);
            break;
        case rm::PostStage::Bloom:
            PostBloom(f);
            break;
        case rm::PostStage::Shafts:
            PostShafts(f);
            break;
        case rm::PostStage::Composite:
            PostComposite(f);
            break;
        case rm::PostStage::Temporal:
            PostTemporal(f);
            break;
        case rm::PostStage::Sharpen:
            PostSharpen(f);
            break;
        case rm::PostStage::DebugView:
            PostDebugView(f);
            break;
        default:
            break; // a stage that has not landed is never planned
    }
}

void EngineGL33::PostResolve(const PostFrame& f)
{
    // The multisampled frame target: colour (with the ambient share in alpha),
    // depth and stencil.
    const PostTargets& p = _post;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, _ssaaFbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, p.colorFbo);
    glBlitFramebuffer(0, 0, f.w, f.h, 0, 0, f.w, f.h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, p.depthFbo);
    glBlitFramebuffer(0, 0, f.w, f.h, 0, 0, f.w, f.h, GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
}

void EngineGL33::BeginPostFullscreen()
{
    Poseidon::render::depthstencil::Disabled(/*hasStencil*/ false);
    glDisable(GL_STENCIL_TEST);
    Poseidon::render::blend::Opaque();
    glDisable(GL_CULL_FACE);
    glDisable(GL_SCISSOR_TEST);
    Poseidon::render::pipeline::DisableDepthClamp();
    glBindVertexArray(_post.vao);
    for (int unit : Poseidon::gl33lighting::kPostUnits)
        glBindSampler(static_cast<GLuint>(unit), 0);
}

void EngineGL33::PostLinearDepth(const PostFrame& f)
{
    // Half-resolution view depth (psPostLinearDepth), for the stages that trace
    // or upsample at half resolution.
    const PostTargets& p = _post;
    glUseProgram(p.progLinearDepth);
    SetInt(p.progLinearDepth, "depthTex", 0);
    Set4(p.progLinearDepth, "projInfo", f.projInfo[0], f.projInfo[1], f.projInfo[2], f.projInfo[3]);
    Set4(p.progLinearDepth, "depthMap", f.depthScale, f.depthBias, 0.0f, 0.0f);
    Set4(p.progLinearDepth, "screen", static_cast<float>(f.w), static_cast<float>(f.h), 1.0f / f.w, 1.0f / f.h);
    Set4(p.progLinearDepth, "worldRect", f.rect[0], f.rect[1], f.rect[2], f.rect[3]);
    BindTex(0, p.depthTex);
    DrawTo(p.linDepthFbo, f.hw, f.hh);
}

void EngineGL33::PostAo(const PostFrame& f)
{
    // Ambient occlusion at half resolution, then the depth-aware blur across
    // and down.
    const PostTargets& p = _post;
    glUseProgram(p.progAo);
    SetInt(p.progAo, "depthTex", 0);
    Set4(p.progAo, "projInfo", f.projInfo[0], f.projInfo[1], f.projInfo[2], f.projInfo[3]);
    Set4(p.progAo, "depthMap", f.depthScale, f.depthBias, 0.0f, 0.0f);
    Set4(p.progAo, "aoParams", kAoRadius, 1.0f, kAoFadeStart, kAoFadeEnd);
    Set4(p.progAo, "screen", static_cast<float>(f.w), static_cast<float>(f.h), 1.0f / f.w, 1.0f / f.h);
    Set4(p.progAo, "worldRect", f.rect[0], f.rect[1], f.rect[2], f.rect[3]);
    SetFloat(p.progAo, "noiseShift", f.noiseShift);
    BindTex(0, p.depthTex);
    DrawTo(p.aoFbo[0], f.hw, f.hh);
    glUseProgram(p.progAoBlur);
    SetInt(p.progAoBlur, "aoTex", 0);
    BindTex(0, p.aoTex[0]);
    Set2(p.progAoBlur, "stepUV", 1.0f / f.hw, 0.0f);
    DrawTo(p.aoFbo[1], f.hw, f.hh);
    BindTex(0, p.aoTex[1]);
    Set2(p.progAoBlur, "stepUV", 0.0f, 1.0f / f.hh);
    DrawTo(p.aoFbo[0], f.hw, f.hh);
}

void EngineGL33::PostBloom(const PostFrame& f)
{
    const PostTargets& p = _post;
    glUseProgram(p.progBloomDown);
    SetInt(p.progBloomDown, "srcTex", 0);
    for (int i = 0; i < PostTargets::kBloomLevels; ++i)
    {
        const bool first = i == 0;
        // The frame with the air applied when the air pass ran.
        const GLuint frame = f.plan.Runs(rm::PostStage::Air) ? p.airOutTex : p.colorTex;
        const GLuint src = first ? frame : p.bloomTex[i - 1];
        const int sw = first ? f.w : p.bloomW[i - 1];
        const int sh = first ? f.h : p.bloomH[i - 1];
        BindTex(0, src);
        Set2(p.progBloomDown, "srcTexel", 1.0f / sw, 1.0f / sh);
        Set4(p.progBloomDown, "bloomParams", kBloomThreshold, kBloomKnee, first ? 1.0f : 0.0f, 0.0f);
        DrawTo(p.bloomFbo[i], p.bloomW[i], p.bloomH[i]);
    }
    glUseProgram(p.progBloomUp);
    SetInt(p.progBloomUp, "srcTex", 0);
    Poseidon::render::blend::Additive(); // (SRC_ALPHA, ONE) with alpha 1 = add
    for (int i = PostTargets::kBloomLevels - 1; i > 0; --i)
    {
        BindTex(0, p.bloomTex[i]);
        Set2(p.progBloomUp, "srcTexel", 1.0f / p.bloomW[i], 1.0f / p.bloomH[i]);
        DrawTo(p.bloomFbo[i - 1], p.bloomW[i - 1], p.bloomH[i - 1]);
    }
    Poseidon::render::blend::Opaque();
}

void EngineGL33::PostShafts(const PostFrame& f)
{
    const PostTargets& p = _post;
    glUseProgram(p.progShafts);
    SetInt(p.progShafts, "depthTex", 0);
    SetInt(p.progShafts, "colorTex", 2);
    BindTex(0, p.depthTex);
    BindTex(2, p.colorTex);
    Set4(p.progShafts, "sunScreen", f.sunU, f.sunV, f.shaftWeight, static_cast<float>(f.w) / static_cast<float>(f.h));
    SetFloat(p.progShafts, "noiseShift", f.noiseShift);
    DrawTo(p.shaftFbo, f.hw, f.hh);
}

void EngineGL33::PostComposite(const PostFrame& f)
{
    // Into the frame target (every sample gets the resolved value), or into
    // the TAA input when the Sharpen stage runs (after the resolve, or alone as
    // the luma sharpen), which then writes the frame target. A stage that did
    // not run this frame reads as its 1x1 stand-in.
    const PostTargets& p = _post;
    const rm::PostStagePlan& plan = f.plan;
    glUseProgram(p.progComposite);
    SetInt(p.progComposite, "colorTex", 0);
    SetInt(p.progComposite, "aoTex", 2);
    SetInt(p.progComposite, "bloomTex", 3);
    SetInt(p.progComposite, "shaftTex", 4);
    BindTex(0, plan.Runs(rm::PostStage::Air) ? p.airOutTex : p.colorTex);
    BindTex(2, plan.Runs(rm::PostStage::Ao) ? p.aoTex[0] : p.whiteTex);
    BindTex(3, plan.Runs(rm::PostStage::Bloom) ? p.bloomTex[0] : p.blackTex);
    BindTex(4, plan.Runs(rm::PostStage::Shafts) ? p.shaftTex : p.blackTex);
    Set4(p.progComposite, "compParams", plan.aoStrength, plan.bloomStrength * 0.25f, _lighting.sunShafts * 0.6f, 1.0f);
    const Color& tint = f.shaftTint;
    const float lum = std::max(1e-4f, 0.2126f * tint.R() + 0.7152f * tint.G() + 0.0722f * tint.B());
    Set4(p.progComposite, "shaftColor", tint.R() / lum, tint.G() / lum, tint.B() / lum, 1.0f);
    // The shafts keep only what the air in front of each pixel scatters; the
    // depth is read only where there is a shaft term.
    SetInt(p.progComposite, "depthTex", 5);
    BindTex(5, p.depthTex);
    Set4(p.progComposite, "projInfo", f.projInfo[0], f.projInfo[1], f.projInfo[2], f.projInfo[3]);
    const float scatterLength = Poseidon::remaster::ShaftScatterLength(GScene ? GScene->GetFogMaxRange() : 0.0f);
    Set4(p.progComposite, "depthMap", f.depthScale, f.depthBias, 1.0f / scatterLength, 0.0f);
    Set4(p.progComposite, "worldRect", f.rect[0], f.rect[1], f.rect[2], f.rect[3]);
    Set4(p.progComposite, "debugView", _lighting.debugView == 4 ? 1.0f : 0.0f, 0.0f, 0.0f, 0.0f);
    DrawTo(plan.Runs(rm::PostStage::Sharpen) ? p.taaInFbo : _ssaaFbo, f.w, f.h);
}

void EngineGL33::PostDebugView(const PostFrame& f)
{
    // Debug view 5: the linear depth over the whole frame target.
    const PostTargets& p = _post;
    glUseProgram(p.progDebugDepth);
    SetInt(p.progDebugDepth, "linDepthTex", 0);
    BindTex(0, p.linDepthTex);
    DrawTo(_ssaaFbo, f.w, f.h);
}

void EngineGL33::EndPostFullscreen()
{
    // Hand the GL state back: frame target bound with its full viewport, unit 0
    // sampler restored, caches invalidated so the next draw re-applies state.
    for (int unit : Poseidon::gl33lighting::kPostUnits)
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
