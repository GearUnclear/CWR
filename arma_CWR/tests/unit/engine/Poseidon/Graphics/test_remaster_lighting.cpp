#include <catch2/catch_approx.hpp>
#include <catch2/catch_message.hpp>
#include <catch2/catch_test_macros.hpp>

#include <glslang/Public/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>

#include <Poseidon/Graphics/Remaster/CloudNoise.hpp>
#include <Poseidon/Graphics/Remaster/LightRig.hpp>
#include <Poseidon/Graphics/Remaster/TerrainHorizon.hpp>
#include <PoseidonGL33/EngineGL33_Lighting.hpp>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <limits>
#include <map>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

// Remastered lighting (Graphics/Remaster + EngineGL33_Lighting*): the pure light
// rig, horizon map and cloud noise math, and the contracts between the C++
// uploads and the GLSL that reads them. The look itself is judged in the
// offline lab (tests/rendering/lighting_lab).

using namespace Poseidon::remaster;

namespace
{
constexpr float kPi = 3.14159265358979323846f;
constexpr float kBrightness = 1.6f; // default user brightness carried by the materials

LightRigInputs ClearSky(float elevationDeg, float azimuthDeg = 200.0f)
{
    LightRigInputs in;
    const float e = elevationDeg * kPi / 180.0f;
    const float a = azimuthDeg * kPi / 180.0f;
    in.towardSun[0] = std::cos(e) * std::sin(a);
    in.towardSun[1] = std::sin(e);
    in.towardSun[2] = std::cos(e) * std::cos(a);
    in.fogColor = Rgb(0.73f, 0.79f, 0.85f);
    in.skyThrough = 1.0f;
    in.nightEffect = elevationDeg > 5.0f ? 0.0f : 1.0f;
    return in;
}

// On-screen fraction of the texture brightness for a surface that receives
// irradiance `e` (rig units), before the display gamma pass.
float Shown(float e, float exposure)
{
    return ToDisplay(e * kBrightness * exposure);
}

float RedOverBlue(const Rgb& c)
{
    return c.r / std::max(c.b, 1e-6f);
}

// skyThrough for a setOvercast value: Weather::SetOvercast interpolates the
// GetWeatherBasics keyframes (Landscape.cpp): overcast -0.08 "jasno" 1.0, 0.5
// "oblacno" 0.7, 0.66 "zatazeno" 0.1, 1.0 0.0.
float SkyThroughAtOvercast(float overcast)
{
    static const float kKeys[4][2] = {{-0.08f, 1.0f}, {0.5f, 0.7f}, {0.66f, 0.1f}, {1.0f, 0.0f}};
    for (int i = 1; i < 4; ++i)
    {
        if (overcast < kKeys[i][0])
        {
            const float t = (overcast - kKeys[i - 1][0]) / (kKeys[i][0] - kKeys[i - 1][0]);
            return kKeys[i - 1][1] + (kKeys[i][1] - kKeys[i - 1][1]) * t;
        }
    }
    return 0.0f;
}
} // namespace

TEST_CASE("Remaster light rig: sunlit ground keeps its classic brightness through the day", "[Graphics][remaster]")
{
    const LightingTuning tuning;
    for (float elevation : {60.0f, 45.0f, 30.0f})
    {
        CAPTURE(elevation);
        const LightRigInputs in = ClearSky(elevation);
        const LightRig rig = ComputeLightRig(in, tuning);
        const float sinSun = in.towardSun[1];
        const float lit = Luminance(rig.sun) * sinSun + Luminance(rig.skyZenith);
        const float shade = Luminance(rig.skyZenith);
        // Classic clamps lit ground to the bare texture (1.0 on screen).
        CHECK(Shown(lit, rig.exposure) > 0.9f);
        CHECK(Shown(lit, rig.exposure) < 1.06f);
        // Cast shadow: bold but not crushed (reviewers rejected ~0.42).
        const float ratio = Shown(shade, rig.exposure) / Shown(lit, rig.exposure);
        CHECK(ratio > 0.5f);
        CHECK(ratio < 0.75f);
    }
}

TEST_CASE("Remaster light rig: golden hour is darker, warmer and more contrasty than noon", "[Graphics][remaster]")
{
    const LightingTuning tuning;
    const LightRig noon = ComputeLightRig(ClearSky(60.0f), tuning);
    const LightRigInputs lowIn = ClearSky(7.0f);
    const LightRig low = ComputeLightRig(lowIn, tuning);
    const float noonGround = Luminance(noon.sun) * std::sin(60.0f * kPi / 180.0f) + Luminance(noon.skyZenith);
    const float lowGround = Luminance(low.sun) * lowIn.towardSun[1] + Luminance(low.skyZenith);
    CHECK(Shown(lowGround, low.exposure) < 0.8f * Shown(noonGround, noon.exposure));
    // The sun is golden and the shade facing away from it stays cool.
    CHECK(RedOverBlue(low.sun) > 1.6f);
    CHECK(RedOverBlue(noon.sun) < 1.1f);
    CHECK(RedOverBlue(low.skyHorizonSun) > RedOverBlue(low.skyHorizon));
    // A wall facing a 7 degree sun is lit well above one facing away from it.
    const float sunWall = Luminance(low.sun) * std::cos(7.0f * kPi / 180.0f) + Luminance(low.skyHorizonSun);
    CHECK(sunWall > 4.0f * Luminance(low.skyHorizon));
}

TEST_CASE("Remaster light rig: evening sun keeps the original's sunset hue on screen", "[Graphics][remaster]")
{
    const LightingTuning tuning;
    // LightSun at 7 degrees: colorFull between FullSunColor and SunsetColor,
    // ambient = BackgroundColor (0.55, 0.6, 0.8) * sinSun * 1.5.
    const Rgb classicSunset(0.814f, 0.43f, 0.279f);
    const float ambientI = std::sin(7.0f * kPi / 180.0f) * 1.5f;
    const Rgb classicAmbient(0.55f * ambientI, 0.6f * ambientI, 0.8f * ambientI);
    LightRigInputs in = ClearSky(7.0f);
    in.classicSunDiffuse = classicSunset;

    // A hull side half-turned to the sun (N.L 0.5, 60 degrees off its
    // azimuth): classic clamps sun + ambient; remastered adds the sky that a
    // vertical surface facing that way sees (shader skyIrradiance).
    constexpr float kNdotL = 0.5f;
    auto classicHull = [&](float sun, float ambient) { return std::min((sun * kNdotL + ambient) * kBrightness, 1.0f); };
    const Rgb classic(classicHull(classicSunset.r, classicAmbient.r), classicHull(classicSunset.g, classicAmbient.g),
                      classicHull(classicSunset.b, classicAmbient.b));
    auto remasteredHull = [&](const LightRig& rig)
    {
        const float toward = 0.5f + 0.5f * kNdotL;
        auto channel = [&](float sun, float horizon, float horizonSun)
        { return Shown(sun * kNdotL + horizon + (horizonSun - horizon) * toward, rig.exposure); };
        return Rgb(channel(rig.sun.r, rig.skyHorizon.r, rig.skyHorizonSun.r),
                   channel(rig.sun.g, rig.skyHorizon.g, rig.skyHorizonSun.g),
                   channel(rig.sun.b, rig.skyHorizon.b, rig.skyHorizonSun.b));
    };
    const Rgb shown = remasteredHull(ComputeLightRig(in, tuning));
    CAPTURE(classic.r, classic.g, classic.b, shown.r, shown.g, shown.b);
    CHECK(shown.r / shown.b == Catch::Approx(classic.r / classic.b).epsilon(0.1));
    CHECK(shown.g / shown.r == Catch::Approx(classic.g / classic.r).epsilon(0.1));
    // The physical sun alone (no classic colour) shows a far paler hull: the
    // ~5600 K look the classic hue replaces.
    LightRigInputs physicalIn = in;
    physicalIn.classicSunDiffuse = Rgb(0.0f, 0.0f, 0.0f);
    const Rgb physical = remasteredHull(ComputeLightRig(physicalIn, tuning));
    CHECK(RedOverBlue(shown) > 1.15f * RedOverBlue(physical));

    // By day the yellow classic sun is not taken (no mustard).
    LightRigInputs day = ClearSky(30.0f);
    day.classicSunDiffuse = Rgb(0.85f, 0.75f, 0.4f);
    const LightRig noonish = ComputeLightRig(day, tuning);
    CHECK(RedOverBlue(noonish.sun) < 1.15f);

    // warmth 0 = a neutral sun at any height.
    LightingTuning neutral;
    neutral.warmth = 0.0f;
    const LightRig flat = ComputeLightRig(in, neutral);
    CHECK(RedOverBlue(flat.sun) < 1.1f);
}

