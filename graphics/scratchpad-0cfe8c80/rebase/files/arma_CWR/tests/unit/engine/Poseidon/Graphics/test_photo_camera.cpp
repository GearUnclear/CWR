#include <catch2/catch_approx.hpp>
#include <catch2/catch_message.hpp>
#include <catch2/catch_test_macros.hpp>
#include <Poseidon/Graphics/Remaster/Atmosphere.hpp>
#include <Poseidon/Graphics/Remaster/FeatureGates.hpp>
#include <Poseidon/Graphics/Remaster/FeatureSwitches.hpp>
#include <Poseidon/Graphics/Remaster/LightRig.hpp>
#include <Poseidon/Graphics/Remaster/PhotoCamera.hpp>
#include <Poseidon/Graphics/Remaster/PostStages.hpp>

#include <chrono>
#include <cmath>
#include <cstdio>
#include <vector>

// The photoreal light and camera (look lane "light", feature switch
// photoLight; Graphics/Remaster/PhotoCamera.hpp): the world pass's encoding,
// the meter, the exposure and its adaptation, the tone curve, the white
// balance, the physical sun and sky from the atmosphere and the rig built on
// them, the gate and the post plan.

using namespace Poseidon::remaster;

namespace
{
constexpr float kPi = 3.14159265f;

const AtmosphereTables& Tables()
{
    static const AtmosphereTables t = BuildAtmosphereTables(1.0f);
    return t;
}

PhysicalSky SkyAt(float deg)
{
    PhysicalSky p = ComputePhysicalSky(Tables(), deg * kPi / 180.0f);
    const PhysicalSky ref = ComputePhysicalSky(Tables(), kPhysicalReferenceSunDeg * kPi / 180.0f);
    p.referenceIlluminant = PhysicalIlluminant(ref);
    p.referenceIrradiance = Luminance(p.referenceIlluminant);
    return p;
}

LightRigInputs PhysicalInputs(float deg, float skyThrough = 1.0f)
{
    LightRigInputs in;
    const float e = deg * kPi / 180.0f;
    in.towardSun[0] = std::cos(e);
    in.towardSun[1] = std::sin(e);
    in.towardSun[2] = 0.0f;
    in.fogColor = Rgb(0.75f, 0.8f, 0.9f);
    in.classicSunDiffuse = Rgb(0.9f, 0.85f, 0.7f);
    in.skyThrough = skyThrough;
    in.physical = SkyAt(deg);
    return in;
}

float RedOverBlue(const Rgb& c)
{
    return c.r / std::max(c.b, 1e-9f);
}
} // namespace

TEST_CASE("PhotoCamera: the world pass's encoding is linear below the knee and invertible above it",
          "[Remaster][PhotoCamera]")
{
    for (float x : {0.0f, 0.01f, 0.18f, 0.5f, kEncodeKnee})
        CHECK(EncodePeak(x) == Catch::Approx(x));
    float prev = 0.0f;
    for (float x = 0.61f; x < 60.0f; x *= 1.3f)
    {
        const float e = EncodePeak(x);
        CHECK(e > prev);
        CHECK(e < 1.0f);
        CHECK(DecodePeak(e) == Catch::Approx(x).epsilon(1e-3));
        prev = e;
    }
    // Hue kept: the encoded colour is the scaled colour.
    const Rgb c(3.0f, 1.5f, 0.3f);
    const Rgb back = CameraDecode(CameraEncode(c));
    CHECK(back.r == Catch::Approx(c.r).epsilon(1e-3));
    CHECK(back.g == Catch::Approx(c.g).epsilon(1e-3));
    CHECK(back.b == Catch::Approx(c.b).epsilon(1e-3));
    // An 8-bit top code decodes to a bounded light.
    CHECK(DecodePeak(1.0f) <= kDecodeMax);
}

TEST_CASE("PhotoCamera: the meter ignores the darkest and brightest shares and finds small bright patches",
          "[Remaster][PhotoCamera]")
{
    // 90 samples at -3, 5 very dark, 5 very bright.
    std::vector<float> l, w, pk;
    for (int i = 0; i < 90; ++i)
        l.push_back(-3.0f);
    for (int i = 0; i < 5; ++i)
        l.push_back(-14.0f);
    for (int i = 0; i < 5; ++i)
        l.push_back(4.0f);
    w.assign(l.size(), 1.0f);
    pk = l;
    MeterResult m = Meter(l.data(), w.data(), pk.data(), static_cast<int>(l.size()));
    REQUIRE(m.valid);
    // The 5 % darkest go; 2 of the 5 bright samples are left inside the cut.
    CHECK(m.mean == Catch::Approx((90.0f * -3.0f + 2.0f * 4.0f) / 92.0f).margin(0.02));
    CHECK(m.highlight == Catch::Approx(4.0f));
    // A bright patch inside a block shows through the block's brightest tap.
    pk.assign(l.size(), -3.0f);
    for (int i = 0; i < 10; ++i)
        pk[i] = 2.0f;
    m = Meter(l.data(), w.data(), pk.data(), static_cast<int>(l.size()));
    CHECK(m.highlight >= 2.0f);
    // No weight, no meter.
    std::vector<float> zero(l.size(), 0.0f);
    CHECK_FALSE(Meter(l.data(), zero.data(), nullptr, static_cast<int>(l.size())).valid);
}

