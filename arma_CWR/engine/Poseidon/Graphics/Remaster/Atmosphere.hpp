#pragma once

#include <Poseidon/Graphics/Remaster/LightRig.hpp>

#include <vector>

// Physically based clear-sky atmosphere for the remastered sky (the sky table,
// feature switch `atmosphere`): Rayleigh, Mie and ozone, after Hillaire 2020
// ("A Scalable and Production Ready Sky and Atmosphere Rendering Technique")
// with the Bruneton 2017 constants. Pure math, no GL, no engine state.
//
// Three tables:
//   - transmittance (kTransmittanceWidth x kTransmittanceHeight, Bruneton's
//     parametrisation) and multiple scattering (Hillaire's isotropic Psi_ms,
//     kMultiScatterSize squared) depend on the haze alone: BuildAtmosphereTables,
//     about 100 ms, run on a worker thread when the haze changes;
//   - the sky-view table, kSkyViewRegions regions of kSkyViewSize squared side
//     by side, depends on the sun elevation and is built on the GPU
//     (EngineGL33_SkyTable.cpp) whenever the sun moves kSkyTableRebuildDeg:
//       region 0: Rayleigh single scattering with its phase plus multiple
//                 scattering;
//       region 1: Mie single scattering without its phase (the shaders apply
//                 the phase per pixel, so the aureole around the sun stays
//                 sharp);
//       region 2: the azimuthal mean of the full radiance of each elevation
//                 row, so a shader relates a direction to the mean of its row
//                 in one tap.
//     u = sqrt(phi / pi), phi = azimuth from the sun in [0, pi];
//     v = 0.5 + 0.5 sign(l) sqrt(|l| / (pi / 2)), l = elevation.
// SkyViewTexel and SkyViewRowMean are the CPU mirrors of the two GPU passes
// (same steps, same lookups), so a texel read back from the GPU can be checked
// against them. Radiance is per unit of solar irradiance at the top of the
// atmosphere.