TEST_CASE("Remaster light rig: twilight fill never outshines the visible sky", "[Graphics][remaster]")
{
    const LightingTuning tuning;
    LightRigInputs in = ClearSky(1.0f);
    in.fogColor = Rgb(0.20f, 0.12f, 0.12f); // dim dusk sky
    const LightRig rig = ComputeLightRig(in, tuning);
    const float skyShown = Luminance(ToLinear(in.fogColor));
    CHECK(Luminance(rig.skyZenith) * kBrightness * rig.exposure <= skyShown * 1.001f);
}

TEST_CASE("Remaster light rig: night keeps the classic ambient level", "[Graphics][remaster]")
{
    const LightingTuning tuning;
    const LightRig rig = ComputeLightRig(ClearSky(-30.0f), tuning);
    CHECK(Luminance(rig.sun) == 0.0f);
    CHECK(Luminance(rig.skyZenith) == Catch::Approx(0.0009f).margin(1e-6));
    CHECK(rig.exposure == Catch::Approx(1.0f));
}

TEST_CASE("Remaster light rig: a full moon lights a cool, pale, readable night", "[Graphics][remaster]")
{
    const LightingTuning tuning;
    // 22:00 on a clear night: sun far below, a full moon 60 degrees up.
    LightRigInputs in = ClearSky(-40.0f);
    const float moonElevation = 60.0f * kPi / 180.0f;
    in.towardMoon[0] = 0.0f;
    in.towardMoon[1] = std::sin(moonElevation);
    in.towardMoon[2] = std::cos(moonElevation);
    in.moonPhaseLight = 1.0f;
    const LightRig rig = ComputeLightRig(in, tuning);
    CHECK(rig.exposure == Catch::Approx(1.0f));

    // Moonlit flat ground: clearly darker than day (noon ground shows the bare
    // texture, 1.0), but readable, not the near-black of a moonless night.
    const float lit = Luminance(rig.moon) * in.towardMoon[1] + Luminance(rig.skyZenith);
    const float shade = Luminance(rig.skyZenith);
    CAPTURE(lit, shade, Shown(lit, rig.exposure), Shown(shade, rig.exposure));
    CHECK(Shown(lit, rig.exposure) > 0.18f);
    CHECK(Shown(lit, rig.exposure) < 0.30f);
    // Moon shade keeps its detail (the moonlit sky fills it), as day shade does.
    const float ratio = Shown(shade, rig.exposure) / Shown(lit, rig.exposure);
    CHECK(ratio > 0.45f);
    CHECK(ratio < 0.75f);

    // Cool but pale: blue over red, no blue cast.
    CHECK(rig.moon.b > rig.moon.g);
    CHECK(rig.moon.g > rig.moon.r);
    CHECK(RedOverBlue(rig.moon) > 0.75f);
    CHECK(RedOverBlue(rig.moon) < 0.9f);
    CHECK(RedOverBlue(rig.skyZenith) < 1.0f);

    // A moonless night keeps the classic ambient level.
    LightRigInputs moonless = in;
    moonless.moonPhaseLight = 0.0f;
    const LightRig dark = ComputeLightRig(moonless, tuning);
    CHECK(Luminance(dark.moon) == 0.0f);
    CHECK(Luminance(dark.skyZenith) == Catch::Approx(0.0009f).margin(1e-6));
    CHECK(Shown(Luminance(dark.skyZenith), dark.exposure) < 0.5f * Shown(lit, rig.exposure));

    // Overcast hides the moon as it dims the sun.
    LightRigInputs overcast = in;
    overcast.skyThrough = 0.0f;
    const LightRig cloudy = ComputeLightRig(overcast, tuning);
    CHECK(Luminance(cloudy.moon) < 0.25f * Luminance(rig.moon));

    // By day the moon adds nothing: the day rig is unchanged with the moon up.
    LightRigInputs day = ClearSky(45.0f);
    const LightRig dayNoMoon = ComputeLightRig(day, tuning);
    day.towardMoon[1] = in.towardMoon[1];
    day.towardMoon[2] = in.towardMoon[2];
    day.moonPhaseLight = 1.0f;
    const LightRig dayMoon = ComputeLightRig(day, tuning);
    CHECK(Luminance(dayMoon.moon) == 0.0f);
    CHECK(Luminance(dayMoon.skyZenith) == Luminance(dayNoMoon.skyZenith));
    CHECK(Luminance(dayMoon.skyHorizon) == Luminance(dayNoMoon.skyHorizon));
    CHECK(Luminance(dayMoon.skyGround) == Luminance(dayNoMoon.skyGround));
}

TEST_CASE("Remaster light rig: cloud shadows carry the classic overcast dimming on average", "[Graphics][remaster]")
{
    // At every overcast, and with the cloud-shadow knob turned down, the mean
    // sun over the field is the classic uniform dimming.
    for (float knob : {1.0f, 0.5f})
    {
        LightingTuning tuning;
        tuning.cloudShadows = knob;
        for (float overcast : {0.0f, 0.02f, 0.05f, 0.1f, 0.25f, 0.5f, 0.6f, 0.66f, 0.8f, 1.0f})
        {
            CAPTURE(knob, overcast);
            LightRigInputs in = ClearSky(45.0f);
            in.skyThrough = SkyThroughAtOvercast(overcast);
            in.cloudShadowsActive = false;
            const LightRig classicDim = ComputeLightRig(in, tuning);
            in.cloudShadowsActive = true;
            const LightRig field = ComputeLightRig(in, tuning);
            const float meanSun = Luminance(field.sun) * (1.0f - field.cloudCover * (1.0f - field.cloudTransmit));
            CHECK(meanSun == Catch::Approx(Luminance(classicDim.sun)).epsilon(1e-4));
            if (overcast >= 0.1f)
                CHECK(field.cloudCover > 0.0f);
            // A clearly cloudy sky shadows the share of the ground it covers.
            if (overcast >= 0.25f)
                CHECK(field.cloudCover == Catch::Approx(1.0f - in.skyThrough));
        }
    }
}