TEST_CASE("PhotoCamera: exposure keeps part of a scene's darkness, guards its highlights and darkens the night",
          "[Remaster][PhotoCamera]")
{
    LightingTuning t;
    t.camEv = 0.0f;
    MeterResult day;
    day.valid = true;
    day.mean = kDaylightMeanLog2;
    day.highlight = kDaylightMeanLog2 + 2.0f;
    // A daylight frame: its mean goes to middle grey.
    CHECK(TargetExposureLog2(day, 0.0f, t) == Catch::Approx(std::log2(kMiddleGrey) - kDaylightMeanLog2));
    // Four stops darker: opened up, but less than four stops.
    MeterResult dark = day;
    dark.mean -= 4.0f;
    dark.highlight -= 4.0f;
    const float opened = TargetExposureLog2(dark, 0.0f, t) - TargetExposureLog2(day, 0.0f, t);
    CHECK(opened > 1.0f);
    CHECK(opened < 4.0f);
    // A bright patch far above the mean pulls the exposure down.
    MeterResult hot = dark;
    hot.highlight = dark.mean + 9.0f;
    CHECK(TargetExposureLog2(hot, 0.0f, t) < TargetExposureLog2(dark, 0.0f, t));
    // At night it never brightens past kExposureNightMaxLog2 and sits lower.
    CHECK(TargetExposureLog2(dark, 1.0f, t) <= kExposureNightMaxLog2 + 1e-6f);
    CHECK(TargetExposureLog2(dark, 1.0f, t) < TargetExposureLog2(dark, 0.0f, t));
    // The camera's night follows the sun, not classic's night effect (1 at
    // any sun under the horizon): the blue hour is not exposed as night.
    CHECK(CameraNight(5.0f) == 0.0f);
    CHECK(CameraNight(-15.0f) == 1.0f);
    CHECK(CameraNight(-4.9f) > 0.0f);
    CHECK(CameraNight(-4.9f) < 0.5f);
    // camEv adds.
    LightingTuning plus = t;
    plus.camEv = 1.0f;
    CHECK(TargetExposureLog2(day, 0.0f, plus) == Catch::Approx(TargetExposureLog2(day, 0.0f, t) + 1.0f));
}

TEST_CASE("PhotoCamera: eye adaptation converges in about its time constant, slower into the dark",
          "[Remaster][PhotoCamera]")
{
    float e = 0.0f;
    for (int i = 0; i < 60; ++i)
        e = AdaptExposureLog2(e, -2.0f, 1.0f / 60.0f, 1.0f);
    CHECK(e == Catch::Approx(-2.0f * (1.0f - std::exp(-1.0f))).margin(0.01));
    float up = 0.0f;
    for (int i = 0; i < 60; ++i)
        up = AdaptExposureLog2(up, 2.0f, 1.0f / 60.0f, 1.0f);
    CHECK(up < 2.0f * (1.0f - std::exp(-1.0f)));
    CHECK(AdaptExposureLog2(0.5f, 1.0f, 0.1f, 0.0f) == 1.0f);
}

TEST_CASE("PhotoCamera: the filmic curve passes middle grey, reaches white and keeps hue until it whitens",
          "[Remaster][PhotoCamera]")
{
    const ToneCurve c = MakeToneCurve(1.35f, 0.97f, 16.0f);
    CHECK(ApplyToneCurve(c, kMiddleGrey) == Catch::Approx(kCurveMidOut).epsilon(1e-3));
    CHECK(ApplyToneCurve(c, kMiddleGrey * 16.0f) == Catch::Approx(1.0f).epsilon(1e-3));
    float prev = 0.0f;
    for (float x = 0.001f; x < 3.0f; x *= 1.2f)
    {
        const float y = ApplyToneCurve(c, x);
        CHECK(y >= prev);
        prev = y;
    }
    // The toe: a dark value comes out darker than linear.
    CHECK(ApplyToneCurve(c, kMiddleGrey / 8.0f) < kMiddleGrey / 8.0f);
    // An orange at middle grey keeps its ratios; far past white it is white.
    const Rgb orange(0.18f, 0.09f, 0.03f);
    const Rgb mid = ApplyToneCurveHueKeeping(c, orange);
    CHECK(mid.g / mid.r == Catch::Approx(0.5f).margin(0.03));
    const Rgb hot = ApplyToneCurveHueKeeping(c, orange * 40.0f);
    CHECK(hot.b == Catch::Approx(1.0f).margin(0.01));
}

