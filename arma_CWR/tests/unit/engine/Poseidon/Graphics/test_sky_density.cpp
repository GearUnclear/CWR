#include <catch2/catch_approx.hpp>
#include <catch2/catch_message.hpp>
#include <catch2/catch_test_macros.hpp>

#include <Poseidon/Graphics/Remaster/LightRig.hpp>
#include <Poseidon/Graphics/Remaster/SkyDensity.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

// The sky's own clouds (INTEGRATION-PLAN.md section 5 B10, Graphics/Remaster/
// SkyDensity.hpp): the dome's texture mapping, the cloud-density map on three
// weather skies made like the shipped ones (a clear gradient, white streaks on
// blue, a grey deck), the shadow cover and coverage, the low-sun fade, the
// sky's turn, the shadow anchor, and the light rig keeping the classic
// overcast dimming over the sky's own covered share.

using namespace Poseidon::remaster;

namespace
{
constexpr float kPi = 3.14159265358979323846f;
constexpr int kW = 64;
constexpr int kH = 48;

// Elevation (degrees) of texel row y of a kH-row weather sky.
float RowElevation(int y)
{
    return SkyDomeElevationDeg((static_cast<float>(y) + 0.5f) / static_cast<float>(kH));
}

// Clear blue overhead, paling toward the horizon, the same all round (jasno).
void ClearTexel(int y, float* c)
{
    const float t = std::max(0.0f, std::min(1.0f, RowElevation(y) / 60.0f)); // 0 horizon .. 1 high
    c[0] = 0.80f + (0.35f - 0.80f) * t;
    c[1] = 0.85f + (0.55f - 0.85f) * t;
    c[2] = 0.90f + (0.95f - 0.90f) * t;
}

std::vector<float> ClearSky()
{
    std::vector<float> rgb(static_cast<size_t>(kW) * kH * 3);
    for (int y = 0; y < kH; ++y)
        for (int x = 0; x < kW; ++x)
            ClearTexel(y, &rgb[(static_cast<size_t>(y) * kW + x) * 3]);
    return rgb;
}

// White streaks over the clear sky (oblacno): a streak where the wave is high.
bool Streak(int x, int y)
{
    return std::sin(static_cast<float>(x) * 2.0f * kPi * 3.0f / kW + static_cast<float>(y) * 0.1f) > 0.3f;
}

std::vector<float> BrokenSky()
{
    std::vector<float> rgb = ClearSky();
    for (int y = 0; y < kH; ++y)
        for (int x = 0; x < kW; ++x)
            if (Streak(x, y))
            {
                float* c = &rgb[(static_cast<size_t>(y) * kW + x) * 3];
                c[0] = 0.92f;
                c[1] = 0.92f;
                c[2] = 0.95f;
            }
    return rgb;
}

// A grey deck all round with darker lumps (zatazeno).
std::vector<float> OvercastSky()
{
    std::vector<float> rgb(static_cast<size_t>(kW) * kH * 3);
    for (int y = 0; y < kH; ++y)
        for (int x = 0; x < kW; ++x)
        {
            const bool lump = ((x / 8) + (y / 6)) % 2 == 0;
            float* c = &rgb[(static_cast<size_t>(y) * kW + x) * 3];
            c[0] = lump ? 0.45f : 0.60f;
            c[1] = lump ? 0.45f : 0.60f;
            c[2] = lump ? 0.47f : 0.62f;
        }
    return rgb;
}

float MaxR(const SkyDensityMap& m)
{
    float r = 0.0f;
    for (int y = 0; y < m.height; ++y)
        for (int x = 0; x < m.width; ++x)
            r = std::max(r, m.R(x, y));
    return r;
}

LightRigInputs Sun45(float skyThrough)
{
    LightRigInputs in;
    const float e = 45.0f * kPi / 180.0f;
    in.towardSun[0] = 0.0f;
    in.towardSun[1] = std::sin(e);
    in.towardSun[2] = std::cos(e);
    in.fogColor = Rgb(0.73f, 0.79f, 0.85f);
    in.skyThrough = skyThrough;
    in.cloudShadowsActive = true;
    return in;
}
} // namespace