TEST_CASE("Remaster light rig: a clear sky casts no cloud shadows", "[Graphics][remaster]")
{
    // setOvercast 0 is the clearest weather the engine has, yet it reports
    // skyThrough 0.959. No cloud shows in that sky, so no cloud shadow may
    // fall on the ground; its slight classic dimming stays uniform.
    const LightingTuning tuning;
    for (float overcast : {0.0f, 0.01f})
    {
        CAPTURE(overcast);
        LightRigInputs in = ClearSky(45.0f);
        in.skyThrough = SkyThroughAtOvercast(overcast);
        CHECK(in.skyThrough < 0.96f);
        in.cloudShadowsActive = false;
        const LightRig uniform = ComputeLightRig(in, tuning);
        in.cloudShadowsActive = true;
        const LightRig rig = ComputeLightRig(in, tuning);
        CHECK(rig.cloudCover == 0.0f);
        CHECK(rig.cloudTransmit == 1.0f);
        CHECK(Luminance(rig.sun) == Catch::Approx(Luminance(uniform.sun)).epsilon(1e-5));
    }
}

TEST_CASE("Remaster light rig: cloud shadows fade in with the overcast", "[Graphics][remaster]")
{
    // A weather change sweeps skyThrough slowly: neither the shadowed share of
    // the ground nor the sun between the shadows may jump on the way.
    const LightingTuning tuning;
    LightRigInputs in = ClearSky(45.0f);
    in.cloudShadowsActive = true;
    float lastCover = 0.0f;
    float lastSun = Luminance(ComputeLightRig(in, tuning).sun);
    for (int i = 1; i <= 1000; ++i)
    {
        in.skyThrough = 1.0f - static_cast<float>(i) / 1000.0f;
        CAPTURE(in.skyThrough);
        const LightRig rig = ComputeLightRig(in, tuning);
        const float sun = Luminance(rig.sun);
        CHECK(rig.cloudCover >= lastCover);
        CHECK(rig.cloudCover - lastCover < 0.005f);
        CHECK(std::abs(sun - lastSun) < 0.005f * lastSun);
        lastCover = rig.cloudCover;
        lastSun = sun;
    }
    CHECK(lastCover == Catch::Approx(1.0f));
}

TEST_CASE("Remaster sun chroma and tonemap", "[Graphics][remaster]")
{
    float prev = 0.0f;
    for (float e : {60.0f, 30.0f, 15.0f, 8.0f, 3.0f, 0.0f})
    {
        CAPTURE(e);
        const Rgb c = SunChroma(e, 1.0f);
        CHECK(Luminance(c) == Catch::Approx(1.0f).epsilon(1e-4));
        CHECK(RedOverBlue(c) > prev); // warmer as the sun drops
        prev = RedOverBlue(c);
    }
    // Neutral Khronos curve without its toe: identity below 0.8, never above 1.
    const Rgb low = TonemapNeutral(Rgb(0.5f, 0.3f, 0.1f));
    CHECK(low.r == 0.5f);
    CHECK(low.b == 0.1f);
    float last = 0.0f;
    for (float v = 0.8f; v < 20.0f; v *= 1.3f)
    {
        const Rgb t = TonemapNeutral(Rgb(v, v, v));
        CHECK(t.r <= 1.0f);
        CHECK(t.r >= last);
        last = t.r;
    }
}

TEST_CASE("Remaster crown ellipsoid sits on the upper part of the shape", "[Graphics][remaster]")
{
    const float mn[3] = {-3.0f, 0.0f, -2.0f};
    const float mx[3] = {3.0f, 10.0f, 2.0f};
    float c[3], ih[3];
    CrownEllipsoid(mn, mx, c, ih);
    CHECK(c[0] == Catch::Approx(0.0f));
    CHECK(c[1] == Catch::Approx(6.0f));
    CHECK(c[2] == Catch::Approx(0.0f));
    CHECK(ih[0] == Catch::Approx(1.0f / 3.0f));
    CHECK(ih[1] == Catch::Approx(1.0f / 5.0f));
    CHECK(ih[2] == Catch::Approx(1.0f / 2.0f));
    // A flat or degenerate box stays finite.
    const float flat[3] = {1.0f, 1.0f, 1.0f};
    CrownEllipsoid(flat, flat, c, ih);
    CHECK(std::isfinite(ih[0]));
    CHECK(std::isfinite(ih[1]));
}

