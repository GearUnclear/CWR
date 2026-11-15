#include <catch2/catch_approx.hpp>
#include <catch2/catch_message.hpp>
#include <catch2/catch_test_macros.hpp>
#include <Poseidon/Graphics/Core/GpuTimings.hpp>
#include <Poseidon/Graphics/Remaster/Atmosphere.hpp>
#include <Poseidon/Graphics/Remaster/FeatureGates.hpp>
#include <Poseidon/Graphics/Remaster/FeatureSwitches.hpp>

#include <cmath>
#include <cstring>
#include <string_view>

// The sky table (Graphics/Remaster/Atmosphere.hpp): the atmosphere model
// against an independent brute-force integration, the sky-view table's lookups,
// and the frame rules (rebuild, night fade, overcast floor, gate, switch and
// sub-knobs).

using namespace Poseidon::remaster;

namespace
{
constexpr double kPi = 3.14159265358979323846;
constexpr double kRg = atmo::kGroundRadius;
constexpr double kRt = atmo::kTopRadius;
constexpr double kEyeRadius = atmo::kGroundRadius + kSkyViewerHeight;

// The atmosphere tables are 100 ms to build: once for every case.
const AtmosphereTables& Tables()
{
    static const AtmosphereTables t = BuildAtmosphereTables(1.0f);
    return t;
}

const SkyViewTable& TableAt(float sunElevationDeg)
{
    static SkyViewTable noon, sunset, dusk;
    SkyViewTable& t = sunElevationDeg > 30.0f ? noon : (sunElevationDeg > 0.0f ? sunset : dusk);
    if (!t.Valid())
        BuildSkyViewTable(Tables(), static_cast<float>(sunElevationDeg * kPi / 180.0), t);
    return t;
}

// ---- an independent reference: double precision, fine uniform steps, no tables

struct Rgbd
{
    double c[3] = {0, 0, 0};
};

void Extinction(double h, double out[3], double& scatM, double scatR[3])
{
    const double hh = std::max(h, 0.0);
    const double rr = std::exp(-hh / atmo::kRayleighScaleHeight);
    const double rm = std::exp(-hh / atmo::kMieScaleHeight);
    const double ro = std::max(0.0, 1.0 - std::fabs(h - atmo::kOzoneCentre) / atmo::kOzoneHalfWidth);
    scatM = atmo::kMieScattering * rm;
    for (int c = 0; c < 3; ++c)
    {
        scatR[c] = atmo::kRayleighScattering[c] * rr;
        out[c] = scatR[c] + atmo::kMieExtinction * rm + atmo::kOzoneAbsorption[c] * ro;
    }
}

bool RefHitsGround(double r, double mu)
{
    return mu < 0.0 && r * r * (mu * mu - 1.0) + kRg * kRg >= 0.0;
}

double RefDistanceToTop(double r, double mu)
{
    return -r * mu + std::sqrt(std::max(r * r * (mu * mu - 1.0) + kRt * kRt, 0.0));
}

double RefDistanceToBottom(double r, double mu)
{
    return -r * mu - std::sqrt(std::max(r * r * (mu * mu - 1.0) + kRg * kRg, 0.0));
}

// Transmittance to the top of the atmosphere by direct integration.
Rgbd RefTransmittance(double r, double mu, int steps = 400)
{
    Rgbd out;
    if (RefHitsGround(r, mu))
        return out;
    const double len = RefDistanceToTop(r, mu);
    const double dt = len / steps;
    double od[3] = {0, 0, 0};
    for (int s = 0; s < steps; ++s)
    {
        const double t = (s + 0.5) * dt;
        const double rs = std::sqrt(r * r + t * t + 2.0 * r * mu * t);
        double ext[3], scatR[3], scatM;
        Extinction(rs - kRg, ext, scatM, scatR);
        for (int c = 0; c < 3; ++c)
            od[c] += ext[c] * dt;
    }
    for (int c = 0; c < 3; ++c)
        out.c[c] = std::exp(-od[c]);
    return out;
}

double RefRayleighPhase(double c)
{
    return 3.0 / (16.0 * kPi) * (1.0 + c * c);
}

// One sky-view texel (region 0: Rayleigh with its phase plus the multiple
// scattering term, read from the model's table; region 1: Mie without its
// phase) by a fine uniform march from the eye.
Rgbd RefSkyView(int region, double elevation, double azimuth, double sunElevation, int steps = 1500)
{
    const double dir[3] = {std::cos(elevation) * std::cos(azimuth), std::sin(elevation),
                           std::cos(elevation) * std::sin(azimuth)};
    const double sun[3] = {std::cos(sunElevation), std::sin(sunElevation), 0.0};
    const double mu = dir[1];
    const double tMax =
        RefHitsGround(kEyeRadius, mu) ? RefDistanceToBottom(kEyeRadius, mu) : RefDistanceToTop(kEyeRadius, mu);
    const double dt = tMax / steps;
    const double phase = RefRayleighPhase(dir[0] * sun[0] + dir[1] * sun[1] + dir[2] * sun[2]);
    double tr[3] = {1, 1, 1};
    Rgbd out;
    for (int s = 0; s < steps; ++s)
    {
        const double t = (s + 0.5) * dt;
        const double p[3] = {dir[0] * t, kEyeRadius + dir[1] * t, dir[2] * t};
        const double rp = std::sqrt(p[0] * p[0] + p[1] * p[1] + p[2] * p[2]);
        double ext[3], scatR[3], scatM;
        Extinction(rp - kRg, ext, scatM, scatR);
        const double muS = (p[0] * sun[0] + p[1] * sun[1] + p[2] * sun[2]) / rp;
        const Rgbd ts = RefTransmittance(rp, muS, 120);
        const Rgb ms = AtmosphereMultiScatter(Tables(), static_cast<float>(rp - kRg), static_cast<float>(muS));
        const double msc[3] = {ms.r, ms.g, ms.b};
        for (int c = 0; c < 3; ++c)
        {
            const double half = std::exp(-ext[c] * dt * 0.5);
            const double src = region == 0 ? scatR[c] * phase * ts.c[c] + (scatR[c] + scatM) * msc[c] : scatM * ts.c[c];
            out.c[c] += tr[c] * half * src * dt;
            tr[c] *= half * half;
        }
    }
    return out;
}

double RelErr(double got, double want)
{
    return std::fabs(got - want) / std::max(std::fabs(want), 1e-12);
}

} // namespace