TEST_CASE("Sky density: the dome's texture mapping is obloha.p3d's", "[remaster][SkyDensity]")
{
    // Each ring's elevation maps to its v, and v inverts back.
    for (int i = 0; i < kSkyDomeRings; ++i)
    {
        CAPTURE(i);
        CHECK(SkyDomeV(kSkyDomeRingElevationDeg[i]) == Catch::Approx(kSkyDomeRingV[i]).margin(1e-6));
    }
    for (float e = -15.0f; e <= 89.0f; e += 0.5f)
    {
        CAPTURE(e);
        CHECK(SkyDomeElevationDeg(SkyDomeV(e)) == Catch::Approx(e).margin(1e-3));
        if (e + 0.25f <= kSkyDomeRingElevationDeg[kSkyDomeRings - 1])
            CHECK(SkyDomeV(e + 0.25f) < SkyDomeV(e)); // v falls toward the zenith
    }
    // u is the azimuth from north toward east over 180 degrees (the texture
    // wraps twice around the horizon); straight up is the top ring.
    CHECK(SkyDomeUv(0.0f, 0.0f, 1.0f).u == Catch::Approx(0.0f).margin(1e-6));
    CHECK(SkyDomeUv(1.0f, 0.0f, 0.0f).u == Catch::Approx(0.5f));
    CHECK(SkyDomeUv(-1.0f, 0.0f, 0.0f).u == Catch::Approx(-0.5f));
    CHECK(std::abs(SkyDomeUv(0.0f, 0.0f, -1.0f).u) == Catch::Approx(1.0f));
    CHECK(SkyDomeUv(0.0f, 1.0f, 0.0f).v == Catch::Approx(kSkyDomeRingV[kSkyDomeRings - 1]));
    CHECK(SkyDomeUv(3.0f, 0.0f, 0.0f).v == Catch::Approx(SkyDomeV(0.0f)));               // any length
    CHECK(SkyDomeUv(0.0f, 1.0f, 1.0f).v == Catch::Approx(SkyDomeV(45.0f)).margin(1e-5)); // 45 deg up
}

TEST_CASE("Sky density: a clear sky has no cloud, and its pale horizon is no deck", "[remaster][SkyDensity]")
{
    const std::vector<float> rgb = ClearSky();
    const SkyDensityMap map = BuildSkyDensityMap(rgb.data(), kW, kH);
    REQUIRE(map.Valid());
    CHECK(map.width == kW);
    CHECK(map.height == kH);
    CHECK(MaxR(map) == 0.0f);
    // The rows near the horizon are grey enough all round to be deck
    // candidates; the weather of a clear sky does not count them.
    CHECK(map.G(0, kH - 2) > 0.0f);
    CHECK(SkyDeckWeight(0.96f) == 0.0f);
    for (float sun : {5.0f, 30.0f, 80.0f})
        CHECK(SkyShadowCoverage(map, SkyDeckWeight(0.96f), sun) == 0.0f);
    for (size_t i = 3; i < map.rgba.size(); i += 4)
        CHECK(map.rgba[i] == 255);
}

