// Remastered lighting: temporal anti-aliasing (TAA; the math is
// Graphics/Remaster/TemporalAA.hpp, the shaders psPostTaa and psPostTaaSharpen
// in EngineGL33_PostShaders.cpp).
//
// A frame runs TAA when the taa switch and its frame gate are on and the frame
// gets a post pass (TaaWantedThisFrame). Then:
//   - the world's passes are drawn with a sub-pixel jitter: ApplyTaaJitter
//     fixes the frame's offset (Halton(2, 3), 16 frames) and camera at its
//     first world pass and hands the offset to the remastered vertex shaders
//     through vpScale.zw (UploadVSScreenConstants keeps it for the world's
//     screen-space draws), with a negative texture LOD bias on the material
//     samplers (the twin sampler set, which InitDraw biases only when the
//     taaMipBias knob changes); EndWorldScene takes both away before the HUD,
//     and so does the next frame's InitDraw;
//   - BeginTranslucentObjects, called by the scene between the opaque world
//     and the translucent objects, copies the frame target for the particle
//     mask;
//   - the post pass runs the Temporal stage (the resolve into the history)
//     and the Sharpen stage (the history back into the frame target).
// With TAA off the Sharpen stage can run alone: the luma sharpen
// (the sharpen switch), on the composited frame.
// Any frame without a resolve breaks the history chain (TaaHistoryUsable): the
// next resolve starts from its own frame, as does a camera cut or a new
// target size. The shadow-map depth pass has its own shaders and is never
// jittered.

#include <PoseidonGL33/EngineGL33.hpp>
#include <PoseidonGL33/EngineGL33_Lighting.hpp>
#include <PoseidonGL33/EngineGL33_PostPass.hpp>
#include <Poseidon/Graphics/Core/GLClear.hpp>
#include <Poseidon/Graphics/Core/GLDepthStencilState.hpp>
#include <Poseidon/Core/Global.hpp>
#include <Poseidon/World/Scene/Scene.hpp>
#include <Poseidon/World/Scene/Camera/Camera.hpp>

#include <algorithm>
#include <cstdlib>

namespace rm = Poseidon::remaster;
using namespace Poseidon::gl33post;

bool EngineGL33::TaaWantedThisFrame() const
{
    // _worldEffectsOn: a world pass of this frame (not a portrait, a 3D
    // preview after the world or the night-vision intensifier). The gates are
    // this frame's (UpdateFeatureGates runs at the world's first pass).
    return _glContext && _worldEffectsOn && _remasterPrograms && _lighting.postEffects && SSAAActive() &&
           !_post.unavailable && _featureGates.computed && _featureGates.frame == GetFrameCounter() &&
           _featureGates[rm::Feature::Taa].on;
}

void EngineGL33::ApplyTaaJitter()
{
    ApplyTemporalReset(); // P0-6: a triResetTemporal request, once (no-op in play)
    const std::uint32_t frame = GetFrameCounter();
    if (!(_taa.frameOn && _taa.frame == frame) && TaaWantedThisFrame())
    {
        // The frame's first world pass: its offset and its camera.
        _taa.frameOn = true;
        _taa.frame = frame;
        _taa.opaqueCopied = false;
        _taa.jitterPx = rm::TaaJitterPixels(_taa.sequence);
        rm::TaaCamera& cam = _taa.camera;
        const GfxMatrix& v = _frameState.view;
        const double rot[9] = {v._11, v._12, v._13, v._21, v._22, v._23, v._31, v._32, v._33};
        for (int i = 0; i < 9; ++i)
            cam.rot[i] = rot[i];
        for (int i = 0; i < 3; ++i)
            cam.pos[i] = _frameState.cameraPos[i];
        cam.p11 = _frameState.projection._11;
        cam.p22 = _frameState.projection._22;
    }
    // The 3D viewport: the pillarbox rect when the scene is cropped.
    int w = 0, h = 0;
    RenderTargetSize(w, h);
    const Poseidon::AspectSettings& as = _aspectSettings;
    if (!(as.worldLeft <= 0.0f && as.worldTop <= 0.0f && as.worldRight >= 1.0f && as.worldBottom >= 1.0f))
    {
        w = static_cast<int>(as.worldRight * w + 0.5f) - static_cast<int>(as.worldLeft * w + 0.5f);
        h = static_cast<int>(as.worldBottom * h + 0.5f) - static_cast<int>(as.worldTop * h + 0.5f);
    }
    const bool worldPass = _worldEffectsOn && !_portraitTargetSize;
    const bool taaFrame = _taa.frameOn && _taa.frame == frame;
    SetTaaJitterState(rm::TaaPassJitterNdc(worldPass, taaFrame, _taa.jitterPx, w, h),
                      worldPass && taaFrame ? _lighting.taaMipBias : 0.0f);
}