TEST_CASE("Sky table: transmittance matches the analytic vertical optical depth", "[remaster][atmosphere]")
{
    // Straight up from the table's eye height: exponential Rayleigh and Mie
    // profiles plus the ozone tent integrate in closed form.
    const double h0 = kSkyViewerHeight;
    const double top = kRt - kRg;
    auto expLayer = [&](double scale) { return scale * (std::exp(-h0 / scale) - std::exp(-top / scale)); };
    // The ozone tent between 10 and 40 km lies wholly above the eye: its area.
    const double ozoneArea = atmo::kOzoneHalfWidth;
    const Rgb t = AtmosphereTransmittance(Tables(), static_cast<float>(kEyeRadius), 1.0f);
    const float got[3] = {t.r, t.g, t.b};
    for (int c = 0; c < 3; ++c)
    {
        const double od = atmo::kRayleighScattering[c] * expLayer(atmo::kRayleighScaleHeight) +
                          atmo::kMieExtinction * expLayer(atmo::kMieScaleHeight) +
                          atmo::kOzoneAbsorption[c] * ozoneArea;
        CAPTURE(c, got[c], std::exp(-od));
        CHECK(RelErr(got[c], std::exp(-od)) < 0.005);
    }
    // Blue light scatters most: the sun at the zenith is a little yellow.
    CHECK(t.b < t.g);
    CHECK(t.g < t.r);
}