TEST_CASE("Sky density: broken sky streaks are cloud, the blue between them is not", "[remaster][SkyDensity]")
{
    const std::vector<float> rgb = BrokenSky();
    const SkyDensityMap map = BuildSkyDensityMap(rgb.data(), kW, kH);
    REQUIRE(map.Valid());
    // In a high row: full cloud at a streak's middle, none halfway between.
    const int y = 6;
    REQUIRE(RowElevation(y) > 45.0f);
    int inside = -1, outside = -1;
    for (int x = 2; x < kW - 2 && (inside < 0 || outside < 0); ++x)
    {
        bool allIn = true, allOut = true;
        for (int k = -2; k <= 2; ++k)
            for (int j = -2; j <= 2; ++j)
            {
                allIn = allIn && Streak(x + k, std::max(0, std::min(kH - 1, y + j)));
                allOut = allOut && !Streak(x + k, std::max(0, std::min(kH - 1, y + j)));
            }
        if (allIn && inside < 0)
            inside = x;
        if (allOut && outside < 0)
            outside = x;
    }
    REQUIRE(inside >= 0);
    REQUIRE(outside >= 0);
    CHECK(map.R(inside, y) > 0.95f);
    CHECK(map.R(outside, y) < 0.05f);
    // The rows are blue on average: no deck.
    CHECK(map.G(inside, y) == 0.0f);
    // A streak casts a full shadow, the blue none: the covered share lies
    // between, near the streaks' share of the sky.
    const float broken = SkyDeckWeight(0.7f);
    CHECK(SkyShadowCover(SkyCloudDensity(map.R(inside, y), map.G(inside, y), map.B(inside, y), broken)) == 1.0f);
    CHECK(SkyShadowCover(SkyCloudDensity(map.R(outside, y), map.G(outside, y), map.B(outside, y), broken)) == 0.0f);
    for (float sun : {30.0f, 60.0f})
    {
        CAPTURE(sun);
        const float coverage = SkyShadowCoverage(map, broken, sun);
        CHECK(coverage > 0.15f);
        CHECK(coverage < 0.45f);
    }
}

TEST_CASE("Sky density: an overcast deck covers the sky once the weather is overcast", "[remaster][SkyDensity]")
{
    const std::vector<float> rgb = OvercastSky();
    const SkyDensityMap map = BuildSkyDensityMap(rgb.data(), kW, kH);
    REQUIRE(map.Valid());
    for (int y = 0; y < kH; ++y)
        CHECK(map.G(0, y) == 1.0f); // grey all round
    CHECK(MaxR(map) < 0.05f);       // no broken-cloud feature in a uniform grey
    // Darker lumps are thicker.
    CHECK(map.B(4, 3) > map.B(12, 3));
    CHECK(SkyDeckWeight(0.1f) == 1.0f);
    CHECK(SkyShadowCoverage(map, SkyDeckWeight(0.1f), 45.0f) > 0.9f);
    // Under a broken sky's weather the same rows are not a deck.
    CHECK(SkyShadowCoverage(map, SkyDeckWeight(0.7f), 45.0f) == 0.0f);
}

TEST_CASE("Sky density: the coverage reads the rows the shadows look up at that sun height", "[remaster][SkyDensity]")
{
    // A map whose broken cloud fills one band of elevations.
    auto band = [](float loDeg, float hiDeg)
    {
        SkyDensityMap m;
        m.width = 32;
        m.height = 256;
        m.rgba.assign(static_cast<size_t>(m.width) * m.height * 4, 0);
        for (int y = 0; y < m.height; ++y)
        {
            const float e = SkyDomeElevationDeg((static_cast<float>(y) + 0.5f) / static_cast<float>(m.height));
            for (int x = 0; x < m.width; ++x)
            {
                std::uint8_t* t = &m.rgba[(static_cast<size_t>(y) * m.width + x) * 4];
                t[0] = (e >= loDeg && e < hiDeg) ? 255 : 0;
                t[3] = 255;
            }
        }
        return m;
    };
    // Ground near the anchor under a high sun looks up overhead, under a low
    // sun toward the horizon (its deck points are kilometres away).
    const SkyDensityMap high = band(45.0f, 90.0f), low = band(4.0f, 14.0f);
    // A sun overhead sees the band from 45 degrees up over the ground within
    // 1.8 km of the anchor: about (1.8 / 3)^2 of the disc.
    CHECK(SkyShadowCoverage(high, 0.0f, 85.0f) == Catch::Approx(0.36f).margin(0.05f));
    CHECK(SkyShadowCoverage(high, 0.0f, 8.0f) == 0.0f);
    CHECK(SkyShadowCoverage(low, 0.0f, 8.0f) > 0.9f);
    CHECK(SkyShadowCoverage(low, 0.0f, 85.0f) == 0.0f);
    // The table holds the same at its grid points and interpolates between.
    const SkyShadowCoverageTable table = BuildSkyShadowCoverageTable(high);
    CHECK(table.At(0.0f, 85.0f) == Catch::Approx(SkyShadowCoverage(high, 0.0f, 85.0f)).margin(1e-5));
    CHECK(table.At(0.5f, 45.0f) == Catch::Approx(SkyShadowCoverage(high, 0.5f, 45.0f)).margin(1e-5));
    const float between = table.At(0.0f, 46.25f);
    const float a = table.At(0.0f, 45.0f), b = table.At(0.0f, 47.5f);
    CHECK(between == Catch::Approx(0.5f * (a + b)).margin(1e-5));
    CHECK(table.At(2.0f, 120.0f) == Catch::Approx(table.At(1.0f, 90.0f))); // clamped
    // No map, no coverage.
    CHECK(SkyShadowCoverage(SkyDensityMap{}, 0.0f, 45.0f) == 0.0f);
}

