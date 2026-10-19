#include <Poseidon/Graphics/Remaster/SurfaceLight.hpp>

#include <Poseidon/Graphics/Remaster/Atmosphere.hpp>

#include <algorithm>
#include <cmath>

namespace Poseidon::remaster
{

namespace
{
constexpr float kPi = 3.14159265358979323846f;

float Clamp01(float v)
{
    return std::min(1.0f, std::max(0.0f, v));
}

float Smoothstep(float e0, float e1, float x)
{
    const float t = Clamp01((x - e0) / (e1 - e0));
    return t * t * (3.0f - 2.0f * t);
}

// The clamped cosine's convolution per band (Ramamoorthi and Hanrahan 2001).
constexpr float kBandA[kSkyShCoeffs] = {kPi, 2.0f * kPi / 3.0f, 2.0f * kPi / 3.0f, kPi / 4.0f, kPi / 4.0f, kPi / 4.0f};
} // namespace

float SkyShBasis(int i, float x, float y, float z)
{
    switch (i)
    {
        case 0:
            return 0.282095f;
        case 1:
            return 0.488603f * y;
        case 2:
            return 0.488603f * x;
        case 3:
            return 0.315392f * (3.0f * y * y - 1.0f);
        case 4:
            return 1.092548f * x * y;
        case 5:
            return 0.546274f * (x * x - z * z);
        default:
            return 0.0f;
    }
}

SkyLightSH ProjectSkyRadiance(const std::function<Rgb(float, float)>& radiance, int elevationSteps, int azimuthSteps)
{
    SkyLightSH out;
    const int ne = std::max(elevationSteps, 1);
    const int na = std::max(azimuthSteps, 1);
    // d(omega) = d(sin e) d(phi); phi over [0, pi], doubled for the mirror half.
    const float cell = (1.0f / static_cast<float>(ne)) * (kPi / static_cast<float>(na)) * 2.0f;
    for (int i = 0; i < ne; ++i)
    {
        const float s = (static_cast<float>(i) + 0.5f) / static_cast<float>(ne);
        const float e = std::asin(s);
        const float c = std::cos(e);
        for (int j = 0; j < na; ++j)
        {
            const float phi = (static_cast<float>(j) + 0.5f) / static_cast<float>(na) * kPi;
            const Rgb l = radiance(e, phi);
            const float x = c * std::cos(phi), y = s, z = c * std::sin(phi);
            for (int k = 0; k < kSkyShCoeffs; ++k)
                out.c[k] = out.c[k] + l * (SkyShBasis(k, x, y, z) * cell);
        }
    }
    for (int k = 0; k < kSkyShCoeffs; ++k)
        out.c[k] = out.c[k] * kBandA[k];
    out.valid = true;
    return out;
}

SkyLightSH ProjectSkyLight(const AtmosphereTables& t, float sunElevationRad, int elevationSteps, int azimuthSteps)
{
    SkyLightSH sh =
        ProjectSkyRadiance([&t, sunElevationRad](float e, float phi)
                           { return SkyRadianceToward(t, e, phi, sunElevationRad); }, elevationSteps, azimuthSteps);
    sh.sunElevationRad = sunElevationRad;
    sh.haze = t.haze;
    sh.valid = t.Valid();
    // The direct sun the same air lets through, at the table's eye height.
    sh.sunTransmittance =
        AtmosphereTransmittance(t, atmo::kGroundRadius + std::max(kSkyViewerHeight, 1.0f), std::sin(sunElevationRad));
    return sh;
}

Rgb EvalSkyLight(const SkyLightSH& sh, float x, float y, float z)
{
    Rgb e;
    for (int k = 0; k < kSkyShCoeffs; ++k)
        e = e + sh.c[k] * SkyShBasis(k, x, y, z);
    return e;
}

float SurfaceDebugView(int debugView)
{
    if (debugView >= kDebugViewSkyLight && debugView <= kDebugViewSurfaceSpecular)
        return static_cast<float>(debugView - kDebugViewSkyLight + 1);
    return 0.0f;
}

float SurfaceShadeFactor(float knob, float sunElevationDeg, float skyThrough)
{
    const float k = std::min(std::max(knob, 0.0f), 1.0f);
    const float w = Smoothstep(3.0f, 18.0f, sunElevationDeg) * Smoothstep(0.35f, 0.85f, skyThrough);
    return 1.0f + (k - 1.0f) * w;
}

float SkyLightWeight(float knob, float sunElevationDeg, float skyThrough)
{
    // Clear to broken skies keep the table's shape; a deck flattens it, where
    // the rig's hemisphere (its overcast fill) takes over.
    return Clamp01(knob) * SkyDayFade(sunElevationDeg) * Smoothstep(0.35f, 0.8f, skyThrough);
}

SurfaceFrameConstants ComputeSurfaceFrameConstants(const LightingTuning& tuning, const FeatureGates& gates,
                                                   const LightRig& rig, const SkyLightSH& sh, float skyThrough)
{
    SurfaceFrameConstants c;
    const bool on = gates.computed && gates[Feature::Surfaces].on;
    if (!on)
        return c;
    c.surface[0] = 1.0f;
    c.surface[1] = std::max(tuning.surfSpec, 0.0f);
    c.surface[2] = std::max(tuning.surfCanopyAo, 0.0f);
    // The texture relief and cavities read the sun's grain: by night (lamps
    // only) they show nothing and cost their texture reads, so they fade out
    // with the sun (look director: the night pays no world-pass cost for them).
    // Under a low sun (look director round 2) they keep kSurfLowSunGrain: a
    // grazing sun lit every grain of the land into frosty speckles on the
    // backlit foreground of a sunrise.
    const float sunGrain =
        Smoothstep(-6.0f, 0.0f, rig.sunElevationDeg) *
        (kSurfLowSunGrain + (1.0f - kSurfLowSunGrain) * Smoothstep(4.0f, 14.0f, rig.sunElevationDeg));
    c.surface2[0] = std::max(tuning.surfRelief, 0.0f) * sunGrain;
    c.surface2[1] = SurfaceDebugView(tuning.debugView);
    c.surface2[3] = SurfaceShadeFactor(tuning.surfShade, rig.sunElevationDeg, skyThrough);
    c.surface3[0] = std::max(tuning.surfCavity, 0.0f) * sunGrain;
    if (!sh.valid)
        return c;
    // The table gives the shape across directions and (by surfSkyTint, in the
    // shader) the colour; the brightness on an up-facing surface is the rig's sky, or (by
    // surfSkyLevel) the table's own sky against the rig's sun: the sky's
    // irradiance over the direct sun's on flat ground, as the same air gives
    // both, times the rig's sun. A clear noon sky lights the shade about half as
    // brightly as the rig's calibrated fill does. Near the horizon the table's
    // sun and the rig's part ways, so the rig's level returns below 10 degrees.
    const Rgb up = EvalSkyLight(sh, 0.0f, 1.0f, 0.0f);
    const float upLum = Luminance(up);
    const float rigLum = Luminance(rig.skyZenith);
    if (!(upLum > 1e-9f) || !(rigLum > 0.0f))
        return c;
    float target = rigLum;
    const float sunT = Luminance(sh.sunTransmittance);
    const float level = Clamp01(tuning.surfSkyLevel) * Smoothstep(4.0f, 10.0f, rig.sunElevationDeg);
    if (level > 0.0f && sunT > 1e-4f)
    {
        const float phys = std::min(std::max(upLum * Luminance(rig.sun) / sunT, 0.35f * rigLum), 1.5f * rigLum);
        target = rigLum + (phys - rigLum) * level;
    }
    // The coefficients keep the physical sky's colour; the shader takes the
    // rig's own colour toward each normal at the brightness they give, and
    // mixes in surfSkyTint of theirs (surface2.z), so the shaded side of a
    // hull keeps the rig's horizon colour instead of the zenith's blue.
    const float scale = target / upLum;
    c.surface2[2] = Clamp01(tuning.surfSkyTint);
    c.surface[3] = SkyLightWeight(tuning.surfSkyLight, rig.sunElevationDeg, skyThrough);
    for (int k = 0; k < kSkyShCoeffs; ++k)
    {
        const Rgb v = sh.c[k] * scale;
        c.sh[k][0] = v.r;
        c.sh[k][1] = v.g;
        c.sh[k][2] = v.b;
        c.sh[k][3] = 0.0f;
    }
    return c;
}

} // namespace Poseidon::remaster