namespace Poseidon::remaster
{

namespace atmo
{
// Earth (metres; scattering and absorption per metre).
constexpr float kGroundRadius = 6360000.0f;
constexpr float kTopRadius = 6460000.0f;
constexpr float kRayleighScaleHeight = 8000.0f;
constexpr float kMieScaleHeight = 1200.0f;
constexpr float kRayleighScattering[3] = {5.802e-6f, 13.558e-6f, 33.1e-6f};
constexpr float kMieScattering = 3.996e-6f;
constexpr float kMieExtinction = 4.40e-6f;
constexpr float kOzoneAbsorption[3] = {0.650e-6f, 1.881e-6f, 0.085e-6f};
constexpr float kOzoneCentre = 25000.0f;
constexpr float kOzoneHalfWidth = 15000.0f;
constexpr float kGroundAlbedo = 0.3f;
constexpr float kMieG = 0.8f; // Cornette-Shanks asymmetry
} // namespace atmo

constexpr int kTransmittanceWidth = 256; // view zenith cosine
constexpr int kTransmittanceHeight = 64; // altitude
constexpr int kMultiScatterSize = 32;    // sun zenith cosine x altitude
constexpr int kSkyViewSize = 64;         // per region: azimuth x elevation
constexpr int kSkyViewRegions = 3;
constexpr int kSkyViewSteps = 40;    // march steps per sky-view texel
constexpr int kSkyViewMeanTaps = 96; // azimuths averaged per row in region 2
// The table is built for an eye this high above the ground; the dome and the
// fog colour of the whole map use it (CWA eyes are within a few hundred metres
// of the sea).
constexpr float kSkyViewerHeight = 150.0f;
// Height of the cloud deck the sky clouds are lit from (the sun seen from
// there is less reddened than from the ground).
constexpr float kSkyCloudDeckHeight = 2500.0f;

// What the frame does with the table.
constexpr float kSkyTableRebuildDeg = 0.02f; // the sun moves this far: rebuild the sky-view table
constexpr float kSkyNightFadeTopDeg = -3.0f; // full strength above this sun elevation
constexpr float kSkyNightFadeEndDeg = -9.0f; // gone below this (the gate turns the feature off there)
// Sky texels this much brighter (display luminance) than the clearest texel of
// their row count as cloud. Small: the classic sky textures are 8-bit
// palettised, and the faint noon cirrus of oblacno.pac sits only a few levels
// above its row (a 0.015 dead zone erased most of it).
constexpr float kSkyCloudMaskDeadZone = 0.004f;
// The hue and saturation the sky ships with (plan section 3.1): half way
// between the original sky's hue relit by the atmosphere and the physical hue,
// and a display-space saturation of 1.15 on the dome and the fog colour only.
constexpr float kSkyHueDefault = 0.5f;
constexpr float kSkySaturationDefault = 1.15f;

struct AtmosphereTables
{
    float haze = -1.0f;               // Mie density multiplier the tables were built for
    std::vector<float> transmittance; // kTransmittanceWidth x kTransmittanceHeight x 3
    std::vector<float> multiScatter;  // kMultiScatterSize x kMultiScatterSize x 3
    bool Valid() const;
};

// Transmittance and multiple-scattering tables for a haze (Mie density
// multiplier, 1 = the Bruneton atmosphere). About 100 ms; a worker thread.
AtmosphereTables BuildAtmosphereTables(float haze);

// Transmittance from radius r (metres from the planet centre) along a ray of
// zenith cosine mu to the top of the atmosphere; 0 when the ray hits the
// ground. Bilinear on the table's grid points (the GPU pass reads the same).
Rgb AtmosphereTransmittance(const AtmosphereTables& t, float r, float mu);
// Multiple-scattering transfer at a height above the ground for a sun of
// zenith cosine muS.
Rgb AtmosphereMultiScatter(const AtmosphereTables& t, float height, float muS);
float AtmosphereRayleighPhase(float cosTheta);
float AtmosphereMiePhase(float cosTheta);

// The sky-view parametrisation at texel centres: the elevation of a row and
// the azimuth from the sun of a column (radians).
float SkyViewRowElevation(int row);
float SkyViewColumnAzimuth(int col);

// One texel of region 0 or 1 of the sky-view table (columns and rows
// 0..kSkyViewSize-1) for a sun at this elevation (radians), as the GPU pass
// computes it.
Rgb SkyViewTexel(const AtmosphereTables& t, int region, int col, int row, float sunElevationRad);
// The full sky radiance (Rayleigh, multiple scattering and Mie with its phase)
// along any direction, marched as a sky-view texel is: the sky light's
// spherical harmonics (SurfaceLight.hpp) integrate it on the CPU.
Rgb SkyRadianceToward(const AtmosphereTables& t, float elevationRad, float azimuthFromSunRad, float sunElevationRad);

struct SkyViewTable
{
    std::vector<float> rgba; // (kSkyViewRegions * kSkyViewSize) x kSkyViewSize x 4, rows bottom up
    float sunElevationRad = 0.0f;
    float haze = -1.0f;
    bool Valid() const;
    // Texel (x over the three regions, y = row) and its alpha.
    Rgb At(int x, int y) const;
    float AlphaAt(int x, int y) const;
};

// The whole table on the CPU (the reference the tests and the GPU check use).
void BuildSkyViewTable(const AtmosphereTables& t, float sunElevationRad, SkyViewTable& out);
// Region 2 of a row from regions 0 and 1 of `table` (the second GPU pass).
Rgb SkyViewRowMean(const SkyViewTable& table, int row);
// Region 2's alpha: the row's mean over the same azimuths of the original
// dome's sun-glow weight max(cos, 0)^6 (TLVertexTable::DoSkyLighting adds
// SunSkyColor times it), for a sun at this elevation. The shaders add the glow
// at this weight to a row's classic brightness reference, so the sky keeps the
// classic dome's brightness row by row, glow included.
float SkyViewRowGlow(float sunElevationRad, int row);
// Sky radiance (Rayleigh, multiple scattering and Mie with its phase) along a
// direction given by its elevation, its azimuth from the sun and its cosine to
// the sun: the lookup the shaders make.
Rgb SampleSkyView(const SkyViewTable& table, float elevationRad, float azimuthFromSunRad, float cosToSun);

// The frame's use of the table.
// True when the sky-view table must be (re)built: none yet, a new haze, or the
// sun moved kSkyTableRebuildDeg since it was built.
bool SkyTableNeedsRebuild(bool built, float builtSunElevationRad, float builtHaze, float sunElevationRad, float haze);
// Strength of the sky table at a sun elevation (degrees): 1 above
// kSkyNightFadeTopDeg, 0 below kSkyNightFadeEndDeg, smooth between, so night
// keeps the classic sky.
float SkyDayFade(float sunElevationDeg);
// Share of the sky that is an overcast deck (the zatazeno.pac share of the
// interpolated sky texture): 0 up to overcast 0.5 (sky-through 0.7), 1 from
// sky-through 0.1. The dome is cloud everywhere by that share and the fog keeps
// its classic colour.
float SkyOvercastCloudFloor(float skyThrough);
// Share of the classic dome light that lights the sky clouds from the sun (the
// rest is sky light): 0.7 by day, fading out as the sun sets.
float SkyCloudSunShare(float sunElevationDeg);

} // namespace Poseidon::remaster