TEST_CASE("Sky density: the blur spreads across the u seam and stops at the v ends", "[remaster][SkyDensity]")
{
    // One white column at u = 0 in a clear sky.
    std::vector<float> rgb = ClearSky();
    for (int y = 0; y < kH; ++y)
    {
        float* c = &rgb[static_cast<size_t>(y) * kW * 3];
        c[0] = c[1] = c[2] = 0.95f;
    }
    const SkyDensityMap map = BuildSkyDensityMap(rgb.data(), kW, kH);
    const int y = kH / 3;
    const float peak = map.R(0, y);
    CHECK(peak > 0.3f);
    CHECK(map.R(kW - 1, y) == Catch::Approx(map.R(1, y)).margin(1.0f / 255.0f));
    CHECK(map.R(kW - 2, y) == Catch::Approx(map.R(2, y)).margin(1.0f / 255.0f));
    CHECK(map.R(3, y) == 0.0f);
    CHECK(map.R(kW - 3, y) == 0.0f);
    // The 5-tap binomial: 6, 4 and 1 sixteenths of the column.
    CHECK(map.R(1, y) == Catch::Approx(peak * 4.0f / 6.0f).margin(2.0f / 255.0f));
    CHECK(map.R(2, y) == Catch::Approx(peak * 1.0f / 6.0f).margin(2.0f / 255.0f));
    // The top and bottom rows clamp (their column keeps its value).
    CHECK(map.R(0, 0) == Catch::Approx(map.R(0, 1)).margin(2.0f / 255.0f));
}

TEST_CASE("Sky density: shadow cover, density and the weather mix", "[remaster][SkyDensity]")
{
    CHECK(SkyShadowCover(kSkyShadowThreshold) == Catch::Approx(0.5f));
    CHECK(SkyShadowCover(kSkyShadowThreshold - kSkyShadowEdge) == 0.0f);
    CHECK(SkyShadowCover(kSkyShadowThreshold + kSkyShadowEdge) == 1.0f);
    // Broken cloud weighs 0.75 to 1 with its darkness, the deck 0.5 to 1.
    CHECK(SkyCloudDensity(1.0f, 0.0f, 0.0f, 0.0f) == Catch::Approx(0.75f));
    CHECK(SkyCloudDensity(1.0f, 0.0f, 1.0f, 0.0f) == Catch::Approx(1.0f));
    CHECK(SkyCloudDensity(0.0f, 1.0f, 0.0f, 1.0f) == Catch::Approx(0.5f));
    CHECK(SkyCloudDensity(0.0f, 1.0f, 1.0f, 1.0f) == Catch::Approx(1.0f));
    CHECK(SkyCloudDensity(0.0f, 1.0f, 1.0f, 0.0f) == 0.0f);
    CHECK(SkyDeckWeight(0.35f) == 1.0f);
    CHECK(SkyDeckWeight(0.75f) == 0.0f);
    CHECK(SkyDeckWeight(0.55f) == Catch::Approx(0.5f));
    // Two weather skies mix as the engine mixes the dome's texture.
    const std::vector<float> a = {0.0f, 0.5f, 1.0f}, b = {1.0f, 0.5f, 0.0f};
    const std::vector<float> m = MixSkyTexels(a, b, 0.25f);
    CHECK(m[0] == Catch::Approx(0.25f));
    CHECK(m[1] == Catch::Approx(0.5f));
    CHECK(m[2] == Catch::Approx(0.75f));
    CHECK(MixSkyTexels(a, {0.0f}, 0.5f) == a);
}

