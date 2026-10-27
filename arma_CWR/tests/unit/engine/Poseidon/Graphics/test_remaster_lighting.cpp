#include <catch2/catch_approx.hpp>
#include <catch2/catch_message.hpp>
#include <catch2/catch_test_macros.hpp>

#include <glslang/Public/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>

#include <Poseidon/Graphics/Remaster/Atmosphere.hpp>
#include <Poseidon/Graphics/Remaster/CloudNoise.hpp>
#include <Poseidon/Graphics/Remaster/FeatureSwitches.hpp>
#include <Poseidon/Graphics/Remaster/LightRig.hpp>
#include <Poseidon/Graphics/Remaster/PhotoCamera.hpp>
#include <Poseidon/Graphics/Remaster/SeaWaves.hpp>
#include <Poseidon/Graphics/Remaster/SkyDensity.hpp>
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
        // Cast shadow: bold but not crushed (~0.42 read as crushed).
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

namespace
{
// Classic's sun colour at an elevation (LightSun::Recalculate's colorFull):
// FullSunColor (0.85, 0.75, 0.4) toward SunsetColor (0.8, 0.3, 0.23) as the
// sine of the elevation falls below sin 25 degrees. At 7 degrees it is the
// (0.814, 0.43, 0.279) of the case above.
Rgb ClassicSunColor(float elevationDeg)
{
    const float s = std::sin(std::max(elevationDeg, 0.0f) * kPi / 180.0f);
    const float t = std::min(std::max(1.0f - s / 0.42262f, 0.0f), 1.0f);
    return Mix(Rgb(0.85f, 0.75f, 0.4f), Rgb(0.8f, 0.30f, 0.23f), t);
}

LightRigInputs ClearSkyClassicSun(float elevationDeg)
{
    LightRigInputs in = ClearSky(elevationDeg);
    in.classicSunDiffuse = ClassicSunColor(elevationDeg);
    return in;
}

LightingTuning WithLowSunRig(bool on)
{
    LightingTuning t;
    t.features.lowSunRig = on ? 1.0f : 0.0f;
    return t;
}

// Light (rig units, linear) on a vertical face whose normal is `offDeg` from
// the sun's azimuth: the sun times N.L and the horizon band between its
// anti-sun and sunward halves, as the shader's skyIrradiance gives it for a
// horizontal normal. The calibration boxes turn a hull 45
// degrees from the sun (lit) and 135 (shaded).
Rgb VerticalFace(const LightRig& rig, float elevationDeg, float offDeg)
{
    const float cosOff = std::cos(offDeg * kPi / 180.0f);
    const float sunTerm = std::max(std::cos(elevationDeg * kPi / 180.0f) * cosOff, 0.0f);
    const float toward = 0.5f + 0.5f * cosOff;
    return rig.sun * sunTerm + Mix(rig.skyHorizon, rig.skyHorizonSun, toward);
}

bool SameRgb(const Rgb& a, const Rgb& b)
{
    return a.r == b.r && a.g == b.g && a.b == b.b;
}
} // namespace

TEST_CASE("Remaster light rig: the low-sun rig keeps the shaded side of a hull readable", "[Graphics][remaster]")
{
    // The face of a hull turned away from a low sun was crushed (0.32 to 0.42
    // of its sunlit side on screen in the calibration boxes; the target is
    // about 0.6, and 0.42 still read as crushed). The sideways fill lifts it into a readable range without
    // flattening the hull. The game measures 0.02 to 0.07 above this model
    // (AO, ground tint and the measured faces' exact angles).
    for (float e : {4.8f, 7.5f, 9.0f, 12.7f, 14.0f})
    {
        CAPTURE(e);
        const LightRigInputs in = ClearSkyClassicSun(e);
        const LightRig on = ComputeLightRig(in, WithLowSunRig(true));
        const LightRig off = ComputeLightRig(in, WithLowSunRig(false));
        auto ratio = [e](const LightRig& rig)
        {
            return Shown(Luminance(VerticalFace(rig, e, 135.0f)), rig.exposure) /
                   Shown(Luminance(VerticalFace(rig, e, 45.0f)), rig.exposure);
        };
        CAPTURE(ratio(on), ratio(off));
        CHECK(ratio(off) < 0.41f);
        CHECK(ratio(on) > 0.5f);
        CHECK(ratio(on) < 0.65f); // still a shaded side, not flat light
        CHECK(ratio(on) > ratio(off) + 0.15f);
        // The sunlit side is not dimmed to get there.
        CHECK(Luminance(VerticalFace(on, e, 45.0f)) >= Luminance(VerticalFace(off, e, 45.0f)));
        // Up-facing ground and its cast shadows keep their light, and the
        // exposure is the meter's: only walls and the sun's hue change.
        CHECK(Luminance(on.skyZenith) == Luminance(off.skyZenith));
        CHECK(on.exposure == off.exposure);
        CHECK(Luminance(on.sun) == Catch::Approx(Luminance(off.sun)).epsilon(1e-5));
    }
    // The fill is full from 8 degrees down to the horizon, gone by 40 and
    // faded out under the horizon; noon and night are untouched to the bit.
    CHECK(LowSunFill(40.0f) == 0.0f);
    CHECK(LowSunFill(8.0f) == Catch::Approx(1.0f));
    CHECK(LowSunFill(3.0f) == Catch::Approx(1.0f));
    CHECK(LowSunFill(-2.0f) == 0.0f);
    float last = 0.0f;
    for (float e = 40.0f; e >= 8.0f; e -= 1.0f)
    {
        CAPTURE(e);
        CHECK(LowSunFill(e) >= last);
        last = LowSunFill(e);
    }
    for (float e : {40.0f, 45.0f, 60.0f, 85.0f, -3.0f, -30.0f})
    {
        CAPTURE(e);
        const LightRigInputs in = ClearSkyClassicSun(e);
        const LightRig on = ComputeLightRig(in, WithLowSunRig(true));
        const LightRig off = ComputeLightRig(in, WithLowSunRig(false));
        CHECK(SameRgb(on.sun, off.sun));
        CHECK(SameRgb(on.skyZenith, off.skyZenith));
        CHECK(SameRgb(on.skyHorizon, off.skyHorizon));
        CHECK(SameRgb(on.skyHorizonSun, off.skyHorizonSun));
        CHECK(SameRgb(on.skyGround, off.skyGround));
        CHECK(on.exposure == off.exposure);
    }
}

TEST_CASE("Remaster light rig: the low-sun rig warms the evening sun from 25 degrees, as classic does",
          "[Graphics][remaster]")
{
    // Classic's sunset colour ramps in from 25 degrees; EveningSunChroma takes
    // it only below 16 (14% of it at 12.7 degrees), so the sunlit ground and
    // hulls read greener and greyer than the original's. The low-sun rig takes
    // a further share of it from 25 degrees down and more in the last 9.
    CHECK(LowSunHueShare(25.0f) == 0.0f);
    CHECK(LowSunHueShare(60.0f) == 0.0f);
    CHECK(LowSunHueShare(4.0f) == Catch::Approx(0.7f));
    CHECK(LowSunHueShare(0.0f) == Catch::Approx(0.7f));
    float last = 0.0f;
    for (float e = 25.0f; e >= 0.0f; e -= 0.5f)
    {
        CAPTURE(e);
        CHECK(LowSunHueShare(e) >= last);
        last = LowSunHueShare(e);
    }
    for (float e : {4.8f, 7.5f, 9.0f, 12.7f, 14.0f, 20.0f})
    {
        CAPTURE(e);
        const LightRigInputs in = ClearSkyClassicSun(e);
        const LightRig on = ComputeLightRig(in, WithLowSunRig(true));
        const LightRig off = ComputeLightRig(in, WithLowSunRig(false));
        // The sun itself: redder and less green than before the step.
        CHECK(RedOverBlue(on.sun) > RedOverBlue(off.sun));
        CHECK(on.sun.g / on.sun.r < off.sun.g / off.sun.r);
        // The sunlit face of a hull (45 degrees from the sun) is at least as
        // warm as before the step, though the fill adds cool sky to it: owner
        // rule, evenings no less warm than the original look on hulls.
        const Rgb litOn = VerticalFace(on, e, 45.0f);
        const Rgb litOff = VerticalFace(off, e, 45.0f);
        CAPTURE(RedOverBlue(litOn), RedOverBlue(litOff));
        CHECK(RedOverBlue(litOn) >= RedOverBlue(litOff));
        CHECK(litOn.g / litOn.r <= litOff.g / litOff.r);
    }
    // Warmth 0 keeps a neutral sun with the rig on, and no classic colour
    // leaves the physical one.
    LightingTuning neutral = WithLowSunRig(true);
    neutral.warmth = 0.0f;
    CHECK(RedOverBlue(ComputeLightRig(ClearSkyClassicSun(7.0f), neutral).sun) < 1.1f);
    LightRigInputs noClassic = ClearSky(7.0f);
    noClassic.classicSunDiffuse = Rgb(0.0f, 0.0f, 0.0f);
    CHECK(SameRgb(ComputeLightRig(noClassic, WithLowSunRig(true)).sun,
                  ComputeLightRig(noClassic, WithLowSunRig(false)).sun));
}

