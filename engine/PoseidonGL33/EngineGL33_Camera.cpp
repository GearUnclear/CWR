// The photoreal camera (feature switch photoLight;
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
//     (with the air applied) to kMeterWidth x kMeterHeight log-luminance
//     texels, read back through a ring of pixel-pack buffers a few frames
//     later (the CPU never waits on the GPU), and the composite applies the
//     camera.
// Pre-exposure: the world's passes store their light at the exposure in use
// when the frame began (litCamera.w; set once per frame), so the stored frame
// is near display level; the meter divides it back out, and the composite
// applies what the exposure moved since (exposureLog2 - preExposureLog2).
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
#include <Poseidon/World/Terrain/Landscape.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <vector>

namespace rm = Poseidon::remaster;
using namespace Poseidon::gl33post;

namespace
{
constexpr float kCutMetres = 50.0f;
constexpr float kCutCos = 0.5f; // 60 degrees

// IEEE half to float (the meter target's texels).
float HalfToFloat(std::uint16_t h)
{
    const std::uint32_t sign = (h & 0x8000u) << 16;
    const std::uint32_t exp = (h >> 10) & 0x1fu;
    std::uint32_t mant = h & 0x3ffu;
    std::uint32_t bits;
    if (exp == 0)
    {
        if (mant == 0)
            bits = sign;
        else
        {
            int e = -1;
            do
            {
                ++e;
                mant <<= 1;
            } while ((mant & 0x400u) == 0);
            bits = sign | (static_cast<std::uint32_t>(127 - 15 - e) << 23) | ((mant & 0x3ffu) << 13);
        }
    }
    else if (exp == 31)
        bits = sign | 0x7f800000u | (mant << 13);
    else
        bits = sign | ((exp + 127 - 15) << 23) | (mant << 13);
    float f;
    std::memcpy(&f, &bits, sizeof(f));
    return f;
}

double NowSeconds()
{
    using clock = std::chrono::steady_clock;
    return std::chrono::duration<double>(clock::now().time_since_epoch()).count();
}
} // namespace