TEST_CASE("Sky density: relief and silver lining only at a low sun", "[remaster][SkyDensity]")
{
    // The critic's midday emboss: gone above 45 degrees, full below 25.
    CHECK(LitCloudLowSun(5.0f) == 1.0f);
    CHECK(LitCloudLowSun(kLitCloudFadeStartDeg) == 1.0f);
    CHECK(LitCloudLowSun(35.0f) == Catch::Approx(0.5f));
    CHECK(LitCloudLowSun(kLitCloudFadeEndDeg) == 0.0f);
    CHECK(LitCloudLowSun(70.0f) == 0.0f);
    // The march: kLitCloudStepDeg times 1, 3, 6, .. 21 (25 degrees at the last).
    CHECK(LitCloudStepAngleDeg(1) == Catch::Approx(kLitCloudStepDeg));
    CHECK(LitCloudStepAngleDeg(2) == Catch::Approx(3.0f * kLitCloudStepDeg));
    CHECK(LitCloudStepAngleDeg(kLitCloudSteps) == Catch::Approx(21.0f * kLitCloudStepDeg));
}

TEST_CASE("Sky density: the lit-cloud terms are identity at a high sun and light as well as shade at a low one",
          "[remaster][SkyDensity]")
{
    // The owner's never-ship list has "anything only darker". A first build
    // faded the relief and the lining but not the darkening of thick cloud
    // against the sun, which then was the only term left at midday and greyed
    // the white cumulus near a high sun. Every term fades now: at 45 degrees and
    // up nothing changes, whatever the cloud, the view or the weather.
    int changed = 0;
    for (float sun : {kLitCloudFadeEndDeg, 60.0f, 90.0f})
        for (float cosT : {-1.0f, 0.0f, 0.7f, 0.95f, 1.0f})
            for (float d : {0.05f, 0.3f, 0.6f, 1.0f})
                for (float tau : {0.0f, 0.3f, 0.6f, 1.0f})
                    for (float feature : {0.0f, 0.6f, 1.0f})
                        for (float dark : {0.0f, 0.5f, 1.0f})
                            for (float deck : {0.0f, 1.0f})
                                for (float through : {0.2f, 0.7f})
                                {
                                    const LitCloudShade s =
                                        LitCloudTerms(sun, cosT, d, tau, feature, dark, deck, through);
                                    if (s.gain != 1.0f || s.warm != 0.0f || s.silver != 0.0f)
                                    {
                                        if (changed == 0)
                                        {
                                            CAPTURE(sun, cosT, d, tau, feature, dark, deck, through, s.gain, s.warm,
                                                    s.silver);
                                            CHECK(s.gain == 1.0f);
                                        }
                                        ++changed;
                                    }
                                }
    CHECK(changed == 0);
    // The reviewer's case: thick white cloud right against a 60 degree sun.
    const LitCloudShade noon = LitCloudTerms(60.0f, 1.0f, 0.8f, 0.8f, 1.0f, 0.2f, 0.0f, 0.7f);
    CHECK(noon.gain == 1.0f);

    // At a low sun (8 degrees, broken sky) the light goes both ways: the edge
    // facing the sun (denser here than toward the sun) lighter and warm, the far
    // side (thinner here) darker, thick cloud against the sun darker, a thin rim
    // near the sun lined with silver.
    const LitCloudShade facing = LitCloudTerms(8.0f, 0.0f, 0.6f, 0.3f, 0.6f, 0.3f, 0.0f, 0.7f);
    CHECK(facing.gain > 1.1f);
    CHECK(facing.warm > 0.0f);
    const LitCloudShade away = LitCloudTerms(8.0f, 0.0f, 0.3f, 0.6f, 0.6f, 0.3f, 0.0f, 0.7f);
    CHECK(away.gain < 0.9f);
    CHECK(away.warm == 0.0f);
    const LitCloudShade against = LitCloudTerms(8.0f, 1.0f, 0.8f, 0.8f, 1.0f, 0.2f, 0.0f, 0.7f);
    CHECK(against.gain < 0.8f);
    const LitCloudShade rim = LitCloudTerms(8.0f, 0.98f, 0.4f, 0.1f, 0.4f, 0.2f, 0.0f, 0.7f);
    CHECK(rim.silver > 0.1f);
    // Under a deck the thin parts lift and the thick bellies deepen.
    CHECK(LitCloudTerms(8.0f, 0.0f, 0.6f, 0.6f, 0.0f, 0.0f, 1.0f, 0.2f).gain > 1.0f);
    CHECK(LitCloudTerms(8.0f, 0.0f, 0.6f, 0.6f, 0.0f, 1.0f, 1.0f, 0.2f).gain < 1.0f);

    // In the fade every term goes the same share of the way: at 35 degrees the
    // darkening against the sun is half of the low sun's.
    const LitCloudShade half = LitCloudTerms(35.0f, 1.0f, 0.8f, 0.8f, 1.0f, 0.2f, 0.0f, 0.7f);
    CHECK(half.gain - 1.0f == Catch::Approx(0.5f * (against.gain - 1.0f)));
    const LitCloudShade halfFacing = LitCloudTerms(35.0f, 0.0f, 0.6f, 0.3f, 0.6f, 0.3f, 0.0f, 0.7f);
    CHECK(halfFacing.gain - 1.0f == Catch::Approx(0.5f * (facing.gain - 1.0f)));
    CHECK(halfFacing.warm == Catch::Approx(0.5f * facing.warm));
}