TEST_CASE("Sky table: transmittance falls and reddens with the air mass", "[remaster][atmosphere]")
{
    for (float height : {static_cast<float>(kSkyViewerHeight), 2500.0f, 12000.0f})
    {
        CAPTURE(height);
        const float r = atmo::kGroundRadius + height;
        Rgb prev = AtmosphereTransmittance(Tables(), r, 1.0f);
        float prevRatio = prev.r / prev.b;
        // From the zenith down to the horizon: more air, less light, redder.
        for (float mu = 0.95f; mu > 0.0f; mu -= 0.05f)
        {
            CAPTURE(mu);
            const Rgb t = AtmosphereTransmittance(Tables(), r, mu);
            CHECK(t.r <= prev.r);
            CHECK(t.g <= prev.g);
            CHECK(t.b <= prev.b);
            CHECK(t.r / t.b >= prevRatio);
            prev = t;
            prevRatio = t.r / t.b;
        }
        // Against a direct integration, away from the grazing directions.
        for (float mu : {1.0f, 0.6f, 0.2f, 0.05f})
        {
            const Rgb t = AtmosphereTransmittance(Tables(), r, mu);
            const Rgbd ref = RefTransmittance(r, mu, 2000);
            CAPTURE(mu, t.b, ref.c[2]);
            CHECK(RelErr(t.r, ref.c[0]) < 0.01);
            CHECK(RelErr(t.b, ref.c[2]) < 0.01);
        }
    }
    // A ray into the ground carries no sunlight.
    CHECK(AtmosphereTransmittance(Tables(), static_cast<float>(kEyeRadius), -0.5f).g == 0.0f);
}

TEST_CASE("Sky table: sky-view texels match a brute-force march at noon, sunset and 4 degrees under",
          "[remaster][atmosphere]")
{
    // Region 0 (Rayleigh plus multiple scattering) and region 1 (Mie) of the
    // 40-step quadratic march against a 1500-step double-precision march with
    // the transmittance integrated directly (no table). The multiple
    // scattering term reads the model's table on both sides. The
    // original march (kept: the shipped look was tuned on it) samples each
    // step's source 0.3 into the step; with steps longer than the 1.2 km Mie
    // scale height that overstates the haze by up to 8 % (measured: 7.5 % on
    // the noon sky, 4 to 6.5 % at sunset), while Rayleigh, on an 8 km scale,
    // stays within 4 %. The bounds pin those, with a floor of 2 % of the
    // texel's brightest channel for channels near zero (deep twilight).
    struct Probe
    {
        float sunDeg;
        int col, row;
    };
    const Probe probes[] = {
        {60.0f, 10, 60}, {60.0f, 40, 45}, {60.0f, 5, 33}, // noon: high sky, mid sky, near the horizon
        {0.5f, 2, 34},   {0.5f, 50, 40},  {0.5f, 20, 50}, // sunset: toward the sun low, away, high
        {-4.0f, 32, 62}, {-4.0f, 8, 36},                  // 4 degrees under: zenith, toward the glow
    };
    for (const Probe& p : probes)
    {
        const double sun = p.sunDeg * kPi / 180.0;
        for (int region = 0; region < 2; ++region)
        {
            const Rgb got = SkyViewTexel(Tables(), region, p.col, p.row, static_cast<float>(sun));
            const Rgbd ref = RefSkyView(region, SkyViewRowElevation(p.row), SkyViewColumnAzimuth(p.col), sun);
            const float g[3] = {got.r, got.g, got.b};
            const double brightest = std::max(ref.c[0], std::max(ref.c[1], ref.c[2]));
            for (int c = 0; c < 3; ++c)
            {
                CAPTURE(p.sunDeg, p.col, p.row, region, c, g[c], ref.c[c]);
                CHECK(std::isfinite(g[c]));
                CHECK(g[c] >= 0.0f);
                const double err = std::fabs(g[c] - ref.c[c]) / std::max(std::fabs(ref.c[c]), 0.02 * brightest);
                CHECK(err < (region == 0 ? 0.045 : 0.085));
            }
        }
    }
}