TEST_CASE("Remaster light rig: with lowSunRig off the rig is the one before the low-sun step", "[Graphics][remaster]")
{
    // The switch's "off" must restore the earlier rig exactly: values of
    // ComputeLightRig before the low-sun rig, the ClearSky inputs with
    // classic's sun colour.
    struct Golden
    {
        float e;
        Rgb sun, zenith, horizon, horizonSun, ground;
        float exposure;
    };
    const Golden golden[] = {
        {7.5f,
         {0.800340891f, 0.308145106f, 0.15937148f},
         {0.0422399715f, 0.0485759676f, 0.0619519576f},
         {0.028278444f, 0.0310057588f, 0.0363595635f},
         {0.0425428152f, 0.0463276245f, 0.0500698723f},
         {0.0201927219f, 0.0201927219f, 0.0193850119f},
         1.04015148f},
        {12.7f,
         {0.605924964f, 0.435198337f, 0.311057419f},
         {0.0577532463f, 0.0664162338f, 0.0847047642f},
         {0.0423756465f, 0.0462110043f, 0.0536355972f},
         {0.0599935316f, 0.0650311857f, 0.0698774755f},
         {0.0336132348f, 0.0336132348f, 0.032268703f},
         1.2436173f},
        {30.0f,
         {0.542771995f, 0.521061122f, 0.488494784f},
         {0.0945555493f, 0.108738877f, 0.138681471f},
         {0.0825667083f, 0.0893924385f, 0.102319837f},
         {0.101729915f, 0.1092733f, 0.116054296f},
         {0.0741237104f, 0.0741237104f, 0.0711587593f},
         1.56306732f},
    };
    auto same = [](const Rgb& a, const Rgb& b)
    {
        return a.r == Catch::Approx(b.r).epsilon(1e-6) && a.g == Catch::Approx(b.g).epsilon(1e-6) &&
               a.b == Catch::Approx(b.b).epsilon(1e-6);
    };
    for (const Golden& g : golden)
    {
        CAPTURE(g.e);
        const LightRigInputs in = ClearSkyClassicSun(g.e);
        const LightRig off = ComputeLightRig(in, WithLowSunRig(false));
        CHECK(same(off.sun, g.sun));
        CHECK(same(off.skyZenith, g.zenith));
        CHECK(same(off.skyHorizon, g.horizon));
        CHECK(same(off.skyHorizonSun, g.horizonSun));
        CHECK(same(off.skyGround, g.ground));
        CHECK(off.exposure == Catch::Approx(g.exposure).epsilon(1e-6));
        // On (the default) differs on the walls.
        const LightRig on = ComputeLightRig(in, LightingTuning{});
        CHECK_FALSE(same(on.skyHorizon, g.horizon));
    }
    // baseline (every switch off) is the same rig.
    const LightRig b = ComputeLightRig(ClearSkyClassicSun(12.7f), TuningForMode(LightingMode::Baseline));
    CHECK(same(b.skyHorizon, golden[1].horizon));
    CHECK(same(b.sun, golden[1].sun));
}

TEST_CASE("Remaster light rig: the low-sun fill never lights a wall above the visible sky in twilight",
          "[Graphics][remaster]")
{
    // The twilight cap of the sky fill holds for walls with the fill on: a
    // vertical face goes at most to the visible sky, and never below what it
    // had without the fill. A clear sky's fog colour caps far above the fill,
    // so by day the fill is unchanged to the bit (the case above).
    for (float e : {-1.0f, 0.0f, 1.0f, 2.0f, 3.0f, 5.0f})
    {
        CAPTURE(e);
        LightRigInputs in = ClearSkyClassicSun(e);
        in.fogColor = Rgb(0.20f, 0.12f, 0.12f); // dim dusk sky
        const float skyShown = Luminance(ToLinear(in.fogColor));
        const LightRig on = ComputeLightRig(in, WithLowSunRig(true));
        const LightRig off = ComputeLightRig(in, WithLowSunRig(false));
        for (const auto& [lit, plain] :
             {std::pair{on.skyHorizon, off.skyHorizon}, std::pair{on.skyHorizonSun, off.skyHorizonSun}})
        {
            const float shown = Luminance(lit) * kBrightness * on.exposure;
            const float before = Luminance(plain) * kBrightness * off.exposure;
            CAPTURE(shown, before, skyShown);
            CHECK(shown <= std::max(skyShown, before) * 1.001f);
            CHECK(shown >= before * 0.999f);
        }
    }
    // Under the dim sky the fill still lifts the shaded wall where it can.
    LightRigInputs in = ClearSkyClassicSun(1.0f);
    in.fogColor = Rgb(0.20f, 0.12f, 0.12f);
    CHECK(Luminance(ComputeLightRig(in, WithLowSunRig(true)).skyHorizon) >
          1.2f * Luminance(ComputeLightRig(in, WithLowSunRig(false)).skyHorizon));
}

TEST_CASE("Remaster debug views: the albedo view is drawn by the lit shaders", "[Graphics][remaster]")
{
    // debug=7 shows each lit surface's albedo (display space), a
    // calibration view: with post=0 a shot of it divides a lit shot
    // into the light on every surface. The lit shaders see it as 4; the post
    // pass's views (4 ambient occlusion, 5 linear depth) leave them alone, and
    // the light tiles' view (6) the backend passes through itself.
    CHECK(kDebugViewAlbedo == 7);
    CHECK(ShaderDebugView(0) == 0.0f);
    CHECK(ShaderDebugView(1) == 1.0f);
    CHECK(ShaderDebugView(2) == 2.0f);
    CHECK(ShaderDebugView(3) == 3.0f);
    CHECK(ShaderDebugView(4) == 0.0f);
    CHECK(ShaderDebugView(5) == 0.0f);
    CHECK(ShaderDebugView(6) == 0.0f);
    CHECK(ShaderDebugView(kDebugViewAlbedo) == kShaderDebugAlbedo);
    CHECK(ShaderDebugView(8) == 0.0f);
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
    // and its debug view, the TAA resolve and its sharpen, the sky table's
    // two passes, the luma sharpen, the canopy's sun-visibility pass, the air
    // pass's march, blur, mean and apply, the reflections' tag readout,
    // shares, trace, rough blur and composite, and the combat shadow mask and
    // its blur, the surfaces' horizon occlusion and its accumulation
    // over frames, the photoreal camera's meter, and the volumetric fog's
    // injection, integration, near field, apply, horizon ring and night glow
    CHECK(post == 35);
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
    CHECK(strings >= 32); // today 3 VS, 10 prelude pieces and 4 mains; 15 post strings
}