TEST_CASE("Sky density: the dome turns with the sky clouds' position, wrapping cleanly", "[remaster][SkyDensity]")
{
    CHECK(SkyDomeTurnU(0.0f) == 0.0f);
    // 180 degrees of azimuth is one u.
    const float half = 90.0f / kSkyTurnDegPerCloudsPos;
    CHECK(SkyDomeTurnU(half) == Catch::Approx(0.5f).margin(1e-5));
    CHECK(SkyDomeTurnU(2.0f * half) == Catch::Approx(0.0f).margin(1e-5));
    CHECK(SkyDomeTurnU(1.0f) == Catch::Approx(kSkyTurnDegPerCloudsPos / 180.0f));
    // About 6 degrees a minute at overcast 0.5 (cloud speed 0.35 per second).
    CHECK(0.35f * 60.0f * kSkyTurnDegPerCloudsPos == Catch::Approx(6.3f));
    // Hours into a session the turn is still in [0, 1) and moves smoothly.
    const float late = 36000.0f;
    const float u0 = SkyDomeTurnU(late), u1 = SkyDomeTurnU(late + 0.5f);
    CHECK(u0 >= 0.0f);
    CHECK(u0 < 1.0f);
    CHECK(u1 - u0 == Catch::Approx(0.5f * kSkyTurnDegPerCloudsPos / 180.0f).margin(1e-4));
}

