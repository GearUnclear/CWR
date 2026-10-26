// The photoreal camera (look lane "light", feature switch photoLight;
// Graphics/Remaster/PhotoCamera.hpp holds the math).
//
// Per frame:
//   - UploadLitFrameConstants asks for the physical sky (PhysicalSkyFor, from
//     the sky table's atmosphere tables) while the camera can run, so the rig
//     is the physical one, and after the gates UpdateCameraFrame decides
//     whether this frame's world passes write scene light (LitConstants
//     Camera slot) and sets the white balance;
//   - EndWorldScene, before its stages, collects the meter readbacks that
//     have arrived and adapts the exposure toward their target (CameraAdapt);
//   - inside the Composite stage the meter pass reduces the resolved frame
//     to kMeterWidth x kMeterHeight log-luminance texels, read back through a
//     ring of pixel-pack buffers a few frames later (the CPU never waits on
//     the GPU), and the composite applies the camera.
// A camera cut (more than 50 m or 60 degrees in a frame), the camera turning
// on and a lighting change snap the exposure to the first meter of a frame
// after it. Black-through meter blocks (an optic's mask) weigh nothing, so
// through gunner optics the camera meters the scene in the optic.

#include <PoseidonGL33/EngineGL33.hpp>
#include <PoseidonGL33/EngineGL33_Lighting.hpp>
#include <PoseidonGL33/EngineGL33_PostPass.hpp>
#include <Poseidon/Graphics/Remaster/FeatureSwitches.hpp>
#include <Poseidon/Graphics/Remaster/PhotoCamera.hpp>
#include <Poseidon/World/Scene/Scene.hpp>
#include <Poseidon/World/Scene/Camera/Camera.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <vector>

namespace rm = Poseidon::remaster;
using namespace Poseidon::gl33post;

namespace
{
constexpr float kCutMetres = 50.0f;
constexpr float kCutCos = 0.5f; // 60 degrees

double NowSeconds()
{
    using clock = std::chrono::steady_clock;
    return std::chrono::duration<double>(clock::now().time_since_epoch()).count();
}
} // namespace

bool EngineGL33::CameraWanted() const
{
    return _lightingActive && rm::FeatureOn(_lighting.features, rm::Feature::PhotoLight) && _lighting.postEffects &&
           _lighting.debugView == 0 && !_nightVision && SSAAActive() && !_portraitTargetSize && !_post.unavailable;
}

void EngineGL33::UpdateCameraFrame(const rm::LightRig& rig, bool atmosphereOn)
{
    CameraState& c = _camera;
    const bool on =
        rig.physical && _featureGates.computed && _featureGates[rm::Feature::PhotoLight].on && CameraWanted();
    if (on && !c.wasOn)
    {
        c.snap = true;
        c.snapFrame = GetFrameCounter();
    }
    c.wasOn = on;
    c.frameOn = on;
    c.frame = GetFrameCounter();
    if (!on)
        return;
    c.nightEffect = rm::CameraNight(rig.sunElevationDeg);
    // Daylight white balance, following the light it is in by a share; it
    // fades out at night.
    c.whiteBalance = _lighting.camWb * (1.0f - std::min(std::max(c.nightEffect, 0.0f), 1.0f));
    const rm::Rgb white =
        rm::WhiteBalanceTarget(c.referenceIlluminant, rig.illuminant, rm::WhiteBalanceShare(rig.sunElevationDeg));
    rm::Mat3 wb = rm::WhiteBalance(white, c.whiteBalance);
    // At night the camera sees moonlight cool (the day-for-night convention):
    // balanced for a light warmer than the night's.
    const float night = std::min(std::max(c.nightEffect, 0.0f), 1.0f);
    if (night > 0.0f && _lighting.camWb > 0.0f)
        wb = rm::Multiply(rm::WhiteBalance(rm::kNightCameraWhite, rm::kNightCameraCool * night), wb);
    c.color = rm::Multiply(rm::Saturation(_lighting.camSat), wb);
    c.skyGain = atmosphereOn ? rig.skyGain : 1.0f;
    const float sinSun = std::sin(rig.sunElevationDeg / 57.29578f);
    c.sunLum = rm::Luminance(rig.sun) * std::max(sinSun, 0.0f);
    c.skyUpLum = rm::Luminance(rig.skyZenith);

    // A cut: snap to the first meter of a frame from now on.
    if (Camera* cam = GScene ? GScene->GetCamera() : nullptr)
    {
        const Vector3 p = cam->Position();
        Vector3 d = cam->Direction();
        d.Normalize();
        if (c.haveCam)
        {
            const float dx = p.X() - c.lastCam[0], dy = p.Y() - c.lastCam[1], dz = p.Z() - c.lastCam[2];
            const float turn = d.X() * c.lastDir[0] + d.Y() * c.lastDir[1] + d.Z() * c.lastDir[2];
            if (dx * dx + dy * dy + dz * dz > kCutMetres * kCutMetres || turn < kCutCos)
            {
                c.snap = true;
                c.snapFrame = GetFrameCounter();
            }
        }
        c.lastCam[0] = p.X();
        c.lastCam[1] = p.Y();
        c.lastCam[2] = p.Z();
        c.lastDir[0] = d.X();
        c.lastDir[1] = d.Y();
        c.lastDir[2] = d.Z();
        c.haveCam = true;
    }
}