TEST_CASE("PhotoCamera: the white balance makes its target white and keeps white's luminance",
          "[Remaster][PhotoCamera]")
{
    const Rgb warm(1.0f, 0.75f, 0.45f);
    const Mat3 wb = WhiteBalance(warm, 1.0f);
    const Rgb w = wb.Apply(warm * (1.0f / Luminance(warm)));
    CHECK(w.r == Catch::Approx(w.g).epsilon(0.01));
    CHECK(w.b == Catch::Approx(w.g).epsilon(0.01));
    CHECK(Luminance(w) == Catch::Approx(1.0f).epsilon(1e-3));
    const Mat3 none = WhiteBalance(warm, 0.0f);
    for (int i = 0; i < 9; ++i)
        CHECK(none.m[i] == Catch::Approx(i % 4 == 0 ? 1.0f : 0.0f));
    // The target follows the light by the share only.
    const Rgb ref(1.0f, 1.0f, 1.0f);
    CHECK(RedOverBlue(WhiteBalanceTarget(ref, warm, 0.0f)) == Catch::Approx(1.0f));
    CHECK(RedOverBlue(WhiteBalanceTarget(ref, warm, 1.0f)) == Catch::Approx(RedOverBlue(warm)));
    CHECK(WhiteBalanceShare(60.0f) > WhiteBalanceShare(8.0f));
    // Saturation keeps grey.
    const Rgb g = Saturation(1.3f).Apply(Rgb(0.4f, 0.4f, 0.4f));
    CHECK(g.r == Catch::Approx(0.4f));
    CHECK(g.b == Catch::Approx(0.4f));
}

TEST_CASE("PhotoCamera: the physical sun out-lights the sky at noon, and a low sun is golden and dim",
          "[Remaster][PhotoCamera]")
{
    const PhysicalSky noon = SkyAt(60.0f);
    const PhysicalSky low = SkyAt(10.0f);
    const PhysicalSky twilight = SkyAt(-4.0f);
    REQUIRE(noon.valid);
    const float noonSun = Luminance(noon.sun) * std::sin(60.0f * kPi / 180.0f);
    const float noonSky = Luminance(noon.skyUp);
    char buf[512];
    std::snprintf(buf, sizeof(buf),
                  "noon sun %.3f (%.3f %.3f %.3f) sky %.3f (%.3f %.3f %.3f) walls %.3f / %.3f horizon %.4f | "
                  "10 deg sun %.3f (%.3f %.3f %.3f) sky %.3f (%.3f %.3f %.3f) walls %.3f / %.3f | -4 deg sky %.4f",
                  noonSun, noon.sun.r, noon.sun.g, noon.sun.b, noonSky, noon.skyUp.r, noon.skyUp.g, noon.skyUp.b,
                  Luminance(noon.skySunward), Luminance(noon.skyAntiSun), noon.horizonRadiance,
                  Luminance(low.sun) * std::sin(10.0f * kPi / 180.0f), low.sun.r, low.sun.g, low.sun.b,
                  Luminance(low.skyUp), low.skyUp.r, low.skyUp.g, low.skyUp.b, Luminance(low.skySunward),
                  Luminance(low.skyAntiSun), Luminance(twilight.skyUp));
    INFO(buf);
    // Clear noon: the sun gives several times the sky's light to the ground.
    CHECK(noonSun / noonSky > 4.0f);
    CHECK(noonSun / noonSky < 15.0f);
    // The sky light is blue, the noon sun near white, a low sun golden.
    CHECK(RedOverBlue(noon.skyUp) < 0.8f);
    CHECK(RedOverBlue(noon.sun) > 1.0f);
    CHECK(RedOverBlue(low.sun) > RedOverBlue(noon.sun) * 1.5f);
    // A low sun lights the ground far less; the wall toward it sees more sky.
    CHECK(Luminance(low.sun) * std::sin(10.0f * kPi / 180.0f) < noonSun * 0.4f);
    CHECK(Luminance(low.skySunward) > Luminance(low.skyAntiSun));
    // Twilight: no sun, a dim sky.
    CHECK(Luminance(twilight.sun) == 0.0f);
    CHECK(Luminance(twilight.skyUp) < noonSky * 0.2f);
    // The integral runs on the render thread when the sun has moved: a few
    // milliseconds at most.
    const auto t0 = std::chrono::steady_clock::now();
    for (int i = 0; i < 5; ++i)
        (void)ComputePhysicalSky(Tables(), (20.0f + i) * kPi / 180.0f);
    const double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count() / 5.0;
    INFO("physical sky integral ms: " << ms);
    CHECK(ms < 25.0);
}