void EngineGL33::SetTaaJitterState(rm::TaaOffset ndc, float bias)
{
    if (ndc.x != _taa.ndcApplied.x || ndc.y != _taa.ndcApplied.y)
    {
        _taa.ndcApplied = ndc;
        UploadVSJitter(ndc.x, ndc.y);
    }
    // The material samplers (units 0 and 1), shared with the HUD: the world's
    // passes take the twin set, which holds the bias (UpdateTaaBiasedSamplers,
    // between frames), and hand the plain set back after them. No sampler is
    // written here.
    SetMaterialSamplerBias(bias != 0.0f);
    _taa.mipBiasApplied = bias;
}

void EngineGL33::BeginTranslucentObjects()
{
    // Once, in a TAA frame's world pass, and only into targets that already
    // exist at this size (creating them here would reset the viewport of a
    // cropped world pass; the post pass creates them at the end of the frame).
    if (!_taa.frameOn || _taa.frame != GetFrameCounter() || !_worldEffectsOn || _taa.opaqueCopied || !SSAAActive())
        return;
    int w = 0, h = 0;
    RenderTargetSize(w, h);
    if (_post.w != w || _post.h != h || !_post.opaqueFbo)
        return;
    FlushAndFreeAllQueues(_queueNo);
    // The multisample resolve of the frame target into the opaque copy. A blit
    // honours the scissor test; the copy must hold the whole frame.
    const bool scissor = glIsEnabled(GL_SCISSOR_TEST) == GL_TRUE;
    if (scissor)
        glDisable(GL_SCISSOR_TEST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, _ssaaFbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _post.opaqueFbo);
    glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, _ssaaFbo); // the viewport is not framebuffer state: it stays
    if (scissor)
        glEnable(GL_SCISSOR_TEST);
    _taa.opaqueCopied = true;
}

void EngineGL33::PostTemporal(const PostFrame& f)
{
    const PostTargets& p = _post;
    const std::uint32_t frame = GetFrameCounter();
    // EX9-WEAK (Q-4): with the world below the window, the history is the
    // window's size and the upsampling resolve writes it.
    const bool up = XpUpscaleActive() && p.progTaaUp && p.histW == _xpDispW && p.histH == _xpDispH;
    const int hw = up ? _xpDispW : f.w;
    const int hh = up ? _xpDispH : f.h;
    const rm::TaaReprojection reproj = rm::BuildTaaReprojection(_taa.prevCamera, _taa.camera);
    const bool usable = rm::TaaHistoryUsable(_taa.history, frame, hw, hh, reproj.valid);
    const int src = _taa.historyIdx;
    const int dst = 1 - src;

    const GLuint prog = up ? p.progTaaUp : p.progTaa;
    glUseProgram(prog);
    SetInt(prog, "curTex", 0);
    SetInt(prog, "depthTex", 2);
    SetInt(prog, "histTex", 3);
    SetInt(prog, "worldTex", 4);
    SetInt(prog, "opaqueTex", 5);
    BindTex(0, p.taaInTex);
    BindTex(2, p.depthTex);
    BindTex(3, p.histTex[src]);
    BindTex(4, p.colorTex);
    BindTex(5, p.opaqueTex);
    Set4(prog, "projInfo", f.projInfo[0], f.projInfo[1], f.projInfo[2], f.projInfo[3]);
    Set4(prog, "depthMap", f.depthScale, f.depthBias, 0.0f, 0.0f);
    Set4(prog, "worldRect", f.rect[0], f.rect[1], f.rect[2], f.rect[3]);
    Set4(prog, "screen", static_cast<float>(hw), static_cast<float>(hh), 1.0f / hw, 1.0f / hh);
    if (up)
    {
        // The Gaussian's coefficient over window pixels (lower is wider and
        // calmer) and the least share of this frame where its nearest sample
        // lies off the centre (EX9-WEAK's tuned values).
        constexpr float upKernel = 4.0f;
        constexpr float upFloor = 0.3f;
        Set4(prog, "srcSize", static_cast<float>(f.w), static_cast<float>(f.h), 1.0f / f.w, 1.0f / f.h);
        Set4(prog, "upParams", upKernel, upFloor, 0.0f, 0.0f);
    }
    const float alpha = _lighting.taaAlpha < 0.01f ? 0.01f : (_lighting.taaAlpha > 1.0f ? 1.0f : _lighting.taaAlpha);
    Set4(prog, "jitter", _taa.jitterPx.x, _taa.jitterPx.y, usable ? 1.0f : 0.0f, alpha);
    const char* rows[3] = {"reproj0", "reproj1", "reproj2"};
    for (int r = 0; r < 3; ++r)
        Set4(prog, rows[r], reproj.rows[r][0], reproj.rows[r][1], reproj.rows[r][2], reproj.rows[r][3]);
    Set4(prog, "prevProj", reproj.prevP11, reproj.prevP22, 0.0f, 0.0f);
    Set4(prog, "reactive", rm::kTaaReactiveLow, rm::kTaaReactiveHigh, rm::kTaaReactiveMaxAlpha,
         _taa.opaqueCopied ? 1.0f : 0.0f);
    DrawTo(p.histFbo[dst], hw, hh);

    // This frame is the next one's history.
    _taa.historyIdx = dst;
    _taa.history.exists = true;
    _taa.history.frame = frame;
    _taa.history.w = hw;
    _taa.history.h = hh;
    _taa.prevCamera = _taa.camera;
    ++_taa.sequence;
}

