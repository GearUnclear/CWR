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

// The photoreal light and camera (feature switch photoLight;
// Graphics/Remaster/PhotoCamera.hpp): the world pass's encoding, the meter, the
// exposure and its adaptation, the tone curve, the white balance, the physical
// sun and sky from the atmosphere and the rig built on them, the gate and the
// post plan.

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

TEST_CASE("PhotoCamera: the world pass's encoding is display gamma below the knee and invertible above it",
          "[Remaster][PhotoCamera]")
{
    for (float x : {0.0f, 0.01f, 0.18f, 0.5f, kEncodeKnee})
        CHECK(EncodePeakCode(x) == Catch::Approx(std::pow(x, 1.0f / 2.2f)));
    float prev = EncodePeakCode(kEncodeKnee);
    for (float x = 0.61f; x < kDecodeMax; x *= 1.2f)
    {
        const float e = EncodePeakCode(x);
        CHECK(e > prev);
        CHECK(e < 1.0f);
        CHECK(DecodePeakCode(e) == Catch::Approx(x).epsilon(1e-3));
        prev = e;
    }
    CHECK(EncodePeakCode(kDecodeMax) == Catch::Approx(1.0f));
    CHECK(DecodePeakCode(1.0f) == Catch::Approx(kDecodeMax));
    // The fold spreads the codes over the bright range: an 8-bit step stays
    // under 10 % of the light up to 4, and 1 to 2 gets a dozen codes (the old
    // fold left 5 codes for 4.9 to 64, and an edge across them jumped 3 x).
    for (float x = 1.0f; x < 4.0f; x *= 1.1f)
    {
        const float up = DecodePeakCode(EncodePeakCode(x) + 1.0f / 255.0f);
        CHECK(up / x < 1.10f);
    }
    CHECK((EncodePeakCode(2.0f) - EncodePeakCode(1.0f)) * 255.0f > 12.0f);
    CHECK(DecodePeakCode(254.0f / 255.0f) > 0.75f * kDecodeMax);
    // Hue kept: the encoded colour is the scaled colour.
    const Rgb c(3.0f, 1.5f, 0.3f);
    const Rgb back = CameraDecode(CameraEncode(c));
    CHECK(back.r == Catch::Approx(c.r).epsilon(1e-3));
    CHECK(back.g == Catch::Approx(c.g).epsilon(1e-3));
    CHECK(back.b == Catch::Approx(c.b).epsilon(1e-3));
    // Below the knee the stored value is the display value.
    const Rgb low = CameraEncode(Rgb(0.3f, 0.2f, 0.1f));
    CHECK(low.r == Catch::Approx(std::pow(0.3f, 1.0f / 2.2f)));
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
    // The highlight counts every block alike: a bright patch in blocks of
    // little weight (the sky between crowns, the frame's edge) still shows.
    std::vector<float> wEdge(l.size(), 1.0f);
    pk.assign(l.size(), -3.0f);
    for (int i = 0; i < 6; ++i)
    {
        pk[i] = 3.0f;
        wEdge[i] = 0.05f;
    }
    m = Meter(l.data(), wEdge.data(), pk.data(), static_cast<int>(l.size()));
    CHECK(m.highlight == Catch::Approx(3.0f));
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
    day.highlight = kDaylightMeanLog2 + 0.5f;
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

TEST_CASE("PhotoCamera: a grey day holds its bright end, the deck or the mist, at a light grey",
          "[Remaster][PhotoCamera]")
{
    // A view through optics into mist metered mean -3.37 and high -0.58; held
    // at kHighlightLimit the mist sat in the curve's shoulder.
    LightingTuning t;
    t.camEv = 0.0f;
    MeterResult mist;
    mist.valid = true;
    mist.mean = -3.37f;
    mist.highlight = -0.58f;
    const float clear = TargetExposureLog2(mist, 0.0f, t);
    const float grey = TargetExposureLog2(mist, 0.0f, t, 0.0f, 0.0f, 0.0f, 1.0f);
    CHECK(kGreyHighlightLimit < kHighlightLimit);
    CHECK(kGreyHighlightLimit > 0.5f); // a light grey, not a dark one
    CHECK(grey < clear);
    // The bright end lands near the grey limit (the guard's pull, not a clamp).
    const float bright = grey + mist.highlight;
    CHECK(bright < std::log2(kHighlightLimit));
    CHECK(bright == Catch::Approx(std::log2(kGreyHighlightLimit)).margin(0.15));
    // Half a veil, halfway; no veil, unchanged.
    const float half = TargetExposureLog2(mist, 0.0f, t, 0.0f, 0.0f, 0.0f, 0.5f);
    CHECK(half < clear);
    CHECK(half > grey);
    CHECK(TargetExposureLog2(mist, 0.0f, t, 0.0f, 0.0f, 0.0f, 0.0f) == Catch::Approx(clear));
    // A scene whose bright end is under the grey limit is not touched.
    MeterResult dim = mist;
    dim.highlight = mist.mean + 0.3f;
    CHECK(TargetExposureLog2(dim, 0.0f, t, 0.0f, 0.0f, 0.0f, 1.0f) == Catch::Approx(TargetExposureLog2(dim, 0.0f, t)));
    // At night the bright end is a lamp: the veil leaves the night's guard alone.
    MeterResult night = mist;
    night.mean = -9.0f;
    night.highlight = -2.0f;
    CHECK(TargetExposureLog2(night, 1.0f, t, 0.0f, 0.0f, 0.0f, 1.0f) ==
          Catch::Approx(TargetExposureLog2(night, 1.0f, t)));
}

TEST_CASE("PhotoCamera: a low sun in frame holds the sky by it golden, not blown", "[Remaster][PhotoCamera]")
{
    // A meter grid with the sun at uv (0.5, 0.6): its own block very bright
    // (the disc), the blocks a little way out the glow, the rest dim.
    constexpr int n = kMeterWidth * kMeterHeight;
    std::vector<float> peak(n, -4.0f), w(n, 1.0f);
    const float su = 0.5f, sv = 0.6f, aspect = 16.0f / 9.0f;
    for (int y = 0; y < kMeterHeight; ++y)
        for (int x = 0; x < kMeterWidth; ++x)
        {
            const float dx = ((x + 0.5f) / kMeterWidth - su) * aspect, dy = (y + 0.5f) / kMeterHeight - sv;
            const float d = std::sqrt(dx * dx + dy * dy);
            if (d < 0.02f)
                peak[y * kMeterWidth + x] = 4.0f; // the disc
            else if (d < 0.1f)
                peak[y * kMeterWidth + x] = 1.5f; // the glow
        }
    // The ring sees the glow, not the disc.
    CHECK(SunRingPeak(peak.data(), w.data(), su, sv, aspect) == Catch::Approx(1.5f));
    // No sun near: nothing.
    CHECK(SunRingPeak(peak.data(), w.data(), 0.05f, 0.05f, aspect) == Catch::Approx(-4.0f));
    // The guard brings the exposure down to keep the glow at kSunRingLimit.
    LightingTuning t;
    MeterResult m;
    m.valid = true;
    m.mean = -6.0f;
    m.highlight = -3.0f;
    const float free = TargetExposureLog2(m, 0.0f, t);
    m.sunRing = 1.5f;
    m.sunRingWeight = 1.0f;
    const float held = TargetExposureLog2(m, 0.0f, t);
    CHECK(held < free);
    CHECK(held + m.sunRing == Catch::Approx(std::log2(kSunRingLimit)).margin(0.01));
    // A high sun does not hold it.
    CHECK(SunRingWeight(60.0f) == 0.0f);
    CHECK(SunRingWeight(5.0f) == 1.0f);
}

TEST_CASE("PhotoCamera: the sky rolls off under white, and the guards' sky limits sit under the cap",
          "[Remaster][PhotoCamera]")
{
    CHECK(SkyRolloff(0.5f) == 0.5f);
    CHECK(SkyRolloff(kSkyRolloffKnee) == Catch::Approx(kSkyRolloffKnee));
    float prev = kSkyRolloffKnee;
    for (float m = 1.3f; m < 10.0f; m *= 1.5f)
    {
        const float r = SkyRolloff(m);
        CHECK(r > prev);
        CHECK(r < kSkyRolloffCap);
        prev = r;
    }
    CHECK(SkyRolloff(1000.0f) <= kSkyRolloffCap);
    // The cap shows under white on the curve.
    LightingTuning t;
    const ToneCurve c = MakeToneCurve(t.camContrast, t.camShoulder, t.camWhite);
    CHECK(ApplyToneCurve(c, kSkyRolloffCap) < 0.9f);
    CHECK(ApplyToneCurve(c, kSkyRolloffCap) > 0.75f);
    // The guards on the sky can still see it.
    CHECK(kSunRingLimit < kSkyRolloffCap);
    CHECK(kSkyTopLimit < kSkyRolloffCap);
    CHECK(kSunDiscLevel > c.white);
}

TEST_CASE("PhotoCamera: a low sun's warm glow is a band along the horizon, not a disc round the sun",
          "[Remaster][PhotoCamera]")
{
    const float deg = 3.14159265f / 180.0f;
    const float sunEl = 5.0f * deg;
    // At the sun, and the horizon band under it is as close as the sun's own
    // height (the glow hugs the horizon); off to the side, the azimuth.
    CHECK(LowSunGlowCos(sunEl, sunEl, 1.0f) == Catch::Approx(1.0f).margin(1e-5));
    CHECK(LowSunGlowCos(sunEl, 0.0f, 1.0f) == Catch::Approx(1.0f).margin(1e-5));
    CHECK(LowSunGlowCos(sunEl, 0.0f, std::cos(20.0f * deg)) ==
          Catch::Approx(LowSunGlowCos(sunEl, sunEl, std::cos(20.0f * deg))).margin(1e-5));
    // The glow mask: none within about 10 degrees (cos 0.985), all from 30
    // (cos 0.86). 15 degrees to the side at the sun's height stays warm; 15
    // degrees straight above the sun is out of the glow (it was inside).
    CHECK(LowSunGlowCos(sunEl, sunEl, std::cos(15.0f * deg)) > 0.95f);
    CHECK(std::cos(15.0f * deg) > 0.95f);
    CHECK(LowSunGlowCos(sunEl, 20.0f * deg, 1.0f) < 0.86f);
    // Wider than it is tall: the same angle along the horizon keeps more glow.
    for (float a = 4.0f; a <= 30.0f; a += 2.0f)
    {
        const float side = LowSunGlowCos(sunEl, sunEl, std::cos(a * deg));
        const float up = LowSunGlowCos(sunEl, sunEl + a * deg, 1.0f);
        INFO("angle " << a);
        CHECK(side > up);
    }
    // Never past straight up.
    CHECK(LowSunGlowCos(sunEl, 80.0f * deg, 1.0f) == Catch::Approx(std::sin(sunEl)).margin(1e-4));
    CHECK(kLowSunGlowFlat >= 2.0f);
    CHECK(kLowSunGlowFlat <= 4.0f);
}

TEST_CASE("PhotoCamera: toward the sun the sky burns white, its glare falls off smoothly to the sky's cap",
          "[Remaster][PhotoCamera]")
{
    LightingTuning t;
    const ToneCurve c = MakeToneCurve(t.camContrast, t.camShoulder, t.camWhite);
    const float deg = 3.14159265f / 180.0f;
    // At the sun the cap is past the curve's white: the sky round the disc
    // clips (no flat cream plateau of 0.85 round a low sun).
    CHECK(SkySunGlow(1.0f) == Catch::Approx(1.0f));
    CHECK(SkyRolloffCapToward(1.0f) == Catch::Approx(kSkySunCap));
    CHECK(kSkySunCap > c.white);
    CHECK(ApplyToneCurve(c, SkyRolloffToward(1000.0f, std::cos(0.5f * deg))) >= 0.999f);
    // Half the rise at kSkySunGlowRad.
    CHECK(SkySunGlow(std::cos(kSkySunGlowRad)) == Catch::Approx(0.5f).margin(0.01f));
    // A bright aureole's output falls with the angle, smoothly: no step of
    // more than a percent of white per tenth of a degree, and no plateau of
    // the same value from 0.6 to 3 degrees.
    float prev = 2.0f;
    for (float a = 0.6f; a <= 6.0f; a += 0.1f)
    {
        const float y = ApplyToneCurve(c, SkyRolloffToward(200.0f, std::cos(a * deg)));
        CHECK(y <= prev + 1e-6f);
        if (prev <= 1.0f)
            CHECK(prev - y < 0.01f);
        prev = y;
    }
    CHECK(ApplyToneCurve(c, SkyRolloffToward(200.0f, std::cos(0.6f * deg))) -
              ApplyToneCurve(c, SkyRolloffToward(200.0f, std::cos(3.0f * deg))) >
          0.05f);
    // Far from the sun the cap is the sky's (within 2% from 20 degrees): the
    // broad sky, a silver lining and the guards are unchanged.
    CHECK(SkyRolloffCapToward(std::cos(20.0f * deg)) < kSkyRolloffCap * 1.02f);
    CHECK(SkyRolloffCapToward(-1.0f) < kSkyRolloffCap * 1.001f);
    // Under the knee nothing changes, at the sun as anywhere.
    CHECK(SkyRolloffToward(0.5f, 1.0f) == 0.5f);
    CHECK(SkyRolloffToward(3.0f, -1.0f) == Catch::Approx(SkyRolloff(3.0f)).epsilon(0.002));
    // The sun's disc goes most of the way to white (a sensor clips every
    // channel of the sun).
    CHECK(kSunDiscWhite >= 0.7f);
    CHECK(kSunDiscWhite <= 1.0f);
}

TEST_CASE("PhotoCamera: look fix exposure: the meter reads the sky round the sun at the plain roll-off",
          "[Remaster][PhotoCamera]")
{
    const float deg = 3.14159265f / 180.0f;
    // The sky's stored light toward the sun, read back, is the plain
    // roll-off of the same light at any angle: the burn is the display's, not
    // the meter's.
    for (float a : {0.3f, 1.0f, 2.0f, 3.0f, 5.0f, 8.0f, 20.0f, 90.0f})
    {
        const float c = std::cos(a * deg);
        for (float m = 0.2f; m < 60.0f; m *= 1.3f)
        {
            CAPTURE(a, m);
            const float stored = SkyRolloffToward(m, c);
            const float read = SkyMeterLight(stored, c);
            const float plain = SkyRolloff(m);
            // log2 within 0.02 EV until the plain roll-off is within a
            // hundredth of its cap (where float precision of the inverse
            // runs out, and the reading saturates at the cap either way).
            if (plain < kSkyRolloffCap - 0.024f)
                CHECK(std::fabs(std::log2(read / plain)) < 0.02f);
            CHECK(read <= kSkyRolloffCap + 1e-4f);
            CHECK(read <= stored + 1e-4f);
        }
    }
    // Under the knee and far from the sun nothing changes.
    CHECK(SkyMeterLight(0.9f, 1.0f) == 0.9f);
    CHECK(SkyMeterLight(2.0f, -1.0f) == Catch::Approx(2.0f).epsilon(0.002));
    // The disc and anything past the cap toward the sun read as the plain cap.
    CHECK(SkyMeterLight(kSunDiscLevel, 1.0f) == Catch::Approx(kSkyRolloffCap).epsilon(0.001));
    CHECK(SkyMeterLight(kSkySunCap * 2.0f, std::cos(0.5f * deg)) == Catch::Approx(kSkyRolloffCap).epsilon(0.001));

    // The camera into a low sun (woods_pine2_evening, lowsun_fp): the sky a
    // few degrees from the sun is many times past the cap. Closed loop: the
    // frame is drawn at the exposure, the meter reads it back, the guard on
    // the sky by the sun sets the next exposure. Reading the dome's own
    // stored light, the burn's higher cap held the guard's tap past its limit
    // and the camera closed down; read through SkyMeterLight it settles where
    // the plain roll-off settled.
    LightingTuning t;
    auto settle = [&](auto stored, auto read)
    {
        float e = 2.0f;
        for (int i = 0; i < 200; ++i)
        {
            MeterResult m;
            m.valid = true;
            m.mean = -7.7f;
            m.highlight = -4.0f;
            // The ring's brightest tap: sky 4 degrees from the sun, scene light 6.
            const float c = std::cos(4.0f * deg);
            const float pre = 6.0f * std::exp2(e);
            m.sunRing = std::log2(read(stored(pre, c), c)) - e;
            m.sunRingWeight = 0.72f;
            e = TargetExposureLog2(m, 0.0f, t);
        }
        return e;
    };
    auto towardSun = [](float pre, float c) { return SkyRolloffToward(pre, c); };
    auto plainSky = [](float pre, float) { return SkyRolloff(pre); };
    auto dome = [](float stored, float) { return stored; };
    auto meter = [](float stored, float c) { return SkyMeterLight(stored, c); };
    const float round3 = settle(plainSky, dome);
    const float burnt = settle(towardSun, dome);
    const float fixed = settle(towardSun, meter);
    CAPTURE(round3, burnt, fixed);
    CHECK(burnt < round3 - 0.3f); // the cause: the meter read the burn
    CHECK(fixed == Catch::Approx(round3).margin(0.05f));
}

TEST_CASE("PhotoCamera: the sky seen through a canopy stays pale sky, the sun's own glare may clip",
          "[Remaster][PhotoCamera]")
{
    constexpr int n = kMeterWidth * kMeterHeight;
    std::vector<float> sky(n, -32.0f); // no sky in most blocks (a canopy)
    // 3 % of the blocks show bright sky through gaps.
    for (int i = 0; i < n * 3 / 100; ++i)
        sky[(i * 37) % n] = 3.0f;
    CHECK(SkyTopPeak(sky.data(), false, 0.5f, 0.5f, 16.0f / 9.0f) == Catch::Approx(3.0f));
    // A single bright block (the sun) is under the percentile; and a sun in
    // front of the camera leaves its own neighbourhood out.
    std::vector<float> sun(n, -32.0f);
    sun[(kMeterHeight / 2) * kMeterWidth + kMeterWidth / 2] = 8.0f;
    CHECK(SkyTopPeak(sun.data(), false, 0.5f, 0.5f, 16.0f / 9.0f) < 0.0f);
    // The guard pulls the exposure toward keeping that bright sky at kSkyTopLimit.
    LightingTuning t;
    MeterResult m;
    m.valid = true;
    m.mean = -7.0f;
    m.highlight = -3.0f;
    const float free = TargetExposureLog2(m, 0.0f, t);
    m.skyTop = 3.0f;
    const float held = TargetExposureLog2(m, 0.0f, t);
    CHECK(held < free);
    CHECK(held > std::log2(kSkyTopLimit) - m.skyTop - 0.01f);
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
    // Far past white a white light is white; a saturated orange rolls off to a
    // yellow-orange, its blue following last (kChromaKeep, kHueRoll; one
    // mix for all three channels turned a flame's core cream).
    const Rgb white(0.18f, 0.18f, 0.17f);
    const Rgb whiteHot = ApplyToneCurveHueKeeping(c, white * 40.0f);
    CHECK(whiteHot.b == Catch::Approx(1.0f).margin(0.01));
    const Rgb hot = ApplyToneCurveHueKeeping(c, orange * 40.0f);
    CHECK(hot.r == Catch::Approx(1.0f).margin(0.01));
    CHECK(hot.g / hot.r > 0.5f);
    CHECK(hot.b < 0.5f * hot.g);
    // A flame bright but under the curve's white keeps most of its hue (the
    // old y^4 path to white turned it white).
    const Rgb flame = ApplyToneCurveHueKeeping(c, orange * 8.0f);
    CHECK(flame.b < 0.5f * flame.r);
    CHECK(flame.g < 0.8f * flame.r);
    // A flame's core a stop past white: yellow-orange, the blue moving least
    // (the single mix gave blue at half the red).
    const Rgb flameCore(1.0f, 0.55f, 0.16f);
    const Rgb core = ApplyToneCurveHueKeeping(c, flameCore * (kMiddleGrey * 32.0f));
    CHECK(core.g / core.r < 0.75f);
    CHECK(core.b / core.r < 0.4f);
    CHECK(core.b < 0.5f * core.g);
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
    // The clear sky is the table's radiance at the rig's scale.
    CHECK(rig.skyScale ==
          Catch::Approx(PhysicalRadianceToDisplay(1.0f / 1.6f / in.physical.referenceIrradiance) * t.camSky)
              .epsilon(1e-3));
    CHECK(grey.skyScale == Catch::Approx(rig.skyScale).epsilon(1e-3));
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
                            "camWb", "camSky", "camSkyHue", "camNight", "camVignette", "camBloom", "camHaze",
                            "camScotopic", "camAdditive", "camAerial", "camLocal"})
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