TEST_CASE("PhotoCamera: the physical rig shows white at the classic noon level and replaces the meter",
          "[Remaster][PhotoCamera]")
{
    LightingTuning t;
    const LightRigInputs in = PhysicalInputs(kPhysicalReferenceSunDeg);
    const LightRig rig = ComputeLightRig(in, t);
    REQUIRE(rig.physical);
    const float sinRef = std::sin(kPhysicalReferenceSunDeg * kPi / 180.0f);
    // A white surface facing up: 1.6 times the irradiance is 1 (as classic noon).
    CHECK(1.6f * (Luminance(rig.sun) * sinRef + Luminance(rig.skyZenith)) == Catch::Approx(1.0f).epsilon(0.02));
    CHECK(rig.exposure == Catch::Approx(t.exposure));
    // The sun dominates; the shade is the blue sky.
    CHECK(Luminance(rig.sun) * sinRef > 4.0f * Luminance(rig.skyZenith));
    CHECK(RedOverBlue(rig.skyZenith) < 0.8f);
    // A low sun: far less light (the camera's exposure brings it back).
    const LightRig low = ComputeLightRig(PhysicalInputs(8.0f), t);
    CHECK(Luminance(low.illuminant) < 0.3f * Luminance(rig.illuminant));
    CHECK(RedOverBlue(low.sun) > RedOverBlue(rig.sun));
    // Overcast: dimmer sun, a sky that greys and gains the cloud's diffuse light.
    const LightRig grey = ComputeLightRig(PhysicalInputs(50.0f, 0.1f), t);
    const LightRig clear = ComputeLightRig(PhysicalInputs(50.0f, 1.0f), t);
    CHECK(Luminance(grey.sun) < 0.5f * Luminance(clear.sun));
    CHECK(Luminance(grey.skyZenith) > Luminance(clear.skyZenith));
    CHECK(RedOverBlue(grey.skyZenith) > RedOverBlue(clear.skyZenith));
    CHECK(grey.skyGain == Catch::Approx(1.0f));
    // The switch off: the old rig, to the bit.
    LightingTuning off = t;
    off.features.photoLight = 0.0f;
    const LightRig old = ComputeLightRig(in, off);
    LightRigInputs plain = in;
    plain.physical = PhysicalSky{};
    const LightRig ref = ComputeLightRig(plain, t);
    CHECK_FALSE(old.physical);
    CHECK(old.sun.r == ref.sun.r);
    CHECK(old.skyZenith.b == ref.skyZenith.b);
    CHECK(old.exposure == ref.exposure);
}

TEST_CASE("PhotoCamera: the switch, its sub-knobs, its gate and the post plan", "[Remaster][PhotoCamera]")
{
    const FeatureInfo* f = FindFeature("photoLight");
    REQUIRE(f);
    CHECK(f->id == Feature::PhotoLight);
    CHECK(f->landed);
    for (const char* key : {"camEv", "camAdapt", "camCurve", "camContrast", "camShoulder", "camWhite", "camSat",
                            "camWb", "camSky", "camSkyHue", "camNight", "camVignette", "camBloom"})
    {
        const LightingKey* k = FindLightingKey(key);
        REQUIRE(k);
        CHECK(std::string(k->parent) == "photoLight");
    }
    LightingTuning t;
    GateInputs in;
    CHECK(ComputeFeatureGates(in, t)[Feature::PhotoLight].on);
    LightingTuning noPost = t;
    noPost.postEffects = false;
    CHECK_FALSE(ComputeFeatureGates(in, noPost)[Feature::PhotoLight].on);
    LightingTuning debug = t;
    debug.debugView = 1;
    CHECK_FALSE(ComputeFeatureGates(in, debug)[Feature::PhotoLight].on);
    GateInputs nv;
    nv.nightVision = true;
    CHECK_FALSE(ComputeFeatureGates(nv, t)[Feature::PhotoLight].on);
    // The camera asks for the post pass by itself, composite included.
    PostFrameInputs pin;
    pin.camera = true;
    const PostStagePlan plan = PlanPostStages(pin);
    CHECK(plan.Runs(PostStage::Composite));
    CHECK(plan.Runs(PostStage::Resolve));
}