TEST_CASE("Sky table: the sky has the colours of a clear day, a sunset and a twilight", "[remaster][atmosphere]")
{
    auto radiance = [](float sunDeg, float elevDeg, float azimuthDeg)
    {
        const float l = elevDeg * static_cast<float>(kPi) / 180.0f;
        const float phi = azimuthDeg * static_cast<float>(kPi) / 180.0f;
        const float s = sunDeg * static_cast<float>(kPi) / 180.0f;
        const float cosToSun = std::cos(l) * std::cos(phi) * std::cos(s) + std::sin(l) * std::sin(s);
        return SampleSkyView(TableAt(sunDeg), l, phi, cosToSun);
    };
    // Noon: a blue zenith, a paler horizon, brighter toward the sun.
    const Rgb zenith = radiance(60.0f, 89.0f, 90.0f);
    CHECK(zenith.b > zenith.g);
    CHECK(zenith.g > zenith.r);
    const Rgb horizon = radiance(60.0f, 2.0f, 120.0f);
    CHECK(horizon.r / horizon.b > zenith.r / zenith.b);
    CHECK(Luminance(radiance(60.0f, 30.0f, 0.0f)) > Luminance(radiance(60.0f, 30.0f, 180.0f)));
    // Sunset: the low sky toward the sun is orange and outshines the sky
    // opposite it many times over (the aureole).
    const Rgb glow = radiance(0.5f, 2.0f, 2.0f);
    CHECK(glow.r > glow.g);
    CHECK(glow.g > glow.b);
    CHECK(Luminance(glow) > 4.0f * Luminance(radiance(0.5f, 2.0f, 178.0f)));
    // Four degrees under: far dimmer than the sunset, the zenith still blue.
    const Rgb dusk = radiance(-4.0f, 89.0f, 90.0f);
    CHECK(Luminance(dusk) < 0.2f * Luminance(radiance(0.5f, 89.0f, 90.0f)));
    CHECK(dusk.b > dusk.r);
    CHECK(Luminance(dusk) > 0.0f);
}

TEST_CASE("Sky table: lookups return the texels and the row mean averages its row", "[remaster][atmosphere]")
{
    const SkyViewTable& t = TableAt(0.5f);
    REQUIRE(t.Valid());
    const float sun = t.sunElevationRad;
    for (int row : {3, 31, 32, 47, 63})
        for (int col : {0, 17, 63})
        {
            CAPTURE(row, col);
            const float l = SkyViewRowElevation(row);
            const float phi = SkyViewColumnAzimuth(col);
            const float cosToSun = std::cos(l) * std::cos(phi) * std::cos(sun) + std::sin(l) * std::sin(sun);
            const Rgb want = t.At(col, row) + t.At(kSkyViewSize + col, row) * AtmosphereMiePhase(cosToSun);
            const Rgb got = SampleSkyView(t, l, phi, cosToSun);
            CHECK(got.r == Catch::Approx(want.r).epsilon(1e-4));
            CHECK(got.b == Catch::Approx(want.b).epsilon(1e-4));
        }
    // Region 2 holds each row's mean, the same in every column, and a dense
    // average of the lookups agrees with it.
    for (int row : {5, 33, 40, 60})
    {
        CAPTURE(row);
        const Rgb mean = SkyViewRowMean(t, row);
        CHECK(t.At(2 * kSkyViewSize, row).g == mean.g);
        CHECK(t.At(3 * kSkyViewSize - 1, row).g == mean.g);
        const float l = SkyViewRowElevation(row);
        Rgb dense;
        constexpr int kN = 2000;
        for (int a = 0; a < kN; ++a)
        {
            const float phi = (a + 0.5f) / kN * static_cast<float>(kPi);
            const float cosToSun = std::cos(l) * std::cos(phi) * std::cos(sun) + std::sin(l) * std::sin(sun);
            dense = dense + SampleSkyView(t, l, phi, cosToSun) * (1.0f / kN);
        }
        CHECK(RelErr(mean.g, dense.g) < 0.02);
        // Its alpha is the classic sun glow's weight over the row.
        CHECK(t.AlphaAt(2 * kSkyViewSize, row) == SkyViewRowGlow(sun, row));
    }
    // The glow weight: max(cos, 0)^6 averaged over the half circle of
    // azimuths. With the sun on the horizon the horizon row averages
    // cos(l)^6 * (1/pi) * integral of cos^6 over a quarter turn = 5/32 cos(l)^6;
    // straight down it is 0, straight up with a high sun it is sin(sun)^6.
    const int horizonRow = kSkyViewSize / 2;
    const float l = SkyViewRowElevation(horizonRow);
    CHECK(SkyViewRowGlow(0.0f, horizonRow) == Catch::Approx(5.0 / 32.0 * std::pow(std::cos(l), 6.0)).epsilon(0.01));
    CHECK(SkyViewRowGlow(0.5f, 0) < 1e-6f);
    const float high = static_cast<float>(60.0 * kPi / 180.0);
    CHECK(SkyViewRowGlow(high, kSkyViewSize - 1) == Catch::Approx(std::pow(std::sin(high), 6.0)).epsilon(0.02));
}