TEST_CASE("PhotoCamera: the camera's haze brightens and whitens the noon horizon against the zenith",
          "[Remaster][PhotoCamera]")
{
    // The sky table's clean reference air leaves the noon horizon about as
    // bright as the zenith; the camera's haze (camHaze) makes it brighter and
    // paler, as a summer sky is.
    LightingTuning t;
    const AtmosphereTables hazy = BuildAtmosphereTables(t.camHaze);
    REQUIRE(hazy.Valid());
    const float sun = 60.0f * kPi / 180.0f;
    auto radiance = [&](const AtmosphereTables& a, int row)
    {
        // The azimuthal mean of the row, Rayleigh and Mie (with its phase).
        Rgb sum;
        const float sinS = std::sin(sun), cosS = std::cos(sun);
        const float e = SkyViewRowElevation(row);
        for (int col = 0; col < 64; col += 3)
        {
            const float az = SkyViewColumnAzimuth(col);
            const float cosToSun = std::sin(e) * sinS + std::cos(e) * cosS * std::cos(az);
            sum = sum + SkyViewTexel(a, 0, col, row, sun) +
                  SkyViewTexel(a, 1, col, row, sun) * AtmosphereMiePhase(cosToSun);
        }
        return sum;
    };
    const Rgb hz = radiance(hazy, 34), zen = radiance(hazy, 63);
    const Rgb hz1 = radiance(Tables(), 34), zen1 = radiance(Tables(), 63);
    INFO("hazy horizon/zenith " << Luminance(hz) / Luminance(zen) << " r/b " << RedOverBlue(hz) << " / "
                                << RedOverBlue(zen) << "; clean " << Luminance(hz1) / Luminance(zen1) << " r/b "
                                << RedOverBlue(hz1) << " / " << RedOverBlue(zen1));
    // The horizon well above the zenith (photographs of a clear day show 2 to
    // 4 x), paler than the zenith and paler than the clean air's horizon.
    CHECK(Luminance(hz) / Luminance(zen) > 1.3f);
    CHECK(RedOverBlue(hz) > RedOverBlue(zen) * 1.2f);
    CHECK(RedOverBlue(hz) > RedOverBlue(hz1));
}