void EngineGL33::PostSharpen(const PostFrame& f)
{
    const PostTargets& p = _post;
    if (!f.plan.Runs(rm::PostStage::Temporal))
    {
        // TAA off: the luma sharpen of the composited frame into every
        // sample of the frame target.
        glUseProgram(p.progLumaSharpen);
        SetInt(p.progLumaSharpen, "srcTex", 0);
        BindTex(0, p.taaInTex);
        const float strength = std::clamp(_lighting.sharpenStrength, 0.0f, 1.0f);
        Set4(p.progLumaSharpen, "sharpen", strength, static_cast<float>(f.w), static_cast<float>(f.h), 0.0f);
        DrawTo(_ssaaFbo, f.w, f.h);
        return;
    }
    // EX9-WEAK (Q-4): the history is the window's size: the sharpen writes the
    // display set, which then takes the frame target's names for the HUD and
    // the rest of the frame (BackToFront hands them back). Its depth and
    // stencil start clear each frame, as nothing of the world's depth is
    // carried up to the window's size.
    const bool up = XpUpscaleActive() && _taa.history.w == _xpDispW && _taa.history.h == _xpDispH;
    const int sw = up ? _xpDispW : f.w;
    const int sh = up ? _xpDispH : f.h;
    if (up)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, _xpDispFbo);
        Poseidon::render::depthstencil::StencilFullWriteMask();
        Poseidon::render::clear::WithMask(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        Poseidon::render::depthstencil::Disabled(/*hasStencil*/ false); // the fullscreen stages' state again
    }
    const float strength = _lighting.taaSharpen;
    // The resolved frame (the history the resolve just wrote) into every
    // sample of the frame target.
    glUseProgram(p.progTaaSharpen);
    SetInt(p.progTaaSharpen, "srcTex", 0);
    BindTex(0, p.histTex[_taa.historyIdx]);
    Set4(p.progTaaSharpen, "sharpen", strength, static_cast<float>(sw), static_cast<float>(sh), 0.0f);
    DrawTo(up ? _xpDispFbo : _ssaaFbo, sw, sh);
    if (up)
    {
        XpSwapFrameTargets();
        _ssrStencilValid = false; // the stencil the reflections' tags left is not this target's
    }
}

// ---- P0-6 temporal determinism (Graphics/Remaster/TemporalDeterminism.hpp) ----
// A triResetTemporal request, applied once at the first pass of the next
// frame that reads temporal state. Play never requests one: the latch then
// has nothing to take and this returns at once.
void EngineGL33::ApplyTemporalReset()
{
    const rm::TemporalControlState& tc = rm::TemporalControl();
    if (!_temporalDet.latch.Take(tc.resetSerial))
        return;
    const std::uint32_t frame = GetFrameCounter();
    _taa.sequence = 0;
    _fog.sequence = 0;
    DropTemporalHistories();
    // The photoreal camera takes the first meter of this frame at once (as at
    // a cut). Deterministic, the exposure in use until then is rounded to a
    // whole stop, so this frame's pre-exposure, and with it the meter the
    // snap reads, is the same whatever the exposure had adapted to.
    _camera.snap = true;
    _camera.snapFrame = frame;
    if (tc.deterministic)
        _camera.exposureLog2 = rm::ResetExposureLog2(_camera.exposureLog2);
    _temporalDet.redropOnSnap = true;
    _temporalDet.frameBase = frame;
    rm::NoteTemporalResetApplied(tc.resetSerial, frame);
    LOG_INFO(Graphics, "GL33: temporal reset {} at frame {} (deterministic {}, mist {})", tc.resetSerial, frame,
             tc.deterministic ? 1 : 0, tc.mistHeld ? "held" : "free");
}

void EngineGL33::DropTemporalHistories()
{
    _taa.history.exists = false;
    _fog.historyValid = false;
    _surfAoHist.exists = false;
}