TEST_CASE("Sky table: the haze's phase has the sun's aureole and conserves the light", "[remaster][atmosphere]")
{
    // A broad lobe plus the aureole's narrow peak.
    auto at = [](double deg)
    { return static_cast<double>(AtmosphereMiePhase(static_cast<float>(std::cos(deg * kPi / 180.0)))); };
    // Over the sphere it integrates to 1 (the scattered light is the same).
    double sum = 0.0;
    constexpr int kSteps = 200000;
    for (int i = 0; i < kSteps; ++i)
    {
        const double th = (i + 0.5) * kPi / kSteps;
        sum += AtmosphereMiePhase(static_cast<float>(std::cos(th))) * 2.0 * kPi * std::sin(th) * (kPi / kSteps);
    }
    CHECK(sum == Catch::Approx(1.0).epsilon(0.01));
    // The glow gathers round the disc: 1 degree from the sun is many times 10
    // (the single broad lobe of 0.8 gave twice), and falls on toward 30.
    CHECK(at(1.0) > 10.0 * at(10.0));
    CHECK(at(3.0) > 3.0 * at(10.0));
    CHECK(at(10.0) > 3.0 * at(30.0));
    // The broad glow and the sky away from the sun keep most of their light.
    auto cs08 = [](double deg)
    {
        const double g = 0.8, c = std::cos(deg * kPi / 180.0);
        return 3.0 / (8.0 * kPi) * (1.0 - g * g) / (2.0 + g * g) * (1.0 + c * c) /
               std::pow(1.0 + g * g - 2.0 * g * c, 1.5);
    };
    for (double deg : {10.0, 20.0, 30.0, 60.0, 90.0, 150.0})
    {
        CAPTURE(deg);
        CHECK(at(deg) > 0.65 * cs08(deg));
        CHECK(at(deg) < 1.25 * cs08(deg));
    }
}

TEST_CASE("Sky table: rebuilt when the sun moves 0.02 degrees or the haze changes", "[remaster][atmosphere]")
{
    const float deg = static_cast<float>(kPi / 180.0);
    CHECK(SkyTableNeedsRebuild(false, 0.1f, 1.0f, 0.1f, 1.0f));
    CHECK_FALSE(SkyTableNeedsRebuild(true, 0.1f, 1.0f, 0.1f, 1.0f));
    CHECK_FALSE(SkyTableNeedsRebuild(true, 0.1f, 1.0f, 0.1f + 0.019f * deg, 1.0f));
    CHECK(SkyTableNeedsRebuild(true, 0.1f, 1.0f, 0.1f + 0.021f * deg, 1.0f));
    CHECK(SkyTableNeedsRebuild(true, 0.1f, 1.0f, 0.1f - 0.021f * deg, 1.0f));
    CHECK(SkyTableNeedsRebuild(true, 0.1f, 1.0f, 0.1f, 1.6f));
    CHECK(kSkyTableRebuildDeg == 0.02f);
}