TEST_CASE("The camera's meter reads the sky round the sun at the plain roll-off (look fix exposure)",
          "[Graphics][Shaders][remaster]")
{
    // The meter pass takes each sky tap through meterSkyLight (PhotoCamera.cpp
    // SkyMeterLight) before its luminance, so the mean, the blocks' brightest
    // tap and their brightest sky tap all read the sky at the plain roll-off;
    // its constants come in as uniforms from PhotoCamera.hpp (EngineGL33_Camera.cpp).
    const std::string meter = gl::PostShaderSource(gl::PostShader::Meter);
    const size_t fn = meter.find("float meterSkyLight(vec3 pre, vec2 uv)");
    REQUIRE(fn != std::string::npos);
    const size_t use = meter.find("if (sky) pre *= meterSkyLight(pre, uv);");
    REQUIRE(use != std::string::npos);
    CHECK(use > fn);
    const size_t lum = meter.find("float l = log2(", use);
    CHECK(lum != std::string::npos);
    for (const char* u :
         {"uniform vec4 meterSky;", "uniform vec4 meterSun;", "uniform vec4 meterProj;", "uniform vec4 meterRect;"})
    {
        CAPTURE(u);
        CHECK(meter.find(u) != std::string::npos);
    }
}

TEST_CASE("The photoreal camera's encoding in the shaders is PhotoCamera's", "[Graphics][Shaders][remaster]")
{
    // The lit prelude stores with the same text the post passes decode with.
    const std::string codec = gl::kCameraCodecGLSL;
    CHECK(Piece(gl::FragmentPiece::Inputs).find(codec) != std::string::npos);
    for (gl::PostShader s :
         {gl::PostShader::Composite, gl::PostShader::BloomDown, gl::PostShader::Shafts, gl::PostShader::Meter,
          gl::PostShader::AirApply, gl::PostShader::SsrTrace, gl::PostShader::SsrComposite})
    {
        CAPTURE(gl::PostShaderName(s));
        CHECK(std::string(gl::PostShaderSource(s)).find(codec) != std::string::npos);
    }
    // Its constants are PhotoCamera.hpp's.
    auto constant = [&](const std::string& src, const char* name)
    {
        const std::regex re(std::string("const float ") + name + R"(\s*=\s*([0-9.]+);)");
        std::smatch m;
        REQUIRE(std::regex_search(src, m, re));
        return std::stof(m[1].str());
    };
    CHECK(constant(codec, "kCamKnee") == Catch::Approx(kEncodeKnee));
    CHECK(constant(codec, "kCamMax") == Catch::Approx(kDecodeMax));
    CHECK(constant(codec, "kCamPower") == Catch::Approx(kEncodePower));
    CHECK(constant(codec, "kCamKneeCode") == Catch::Approx(std::pow(kEncodeKnee, 1.0f / 2.2f)).epsilon(1e-5));
    CHECK(constant(codec, "kCamFoldSpan") ==
          Catch::Approx(1.0f - std::pow(kEncodeKnee / kDecodeMax, kEncodePower)).epsilon(1e-5));
    const std::string inputs = Piece(gl::FragmentPiece::Inputs);
    CHECK(constant(inputs, "kCamSunRadius") == Catch::Approx(kSunDiscRadiusDeg * 3.14159265f / 180.0f).epsilon(1e-4));
    CHECK(constant(inputs, "kCamSunLevel") == Catch::Approx(kSunDiscLevel));
    CHECK(constant(inputs, "kCamSunWhite") == Catch::Approx(kSunDiscWhite));
    const std::string skyPiece = Piece(gl::FragmentPiece::Sky);
    CHECK(constant(skyPiece, "kCamSkyKnee") == Catch::Approx(kSkyRolloffKnee));
    CHECK(constant(skyPiece, "kCamSkyCap") == Catch::Approx(kSkyRolloffCap));
    // The grey deck's cloud base, faded in above the band
    // the grey-sky horizon ring measures (3.8 degrees up, sine 0.066).
    CHECK(constant(skyPiece, "kCamDeckRelief") == Catch::Approx(kDeckRelief));
    CHECK(constant(skyPiece, "kCamDeckDark") == Catch::Approx(kDeckDark));
    CHECK(constant(skyPiece, "kCamDeckFromSin") == Catch::Approx(kDeckFromSin));
    CHECK(constant(skyPiece, "kCamDeckFullSin") == Catch::Approx(kDeckFullSin));
    // Under rain the deck darkens further, read from the
    // wet weather's rain density.
    CHECK(constant(skyPiece, "kCamDeckRainDark") == Catch::Approx(kDeckRainDark));
    CHECK(kDeckRainDark > 0.0f);
    CHECK(kDeckRainDark < 0.6f);
    CHECK(skyPiece.find("kCamDeckRainDark * clamp(litWeather.y, 0.0, 1.0)") != std::string::npos);
    CHECK(kDeckDark < 1.0f);
    CHECK(kDeckFromSin > 0.1f);
    CHECK(kDeckFullSin > kDeckFromSin);
    CHECK(constant(skyPiece, "kCamSkySunCap") == Catch::Approx(kSkySunCap));
    CHECK(constant(skyPiece, "kCamSkyGlowRad") == Catch::Approx(kSkySunGlowRad));
    // The low sun's warm glow is a band along the horizon:
    // the blue-turning mask measures the angle with the height above the sun
    // stretched (LowSunGlowCos).
    CHECK(constant(skyPiece, "kCamGlowFlat") == Catch::Approx(kLowSunGlowFlat));
    CHECK(skyPiece.find("float away = 1.0 - smoothstep(0.86, 0.985, camLowSunGlowCos(d));") != std::string::npos);
    // Every sky roll-off knows its direction to the sun (the glow round it),
    // and the sun's disc is round on the screen (solved per pixel, not the
    // angle's oval near the corners).
    CHECK(skyPiece.find("vec3 camSkyRolloff(vec3 lin, vec3 d)") != std::string::npos);
    CHECK(skyPiece.find("camSkyRolloff(a)") == std::string::npos);
    const std::string common = gl::RemasterFragmentCommon();
    const size_t disc = common.find("vec4 camSunDisc()");
    REQUIRE(disc != std::string::npos);
    const std::string discBody = common.substr(disc, common.find("\n}\n", disc) - disc);
    CHECK(discBody.find("dFdx(d)") != std::string::npos);
    CHECK(discBody.find("kCamSunWhite") != std::string::npos);
    // The clear day's blue (DaySkyDeepen) is the dome's, with
    // DaySkyDeepWeight's ramps, inside the camera's look of the clear sky.
    CHECK(constant(skyPiece, "kCamSkyDeep") == Catch::Approx(kCameraDaySkyDeep));
    CHECK(constant(skyPiece, "kCamSkyDeepFloor") == Catch::Approx(kCameraDaySkyFloor));
    CHECK(skyPiece.find("a = camSkyDeepen(a, day * smoothstep(0.03, 0.4, e)") != std::string::npos);
    CHECK(skyPiece.find("smoothstep(0.86, 0.985, dot(d, skySunDir()))));") != std::string::npos);
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
        {P::Sky, "vec3 skyRadiance("},
        {P::Sky, "vec3 atmoFogColor("},
        {P::Sky, "vec3 shadeSkyDome("},
        {P::Sky, "vec3 skyReflection("},
        {P::Puddles, "vec2 wetPuddle("},
        {P::Lights, "uniform FrameLights {"},
        {P::Lights, "vec3 localLight("},
        {P::Lights, "vec3 localLightsTiled("},
        {P::Lights, "vec3 localLights("},
        {P::Lights, "vec3 lightTileView("},
        {P::Surface, "vec3 glossy("},
        {P::Surface, "void wetSurface("},
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

    // The screen vertex shader is assembled the same way (kScreenVertexPieces):
    // its head, the shared LitConstants block and its main.
    const std::string vsStart = "kScreenVertexPieces[] = {";
    const size_t vsOpen = source.find(vsStart);
    REQUIRE(vsOpen != std::string::npos);
    const size_t vsClose = source.find("};", vsOpen);
    REQUIRE(vsClose != std::string::npos);
    const std::vector<std::string> vsNames =
        ListedNames(source.substr(vsOpen + vsStart.size(), vsClose - vsOpen - vsStart.size()));
    CHECK(vsNames.size() == 3);
    const char* screen = gl::RemasterVertexSource(0); // VSScreen
    REQUIRE(screen != nullptr);
    CHECK(Joined(vsNames, RawStringsOf("EngineGL33_LightingShaders")) == screen);
    CHECK(CountOf(screen, gl::LitConstantsBlock()) == 1);
}