TEST_CASE("PhotoCamera: critic round 2: the histogram meter, the low sun's balance, the weather's veil and the night",
          "[Remaster][PhotoCamera]")
{
    // The meter is a histogram of 1/32 EV bins (no sort on the render thread):
    // the mean between the cuts of a ramp is its own within a bin.
    std::vector<float> l(2304), w(2304, 1.0f);
    for (size_t i = 0; i < l.size(); ++i)
        l[i] = -8.0f + 6.0f * static_cast<float>(i) / static_cast<float>(l.size() - 1);
    const MeterResult m = Meter(l.data(), w.data(), nullptr, static_cast<int>(l.size()));
    REQUIRE(m.valid);
    // The ramp's mean between the 5 % and 97 % cuts.
    CHECK(m.mean == Catch::Approx(-8.0f + 6.0f * 0.51f).margin(1.0f / 32.0f));
    CHECK(m.low == Catch::Approx(-8.0f + 6.0f * 0.05f).margin(1.0f / 32.0f));
    CHECK(m.high == Catch::Approx(-8.0f + 6.0f * 0.97f).margin(1.0f / 32.0f));
    // A daylight balance at a low sun (the golden hour stays golden), most of
    // the light's colour taken out at a high one.
    CHECK(WhiteBalanceShare(5.0f) == Catch::Approx(kWhiteBalanceShareLowSun));
    CHECK(WhiteBalanceShare(60.0f) == Catch::Approx(kWhiteBalanceShareHighSun));
    // The veil: none under the clearest sky the engine has nor under the
    // broken cloud of setOvercast 0.5 (its blue gaps and its sun stay), part of it under a thickening deck, all of it under
    // a full one.
    CHECK(CameraWeatherVeil(0.959f) == 0.0f);
    CHECK(CameraWeatherVeil(0.7f) == 0.0f);
    CHECK(CameraWeatherVeil(0.45f) > 0.3f);
    CHECK(CameraWeatherVeil(0.45f) < 0.8f);
    CHECK(CameraWeatherVeil(0.1f) == 1.0f);
    // Under the veil the sun loses its direct light to an even grey sky.
    LightingTuning t;
    const LightRig clear = ComputeLightRig(PhysicalInputs(50.0f, 0.959f), t);
    const LightRig grey = ComputeLightRig(PhysicalInputs(50.0f, 0.3f), t);
    CHECK(Luminance(grey.sun) < 0.6f * Luminance(clear.sun));
    CHECK(Luminance(grey.skyZenith) > Luminance(clear.skyZenith));
    // The aerosol on the sun's path: a 10 degree sun is warmer than without it
    // would be at the ground (red over blue well above the noon sun's).
    const LightRig low = ComputeLightRig(PhysicalInputs(10.0f), t);
    const LightRig noon = ComputeLightRig(PhysicalInputs(60.0f), t);
    CHECK(RedOverBlue(low.sun) > 1.8f * RedOverBlue(noon.sun));
    // The night sits lower than before (camNight, the night's lift).
    CHECK(t.camNight <= -3.0f);
    CHECK(kExposureNightMaxLog2 <= -0.5f);
    // The camera turns the screen-space shafts off (their fan round a low sun).
    PostFrameInputs pin;
    pin.camera = true;
    pin.worldDepth = true;
    pin.glow = true;
    pin.sunShafts = 1.0f;
    pin.shaftWeight = 1.0f;
    CHECK_FALSE(PlanPostStages(pin).Runs(PostStage::Shafts));
    pin.camera = false;
    CHECK(PlanPostStages(pin).Runs(PostStage::Shafts));
}
TEST_CASE("PhotoCamera: the golden hour warms the light the low sun shows and leaves the blue sky blue",
          "[Remaster][PhotoCamera]")
{
    // The warm balance over the whole frame turned the clear sky and its clouds
    // teal-grey ((129, 148, 163) over a forest edge at the golden hour). The
    // gain: none out of the golden hour, kCameraGoldenWarm at unit luminance in
    // it.
    const Rgb none = GoldenWarmGain(0.0f);
    CHECK(none.r == 1.0f);
    CHECK(none.g == 1.0f);
    CHECK(none.b == 1.0f);
    const Rgb full = GoldenWarmGain(1.0f);
    CHECK(Luminance(full) == Catch::Approx(1.0f).epsilon(1e-4));
    CHECK(full.r > 1.05f);
    CHECK(full.b < 0.8f);
    CHECK(GoldenWarmGain(2.0f).r == Catch::Approx(full.r));
    // The share: a clear sky's blue takes none, a sunlit field all of it, a
    // grey cloud most of it; it rises with the warmth.
    CHECK(GoldenWarmShare(Rgb(0.20f, 0.30f, 0.49f)) < 0.05f);
    CHECK(GoldenWarmShare(Rgb(0.10f, 0.20f, 0.50f)) == 0.0f);
    CHECK(GoldenWarmShare(Rgb(0.28f, 0.17f, 0.05f)) == 1.0f);
    const float grey = GoldenWarmShare(Rgb(0.5f, 0.5f, 0.5f));
    CHECK(grey > 0.5f);
    CHECK(grey < 1.0f);
    CHECK(GoldenWarmShare(Rgb(0.45f, 0.5f, 0.55f)) < grey);
    CHECK(GoldenWarmShare(Rgb(0.0f, 0.0f, 0.0f)) > 0.0f);
}