TEST_CASE("Sky density: the shadow anchor stays put, steps after the camera and cross-fades", "[remaster][SkyDensity]")
{
    SkyShadowAnchor a;
    a.Update(1000.0f, 2000.0f, 0.016f, false); // first step: at the camera
    CHECK(a.valid);
    CHECK(a.x == 1000.0f);
    CHECK(a.z == 2000.0f);
    CHECK(a.prevWeight == 0.0f);
    // Walking away: the shadows stay where they are on the ground.
    float x = 1000.0f;
    for (int i = 0; i < 1400; ++i)
    {
        x += 1.0f;
        a.Update(x, 2000.0f, 0.5f, false);
    }
    CHECK(a.x == 1000.0f);
    CHECK(a.prevWeight == 0.0f);
    // Past the radius it steps the radius toward the camera; the old anchor is
    // the radius behind and fades out.
    while (x - 1000.0f <= kSkyShadowAnchorRadius)
    {
        x += 1.0f;
        a.Update(x, 2000.0f, 0.016f, false);
    }
    CHECK(a.x == Catch::Approx(1000.0f + kSkyShadowAnchorRadius));
    CHECK(a.z == Catch::Approx(2000.0f));
    CHECK(a.prevWeight == 1.0f);
    CHECK(a.PrevX() == Catch::Approx(1000.0f).margin(1e-2));
    CHECK(a.PrevZ() == Catch::Approx(2000.0f).margin(1e-2));
    a.Update(x, 2000.0f, kSkyShadowFadeSeconds * 0.5f, false);
    CHECK(a.prevWeight == Catch::Approx(0.5f));
    a.Update(x, 2000.0f, kSkyShadowFadeSeconds, false);
    CHECK(a.prevWeight == 0.0f);
    // A cut (a new view far away) snaps without a fade.
    a.Update(x + kSkyShadowAnchorJump + 10.0f, 9000.0f, 0.016f, false);
    CHECK(a.x == x + kSkyShadowAnchorJump + 10.0f);
    CHECK(a.z == 9000.0f);
    CHECK(a.prevWeight == 0.0f);
    // Held (captures): it never steps, but a cut still snaps.
    SkyShadowAnchor h;
    h.Snap(0.0f, 0.0f);
    for (int i = 1; i <= 400; ++i)
        h.Update(static_cast<float>(i) * 4.0f, 0.0f, 0.016f, true);
    CHECK(h.x == 0.0f);
    CHECK(h.prevWeight == 0.0f);
}

TEST_CASE("Sky density: the rig keeps the classic overcast dimming over the sky's covered share",
          "[remaster][SkyDensity]")
{
    const LightingTuning tuning;
    for (float through : {0.7f, 0.5f, 0.2f})
    {
        LightRigInputs in = Sun45(through);
        in.cloudShadowsActive = false;
        const LightRig classicDim = ComputeLightRig(in, tuning);
        in.cloudShadowsActive = true;
        const LightRig noise = ComputeLightRig(in, tuning);
        for (float coverage : {0.0f, 0.05f, 0.3f, 0.6f, 0.95f})
        {
            CAPTURE(through, coverage);
            in.skyShadowCoverage = coverage;
            const LightRig sky = ComputeLightRig(in, tuning);
            // Same cover (the gates and the noise field read it).
            CHECK(sky.cloudCover == noise.cloudCover);
            const float meanSun = Luminance(sky.sun) * (1.0f - coverage * (1.0f - sky.cloudTransmit));
            CHECK(meanSun == Catch::Approx(Luminance(classicDim.sun)).epsilon(1e-4));
            // Never darker under a cloud than the noise field's cloud.
            CHECK(sky.cloudTransmit >= noise.cloudTransmit - 1e-6f);
            // Over a share at most the cover, the cloud keeps its 0.2 and the
            // rest of the dimming stays uniform; over a larger share the
            // cloud lets more through.
            if (coverage <= noise.cloudCover)
                CHECK(sky.cloudTransmit == Catch::Approx(noise.cloudTransmit));
            else
                CHECK(sky.cloudTransmit > noise.cloudTransmit);
        }
        // Negative: the noise field, unchanged.
        in.skyShadowCoverage = -1.0f;
        const LightRig back = ComputeLightRig(in, tuning);
        CHECK(back.cloudTransmit == noise.cloudTransmit);
        CHECK(Luminance(back.sun) == Luminance(noise.sun));
    }
    // A clear sky casts no shadow whatever the map says (F4).
    LightRigInputs clear = Sun45(0.959f);
    clear.skyShadowCoverage = 0.4f;
    const LightRig rig = ComputeLightRig(clear, tuning);
    CHECK(rig.cloudCover == 0.0f);
    CHECK(rig.cloudTransmit == 1.0f);
}