TEST_CASE("Post shaders are the pieces their table names", "[Graphics][Shaders][remaster]")
{
    // The same contract for the post pass: kPostShaders in
    // EngineGL33_PostShaders.cpp, read the way the lighting lab reads it, gives
    // each program the engine compiles, under its override name. A row may
    // name pieces of the remastered prelude (the air pass pastes the
    // LitConstants block, the terrain and clouds piece and the sky piece, the
    // reflections' trace the block and the sky piece), so the names resolve
    // over both families, as glsl_extract.py does.
    const std::string source = ReadTextFile(Gl33Dir() / "EngineGL33_PostShaders.cpp");
    const size_t open = source.find("kPostShaders[] = {");
    REQUIRE(open != std::string::npos);
    const size_t close = source.find("\n};", open);
    REQUIRE(close != std::string::npos);
    const std::string table = source.substr(open, close - open);
    auto raw = RawStringsOf("EngineGL33_Post");
    for (auto& [name, body] : RawStringsOf("EngineGL33_LightingShaders"))
        raw.emplace(name, body);
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

TEST_CASE("Remaster tree crowns as a canopy: the shader's switch, its terms and its off path",
          "[Graphics][Shaders][remaster]")
{
    // crownCanopy: the backend
    // sets litHorizonSun.w from the switch's frame gate, and shadeRemastered
    // lights a crown (not grass, which shades as terrain) as a canopy only
    // while it is 1. Off, every term is the stock one.
    const std::string lighting = ReadTextFile(Gl33Dir() / "EngineGL33_Lighting.cpp");
    CHECK(lighting.find("const float crownCanopy = _featureGates[rm::Feature::CrownCanopy].on ? 1.0f : 0.0f;") !=
          std::string::npos);
    CHECK(lighting.find("SetSlot(LitConst::HorizonSun, rig.skyHorizonSun.r, rig.skyHorizonSun.g, "
                        "rig.skyHorizonSun.b, crownCanopy);") != std::string::npos);
    // The debug view the lit shaders draw comes from one mapping (the light
    // tiles' and the combat mask's views aside, which the backend passes through).
    CHECK(lighting.find("const float debug = dv == fl::kDebugViewLightTiles || dv == fl::kDebugViewCombatMask ? "
                        "static_cast<float>(dv)") != std::string::npos);
    CHECK(lighting.find(": rm::ShaderDebugView(dv);") != std::string::npos);

    const std::string piece = Piece(gl::FragmentPiece::Shade);
    const size_t start = piece.find("vec3 shadeRemastered(");
    REQUIRE(start != std::string::npos);
    const std::string shade = piece.substr(start);
    CHECK(shade.find("bool canopy = crown && !terrain && litHorizonSun.w > 0.5;") != std::string::npos);
    // Off: the stock crown core (0.6 of the sky) and the stock leaf shadow;
    // on: the deeper core, the sun through the crown and the low-sun canopy.
    CHECK(shade.find("crownSky = mix(canopy ? 0.45 : 0.6, 1.0, smoothstep(0.1, 0.95, vCrown.w));") !=
          std::string::npos);
    CHECK(shade.find("sunShadow = mix(0.45, 1.0, sunVisibility) * crownSun;") != std::string::npos);
    // The low sun is the sun (under the moon key litSun carries the moon).
    CHECK(shade.find("float lowSun = 1.0 - smoothstep(12.0, 35.0, skySunElevation());") != std::string::npos);
    CHECK(shade.find("crownSun = mix(1.0, exp(-crownSunPath(cn, vCrown.w, l)), 0.75) * mix(1.0, 0.65, lowSun);") !=
          std::string::npos);
    CHECK(shade.find("crownSky *= mix(1.0, 0.75, lowSun);") != std::string::npos);
    // The canopy terms sit inside `if (canopy)` blocks, crownSun and crownSky
    // start at 1 (the stock values), and the saturated canopy albedo too.
    CHECK(shade.find("float crownSun = 1.0;") != std::string::npos);
    CHECK(std::regex_search(shade, std::regex(R"(if \(canopy\) \{\s*float lowSun)")));
    CHECK(
        std::regex_search(shade, std::regex(R"(if \(canopy\) \{\s*(//[^\n]*\n\s*)*vec3 p = pow\(a, vec3\(1\.3\)\);)")));
    CHECK(CountOf(shade, "pow(a, vec3(1.3))") == 1);
    // The path through the crown is declared in the same piece, before it.
    CHECK(piece.find("float crownSunPath(vec3 cnDir, float r, vec3 l) {") < start);
    // The albedo debug view.
    CHECK(shade.find("if (litFlags.z < 3.5) return nLight * 0.5 + 0.5;\n        return albedo;") != std::string::npos);
    // The LitConstants block names the w.
    CHECK(
        std::string(gl::LitConstantsBlock())
            .find(
                "vec4 litHorizonSun;   // 23: rgb on a vertical surface facing the sun, w = crowns lit as a canopy") !=
        std::string::npos);
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
        {"litMatClass", LC::MatClass, 1},
        {"litTaa", LC::Taa, 1},
        {"litPcss", LC::Pcss, 1},
        {"litSkyRight", LC::SkyRight, 1},
        {"litSkyUp", LC::SkyUp, 1},
        {"litSkyFwd", LC::SkyFwd, 1},
        {"litAtmo", LC::Atmo, 1},
        {"litAtmoCloudSun", LC::AtmoCloudSun, 1},
        {"litAtmoGlow", LC::AtmoGlow, 1},
        {"litAtmoDome", LC::AtmoDome, 1},
        {"litSky", LC::Sky, 1},
        {"litSkyShadow", LC::SkyShadow, 1},
        {"litSkySun", LC::SkySun, 1},
        {"litNight", LC::Night, 1},
        {"litPlanned", LC::Planned, LC::PlannedCount},
        {"litWeather", LC::Weather, 1},
        {"litWater0", LC::Water0, 1},
        {"litWater1", LC::Water1, 1},
        {"litCanopy", LC::Canopy, 1},
        {"litDetail", LC::Detail, 1},
        {"litHaze", LC::Haze, 1},
        {"litSurface", LC::Surface, 1},
        {"litSurfCanopy", LC::SurfaceCanopy, 1},
        {"litSurface2", LC::Surface2, 1},
        {"litSkySh", LC::SurfaceSh, LC::SurfaceShCount},
        {"litSurface3", LC::Surface3, 1},
        {"litCamera", LC::Camera, 1},
        {"litCameraLook", LC::CameraLook, 1},
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
    // only the one: a hand copy (earlier versions pasted their own into post
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

    // The planned slots: the one between the night and the weather is the
    // planned padding, the rest continue the declared ones (the material class
    // at 58, the sun shadow slots 59 and 60, the sky slots 61 to 67, the clouds
    // slots 68 and 69, the night slots 70 and 71, the weather at 73, the water
    // slots 74 and 75, the canopy at 76).
    CHECK(static_cast<int>(LC::MatClass) == 58);
    CHECK(static_cast<int>(LC::Taa) == 59);
    CHECK(static_cast<int>(LC::Pcss) == 60);
    CHECK(static_cast<int>(LC::SkyRight) == 61);
    CHECK(static_cast<int>(LC::AtmoDome) == 67);
    CHECK(static_cast<int>(LC::Sky) == 68);
    CHECK(static_cast<int>(LC::SkyShadow) == 69);
    // The night sits at 71 and the sky's own sun at 70, which the tiled light
    // list left free (it rides in slot 22).
    CHECK(static_cast<int>(LC::SkySun) == 70);
    CHECK(static_cast<int>(LC::Night) == 71);
    CHECK(static_cast<int>(LC::Planned) == 72);
    CHECK(LC::PlannedCount == 1);
    CHECK(gl::LitConstPlanned::CombatShadow == static_cast<int>(LC::Planned));
    CHECK(static_cast<int>(LC::Weather) == 73);
    CHECK(static_cast<int>(LC::Water0) == 74);
    CHECK(static_cast<int>(LC::Water1) == 75);
    // The canopy at 76 and the terrain detail at 77.
    CHECK(static_cast<int>(LC::Canopy) == 76);
    CHECK(static_cast<int>(LC::Canopy) == static_cast<int>(LC::Water1) + 1);
    CHECK(static_cast<int>(LC::Detail) == 77);
    // The haze at 78, the last planned slot; the surface slots follow it (79
    // to 88).
    CHECK(static_cast<int>(LC::Haze) == 78);
    CHECK(static_cast<int>(LC::Haze) == static_cast<int>(LC::Detail) + 1);
    CHECK(gl::LitConstPlanned::End == static_cast<int>(LC::Haze) + 1);
    CHECK(static_cast<int>(LC::Surface) == 79);
    CHECK(static_cast<int>(LC::Surface3) == 88);
    // The photoreal camera follows the surface slots.
    CHECK(static_cast<int>(LC::Camera) == 89);
    CHECK(static_cast<int>(LC::Count) == static_cast<int>(LC::CameraLook) + 1);

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
    // The soft shadows read the same array as raw depth on unit 6.
    CHECK(gl::kShadowRawUnit == 6);

    // Every sampler a remastered program declares has its unit in the table
    // (tex0 and tex1 are the material units): an unbound sampler reads unit 0.
    // Integer samplers (usampler2D, the light tiles) count too.
    std::set<std::string> declared;
    static const std::regex sampler(R"(uniform\s+[iu]?sampler\w+\s+(\w+)\s*;)");
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

TEST_CASE("Remaster sky table: with its blends at zero every draw is the one before it",
          "[Graphics][Shaders][remaster][atmosphere]")
{
    // The sky table is off (switch, gate, classic) exactly when the engine
    // uploads litAtmo.x (dome blend) and litAtmo.z (aerial perspective) as 0.
    // Then the screen vertex shader keeps vLitMode 0, the fragment shaders take
    // their pre-existing paths and the fog keeps its classic colours: the
    // feature's "off" is the frame before it.
    const std::string screen = gl::RemasterVertexSource(0); // VSScreen
    // The sky clouds widened the gate by their slots (litSky, zero while they are
    // off: see the clouds' own off case), the photoreal camera by its own
    // (litCamera.x, zero while it is off), which only gives the sun disk its
    // view ray (vLitMode stays 0).
    const size_t gate = screen.find(
        "if (litAtmo.x > 0.0 || litAtmo.z > 0.0 || litSky.x > 0.0 || litSky.z != 0.0 || litCamera.x > 0.5) {");
    REQUIRE(gate != std::string::npos);
    // vLitMode is 0 before the gate, 2 (dome), -2 (lit sky clouds) or -1
    // (pre-lit world) inside it.
    CHECK(screen.find("vLitMode = 0.0;") < gate);
    CHECK(screen.find("vLitMode = 2.0;") > gate);
    CHECK(screen.find("vLitMode = -2.0;") > gate);
    CHECK(screen.find("vLitMode = -1.0;") > gate);
    CHECK(CountOf(screen, "vLitMode = ") == 4);
    CHECK(screen.find("bool dome = litSkyFwd.w > 0.5 && litSkyFwd.w < 1.5;") != std::string::npos);
    CHECK(screen.find("if (dome && (litAtmo.x > 0.0 || litSky.x > 0.0 || litSky.z != 0.0)) {") != std::string::npos);
    // The pre-lit world (and not the sun disk, sky mode 3) takes the table's fog.
    CHECK(screen.find("} else if (litAtmo.z > 0.0 && litSkyFwd.w < 2.5) {") != std::string::npos);
    // The ray from the screen position: NDC from the pixel position the vertex
    // shader itself projects with, times the perspective w.
    CHECK(screen.find("vec2 ndc = vec2(aPos.x * vpScale.x - 1.0, 1.0 - aPos.y * vpScale.y);") != std::string::npos);
    CHECK(screen.find("(litSkyFwd.xyz + ndc.x * litSkyRight.xyz + ndc.y * litSkyUp.xyz) * w") != std::string::npos);

    // Fog: the table's horizon colour only behind litAtmo.z, and read only
    // where fog shows (vFogTC 1 ignores the fog colour).
    const std::string finish = Piece(gl::FragmentPiece::Finish);
    CHECK(CountOf(finish, "atmoFogColor(") == 2);
    CHECK(finish.find("if (litAtmo.z > 0.0 && vFogTC < 1.0) fogC = atmoFogColor(vWorldRel, fogC);") !=
          std::string::npos);
    CHECK(finish.find("} else if (vLitMode < -0.5 && litAtmo.z > 0.0 && vFogTC < 1.0) {") != std::string::npos);
    // The sea fogs through finishFragment like the detail program it
    // replaces, and its sky reflection is the classic one while litAtmo.x is 0.
    const std::string water = gl::RemasterFragmentMain(3); // PSWater
    CHECK(CountOf(water, "atmoFogColor(") == 0);
    CHECK(CountOf(water, "finishFragment(r0, ambientShare, true);") == 1);
    // The dome branch is PSNormal's; the other mains light only vLitMode 1.
    const std::string normal = gl::RemasterFragmentMain(0); // PSNormal
    CHECK(normal.find("if (vLitMode > 1.5) {") != std::string::npos);
    CHECK(normal.find("if (litAtmo.x > 0.0) rgb = shadeSkyDome(t0, classic.rgb);") != std::string::npos);
    for (int ps : {1, 2}) // PSDetail, PSGrass
    {
        CAPTURE(ps);
        CHECK(std::string(gl::RemasterFragmentMain(ps)).find("if (vLitMode > 0.5 && vLitMode < 1.5) {") !=
              std::string::npos);
    }
    CHECK(water.find("bool lit = vLitMode > 0.5 && vLitMode < 1.5;") != std::string::npos);
    // The dome returns to the classic colour as its blend goes to 0 (night).
    const std::string sky = Piece(gl::FragmentPiece::Sky);
    CHECK(sky.find("return mix(classic, disp, litAtmo.x);") != std::string::npos);
    CHECK(sky.find("return mix(classicFog, disp, litAtmo.z);") != std::string::npos);
    CHECK(sky.find("if (litAtmo.x <= 0.0) return classic;") != std::string::npos);
}

TEST_CASE("Remaster sky table: the shaders read the table the way Atmosphere.cpp lays it out",
          "[Graphics][Shaders][remaster][atmosphere]")
{
    const std::string sky = Piece(gl::FragmentPiece::Sky);
    // Three regions of kSkyViewSize, the parametrisation of SampleSkyView.
    CHECK(kSkyViewRegions == 3);
    CHECK(sky.find("const float n = " + std::to_string(kSkyViewSize) + ".0;") != std::string::npos);
    CHECK(sky.find("texture(skyLut, vec2(u / 3.0, v))") != std::string::npos);
    CHECK(sky.find("texture(skyLut, vec2((1.0 + u) / 3.0, v))") != std::string::npos);
    CHECK(sky.find("mean = texture(skyLut, vec2(2.5 / 3.0, v))") != std::string::npos);
    CHECK(sky.find("return 0.5 + 0.5 * sign(l) * sqrt(abs(l) / (0.5 * kSkyPi));") != std::string::npos);
    // The Mie asymmetry and the cloud mask's dead zone come from the engine
    // (Atmosphere.hpp), not from constants of the shader's own.
    CHECK(sky.find("float g = litSkyRight.w;") != std::string::npos);
    // The aureole's peak (AtmosphereMiePhase) in the dome and in the row mean
    // pass, the numbers of Atmosphere.hpp.
    CHECK(atmo::kMieAureoleG == 0.97f);
    CHECK(atmo::kMieAureoleShare == 0.3f);
    CHECK(sky.find("const float kSkyAureoleG = 0.97;") != std::string::npos);
    CHECK(sky.find("const float kSkyAureoleShare = 0.3;") != std::string::npos);
    CHECK(sky.find("return mix(broad, peak, kSkyAureoleShare);") != std::string::npos);
    {
        const std::string meanPass = gl::PostShaderSource(gl::PostShader::SkyViewMean);
        CHECK(meanPass.find("const float kAureoleG = 0.97;") != std::string::npos);
        CHECK(meanPass.find("const float kAureoleShare = 0.3;") != std::string::npos);
        CHECK(meanPass.find("return mix(broad, peak, kAureoleShare);") != std::string::npos);
    }
    CHECK(sky.find("(lumT - bg - litAtmo.w)") != std::string::npos);
    CHECK(sky.find("(lumT - bg - 0.015)") == std::string::npos);
    // A row keeps the original dome's brightness, its sun glow included (the
    // glow weight is region 2's alpha), and the hue blend moves the hue only:
    // the original hue is relit by the brightness ratio, not channel by
    // channel (that turned the sunset horizon olive).
    CHECK(sky.find("vec3 classicLin = toLinear(min(baseDisp + glowDisp * mean.a, capDisp));") != std::string::npos);
    CHECK(sky.find("vec3 relit = classicLin * (dot(a, kLuma) / rowLum);") != std::string::npos);
    CHECK(sky.find("a / m") == std::string::npos);
    CHECK(sky.find("skyClear(d, bgC * litAtmoDome.rgb, bgC * litAtmoGlow.rgb, bgC)") != std::string::npos);
    // A cloud's colour is the texel taken out of the sky behind it by the
    // mask's alpha (a thin cirrus is the mask's white, not the composited
    // texel), and the clouds take the row's glow too.
    CHECK(sky.find("vec3 cloudDisp = clamp(bgC + (t0.rgb - bgC) / max(alpha, 0.05), 0.0, 1.0);") != std::string::npos);
    CHECK(sky.find("vec3 rowLight = toLinear(min(litAtmoDome.rgb + litAtmoGlow.rgb * rowGlow, vec3(1.0)));") !=
          std::string::npos);

    // The GPU passes march as SkyViewTexel and average as SkyViewRowMean.
    const std::string view = gl::PostShaderSource(gl::PostShader::SkyView);
    const std::string mean = gl::PostShaderSource(gl::PostShader::SkyViewMean);
    REQUIRE_FALSE(view.empty());
    REQUIRE_FALSE(mean.empty());
    CHECK(gl::PostShaderName(gl::PostShader::SkyView) == std::string("psSkyView"));
    CHECK(gl::PostShaderName(gl::PostShader::SkyViewMean) == std::string("psSkyViewMean"));
    CHECK(view.find("float tNext = tMax * f1 * f1;") != std::string::npos);
    CHECK(view.find("float t = tPrev + 0.3 * dt;") != std::string::npos);
    CHECK(view.find("float disc = rmu * rmu + (atmoRadii.y - r) * (atmoRadii.y + r);") != std::string::npos);
    CHECK(view.find("float disc = rmu * rmu - (r - atmoRadii.x) * (r + atmoRadii.x);") != std::string::npos);
    CHECK(view.find("vec3 srcR = scatR * phaseR * ts + (scatR + scatM) * ms;") != std::string::npos);
    CHECK(mean.find("float phi = (float(a) + 0.5) / float(taps) * kPi;") != std::string::npos);
    CHECK(mean.find("if (ij.x < 2 * n) {") != std::string::npos);
    CHECK(mean.find("fragColor = vec4(sum * (1.0 / float(taps)), glow * (1.0 / float(taps)));") != std::string::npos);
    // The numbers reach them from Atmosphere.hpp.
    const std::string engine = ReadTextFile(Gl33Dir() / "EngineGL33_SkyTable.cpp");
    for (const char* use : {"rm::kSkyViewSteps", "rm::kSkyViewMeanTaps", "rm::atmo::kMieG", "rm::kSkyViewSize",
                            "rm::kTransmittanceWidth", "rm::kMultiScatterSize", "rm::kSkyViewRegions * n"})
    {
        CAPTURE(use);
        CHECK(engine.find(use) != std::string::npos);
    }
}

TEST_CASE("Remaster sea: the water program is the variant C model on the shared pieces",
          "[Graphics][Shaders][remaster][SeaWaves]")
{
    const std::string water = gl::RemasterFragmentMain(3); // PSWater
    REQUIRE_FALSE(water.empty());
    // The next animation frame on its own unit, from the one unit table.
    bool row = false;
    for (const auto& s : gl::kLitSamplers)
        row = row || (std::string(s.name) == "seaNext" && s.unit == gl::kSeaNextFrameUnit);
    CHECK(row);
    CHECK(gl::kSeaNextFrameUnit == 11);
    CHECK(water.find("uniform sampler2D seaNext;") != std::string::npos);
    // The cross-fade: litWater0.y of the next frame, and the relief gain with
    // SeaWaves.hpp's frame correlation.
    CHECK(water.find("mix(a, texture(seaNext, uv), litWater0.y)") != std::string::npos);
    std::ostringstream rho;
    rho << "const float kSeaFrameRho = " << kSeaFrameCorrelation << ";";
    CHECK(water.find(rho.str()) != std::string::npos);
    CHECK(water.find("2.0 * w * (1.0 - w) * kSeaFrameRho") != std::string::npos);
    CHECK(water.find("grad *= litWater0.x * inversesqrt(max(spread, 1e-6));") != std::string::npos);
    // The engine's constants: the relief, the mean sky weight and its tint,
    // the sheen light and its flat-normal share at a high sun.
    for (const char* use : {"litWater0.x", "litWater0.z", "litWater0.w", "litWater1.rgb", "litWater1.w"})
    {
        CAPTURE(use);
        CHECK(water.find(use) != std::string::npos);
    }
    // The sky term is the shared reflection lookup (the sky table's sky when
    // it is on), not a copy of its own.
    CHECK(CountOf(water, "skyReflection(r)") == 1);
    CHECK(CountOf(gl::RemasterFragmentCommon(), "vec3 skyReflection(vec3 r) {") == 1);
    // Local light through the shared lights piece with the shaded surfaces'
    // clamp (1.0), not an earlier 1.5.
    CHECK(CountOf(water, "localLights(n, kd, ka) * litGround.w, vec3(1.0))") == 1);
    CHECK(water.find("litGround.w, vec3(1.5)") == std::string::npos);
    // No sea detection by material any more (an earlier isSea()), and the
    // previous main's constant bump normal (the detail texture read as one) is
    // gone.
    CHECK(water.find("isSea(") == std::string::npos);
    CHECK(water.find("bumpNormal") == std::string::npos);
    // Pre-lit faces draw exactly as the detail program's pre-lit branch, and
    // every branch ends in the shared tail (night eye, AO alpha, fog).
    const std::string detail = gl::RemasterFragmentMain(1); // PSDetail
    const std::string preLit = "r0 = vColor * t0;\n        r0 *= constColor;\n        r0.rgb *= t1.a * 2.0;\n"
                               "        r0 += vSpecColor;";
    CHECK(detail.find(preLit) != std::string::npos);
    CHECK(water.find(preLit) != std::string::npos);
    CHECK(water.find("finishFragment(r0, ambientShare, true);") != std::string::npos);
}

TEST_CASE("Remaster sea: the backend routes the landscape's sea faces only inside the sea scope",
          "[Graphics][remaster][SeaWaves]")
{
    // The descriptor flag comes from the scope, and the scope opens only in the
    // remastered world pass with the switch on and gated on.
    const std::string queue = ReadTextFile(Gl33Dir() / "EngineGL33_Queue.cpp");
    CHECK(CountOf(queue, "ctx.remasteredSea = SeaRouted();") == 1);
    const std::string sea = ReadTextFile(Gl33Dir() / "EngineGL33_Sea.cpp");
    REQUIRE_FALSE(sea.empty());
    for (const char* gate :
         {"!_remasterPrograms", "_nightVision", "_portraitTargetSize", "_worldSceneEnded",
          "!rm::FeatureOn(_lighting.features, rm::Feature::SeaWaves)", "!_featureGates[rm::Feature::SeaWaves].on"})
    {
        CAPTURE(gate);
        CHECK(sea.find(gate) < sea.find("_seaOpen = true;"));
    }
    CHECK(CountOf(sea, "_seaOpen = true;") == 1);
    CHECK(CountOf(sea, "_seaOpen = false;") == 1);
    // The sea keeps its ripple layer on unit 1 under the water family.
    const std::string state = ReadTextFile(Gl33Dir() / "EngineGL33_State.cpp");
    CHECK(state.find("fmt = d.texGen == Poseidon::render::TexGenMode::Detail ? SpecularTex : DetailTex;") !=
          std::string::npos);
    // The landscape opens the scope around its sea draws, and its sea faces
    // are the specular layer alone (never IsWater).
    const std::string land = ReadTextFile(std::filesystem::path(TESTS_ROOT_DIR).parent_path() / "engine" / "Poseidon" /
                                          "World" / "Terrain" / "LandscapeRender.cpp");
    CHECK(land.find("const int WaterFlags = SpecularTexture;") != std::string::npos);
    CHECK(CountOf(land, "water.OrSpecial(WaterFlags | NoClamp);") == 2);
    const size_t draw = land.find("void Landscape::DrawWater(");
    REQUIRE(draw != std::string::npos);
    const size_t begin = land.find("GEngine->BeginSea(other, otherWeight);", draw);
    const size_t loop = land.find("for (int x = bigRect.xBeg;", draw);
    const size_t end = land.find("GEngine->EndSea();", draw);
    const size_t next = land.find("struct GroundLayerInfo", draw);
    CHECK(begin < loop);
    CHECK(loop < end);
    CHECK(end < next);
    // The water bump map nothing sampled is gone with its missing file.
    CHECK(ReadTextFile(Gl33Dir() / "TextureGL33.hpp").find("GetWaterBumpMap") == std::string::npos);
}

namespace
{
// The value of `const float <name> = <value>;` in a GLSL source, or NaN.
float GlslConst(const std::string& source, const std::string& name)
{
    const std::regex decl("const float " + name + R"(\s*=\s*(-?[0-9.]+)\s*;)");
    std::smatch m;
    if (!std::regex_search(source, m, decl))
        return std::numeric_limits<float>::quiet_NaN();
    return std::stof(m[1].str());
}

// A float as the shaders write it: the shortest of 1 to 4 decimals that reads
// back exactly.
std::string GlslNumber(float v)
{
    for (int digits = 1; digits <= 4; ++digits)
    {
        std::ostringstream s;
        s << std::fixed;
        s.precision(digits);
        s << v;
        if (std::abs(std::stof(s.str()) - v) < 1e-5f)
            return s.str();
    }
    std::ostringstream s;
    s << v;
    return s.str();
}
} // namespace

TEST_CASE("Remaster sky clouds: with their slots at zero every draw is the one before them",
          "[Graphics][Shaders][remaster][clouds]")
{
    // The clouds switches are off (switch, gate, classic, night vision) exactly
    // when the engine uploads litSky as zero (EngineGL33_SkyClouds.cpp
    // UploadLitClouds). Then the dome keeps its texture coordinate and its
    // draw, the sky clouds and the sun disk pass through, and the shadows come
    // from the noise field: the frame before the sky clouds.
    const std::string screen = gl::RemasterVertexSource(0); // VSScreen
    CHECK(screen.find("if (dome && litSky.z != 0.0) vUV0.x += litSky.z;") != std::string::npos);
    CHECK(CountOf(screen, "vUV0 ") == 1);      // set once ...
    CHECK(CountOf(screen, "vUV0.x += ") == 1); // ... and turned once
    const size_t sheets = screen.find("} else if (litSkyFwd.w > 1.5 && litSkyFwd.w < 2.5 && litSky.x > 0.0) {");
    REQUIRE(sheets != std::string::npos);
    CHECK(screen.find("vLitMode = -2.0;", sheets) != std::string::npos);

    const std::string normal = gl::RemasterFragmentMain(0); // PSNormal
    CHECK(normal.find("else if (litSky.x > 0.0) rgb = litCloudsOverClassic(rgb, litCloudTermsDome(vUV0));") !=
          std::string::npos);
    CHECK(normal.find("if (vLitMode < -1.5) r0.rgb = litCloudsOverClassic(r0.rgb, litCloudTermsSheet(t0.a));") !=
          std::string::npos);
    CHECK(normal.find("if (litSkyFwd.w > 2.5 && litSky.y > 0.5)") != std::string::npos);
    // The sun disk fades (colour and coverage), it does not turn into a dark disk.
    CHECK(normal.find("r0 *= mix(1.0, kSkySunBehindCloud, skyShadowCoverToward(skySunDir()));") != std::string::npos);
    // Only PSNormal draws the sky.
    for (int ps : {1, 2, 3})
    {
        CAPTURE(ps);
        CHECK(std::string(gl::RemasterFragmentMain(ps)).find("litSky") == std::string::npos);
    }

    // The shadows: the dome only behind litSky.y, before the noise lookup.
    const std::string terrain = Piece(gl::FragmentPiece::TerrainClouds);
    const size_t dome = terrain.find("if (litSky.y > 0.5) return mix(1.0, litCloud.y, domeCloudCover(");
    REQUIRE(dome != std::string::npos);
    CHECK(dome < terrain.find("texture(cloudNoise,"));
    // The previous anchor's lookup only while it fades.
    CHECK(terrain.find("if (litSkyShadow.w > 0.0) {") != std::string::npos);

    // Lit clouds: on the dome only behind litSky.x, and the terms weigh
    // nothing at zero strength.
    const std::string sky = Piece(gl::FragmentPiece::Sky);
    const size_t lit = sky.find("if (litSky.x > 0.0) {\n        vec4 lc = litCloudTermsDome(vUV0);");
    REQUIRE(lit != std::string::npos);
    CHECK(lit < sky.find("vec3 disp = mix(skyDisplay(a), skyDisplay(cloudLin), cloud);"));
    CHECK(sky.find("if (t.w <= 0.0) return lin;") != std::string::npos);
    CHECK(sky.find("if (t.w <= 0.0) return disp;") != std::string::npos);
    CHECK(sky.find("if (litSky.x <= 0.0 || skySunElevation() >= kLitCloudFadeEndDeg) return vec4(1.0, 0.0, 0.0, "
                   "0.0);") != std::string::npos);
}

TEST_CASE("Remaster sky clouds: at a high sun the lit-cloud terms are identity, as SkyDensity's LitCloudTerms",
          "[Graphics][Shaders][remaster][clouds]")
{
    // litCloudTerms computes the terms the C++ mirror computes (relief, bellies,
    // darkening against the sun, lining) and fades every one of them with the
    // low sun on its way out, so from kLitCloudFadeEndDeg up a cloud keeps its
    // light: never only darker at midday (test_sky_density.cpp checks the
    // mirror is identity there).
    const std::string sky = Piece(gl::FragmentPiece::Sky);
    const size_t begin =
        sky.find("vec4 litCloudTerms(vec3 v, float d, float tau, float feature, float dark, float deck) {");
    REQUIRE(begin != std::string::npos);
    const size_t end = sky.find("\n}\n", begin);
    REQUIRE(end != std::string::npos);
    const std::string body = sky.substr(begin, end - begin);
    for (const char* line : {
             "float direct = smoothstep(-0.06, 0.04, l.y) * mix(0.45, 1.0, clamp(litSky.w, 0.0, 1.0));",
             "float relief = clamp((d - tau) * kLitCloudRelief, -0.35, 0.8);",
             "float gain = 1.0 + relief * direct;",
             "gain *= 1.0 + (0.55 - dark) * 0.7 * deck;",
             "gain *= 1.0 - 0.35 * pow(max(cosT, 0.0), 6.0) * smoothstep(0.35, 0.9, feature) * direct;",
             "float warm = clamp(relief, 0.0, 1.0) * direct;",
             "float rim = clamp((d - tau) * 3.0, 0.0, 1.0) * (1.0 - 0.5 * d);",
             "float hg = 0.008 / pow(max(1.64 - 1.6 * cosT, 1e-4), 1.5);",
             "float silver = hg * rim * kLitCloudSilver * direct;",
             "float lowSun = 1.0 - smoothstep(kLitCloudFadeStartDeg, kLitCloudFadeEndDeg, skySunElevation());",
             "return vec4(mix(1.0, gain, lowSun), warm * lowSun, silver * lowSun, litSky.x * smoothstep(0.02, 0.25, "
             "d));",
         })
    {
        CAPTURE(line);
        CHECK(body.find(line) != std::string::npos);
    }
    // The fade comes once, on the way out: no term is faded on its own.
    CHECK(CountOf(body, "lowSun") == 4);
    // From there up the shaders skip the march (the terms would be identity).
    CHECK(sky.find("if (litSky.x <= 0.0 || skySunElevation() >= kLitCloudFadeEndDeg) return vec4(1.0, 0.0, 0.0, "
                   "0.0);") != std::string::npos);
    CHECK(sky.find("if (litSky.x <= 0.0 || skySunElevation() >= kLitCloudFadeEndDeg || alpha <= 0.02) return "
                   "vec4(1.0, 0.0, 0.0, 0.0);") != std::string::npos);
    const LitCloudShade noon = LitCloudTerms(60.0f, 1.0f, 0.8f, 0.8f, 1.0f, 0.2f, 0.0f, 0.7f);
    CHECK(noon.gain == 1.0f);
    CHECK(noon.warm == 0.0f);
    CHECK(noon.silver == 0.0f);
}

TEST_CASE("Remaster sky clouds: the shaders use SkyDensity.hpp's constants and dome mapping",
          "[Graphics][Shaders][remaster][clouds]")
{
    const std::string terrain = Piece(gl::FragmentPiece::TerrainClouds);
    const std::string sky = Piece(gl::FragmentPiece::Sky);
    struct Const
    {
        const std::string* source;
        const char* glsl;
        float value;
    };
    const Const consts[] = {
        {&terrain, "kSkyShadowDeck", kSkyShadowDeckAltitude},
        {&terrain, "kSkyShadowThreshold", kSkyShadowThreshold},
        {&terrain, "kSkyShadowEdge", kSkyShadowEdge},
        {&terrain, "kSkyShadowAnchorRadius", kSkyShadowAnchorRadius},
        {&terrain, "kSkySunBehindCloud", kSkySunBehindCloud},
        {&sky, "kLitCloudRelief", kLitCloudRelief},
        {&sky, "kLitCloudSilver", kLitCloudSilver},
        {&sky, "kLitCloudStepDeg", kLitCloudStepDeg},
        {&sky, "kLitCloudFadeStartDeg", kLitCloudFadeStartDeg},
        {&sky, "kLitCloudFadeEndDeg", kLitCloudFadeEndDeg},
    };
    for (const Const& c : consts)
    {
        CAPTURE(c.glsl);
        CHECK(GlslConst(*c.source, c.glsl) == Catch::Approx(c.value));
    }

    // skyDomeUv: the rings of SkyDomeV, segment by segment.
    const float* e = kSkyDomeRingElevationDeg;
    const float* v = kSkyDomeRingV;
    REQUIRE(kSkyDomeRings == 5);
    const std::string segments[] = {
        "if (e < " + GlslNumber(e[1]) + ") v = mix(" + GlslNumber(v[0]) + ", " + GlslNumber(v[1]) + ", clamp((e + " +
            GlslNumber(-e[0]) + ") / " + GlslNumber(e[1] - e[0]) + ", 0.0, 1.0));",
        "else if (e < " + GlslNumber(e[2]) + ") v = mix(" + GlslNumber(v[1]) + ", " + GlslNumber(v[2]) + ", (e - " +
            GlslNumber(e[1]) + ") / " + GlslNumber(e[2] - e[1]) + ");",
        "else if (e < " + GlslNumber(e[3]) + ") v = mix(" + GlslNumber(v[2]) + ", " + GlslNumber(v[3]) + ", (e - " +
            GlslNumber(e[2]) + ") / " + GlslNumber(e[3] - e[2]) + ");",
        "else v = mix(" + GlslNumber(v[3]) + ", " + GlslNumber(v[4]) + ", clamp((e - " + GlslNumber(e[3]) + ") / " +
            GlslNumber(e[4] - e[3]) + ", 0.0, 1.0));",
    };
    for (const std::string& seg : segments)
    {
        CAPTURE(seg);
        CHECK(terrain.find(seg) != std::string::npos);
    }
    // u: azimuth from north toward east over 180 degrees, plus the turn.
    CHECK(terrain.find("float az = atan(w.x, w.z) * 57.29578;") != std::string::npos);
    CHECK(terrain.find("return vec2(az / 180.0 + litSky.z, v);") != std::string::npos);
    // The density and the deck weight as SkyCloudDensity and SkyDeckWeight.
    CHECK(terrain.find("float deck = m.g * (1.0 - smoothstep(0.35, 0.75, litSky.w));") != std::string::npos);
    CHECK(terrain.find("return max(m.r, deck) * mix(0.75 + 0.25 * m.b, 0.5 + 0.5 * m.b, deck);") != std::string::npos);
    const float w = SkyDeckWeight(0.5f);
    CHECK(w == Catch::Approx(1.0f - 0.375f * 0.375f * (3.0f - 2.0f * 0.375f)));
    CHECK(SkyCloudDensity(0.6f, 1.0f, 0.4f, w) ==
          Catch::Approx(std::max(0.6f, w) * ((0.75f + 0.25f * 0.4f) * (1.0f - w) + (0.5f + 0.5f * 0.4f) * w)));
    // The march steps as LitCloudStepAngleDeg, the low-sun fade as LitCloudLowSun.
    CHECK(sky.find("for (int k = 1; k <= " + std::to_string(kLitCloudSteps) + "; ++k) {") != std::string::npos);
    CHECK(sky.find("float a = radians(kLitCloudStepDeg) * fk * (0.5 + 0.5 * fk);") != std::string::npos);
    CHECK(sky.find("1.0 - smoothstep(kLitCloudFadeStartDeg, kLitCloudFadeEndDeg, skySunElevation())") !=
          std::string::npos);
    // The shadow edge as SkyShadowCover.
    CHECK(terrain.find("smoothstep(kSkyShadowThreshold - kSkyShadowEdge, kSkyShadowThreshold + kSkyShadowEdge, d)") !=
          std::string::npos);

    // The engine builds the map, its coverage and the turn from SkyDensity.hpp.
    const std::string engine = ReadTextFile(Gl33Dir() / "EngineGL33_SkyClouds.cpp");
    for (const char* use : {"rm::BuildSkyDensityMap(", "rm::BuildSkyShadowCoverageTable(", "rm::SkyDomeTurnU(",
                            "rm::MixSkyTexels(", "rm::SkyDeckWeight(", "s_clouds.anchor.Update("})
    {
        CAPTURE(use);
        CHECK(engine.find(use) != std::string::npos);
    }
}