TEST_CASE("PhotoCamera: look fix noon: a bright plain at a high sun is exposed a little under, not dull",
          "[Remaster][PhotoCamera]")
{
    // An earlier tuning put a bright plain 0.38 EV under on top of the high sun's
    // step: the Sinai sand sat 35 levels under the original and the clear
    // sky over it went slate. Now a plain that meters about 0.84 EV above the
    // average scene sits between a quarter and under half a
    // stop under the average scene's grey, the high sun's step included.
    LightingTuning t;
    t.camEv = 0.0f;
    MeterResult plain;
    plain.valid = true;
    plain.mean = kDaylightMeanLog2 + 0.84f;
    plain.highlight = plain.mean + 1.0f;
    const float timeEv = CameraTimeOfDayEv(60.0f, 0.0f);
    const float under = TargetExposureLog2(plain, 0.0f, t, 0.0f, 0.0f, timeEv) - (std::log2(kMiddleGrey) - plain.mean);
    CHECK(under < -0.2f);
    CHECK(under > -0.45f);
    // Still under the average scene: a bright plain is not pulled up to grey.
    CHECK(kBrightKeep <= 0.0f);
}

TEST_CASE("PhotoCamera: look fix noon: the clear day sky deepens toward a camera's blue, the horizon stays pale",
          "[Remaster][PhotoCamera]")
{
    // The table's noon zenith (linear, the slate (91, 114, 148) of an earlier tuning).
    const Rgb zenith(0.104f, 0.170f, 0.302f);
    const float w = DaySkyDeepWeight(60.0f, 0.5f, 0.3f, 0.0f);
    CHECK(w == Catch::Approx(1.0f));
    const Rgb deep = DaySkyDeepen(zenith, w);
    // Same luminance, more chroma, the same dominant hue (the chroma vector
    // is scaled, not turned), no channel under the floor.
    const float l = Luminance(zenith);
    CHECK(Luminance(deep) == Catch::Approx(l).epsilon(1e-4));
    CHECK(deep.b - deep.r > 1.5f * (zenith.b - zenith.r));
    CHECK((deep.g - l) / (deep.b - l) == Catch::Approx((zenith.g - l) / (zenith.b - l)).epsilon(1e-4));
    CHECK(std::min(deep.r, std::min(deep.g, deep.b)) >= kCameraDaySkyFloor * l - 1e-6f);
    // In display terms a deep, clean blue: red well under half of blue.
    CHECK(std::pow(deep.r, 1.0f / 2.2f) < 0.45f * std::pow(deep.b, 1.0f / 2.2f));
    // A strongly coloured sky deepens only to the floor (no neon), and one
    // already past it is left as it is.
    const Rgb blue(0.06f, 0.12f, 0.50f);
    const Rgb held = DaySkyDeepen(blue, 1.0f);
    CHECK(held.b > blue.b);
    CHECK(std::min(held.r, std::min(held.g, held.b)) >= kCameraDaySkyFloor * Luminance(blue) - 1e-5f);
    const Rgb neon(0.02f, 0.10f, 0.60f);
    CHECK(DaySkyDeepen(neon, 1.0f).r == Catch::Approx(neon.r));
    // Grey and black stay as they are; no weight is the identity.
    const Rgb grey(0.3f, 0.3f, 0.3f);
    CHECK(DaySkyDeepen(grey, 1.0f).r == Catch::Approx(0.3f));
    CHECK(DaySkyDeepen(grey, 1.0f).b == Catch::Approx(0.3f));
    CHECK(DaySkyDeepen(Rgb(0.0f, 0.0f, 0.0f), 1.0f).b == 0.0f);
    CHECK(DaySkyDeepen(zenith, 0.0f).r == zenith.r);
    // The weight: none at the horizon (the pale band), at a low sun (golden
    // hour, dusk, night keep their own sky), in the sun's glow, under the
    // weather's veil; it grows with the view's elevation.
    CHECK(DaySkyDeepWeight(60.0f, 0.0f, 0.3f, 0.0f) == 0.0f);
    CHECK(DaySkyDeepWeight(60.0f, 0.1f, 0.3f, 0.0f) < DaySkyDeepWeight(60.0f, 0.3f, 0.3f, 0.0f));
    CHECK(DaySkyDeepWeight(6.0f, 0.5f, 0.3f, 0.0f) == 0.0f);
    CHECK(DaySkyDeepWeight(-10.0f, 0.5f, 0.3f, 0.0f) == 0.0f);
    CHECK(DaySkyDeepWeight(12.0f, 0.5f, 0.3f, 0.0f) < 0.2f);
    CHECK(DaySkyDeepWeight(60.0f, 0.5f, 0.99f, 0.0f) == 0.0f);
    CHECK(DaySkyDeepWeight(60.0f, 0.5f, 0.3f, 1.0f) == 0.0f);
    CHECK(DaySkyDeepWeight(60.0f, 0.5f, 0.3f, 0.5f) == Catch::Approx(0.5f));
}