void EngineGL33::CameraAdapt()
{
    CameraState& c = _camera;
    // The meters that have arrived, oldest first; the newest wins.
    bool fresh = false;
    std::uint32_t freshFrame = 0;
    std::vector<float>& s_texels = c.texels;
    std::vector<float>& s_log = c.logs;
    std::vector<float>& s_w = c.weights;
    std::vector<float>& s_peak = c.peaks;
    constexpr int n = rm::kMeterWidth * rm::kMeterHeight;
    for (int k = 0; k < CameraState::kRing; ++k)
    {
        const int i = (c.next + k) % CameraState::kRing;
        if (!c.fence[i])
            continue;
        GLsync sync = static_cast<GLsync>(c.fence[i]);
        const GLenum st = glClientWaitSync(sync, 0, 0);
        if (st != GL_ALREADY_SIGNALED && st != GL_CONDITION_SATISFIED)
            continue;
        glDeleteSync(sync);
        c.fence[i] = nullptr;
        // The fence has passed: copying the buffer out does not wait (no
        // direct buffer mapping in the backend, GLBufferMap.hpp audit).
        glBindBuffer(GL_PIXEL_PACK_BUFFER, c.pbo[i]);
        s_texels.resize(static_cast<size_t>(n) * 4);
        glGetBufferSubData(GL_PIXEL_PACK_BUFFER, 0, static_cast<GLsizeiptr>(n * 4 * sizeof(float)), s_texels.data());
        {
            s_log.resize(n);
            s_w.resize(n);
            s_peak.resize(n);
            for (int t = 0; t < n; ++t)
            {
                s_log[t] = s_texels[t * 4 + 0];
                s_w[t] = s_texels[t * 4 + 1];
                s_peak[t] = s_texels[t * 4 + 2];
            }
            const rm::MeterResult m = rm::Meter(s_log.data(), s_w.data(), s_peak.data(), n);
            if (m.valid)
            {
                c.meter = m;
                fresh = true;
                freshFrame = c.pboFrame[i];
                ++c.readbacks;
            }
        }
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    }
    const double now = NowSeconds();
    const float dt = c.lastStep < 0.0 ? 0.0f : static_cast<float>(std::min(std::max(now - c.lastStep, 0.0), 0.25));
    c.lastStep = now;
    if (c.meter.valid)
        c.targetLog2 = rm::TargetExposureLog2(c.meter, c.nightEffect, _lighting);
    if (c.snap)
    {
        if (fresh && freshFrame >= c.snapFrame)
        {
            c.exposureLog2 = c.targetLog2;
            c.snap = false;
            c.adapted = true;
        }
        return;
    }
    if (c.meter.valid)
    {
        c.exposureLog2 = rm::AdaptExposureLog2(c.exposureLog2, c.targetLog2, dt, _lighting.camAdapt);
        c.adapted = true;
    }
}

void EngineGL33::PostCameraMeter(const PostFrame& f)
{
    CameraState& c = _camera;
    const PostTargets& p = _post;
    if (!p.progMeter || !p.meterFbo)
        return;
    glUseProgram(p.progMeter);
    SetInt(p.progMeter, "colorTex", 0);
    SetInt(p.progMeter, "depthTex", 2);
    BindTex(0, p.colorTex);
    BindTex(2, p.depthTex);
    Set4(p.progMeter, "meterParams", rm::kMeterSkyWeight, 1.0f, static_cast<float>(rm::kMeterWidth),
         static_cast<float>(rm::kMeterHeight));
    DrawTo(p.meterFbo, rm::kMeterWidth, rm::kMeterHeight);

    // Queue the readback into the next slot of the ring.
    constexpr GLsizeiptr bytes = rm::kMeterWidth * rm::kMeterHeight * 4 * sizeof(float);
    const int i = c.next;
    if (!c.pbo[i])
    {
        glGenBuffers(1, &c.pbo[i]);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, c.pbo[i]);
        glBufferData(GL_PIXEL_PACK_BUFFER, bytes, nullptr, GL_STREAM_READ);
    }
    if (c.fence[i])
    {
        glDeleteSync(static_cast<GLsync>(c.fence[i]));
        c.fence[i] = nullptr;
    }
    glBindFramebuffer(GL_READ_FRAMEBUFFER, p.meterFbo);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, c.pbo[i]);
    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glReadPixels(0, 0, rm::kMeterWidth, rm::kMeterHeight, GL_RGBA, GL_FLOAT, nullptr);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    c.fence[i] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    c.pboFrame[i] = GetFrameCounter();
    c.next = (i + 1) % CameraState::kRing;
    (void)f;
}

void EngineGL33::DestroyCamera()
{
    CameraState& c = _camera;
    for (int i = 0; i < CameraState::kRing; ++i)
    {
        if (c.fence[i])
            glDeleteSync(static_cast<GLsync>(c.fence[i]));
        c.fence[i] = nullptr;
        if (c.pbo[i])
            glDeleteBuffers(1, &c.pbo[i]);
        c.pbo[i] = 0;
    }
    c.next = 0;
    c.snap = true;
    c.wasOn = false;
}

rm::CameraReadout EngineGL33::CameraReadoutNow() const
{
    rm::CameraReadout r;
    const CameraState& c = _camera;
    // The state of the last world pass, while it is recent (the classic
    // programs do not reach UpdateCameraFrame).
    r.valid = c.frameOn && GetFrameCounter() - c.frame <= 2;
    r.exposureLog2 = c.exposureLog2;
    r.targetLog2 = c.targetLog2;
    r.meterMean = c.meter.mean;
    r.meterHighlight = c.meter.highlight;
    r.whiteBalance = c.whiteBalance;
    r.skyGain = c.skyGain;
    r.sunLum = c.sunLum;
    r.skyUpLum = c.skyUpLum;
    r.readbacks = c.readbacks;
    return r;
}