TEST_CASE("Sky table: night keeps the classic sky, the overcast deck keeps the classic fog", "[remaster][atmosphere]")
{
    CHECK(SkyDayFade(30.0f) == 1.0f);
    CHECK(SkyDayFade(kSkyNightFadeTopDeg) == 1.0f);
    CHECK(SkyDayFade(kSkyNightFadeEndDeg) == 0.0f);
    CHECK(SkyDayFade(-20.0f) == 0.0f);
    float prev = 0.0f;
    for (float e = -10.0f; e <= 0.0f; e += 0.25f)
    {
        CHECK(SkyDayFade(e) >= prev);
        prev = SkyDayFade(e);
    }
    // The fade ends where the gate turns the feature off.
    CHECK(kSkyNightFadeEndDeg == kGateSkyTableMinSunDeg);

    CHECK(SkyOvercastCloudFloor(1.0f) == 0.0f);
    CHECK(SkyOvercastCloudFloor(0.7f) == 0.0f);
    CHECK(SkyOvercastCloudFloor(0.4f) == Catch::Approx(0.5f));
    CHECK(SkyOvercastCloudFloor(0.1f) == Catch::Approx(1.0f));
    CHECK(SkyOvercastCloudFloor(0.0f) == 1.0f);
    CHECK(SkyCloudSunShare(30.0f) == Catch::Approx(0.7f));
    CHECK(SkyCloudSunShare(-5.0f) == 0.0f);
}

TEST_CASE("Sky table: the switch has landed with its gate, GPU section and sub-knobs", "[remaster][atmosphere]")
{
    const FeatureInfo& info = GetFeatureInfo(Feature::Atmosphere);
    CHECK(info.landed);
    // Its GPU section is one GpuTimings measures.
    bool found = false;
    for (int i = 0; i < Poseidon::gpu::kSectionCount; ++i)
        found |= std::strcmp(Poseidon::gpu::SectionName(static_cast<Poseidon::gpu::Section>(i)), info.gpuSection) == 0;
    CHECK(found);
    // The shipped hue and saturation.
    const LightingTuning t{};
    CHECK(t.atmoHue == kSkyHueDefault);
    CHECK(t.atmoSat == kSkySaturationDefault);
    CHECK(kSkyHueDefault == 0.5f);
    CHECK(kSkySaturationDefault == 1.15f);
    CHECK(t.atmoHaze == 1.0f);
    for (const char* key : {"atmoHue", "atmoSat", "atmoHaze"})
    {
        INFO(key);
        const LightingKey* k = FindLightingKey(key);
        REQUIRE(k);
        CHECK(k->kind == KeyKind::Knob);
        REQUIRE(k->parent);
        CHECK(std::string_view(k->parent) == "atmosphere");
    }
    // The noon cirrus survives: the cloud mask's dead zone is well under an
    // earlier 0.015.
    CHECK(kSkyCloudMaskDeadZone <= 0.005f);

    // The gate: above -9 degrees, once the atmosphere tables exist.
    GateInputs in;
    in.sunElevationDeg = 10.0f;
    CHECK(ComputeFeatureGates(in, LightingTuning{})[Feature::Atmosphere].on);
    in.skyTablesReady = false;
    const Gate building = ComputeFeatureGates(in, LightingTuning{})[Feature::Atmosphere];
    CHECK_FALSE(building.on);
    CHECK(std::string_view(building.reason) == "atmosphere tables building");
}