TEST_CASE("PhotoCamera: look fix particles: smoke and dust take the rig's light, not the classic sun",
          "[Remaster][PhotoCamera]")
{
    // A lit particle (Object::DrawDecal) under the camera: kParticleKeyShare of
    // the key light (a thick puff's lit half, about a sunlit wall's share) plus
    // the sky's irradiance averaged over every direction, the sky term times
    // the world's shade share.
    LightingTuning t{};
    LightRigInputs in = PhysicalInputs(60.0f);
    LightRig noon = ComputeLightRig(in, t);
    REQUIRE(noon.physical);
    const KeyLight key = ComputeKeyLight(in, noon, 0.0f);
    const Rgb irr = ParticleRigIrradiance(noon, key, 1.0f);
    const Rgb sky = (noon.skyZenith + noon.skyHorizon + noon.skyHorizonSun + noon.skyGround) * 0.25f;
    CHECK(irr.r == Catch::Approx(noon.sun.r * kParticleKeyShare + sky.r));
    CHECK(irr.g == Catch::Approx(noon.sun.g * kParticleKeyShare + sky.g));
    CHECK(irr.b == Catch::Approx(noon.sun.b * kParticleKeyShare + sky.b));
    // A noon puff is lit less than a white up-facing surface in the sun, more
    // than the shade: the sun dominates it.
    const float upLit = Luminance(noon.sun) * std::sin(60.0f * kPi / 180.0f) + Luminance(noon.skyZenith);
    CHECK(Luminance(irr) < upLit);
    CHECK(Luminance(irr) > 0.6f * upLit);
    CHECK(Luminance(irr) > Luminance(noon.skyZenith));
    CHECK(Luminance(noon.sun) * kParticleKeyShare > Luminance(sky));
    // The shade share scales the sky's part only.
    const Rgb shaded = ParticleRigIrradiance(noon, key, 0.5f);
    CHECK(shaded.g == Catch::Approx(noon.sun.g * kParticleKeyShare + 0.5f * sky.g));
    // The cloud field's mean dims the sun's part as it dims the open ground.
    LightRig cloudy = noon;
    cloudy.cloudCover = 1.0f;
    cloudy.cloudTransmit = 0.08f;
    const Rgb under = ParticleRigIrradiance(cloudy, key, 1.0f);
    CHECK(under.g == Catch::Approx(noon.sun.g * kParticleKeyShare * 0.08f + sky.g));

    // firefight_3: 19:35, the sun 1.7 degrees under the horizon. The classic
    // model still shines its sunset sun (pink) on every puff; the rig's sun has
    // set, so the burst's dust takes the dusk sky's light alone: dim and not
    // warmer than the sky that lights it (the camera balances that sky to
    // white), so the foot reads grey against the twilight, not salmon.
    LightRigInputs duskIn = PhysicalInputs(-1.7f);
    duskIn.classicSunDiffuse = Rgb(0.6f, 0.3f, 0.25f);
    duskIn.nightEffect = 0.3f;
    LightRig dusk = ComputeLightRig(duskIn, t);
    REQUIRE(dusk.physical);
    const KeyLight duskKey = ComputeKeyLight(duskIn, dusk, 0.0f);
    CHECK(Luminance(duskKey.radiance) == 0.0f);
    const Rgb duskIrr = ParticleRigIrradiance(dusk, duskKey, 1.0f);
    CHECK(Luminance(duskIrr) > 0.0f);
    CHECK(Luminance(duskIrr) < 0.1f * Luminance(irr));
    CHECK(RedOverBlue(duskIrr) <= RedOverBlue(dusk.skyZenith + dusk.skyHorizon + dusk.skyHorizonSun) * 1.02f);
    CHECK(RedOverBlue(duskIrr) < 1.1f);

    // Under the moon key the moon's key share and its shadowless rest both
    // light the puff at the key share (no cloud field on the moon).
    LightRig night = noon;
    night.sun = Rgb();
    night.moon = Rgb(0.01f, 0.011f, 0.012f);
    KeyLight moonKey;
    moonKey.moonShare = 0.6f;
    moonKey.radiance = night.moon * 0.6f;
    night.moon = night.moon * 0.4f;
    night.cloudCover = 1.0f;
    night.cloudTransmit = 0.08f;
    const Rgb nightSky = (night.skyZenith + night.skyHorizon + night.skyHorizonSun + night.skyGround) * 0.25f;
    const Rgb moonLit = ParticleRigIrradiance(night, moonKey, 1.0f);
    CHECK(moonLit.b == Catch::Approx(0.012f * kParticleKeyShare + nightSky.b));
}