TEST_CASE("Remaster crown ellipsoid stays finite for degenerate boxes", "[Graphics][remaster]")
{
    // The constants reach every vertex of the draw (VS slots 22-23): a NaN or
    // infinite one turns the whole crown's lighting normal into NaN.
    const float nan = std::numeric_limits<float>::quiet_NaN();
    const float inf = std::numeric_limits<float>::infinity();
    const float big = std::numeric_limits<float>::max();
    struct Box
    {
        const char* what;
        float mn[3];
        float mx[3];
    };
    const Box boxes[] = {
        {"NaN min", {nan, 0.0f, -2.0f}, {3.0f, 10.0f, 2.0f}},
        {"NaN max", {-3.0f, 0.0f, -2.0f}, {3.0f, nan, 2.0f}},
        {"infinite max", {-3.0f, 0.0f, -2.0f}, {inf, 10.0f, 2.0f}},
        {"infinite both ways", {-inf, -inf, -inf}, {inf, inf, inf}},
        {"extent overflows", {-big, 0.0f, -2.0f}, {big, 10.0f, 2.0f}},
        {"inverted (empty)", {1e10f, 1e10f, 1e10f}, {-1e10f, -1e10f, -1e10f}},
        {"point", {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
    };
    for (const Box& b : boxes)
    {
        CAPTURE(b.what);
        float c[3], ih[3];
        CrownEllipsoid(b.mn, b.mx, c, ih);
        for (int i = 0; i < 3; ++i)
        {
            CAPTURE(i);
            CHECK(std::isfinite(c[i]));
            CHECK(std::isfinite(ih[i]));
            CHECK(ih[i] > 0.0f);
            CHECK(ih[i] <= 20.0f); // half extent never below 5 cm
        }
    }
    // The broken axis is the only one replaced: the others keep their fit.
    const float mn[3] = {nan, 0.0f, -2.0f};
    const float mx[3] = {3.0f, 10.0f, 2.0f};
    float c[3], ih[3];
    CrownEllipsoid(mn, mx, c, ih);
    CHECK(c[1] == Catch::Approx(6.0f));
    CHECK(ih[1] == Catch::Approx(1.0f / 5.0f));
    CHECK(c[2] == Catch::Approx(0.0f));
    CHECK(ih[2] == Catch::Approx(1.0f / 2.0f));
}

TEST_CASE("Remaster terrain horizon: a ridge shadows the valley behind it from a low sun", "[Graphics][remaster]")
{
    // 128 x 128 at 25 m; a 120 m north-south ridge along x = 40.
    const int n = 128;
    const float cell = 25.0f;
    std::vector<float> h(static_cast<size_t>(n) * n, 0.0f);
    for (int z = 0; z < n; ++z)
        for (int x = 0; x < n; ++x)
        {
            const float d = static_cast<float>(x - 40);
            h[static_cast<size_t>(z) * n + x] = 120.0f * std::exp(-d * d / 18.0f);
        }
    TerrainHorizonOptions opt;
    opt.threads = 2;
    TerrainHorizonMap map;
    REQUIRE(BuildTerrainHorizonMap(h.data(), n, cell, opt, map));
    REQUIRE(map.Valid());
    REQUIRE(map.size == n);
    REQUIRE(map.horizon.size() == static_cast<size_t>(TerrainHorizonMap::kLayers) * n * n * 4);

    // Sun in the west (-x) at 5 degrees; a point 250 m east of the crest sees
    // the ridge at ~25 degrees: in shadow. The same sun at 40 degrees clears it.
    const float e5 = 5.0f * kPi / 180.0f;
    const float e40 = 40.0f * kPi / 180.0f;
    CHECK(SunVisibility(map, 50, 64, -std::cos(e5), std::sin(e5), 0.0f, 1.2f) < 0.05f);
    CHECK(SunVisibility(map, 50, 64, -std::cos(e40), std::sin(e40), 0.0f, 1.2f) > 0.95f);
    // The far side of the ridge, facing the sun, stays lit.
    CHECK(SunVisibility(map, 30, 64, -std::cos(e5), std::sin(e5), 0.0f, 1.2f) > 0.95f);
    // Open plain far from the ridge sees nearly the whole sky; the foot of the
    // ridge sees less.
    CHECK(map.SkyVisibility(110, 64) > 0.9f);
    CHECK(map.SkyVisibility(46, 64) < map.SkyVisibility(110, 64));

    // A cancelled build stops and reports failure.
    std::atomic<bool> cancel{true};
    TerrainHorizonOptions cancelled = opt;
    cancelled.cancel = &cancel;
    TerrainHorizonMap dropped;
    CHECK_FALSE(BuildTerrainHorizonMap(h.data(), n, cell, cancelled, dropped));
    CHECK_FALSE(dropped.Valid());

    // The 16-bit encoding round-trips to well under the shader penumbra.
    for (float deg : {-5.0f, -1.3f, 0.0f, 2.7f, 12.0f, 45.0f})
        CHECK(DecodeHorizonDeg(EncodeHorizonDeg(deg)) == Catch::Approx(deg).margin(0.001f));
    CHECK(HashHeights(h.data(), n, cell) != HashHeights(h.data(), n, cell * 2.0f));
}

TEST_CASE("Remaster ground tint colours the bounce without changing its brightness", "[Graphics][remaster]")
{
    auto green = [](float r, float b) { return (1.0f - 0.2126f * r - 0.0722f * b) / 0.7152f; };
    struct Case
    {
        float r, g, b;
    };
    const Case cases[] = {{1, 1, 1},          {0.76f, 0.65f, 0.45f}, {0.30f, 0.42f, 0.18f}, {0.0f, 1.0f, 0.0f},
                          {1.0f, 0.0f, 1.0f}, {0.02f, 0.02f, 0.02f}, {0.20f, 0.35f, 0.60f}};
    for (const Case& c : cases)
    {
        CAPTURE(c.r, c.g, c.b);
        float tr = 0, tb = 0;
        GroundTint(c.r, c.g, c.b, 0.75f, tr, tb);
        const float tg = green(tr, tb);
        CHECK(0.2126f * tr + 0.7152f * tg + 0.0722f * tb == Catch::Approx(1.0f).epsilon(1e-4));
        for (float v : {tr, tg, tb})
        {
            CHECK(v >= 0.5f - 1e-4f);
            CHECK(v <= 1.6f + 1e-4f);
        }
    }
    float tr = 0, tb = 0;
    GroundTint(1, 1, 1, 0.75f, tr, tb);
    CHECK(tr == Catch::Approx(1.0f));
    CHECK(tb == Catch::Approx(1.0f));
    GroundTint(0.76f, 0.65f, 0.45f, 0.75f, tr, tb); // desert sand: warm bounce
    CHECK(tr > 1.15f);
    CHECK(tb < 0.7f);
    GroundTint(0.76f, 0.65f, 0.45f, 0.0f, tr, tb); // strength 0 = neutral
    CHECK(tr == Catch::Approx(1.0f));
    CHECK(tb == Catch::Approx(1.0f));

    // The builder downsamples the colours with the heights.
    const int n = 32;
    std::vector<float> h(static_cast<size_t>(n) * n, 0.0f);
    std::vector<float> rgb(static_cast<size_t>(n) * n * 3);
    for (size_t i = 0; i < static_cast<size_t>(n) * n; ++i)
    {
        rgb[i * 3 + 0] = 0.76f;
        rgb[i * 3 + 1] = 0.65f;
        rgb[i * 3 + 2] = 0.45f;
    }
    TerrainHorizonOptions opt;
    opt.maxSize = 16;
    opt.threads = 1;
    TerrainHorizonMap map;
    REQUIRE(BuildTerrainHorizonMap(h.data(), n, 10.0f, opt, map, rgb.data()));
    REQUIRE(map.size == 16);
    REQUIRE(map.groundTint.size() == static_cast<size_t>(16) * 16 * 2);
    GroundTint(0.76f, 0.65f, 0.45f, 0.75f, tr, tb);
    CHECK(map.groundTint[0] == Catch::Approx(tr));
    CHECK(map.groundTint[1] == Catch::Approx(tb));
    TerrainHorizonMap plain;
    REQUIRE(BuildTerrainHorizonMap(h.data(), n, 10.0f, opt, plain));
    CHECK(plain.groundTint.empty());
}

TEST_CASE("Remaster cloud noise is uniform, so a threshold sets the covered fraction exactly", "[Graphics][remaster]")
{
    const int n = 128;
    const std::vector<std::uint8_t> noise = BuildCloudNoise(n, 7);
    REQUIRE(noise.size() == static_cast<size_t>(n) * n);
    for (float cover : {0.1f, 0.4f, 0.8f})
    {
        CAPTURE(cover);
        int above = 0;
        for (std::uint8_t v : noise)
            if (v / 255.0f >= 1.0f - cover)
                ++above;
        CHECK(static_cast<float>(above) / static_cast<float>(noise.size()) == Catch::Approx(cover).margin(0.02f));
    }
    // Tileable: the seam between the last and first column is as smooth as
    // any other pair of neighbouring columns.
    auto columnStep = [&](int a, int b)
    {
        double sum = 0.0;
        for (int z = 0; z < n; ++z)
            sum += std::abs(static_cast<int>(noise[static_cast<size_t>(z) * n + a]) -
                            static_cast<int>(noise[static_cast<size_t>(z) * n + b]));
        return sum / n;
    };
    CHECK(columnStep(n - 1, 0) < 2.0 * columnStep(n / 2, n / 2 + 1) + 4.0);
}

TEST_CASE("Remaster cloud shadows cover the share of the ground the rig asks for", "[Graphics][remaster]")
{
    // The engine's field (BuildCloudNoise(256, 1), read with GL_LINEAR and
    // GL_REPEAT) under the shader's soft threshold (cloudSunVisibility: an edge
    // of kCloudEdge 0.08 that narrows toward a clear or a full sky): from a
    // nearly clear sky to a full overcast the shadowed share is the rig's cover,
    // so the mean sun on the ground is the classic dimming.
    const int n = 256;
    const std::vector<std::uint8_t> noise = BuildCloudNoise(n, 1);
    REQUIRE(noise.size() == static_cast<size_t>(n) * n);
    auto texel = [&](int x, int y) { return noise[static_cast<size_t>(y & (n - 1)) * n + (x & (n - 1))] / 255.0f; };
    auto sample = [&](float u, float v)
    {
        const float x = u * n - 0.5f;
        const float y = v * n - 0.5f;
        const int x0 = static_cast<int>(std::floor(x));
        const int y0 = static_cast<int>(std::floor(y));
        const float tx = x - static_cast<float>(x0);
        const float ty = y - static_cast<float>(y0);
        const float a = texel(x0, y0) + (texel(x0 + 1, y0) - texel(x0, y0)) * tx;
        const float b = texel(x0, y0 + 1) + (texel(x0 + 1, y0 + 1) - texel(x0, y0 + 1)) * tx;
        return a + (b - a) * ty;
    };
    auto smoothstep = [](float e0, float e1, float x)
    {
        const float t = std::min(1.0f, std::max(0.0f, (x - e0) / (e1 - e0)));
        return t * t * (3.0f - 2.0f * t);
    };
    const LightingTuning tuning;
    for (float overcast : {0.02f, 0.05f, 0.1f, 0.3f, 0.5f, 0.6f, 0.66f, 0.8f})
    {
        CAPTURE(overcast);
        LightRigInputs in = ClearSky(45.0f);
        in.skyThrough = SkyThroughAtOvercast(overcast);
        in.cloudShadowsActive = false;
        const float classicSun = Luminance(ComputeLightRig(in, tuning).sun);
        in.cloudShadowsActive = true;
        const LightRig rig = ComputeLightRig(in, tuning);
        const float cover = rig.cloudCover;
        const float edge = 0.08f * std::min(1.0f, std::max(0.0f, 4.0f * cover * (1.0f - cover)));
        // Two samples per texel each way, off the texel centres.
        const int m = 2 * n;
        double covered = 0.0;
        for (int j = 0; j < m; ++j)
        {
            for (int i = 0; i < m; ++i)
            {
                const float v = sample((static_cast<float>(i) + 0.37f) / m, (static_cast<float>(j) + 0.71f) / m);
                covered += edge > 1e-4f ? smoothstep(1.0f - cover - edge, 1.0f - cover + edge, v)
                                        : (v >= 1.0f - cover ? 1.0f : 0.0f);
            }
        }
        covered /= static_cast<double>(m) * m;
        CHECK(covered == Catch::Approx(cover).margin(0.01));
        const float meanSun = Luminance(rig.sun) * (1.0f - static_cast<float>(covered) * (1.0f - rig.cloudTransmit));
        CHECK(meanSun == Catch::Approx(classicSun).epsilon(0.01));
    }
}

// --- GLSL contracts --------------------------------------------------------
//
// The tests read the shaders through the accessors of EngineGL33_Lighting.hpp
// (the sources the engine compiles, assembled from their pieces). They read
// the C++ sources only to check that every raw GLSL string reaches a program,
// that the assembly tables the lighting lab parses are the ones the engine
// uses and that the LitConstants block has no hand copy.

namespace
{
namespace gl = Poseidon::gl33lighting;

std::string ReadTextFile(const std::filesystem::path& p)
{
    std::ifstream f(p);
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

std::filesystem::path Gl33Dir()
{
    return std::filesystem::path(TESTS_ROOT_DIR).parent_path() / "engine" / "PoseidonGL33";
}

// The backend's .cpp files whose name starts with `prefix`, sorted.
std::vector<std::filesystem::path> Gl33Sources(const std::string& prefix)
{
    std::vector<std::filesystem::path> out;
    for (const auto& entry : std::filesystem::directory_iterator(Gl33Dir()))
    {
        const std::string name = entry.path().filename().string();
        if (entry.is_regular_file() && entry.path().extension() == ".cpp" && name.rfind(prefix, 0) == 0)
            out.push_back(entry.path());
    }
    std::sort(out.begin(), out.end());
    return out;
}

// Every `char name[] = R"(...)"` raw string of a C++ source, by name.
std::map<std::string, std::string> RawStrings(const std::string& src)
{
    std::map<std::string, std::string> out;
    static const std::regex header(R"(\bchar\s+(\w+)\s*\[\s*\]\s*=\s*R\"\()");
    for (auto it = std::sregex_iterator(src.begin(), src.end(), header); it != std::sregex_iterator(); ++it)
    {
        const size_t body = static_cast<size_t>(it->position() + it->length());
        const size_t end = src.find(")\"", body);
        if (end != std::string::npos)
            out[(*it)[1].str()] = src.substr(body, end - body);
    }
    return out;
}

// The raw strings of every backend file whose name starts with `prefix`.
std::map<std::string, std::string> RawStringsOf(const std::string& prefix)
{
    std::map<std::string, std::string> out;
    for (const auto& path : Gl33Sources(prefix))
        for (auto& [name, body] : RawStrings(ReadTextFile(path)))
            out[name] = body;
    return out;
}

// The identifiers of a C++ brace-list body, `//` comments dropped and any
// namespace qualification stripped.
std::vector<std::string> ListedNames(const std::string& list)
{
    std::vector<std::string> out;
    static const std::regex comment(R"(//[^\n]*)");
    static const std::regex ident(R"(([A-Za-z_][\w:]*))");
    const std::string clean = std::regex_replace(list, comment, "");
    for (auto it = std::sregex_iterator(clean.begin(), clean.end(), ident); it != std::sregex_iterator(); ++it)
    {
        const std::string name = (*it)[1].str();
        const size_t colon = name.rfind("::");
        out.push_back(colon == std::string::npos ? name : name.substr(colon + 2));
    }
    return out;
}

// The raw strings named, joined in order.
std::string Joined(const std::vector<std::string>& names, const std::map<std::string, std::string>& raw)
{
    std::string out;
    for (const std::string& name : names)
    {
        const auto it = raw.find(name);
        if (it == raw.end())
            return "<no raw string " + name + ">";
        out += it->second;
    }
    return out;
}

int CountOf(const std::string& haystack, const std::string& needle)
{
    int n = 0;
    for (size_t at = haystack.find(needle); at != std::string::npos; at = haystack.find(needle, at + needle.size()))
        ++n;
    return n;
}

// Plain GLSL 3.30 core validation (glslangValidator's default). No SPIR-V
// client, unlike the I-28 harness: the post shaders use loose uniforms, which
// GL 3.3 allows and SPIR-V for OpenGL does not.
bool Compiles(const std::string& src, EShLanguage stage, std::string& log)
{
    glslang::TShader shader(stage);
    const char* strings[1] = {src.c_str()};
    shader.setStrings(strings, 1);
    const bool ok = shader.parse(GetDefaultResources(), 330, false, EShMsgDefault);
    log = shader.getInfoLog();
    return ok;
}

struct GlslangInit
{
    GlslangInit() { glslang::InitializeProcess(); }
    ~GlslangInit() { glslang::FinalizeProcess(); }
};

// `<qualifier> <type> <name>;` declarations (varyings) of a GLSL block.
std::set<std::string> Varyings(const std::string& glsl, const std::string& qualifier)
{
    std::set<std::string> out;
    const std::regex decl("(?:flat\\s+)?" + qualifier + "\\s+(\\w+)\\s+(\\w+)\\s*;");
    for (auto it = std::sregex_iterator(glsl.begin(), glsl.end(), decl); it != std::sregex_iterator(); ++it)
    {
        const std::string name = (*it)[2].str();
        if (name != "fragColor")
            out.insert((*it)[1].str() + " " + name);
    }
    return out;
}

// Covers every VertexShaderID and PixelShaderID; the accessors return null
// past the last one.
constexpr int kShaderIds = 16;

std::string Piece(gl::FragmentPiece piece)
{
    const char* text = gl::RemasterFragmentPiece(piece);
    return text ? text : "";
}

// The remastered programs: each vertex shader and the prelude with each main.
std::vector<std::string> RemasterPrograms()
{
    std::vector<std::string> out;
    for (int vs = 0; vs < kShaderIds; ++vs)
        if (const char* src = gl::RemasterVertexSource(vs))
            out.emplace_back(src);
    for (int ps = 0; ps < kShaderIds; ++ps)
        if (const char* main = gl::RemasterFragmentMain(ps))
            out.push_back(std::string(gl::RemasterFragmentCommon()) + main);
    return out;
}

// Every program the remastered set and the post pass compile.
std::vector<std::string> AssembledPrograms()
{
    std::vector<std::string> out = RemasterPrograms();
    for (int i = 0; i < static_cast<int>(gl::PostShader::Count); ++i)
        out.emplace_back(gl::PostShaderSource(static_cast<gl::PostShader>(i)));
    return out;
}
} // namespace

TEST_CASE("Remaster shaders compile under glslang and their varyings match", "[Graphics][Shaders][remaster]")
{
    GlslangInit init;
    const std::string common = gl::RemasterFragmentCommon();
    REQUIRE_FALSE(common.empty());

    int vertex = 0, fragment = 0;
    for (int vs = 0; vs < kShaderIds; ++vs)
    {
        const char* src = gl::RemasterVertexSource(vs);
        if (!src)
            continue;
        CAPTURE(vs);
        std::string log;
        const bool ok = Compiles(src, EShLangVertex, log);
        CAPTURE(log);
        CHECK(ok);
        // Every remastered VS feeds the same varyings the common FS reads.
        CHECK(Varyings(src, "out") == Varyings(common, "in"));
        ++vertex;
    }
    for (int ps = 0; ps < kShaderIds; ++ps)
    {
        const char* main = gl::RemasterFragmentMain(ps);
        if (!main)
            continue;
        CAPTURE(ps);
        std::string log;
        const bool ok = Compiles(common + main, EShLangFragment, log);
        CAPTURE(log);
        CHECK(ok);
        ++fragment;
    }
    CHECK(vertex == 3);   // VSScreen, VSTransform, VSShadow
    CHECK(fragment == 4); // PSNormal, PSDetail, PSGrass, PSWater

    int post = 0;
    for (int i = 0; i < static_cast<int>(gl::PostShader::Count); ++i)
    {
        const auto shader = static_cast<gl::PostShader>(i);
        const char* name = gl::PostShaderName(shader);
        REQUIRE(name != nullptr);
        CAPTURE(name);
        std::string log;
        const bool ok = Compiles(gl::PostShaderSource(shader),
                                 shader == gl::PostShader::Vertex ? EShLangVertex : EShLangFragment, log);
        CAPTURE(log);
        CHECK(ok);
        ++post;
    }
    // vsPost, AO, AO blur, bloom down and up, shafts, composite, linear depth
    // and its debug view
    CHECK(post == 9);
}

TEST_CASE("Every raw GLSL string of the lighting and post sources reaches a compiled program",
          "[Graphics][Shaders][remaster]")
{
    // A piece left out of its assembly table, or a shader in a new
    // EngineGL33_Post*.cpp that no table names, would never be compiled or
    // tested: every raw string must sit inside a program the engine builds.
    const std::vector<std::string> programs = AssembledPrograms();
    const auto lighting = Gl33Sources("EngineGL33_LightingShaders");
    const auto post = Gl33Sources("EngineGL33_Post");
    CHECK(lighting.size() == 4); // the main file, Sky, Lights, Surface
    REQUIRE_FALSE(post.empty());
    int strings = 0;
    for (const auto& files : {lighting, post})
        for (const auto& path : files)
            for (const auto& [name, body] : RawStrings(ReadTextFile(path)))
            {
                CAPTURE(path.filename().string(), name);
                const std::string& text = body;
                const bool reached = std::any_of(programs.begin(), programs.end(), [&text](const std::string& p)
                                                 { return p.find(text) != std::string::npos; });
                CHECK(reached);
                ++strings;
            }
    CHECK(strings >= 29); // today 3 VS, 10 prelude pieces and 4 mains; 12 post strings
}

TEST_CASE("Remaster fragment prelude is its concern pieces in order", "[Graphics][Shaders][remaster]")
{
    using P = gl::FragmentPiece;
    std::string joined;
    for (int i = 0; i < static_cast<int>(P::Count); ++i)
    {
        const std::string piece = Piece(static_cast<P>(i));
        CAPTURE(i);
        CHECK_FALSE(piece.empty());
        joined += piece;
    }
    const std::string common = gl::RemasterFragmentCommon();
    CHECK(joined == common);
    CHECK(gl::RemasterFragmentPiece(P::Count) == nullptr);
    CHECK(Piece(P::LitConstants) == gl::LitConstantsBlock());
    CHECK(Piece(P::Header).rfind("#version 330 core\n", 0) == 0);

    // Each concern's declarations sit in its own piece, once, in this order.
    const std::pair<P, const char*> owners[] = {
        {P::Header, "uniform PSConstants {"},
        {P::LitConstants, "uniform LitConstants {"},
        {P::Inputs, "uniform sampler2DArrayShadow shadowMap;"},
        {P::Inputs, "vec3 tonemapNeutral("},
        {P::Shadows, "float shadowMapVisibility("},
        {P::TerrainClouds, "float terrainSunVisibility("},
        {P::TerrainClouds, "float cloudSunVisibility("},
        {P::Sky, "vec3 skyIrradiance("},
        {P::Sky, "vec3 fogColorToward("},
        {P::Lights, "vec3 localLights("},
        {P::Surface, "vec3 glossy("},
        {P::Shade, "vec3 shadeRemastered("},
        {P::Finish, "void finishFragment("},
    };
    size_t last = 0;
    for (const auto& [piece, decl] : owners)
    {
        CAPTURE(static_cast<int>(piece), decl);
        CHECK(Piece(piece).find(decl) != std::string::npos);
        CHECK(CountOf(common, decl) == 1);
        const size_t at = common.find(decl);
        CHECK(at >= last);
        last = at;
    }

    // The lighting lab assembles the prelude from the kFragmentPieces table it
    // parses out of EngineGL33_LightingShaders.cpp: that table, read the same
    // way, gives the prelude the engine compiles.
    const std::string source = ReadTextFile(Gl33Dir() / "EngineGL33_LightingShaders.cpp");
    const std::string tableStart = "kFragmentPieces[] = {";
    const size_t open = source.find(tableStart);
    REQUIRE(open != std::string::npos);
    const size_t close = source.find("};", open);
    REQUIRE(close != std::string::npos);
    const std::vector<std::string> names =
        ListedNames(source.substr(open + tableStart.size(), close - open - tableStart.size()));
    CHECK(names.size() == static_cast<size_t>(P::Count));
    CHECK(Joined(names, RawStringsOf("EngineGL33_LightingShaders")) == common);
}

TEST_CASE("Post shaders are the pieces their table names", "[Graphics][Shaders][remaster]")
{
    // The same contract for the post pass: kPostShaders in
    // EngineGL33_PostShaders.cpp, read the way the lighting lab reads it, gives
    // each program the engine compiles, under its override name.
    const std::string source = ReadTextFile(Gl33Dir() / "EngineGL33_PostShaders.cpp");
    const size_t open = source.find("kPostShaders[] = {");
    REQUIRE(open != std::string::npos);
    const size_t close = source.find("\n};", open);
    REQUIRE(close != std::string::npos);
    const std::string table = source.substr(open, close - open);
    const auto raw = RawStringsOf("EngineGL33_Post");
    static const std::regex row(R"re(\{\s*"(\w+)"\s*,\s*\{([^{}]*)\}\s*\})re");
    int i = 0;
    for (auto it = std::sregex_iterator(table.begin(), table.end(), row); it != std::sregex_iterator(); ++it, ++i)
    {
        REQUIRE(i < static_cast<int>(gl::PostShader::Count));
        const auto shader = static_cast<gl::PostShader>(i);
        const std::string name = (*it)[1].str();
        CAPTURE(name);
        CHECK(name == gl::PostShaderName(shader));
        CHECK(Joined(ListedNames((*it)[2].str()), raw) == gl::PostShaderSource(shader));
    }
    CHECK(i == static_cast<int>(gl::PostShader::Count));
    CHECK(gl::PostShaderName(gl::PostShader::Count) == nullptr);

    // psPostAo and psPostLinearDepth read the shared depth reconstruction.
    const std::string ao = gl::PostShaderSource(gl::PostShader::Ao);
    CHECK(CountOf(ao, "float viewZ(vec2 uv) {") == 1);
    CHECK(CountOf(ao, "vec3 viewPos(vec2 uv, float z) {") == 1);
    const std::string linear = gl::PostShaderSource(gl::PostShader::LinearDepth);
    CHECK(CountOf(linear, "float viewZ(vec2 uv) {") == 1);
    CHECK(CountOf(linear, "vec3 viewPos(vec2 uv, float z) {") == 1);
    CHECK(CountOf(linear, "viewZ(uv)") == 1);
}

TEST_CASE("Post linear depth: half resolution, nearest and farthest on a checkerboard, the sky at 1e6",
          "[Graphics][Shaders][remaster][PostStages]")
{
    // The contract its readers rely on (the stages that trace or upsample at
    // half resolution): one texel per 2x2 full-resolution pixels, read at their
    // exact texel centres; even texels keep the nearest view depth and odd
    // texels the farthest; the sky is 1e6, by the threshold psPostAo uses.
    const std::string linear = gl::PostShaderSource(gl::PostShader::LinearDepth);
    REQUIRE_FALSE(linear.empty());
    CHECK(linear.find("vec2 uv = (vec2(ij * 2 + ivec2(k & 1, k >> 1)) + 0.5) * screen.zw;") != std::string::npos);
    CHECK(linear.find("float z = d >= 0.999999 ? 1e6 : viewZ(uv);") != std::string::npos);
    CHECK(linear.find("((ij.x + ij.y) & 1) == 0 ? zNear : zFar") != std::string::npos);
    const std::string ao = gl::PostShaderSource(gl::PostShader::Ao);
    CHECK(ao.find("if (d0 >= 0.999999) { fragColor = vec4(1.0, 1e6, 0.0, 1.0); return; } // sky") != std::string::npos);
    // The debug view reads it and nothing else.
    const std::string debug = gl::PostShaderSource(gl::PostShader::DebugDepth);
    CHECK(debug.find("uniform sampler2D linDepthTex;") != std::string::npos);
    CHECK(CountOf(debug, "uniform sampler2D") == 1);
    // The target is one float channel at half resolution, sampled nearest.
    const std::string post = ReadTextFile(Gl33Dir() / "EngineGL33_PostFX.cpp");
    CHECK(post.find("p.linDepthTex = MakeTexture(GL_R32F, GL_RED, GL_FLOAT, hw, hh, false);") != std::string::npos);
    CHECK(post.find("DrawTo(p.linDepthFbo, f.hw, f.hh);") != std::string::npos);
}

TEST_CASE("Remaster lighting normals survive leaves without a mesh normal", "[Graphics][Shaders][remaster]")
{
    // Many CWA tree crowns (the town birch on Malden, several pines and
    // broadleaves) have leaf cards whose mesh normals are zero. normalize() of a
    // zero vector is undefined (NaN on NVIDIA), and a NaN lighting normal makes
    // the crown fall back to ground-only sky light with no sun shadow. The
    // vertex shaders and the per-fragment normal setup must use the guarded
    // form, x * inversesqrt(max(dot(x, x), eps)), which keeps a zero vector zero.
    int vertex = 0;
    for (int vs = 0; vs < kShaderIds; ++vs)
    {
        const char* src = gl::RemasterVertexSource(vs);
        if (!src)
            continue;
        CAPTURE(vs);
        CHECK(std::string(src).find("normalize(") == std::string::npos);
        ++vertex;
    }
    CHECK(vertex == 3);
    const std::string piece = Piece(gl::FragmentPiece::Shade);
    const size_t start = piece.find("vec3 shadeRemastered(");
    REQUIRE(start != std::string::npos);
    const size_t end = piece.find("\n}\n", start);
    REQUIRE(end != std::string::npos);
    const std::string shade = piece.substr(start, end - start);
    CHECK(shade.find("normalize(") == std::string::npos);
    // A foliage fragment without a normal of its own lights with the crown's.
    CHECK(std::regex_search(shade, std::regex(R"(if\s*\(\s*foliage\s*&&\s*dot\(n,\s*n\)\s*<[^)]*\)\s*n\s*=\s*cn;)")));
}

TEST_CASE("Remaster sun shafts brighten the air, not the surfaces near the eye", "[Graphics][Shaders][remaster]")
{
    // Clear air: the fog end of any view distance from 900 m up.
    for (float fogEnd : {880.0f, 1480.0f, 3980.0f})
    {
        CAPTURE(fogEnd);
        const float length = ShaftScatterLength(fogEnd);
        CHECK(length <= kShaftScatterLength);
        CHECK(length > 0.9f * kShaftScatterLength);
        // The acceptance props at a low sun: soldier 7 m, Jeep 10 m, M113 16 m.
        // At full weight they were veiled grey.
        CHECK(ShaftInscatter(7.0f, length) < 0.03f);
        CHECK(ShaftInscatter(10.0f, length) < 0.04f);
        CHECK(ShaftInscatter(16.0f, length) < 0.06f);
        // Distant ground and the sky keep the shafts.
        CHECK(ShaftInscatter(1000.0f, length) > 0.95f);
        CHECK(ShaftInscatter(std::numeric_limits<float>::infinity(), length) == 1.0f);
        float prev = -1.0f;
        for (float d = 0.0f; d < 2000.0f; d += 50.0f)
        {
            CHECK(ShaftInscatter(d, length) > prev);
            prev = ShaftInscatter(d, length);
        }
    }
    CHECK(ShaftScatterLength(0.0f) == kShaftScatterLength); // no fog range yet
    // Thick fog scatters closer to the eye.
    CHECK(ShaftScatterLength(150.0f) == Catch::Approx(50.0f));
    CHECK(ShaftInscatter(30.0f, ShaftScatterLength(150.0f)) > 2.0f * ShaftInscatter(30.0f, ShaftScatterLength(880.0f)));

    // The composite applies the same falloff to the shaft term, with the
    // length the engine takes from ShaftScatterLength.
    const std::string composite = gl::PostShaderSource(gl::PostShader::Composite);
    REQUIRE_FALSE(composite.empty());
    CHECK(composite.find("uniform sampler2D depthTex;") != std::string::npos);
    CHECK(composite.find("1.0 - exp(-max(dist, 0.0) * depthMap.z)") != std::string::npos);
    CHECK(composite.find("shaft *= shaftInscatter()") != std::string::npos);
    const std::string post = ReadTextFile(Gl33Dir() / "EngineGL33_PostFX.cpp");
    CHECK(post.find("1.0f / scatterLength") != std::string::npos);
}

TEST_CASE("Remaster LitConstants GLSL block matches the LitConst slot layout", "[Graphics][Shaders][remaster]")
{
    namespace LC = gl::LitConst;
    // Every member of the block, in order, with its LitConst slot and size.
    struct Member
    {
        const char* name;
        int slot;
        int count;
    };
    static const Member members[] = {
        {"litSunDir", LC::SunDir, 1},
        {"litSun", LC::Sun, 1},
        {"litZenith", LC::Zenith, 1},
        {"litHorizon", LC::Horizon, 1},
        {"litGround", LC::Ground, 1},
        {"litMoonDir", LC::MoonDir, 1},
        {"litMoon", LC::Moon, 1},
        {"litCamPos", LC::CamPos, 1},
        {"litMatDiffuse", LC::MatDiffuse, 1},
        {"litMatAmbient", LC::MatAmbient, 1},
        {"litMatForced", LC::MatForced, 1},
        {"litMatEmissive", LC::MatEmissive, 1},
        {"litMatSpecular", LC::MatSpecular, 1},
        {"litTerrain", LC::Terrain, 1},
        {"litTerrainSel0", LC::TerrainSel0, 1},
        {"litTerrainSel1", LC::TerrainSel1, 1},
        {"litTerrainSun", LC::TerrainSun, 1},
        {"litTerrainLift", LC::TerrainLift, 1},
        {"litCloud", LC::Cloud, 1},
        {"litCloudOffset", LC::CloudOffset, 1},
        {"litShadowDelta", LC::ShadowDelta, 1},
        {"litFlags", LC::Flags, 1},
        {"litLightCount", LC::LightCount, 1},
        {"litHorizonSun", LC::HorizonSun, 1},
        {"litFogSun", LC::FogSun, 1},
        {"litFogCap", LC::FogCap, 1},
        {"litLightPos", LC::LightPos, LC::MaxLights},
        {"litLightColor", LC::LightColor, LC::MaxLights},
        {"litLightAmb", LC::LightAmb, LC::MaxLights},
        {"litLightDir", LC::LightDir, LC::MaxLights},
    };
    // Walks the members of one declaration, slot by slot.
    auto checkBlock = [](const std::string& block)
    {
        static const std::regex member(R"(vec4\s+(\w+)(?:\[(\d+)\])?\s*;)");
        int at = 0;
        size_t i = 0;
        for (auto it = std::sregex_iterator(block.begin(), block.end(), member); it != std::sregex_iterator(); ++it)
        {
            const std::string name = (*it)[1].str();
            const int count = (*it)[2].matched ? std::stoi((*it)[2].str()) : 1;
            CAPTURE(name, at);
            REQUIRE(i < std::size(members));
            CHECK(name == members[i].name);
            CHECK(at == members[i].slot);
            CHECK(count == members[i].count);
            at += count;
            ++i;
        }
        CHECK(i == std::size(members));
        CHECK(at == LC::Count);
    };
    auto blockAt = [](const std::string& text, size_t open)
    {
        const size_t close = text.find("};", open);
        return close == std::string::npos ? std::string() : text.substr(open, close - open);
    };

    const std::string shared = gl::LitConstantsBlock();
    const size_t sharedOpen = shared.find("uniform LitConstants {");
    REQUIRE(sharedOpen != std::string::npos);
    checkBlock(blockAt(shared, sharedOpen));

    // Every declaration in the backend's sources, slot by slot, and there is
    // only the one: a hand copy (four prototypes pasted their own into post
    // and vertex shaders) would drift from the layout.
    int declarations = 0;
    for (const auto& entry : std::filesystem::directory_iterator(Gl33Dir()))
    {
        const auto ext = entry.path().extension().string();
        if (!entry.is_regular_file() || (ext != ".cpp" && ext != ".hpp"))
            continue;
        const std::string text = ReadTextFile(entry.path());
        for (size_t at = text.find("uniform LitConstants {"); at != std::string::npos;
             at = text.find("uniform LitConstants {", at + 1))
        {
            CAPTURE(entry.path().filename().string());
            checkBlock(blockAt(text, at));
            ++declarations;
        }
    }
    CHECK(declarations == 1);

    // A program that reads the rig pastes the shared block whole, once.
    for (const std::string& program : AssembledPrograms())
        if (program.find("uniform LitConstants") != std::string::npos)
            CHECK(CountOf(program, shared) == 1);
    CHECK(CountOf(gl::RemasterFragmentCommon(), shared) == 1);

    // The planned slots continue the declared ones.
    CHECK(gl::LitConstPlanned::MatClass == static_cast<int>(LC::Count));

    // The decoded horizon range in the shader is the encoder's.
    std::ostringstream range;
    range << "mix(" << std::fixed;
    range.precision(1);
    range << TerrainHorizonMap::kMinAngleDeg << ", " << TerrainHorizonMap::kMaxAngleDeg << ",";
    CHECK(Piece(gl::FragmentPiece::TerrainClouds).find(range.str()) != std::string::npos);
}

TEST_CASE("Remaster texture units come from one table that binds every sampler", "[Graphics][Shaders][remaster]")
{
    // The table's units are distinct, clear of the material units 0 and 1 and
    // the upload unit 7, and within the 16 units GL 3.3 guarantees.
    std::set<int> units;
    std::set<std::string> names;
    for (const auto& s : gl::kLitSamplers)
    {
        CAPTURE(s.name, s.unit);
        CHECK(s.unit >= 2);
        CHECK(s.unit != 7);
        CHECK(s.unit <= 15);
        CHECK(units.insert(s.unit).second);
        CHECK(names.insert(s.name).second);
    }
    // The shadow-map audits of test_gl_state_cache_audit.cpp pin binds to this
    // constant; the shaders' comments and the classic programs say unit 2.
    CHECK(gl::kShadowMapUnit == 2);

    // Every sampler a remastered program declares has its unit in the table
    // (tex0 and tex1 are the material units): an unbound sampler reads unit 0.
    std::set<std::string> declared;
    static const std::regex sampler(R"(uniform\s+sampler\w+\s+(\w+)\s*;)");
    for (const std::string& program : RemasterPrograms())
        for (auto it = std::sregex_iterator(program.begin(), program.end(), sampler); it != std::sregex_iterator();
             ++it)
            declared.insert((*it)[1].str());
    for (const std::string& name : declared)
    {
        CAPTURE(name);
        CHECK((name == "tex0" || name == "tex1" || names.count(name) == 1));
    }
    // And every row names a sampler the remastered programs declare.
    for (const std::string& name : names)
    {
        CAPTURE(name);
        CHECK(declared.count(name) == 1);
    }
}