bool EngineGL33::CameraWanted() const
{
    return _lightingActive && rm::FeatureOn(_lighting.features, rm::Feature::PhotoLight) && _lighting.postEffects &&
           _lighting.debugView == 0 && !_nightVision && SSAAActive() && !_post.unavailable;
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
    // The frame's pre-exposure: the exposure in use as it begins, the same
    // for every world pass of the frame.
    if (c.preExposureFrame != c.frame)
    {
        c.preExposureFrame = c.frame;
        c.preExposureLog2 = std::isfinite(c.exposureLog2) ? c.exposureLog2 : 0.0f;
    }
    c.nightEffect = rm::CameraNight(rig.sunElevationDeg);
    c.duskEffect = rm::CameraDusk(rig.sunElevationDeg);
    c.duskKeep = rm::CameraDuskKeep(rig.sunElevationDeg);
    c.golden = rm::CameraGolden(rig.sunElevationDeg);
    // Daylight white balance, following the light it is in by a share; it
    // fades out at night (the camera stays on daylight: lamps warm, the moon
    // the rig's cool moon, the composite's rod shift the cool dim light).
    c.whiteBalance = _lighting.camWb * (1.0f - std::min(std::max(c.nightEffect, 0.0f), 1.0f));
    const rm::Rgb white =
        rm::WhiteBalanceTarget(c.referenceIlluminant, rig.illuminant, rm::WhiteBalanceShare(rig.sunElevationDeg));
    const rm::Mat3 wb = rm::WhiteBalance(white, c.whiteBalance);
    // The scene saturation, a little less in the golden light.
    const float lowSun = 1.0f - std::min(std::max((rig.sunElevationDeg - 5.0f) / 30.0f, 0.0f), 1.0f);
    // And less on a grey day (the weather's veil).
    Landscape* land = GScene ? GScene->GetLandscape() : nullptr;
    const float veil = rm::CameraWeatherVeil(land ? land->SkyThrough() : 1.0f);
    const float sat =
        _lighting.camSat - rm::kLowSunSaturationDrop * lowSun * (1.0f - c.nightEffect) - rm::kVeilSaturationDrop * veil;
    c.color = rm::Multiply(rm::Saturation(sat), wb);
    c.timeEv = rm::CameraTimeOfDayEv(rig.sunElevationDeg, veil);
    c.veil = veil;
    // The golden hour's warm balance (kCameraGoldenWarm, at unit luminance):
    // the composite lays it on the light that is not blue (GoldenWarmShare),
    // so the clear sky and the sky-lit shade keep their blue.
    c.goldenGain = rm::GoldenWarmGain(c.golden * (1.0f - veil));
    // The cool shade of a low sun.
    const float lowShade = 1.0f - rm::SmoothStep(15.0f, 45.0f, rig.sunElevationDeg);
    c.shadeCool =
        rm::kCameraShadeCool * lowShade * (1.0f - std::min(std::max(c.nightEffect, 0.0f), 1.0f)) * (1.0f - veil);
    c.skyScale = atmosphereOn ? rig.skyScale : 0.0f;
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
    const auto t0 = std::chrono::steady_clock::now();
    // The meters that have arrived: fences pass in submission order, so walk
    // the ring from the oldest slot and stop at the first one still in
    // flight. Every passed slot is freed; only the newest is read and metered
    // (an older one would be overwritten by it at once).
    bool fresh = false;
    std::uint32_t freshFrame = 0;
    int newest = -1;
    for (int k = 0; k < CameraState::kRing; ++k)
    {
        const int i = (c.next + k) % CameraState::kRing;
        if (!c.fence[i])
            continue;
        GLsync sync = static_cast<GLsync>(c.fence[i]);
        const GLenum st = glClientWaitSync(sync, 0, 0);
        if (st != GL_ALREADY_SIGNALED && st != GL_CONDITION_SATISFIED)
            break;
        glDeleteSync(sync);
        c.fence[i] = nullptr;
        newest = i;
    }
    constexpr int n = rm::kMeterWidth * rm::kMeterHeight;
    if (newest >= 0)
    {
        const int i = newest;
        // The fence has passed: copying the buffer out does not wait (no
        // direct buffer mapping in the backend, GLBufferMap.hpp audit). The
        // target's own half floats, converted here.
        c.halfTexels.resize(static_cast<size_t>(n) * 4);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, c.pbo[i]);
        glGetBufferSubData(GL_PIXEL_PACK_BUFFER, 0, static_cast<GLsizeiptr>(n * 4 * sizeof(std::uint16_t)),
                           c.halfTexels.data());
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        c.logs.resize(n);
        c.weights.resize(n);
        c.peaks.resize(n);
        c.skyPeaks.resize(n);
        const std::uint16_t* h = c.halfTexels.data();
        for (int t = 0; t < n; ++t)
        {
            c.logs[t] = HalfToFloat(h[t * 4 + 0]);
            c.weights[t] = HalfToFloat(h[t * 4 + 1]);
            c.peaks[t] = HalfToFloat(h[t * 4 + 2]);
            c.skyPeaks[t] = HalfToFloat(h[t * 4 + 3]);
        }
        rm::MeterResult m = rm::Meter(c.logs.data(), c.weights.data(), c.peaks.data(), n);
        m.sunRingWeight = c.pboSun[i][3];
        if (m.sunRingWeight > 0.0f)
            m.sunRing =
                rm::SunRingPeak(c.peaks.data(), c.weights.data(), c.pboSun[i][0], c.pboSun[i][1], c.pboSun[i][2]);
        m.skyTop = rm::SkyTopPeak(c.skyPeaks.data(), c.pboSun[i][3] > 0.0f || c.pboSunFront[i], c.pboSun[i][0],
                                  c.pboSun[i][1], c.pboSun[i][2]);
        if (m.valid)
        {
            c.meter = m;
            fresh = true;
            freshFrame = c.pboFrame[i];
            ++c.readbacks;
        }
    }
    const float ms = std::chrono::duration<float, std::milli>(std::chrono::steady_clock::now() - t0).count();
    c.cpuCollectMs += (ms - c.cpuCollectMs) * 0.05f;
    const double now = NowSeconds();
    const float dt = c.lastStep < 0.0 ? 0.0f : static_cast<float>(std::min(std::max(now - c.lastStep, 0.0), 0.25));
    c.lastStep = now;
    if (c.meter.valid)
        c.targetLog2 =
            rm::TargetExposureLog2(c.meter, c.nightEffect, _lighting, c.duskEffect, c.duskKeep, c.timeEv, c.veil);
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
    // The ring's next slot is still in flight (the GPU is more than kRing - 1
    // frames behind): skip this frame's meter rather than reuse it.
    const int i = c.next;
    if (c.fence[i])
    {
        ++c.skippedMeters;
        return;
    }
    const auto t0 = std::chrono::steady_clock::now();
    glUseProgram(p.progMeter);
    SetInt(p.progMeter, "colorTex", 0);
    SetInt(p.progMeter, "depthTex", 2);
    // The frame the composite reads: with the air applied when it ran.
    BindTex(0, PostSceneTex(f)); // the frame after the Air stage (the fog or the air pass)
    BindTex(2, p.depthTex);
    // Scene light: the stored frame divided by its pre-exposure.
    Set4(p.progMeter, "meterParams", rm::kMeterSkyWeight, std::exp2(-c.preExposureLog2),
         static_cast<float>(rm::kMeterWidth), static_cast<float>(rm::kMeterHeight));
    // The sky at its plain roll-off (SkyMeterLight): the dome's cap rises
    // toward the sun so its glow burns white; the meter takes that rise back
    // out along each sky tap's view ray, so the guards do not close the camera
    // down for the sun's burn.
    Set4(p.progMeter, "meterSky", rm::kSkyRolloffKnee, rm::kSkyRolloffCap, rm::kSkySunCap, rm::kSkySunGlowRad);
    {
        float sunView[3] = {0.0f, 0.0f, 1.0f};
        bool sunOk = false;
        LightSun* sun = GScene ? GScene->MainLight() : nullptr;
        Camera* cam = GScene ? GScene->GetCamera() : nullptr;
        if (sun && cam)
        {
            // Camera space as PostFrameSun's: x right, y up, z forward.
            const Vector3 v = cam->InverseScaled().Rotate(-sun->SunDirection());
            const float len = v.Size();
            if (len > 1e-6f)
            {
                sunView[0] = v.X() / len;
                sunView[1] = v.Y() / len;
                sunView[2] = v.Z() / len;
                sunOk = true;
            }
        }
        Set4(p.progMeter, "meterSun", sunView[0], sunView[1], sunView[2], sunOk ? 1.0f : 0.0f);
    }
    Set4(p.progMeter, "meterProj", f.projInfo[2], f.projInfo[3], 0.0f, 0.0f);
    Set4(p.progMeter, "meterRect", f.rect[0], f.rect[1], f.rect[2], f.rect[3]);
    DrawTo(p.meterFbo, rm::kMeterWidth, rm::kMeterHeight);

    // Queue the readback into the free slot: the target's own format (RGBA16F
    // read as half floats, a plain copy on the GPU, no conversion path).
    constexpr GLsizeiptr bytes = rm::kMeterWidth * rm::kMeterHeight * 4 * sizeof(std::uint16_t);
    if (!c.pbo[i])
    {
        glGenBuffers(1, &c.pbo[i]);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, c.pbo[i]);
        glBufferData(GL_PIXEL_PACK_BUFFER, bytes, nullptr, GL_STREAM_READ);
    }
    glBindFramebuffer(GL_READ_FRAMEBUFFER, p.meterFbo);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, c.pbo[i]);
    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glReadPixels(0, 0, rm::kMeterWidth, rm::kMeterHeight, GL_RGBA, GL_HALF_FLOAT, nullptr);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    c.fence[i] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    c.pboFrame[i] = GetFrameCounter();
    // Where the sun was in this frame, for the guard on the sky by it.
    LightSun* sun = GScene ? GScene->MainLight() : nullptr;
    const float elev = sun ? std::asin(std::max(-1.0f, std::min(1.0f, -sun->SunDirection().Y()))) * 57.29578f : 90.0f;
    const bool inFront = f.sunInFront;
    c.pboSun[i][0] = f.sunU;
    c.pboSun[i][1] = f.sunV;
    c.pboSun[i][2] = static_cast<float>(f.w) / static_cast<float>(std::max(f.h, 1));
    c.pboSun[i][3] = inFront && elev > -1.0f ? rm::SunRingWeight(elev) : 0.0f;
    c.pboSunFront[i] = inFront && elev > -1.0f;
    c.next = (i + 1) % CameraState::kRing;
    const float ms = std::chrono::duration<float, std::milli>(std::chrono::steady_clock::now() - t0).count();
    c.cpuQueueMs += (ms - c.cpuQueueMs) * 0.05f;
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
    r.skyScale = c.skyScale;
    r.preExposureLog2 = c.preExposureLog2;
    r.sunLum = c.sunLum;
    r.skyUpLum = c.skyUpLum;
    r.readbacks = c.readbacks;
    r.cpuCollectMs = c.cpuCollectMs;
    r.cpuQueueMs = c.cpuQueueMs;
    r.skippedMeters = c.skippedMeters;
    r.meterSkyTop = c.meter.skyTop;
    r.meterSunRing = c.meter.sunRing;
    r.meterSunRingWeight = c.meter.sunRingWeight;
    if (c.meter.valid)
    {
        rm::MeterResult free = c.meter;
        free.highlight = rm::kMeterFloorLog2;
        free.skyTop = rm::kMeterFloorLog2;
        free.sunRingWeight = 0.0f;
        r.unguardedLog2 =
            rm::TargetExposureLog2(free, c.nightEffect, _lighting, c.duskEffect, c.duskKeep, c.timeEv, c.veil);
    }
    return r;
}
