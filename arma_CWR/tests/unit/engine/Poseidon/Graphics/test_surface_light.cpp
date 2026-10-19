#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <Poseidon/Graphics/Remaster/Atmosphere.hpp>
#include <Poseidon/Graphics/Remaster/FeatureGates.hpp>
#include <Poseidon/Graphics/Remaster/FeatureSwitches.hpp>
#include <Poseidon/Graphics/Remaster/PostStages.hpp>
#include <Poseidon/Graphics/Remaster/SurfaceMaterials.hpp>
#include <Poseidon/Graphics/Remaster/SurfaceLight.hpp>
#include <PoseidonGL33/EngineGL33_Lighting.hpp>

#include <algorithm>
#include <cmath>
#include <string>

// Surfaces and ambient light (the look lane "surface",
// Graphics/Remaster/SurfaceLight.hpp): the sky light's spherical harmonics on
// skies whose irradiance is known in closed form, the frame constants against
// the rig, the surface classes of known textures, and the contracts that tie
// the shaders to the C++.

using namespace Poseidon::remaster;
namespace gl = Poseidon::gl33lighting;
using Catch::Approx;

namespace
{
constexpr float kPi = 3.14159265358979f;

// A uniform sky of radiance 1 over the upper hemisphere.
Rgb UniformSky(float, float)
{
    return Rgb(1.0f, 1.0f, 1.0f);
}

// A sky brighter toward the sun's azimuth (x) and the horizon.
Rgb LopsidedSky(float elevation, float azimuth)
{
    const float toward = 0.5f + 0.5f * std::cos(azimuth);
    const float horizon = 1.0f - 0.5f * std::sin(elevation);
    return Rgb(0.6f, 0.8f, 1.2f) * (0.5f + toward) * horizon;
}

FeatureGates GatesWith(bool surfacesOn)
{
    GateInputs in;
    in.sunElevationDeg = 30.0f;
    LightingTuning t;
    t.features.surfaces = surfacesOn ? 1.0f : 0.0f;
    return ComputeFeatureGates(in, t);
}
} // namespace

TEST_CASE("Sky light SH: a uniform sky gives the hemisphere's irradiance", "[SurfaceLight]")
{
    const SkyLightSH sh = ProjectSkyRadiance(UniformSky, 24, 32);
    REQUIRE(sh.valid);
    // Up-facing: pi; a vertical surface: pi / 2; down-facing: nothing. Band
    // 2 keeps the hemisphere step within a few percent.
    CHECK(EvalSkyLight(sh, 0.0f, 1.0f, 0.0f).g == Approx(kPi).epsilon(0.05));
    CHECK(EvalSkyLight(sh, 1.0f, 0.0f, 0.0f).g == Approx(kPi * 0.5f).epsilon(0.05));
    CHECK(EvalSkyLight(sh, 0.0f, 0.0f, 1.0f).g == Approx(kPi * 0.5f).epsilon(0.05));
    CHECK(std::fabs(EvalSkyLight(sh, 0.0f, -1.0f, 0.0f).g) < 0.15f * kPi);
}

TEST_CASE("Sky light SH: a lopsided sky lights the sun side more and stays mirror-symmetric", "[SurfaceLight]")
{
    const SkyLightSH sh = ProjectSkyRadiance(LopsidedSky, 24, 32);
    const Rgb toward = EvalSkyLight(sh, 1.0f, 0.0f, 0.0f);
    const Rgb away = EvalSkyLight(sh, -1.0f, 0.0f, 0.0f);
    CHECK(Luminance(toward) > 1.5f * Luminance(away));
    // The basis holds only terms even in z: +z and -z are the same.
    const float s = std::sqrt(0.5f);
    CHECK(Luminance(EvalSkyLight(sh, s, 0.0f, s)) == Approx(Luminance(EvalSkyLight(sh, s, 0.0f, -s))));
    // The sky's own colour survives.
    CHECK(toward.b > toward.r);
}

TEST_CASE("Sky light SH: the clear sky of the atmosphere tables is blue and brighter toward a low sun",
          "[SurfaceLight][Atmosphere]")
{
    const AtmosphereTables t = BuildAtmosphereTables(1.0f);
    REQUIRE(t.Valid());
    const SkyLightSH sh = ProjectSkyLight(t, 10.0f * kPi / 180.0f, 8, 12);
    REQUIRE(sh.valid);
    const Rgb up = EvalSkyLight(sh, 0.0f, 1.0f, 0.0f);
    CHECK(up.b > up.r);
    CHECK(Luminance(EvalSkyLight(sh, 1.0f, 0.0f, 0.0f)) > Luminance(EvalSkyLight(sh, -1.0f, 0.0f, 0.0f)));
}

TEST_CASE("Surface frame constants: zero while off, the rig's zenith brightness while on", "[SurfaceLight]")
{
    const SkyLightSH sh = ProjectSkyRadiance(LopsidedSky, 16, 16);
    LightRig rig;
    rig.skyZenith = Rgb(0.10f, 0.12f, 0.16f);
    rig.sunElevationDeg = 30.0f;
    LightingTuning t;

    const SurfaceFrameConstants off = ComputeSurfaceFrameConstants(t, GatesWith(false), rig, sh, 1.0f);
    CHECK(off.surface[0] == 0.0f);
    CHECK(off.surface[3] == 0.0f);

    // The table's sky light is off by default (the rig's hemisphere); at 1:
    t.surfSkyLight = 1.0f;
    t.surfSkyTint = 1.0f;
    t.surfSkyLevel = 0.0f;
    const SurfaceFrameConstants on = ComputeSurfaceFrameConstants(t, GatesWith(true), rig, sh, 1.0f);
    CHECK(on.surface[0] == 1.0f);
    CHECK(on.surface[1] == Approx(t.surfSpec));
    CHECK(on.surface[3] == Approx(1.0f));
    SkyLightSH scaled;
    for (int k = 0; k < kSkyShCoeffs; ++k)
        scaled.c[k] = Rgb(on.sh[k][0], on.sh[k][1], on.sh[k][2]);
    CHECK(Luminance(EvalSkyLight(scaled, 0.0f, 1.0f, 0.0f)) == Approx(Luminance(rig.skyZenith)).epsilon(1e-3));

    // The coefficients keep the table's colour; the tint (the share of it the
    // shader mixes into the rig's own colour) rides in surface2.z.
    CHECK(on.surface2[2] == Approx(1.0f));
    t.surfSkyTint = 0.0f;
    const SurfaceFrameConstants grey = ComputeSurfaceFrameConstants(t, GatesWith(true), rig, sh, 1.0f);
    CHECK(grey.surface2[2] == 0.0f);
    CHECK(grey.sh[0][2] / grey.sh[0][0] == Approx(sh.c[0].b / sh.c[0].r).epsilon(1e-3));

    // The physical level: the table's sky against the rig's sun, through the
    // air's transmittance (bounded to 0.35..1.5 of the rig's fill).
    SkyLightSH withSun = sh;
    withSun.sunTransmittance = Rgb(0.8f, 0.8f, 0.8f);
    rig.sun = Rgb(0.5f, 0.5f, 0.5f);
    t.surfSkyLevel = 1.0f;
    const SurfaceFrameConstants phys = ComputeSurfaceFrameConstants(t, GatesWith(true), rig, withSun, 1.0f);
    SkyLightSH physScaled;
    for (int k = 0; k < kSkyShCoeffs; ++k)
        physScaled.c[k] = Rgb(phys.sh[k][0], phys.sh[k][1], phys.sh[k][2]);
    const float upPhys = Luminance(EvalSkyLight(sh, 0.0f, 1.0f, 0.0f)) * 0.5f / 0.8f;
    const float want = std::min(std::max(upPhys, 0.35f * Luminance(rig.skyZenith)), 1.5f * Luminance(rig.skyZenith));
    CHECK(Luminance(EvalSkyLight(physScaled, 0.0f, 1.0f, 0.0f)) == Approx(want).epsilon(1e-3));
    t.surfSkyLevel = 0.0f;

    // Overcast and night fall back to the rig's hemisphere.
    CHECK(ComputeSurfaceFrameConstants(t, GatesWith(true), rig, sh, 0.1f).surface[3] == 0.0f);
    CHECK(SkyLightWeight(1.0f, -12.0f, 1.0f) == 0.0f);
}

TEST_CASE("Surfaces: the switch is registered with its knobs and lands", "[SurfaceLight][FeatureSwitches]")
{
    const FeatureInfo* f = FindFeature("surfaces");
    REQUIRE(f != nullptr);
    CHECK(f->landed);
    for (const char* key : {"surfAo", "surfAoRadius", "surfCanopyAo", "surfSkyLight", "surfSkyTint", "surfSpec",
                            "surfContact", "surfRelief", "surfSkyLevel", "surfShade", "surfCavity"})
    {
        CAPTURE(key);
        const LightingKey* k = FindLightingKey(key);
        REQUIRE(k != nullptr);
        CHECK(std::string(k->parent) == "surfaces");
    }
    // The lane's debug views reach the shaders as 1 to 3.
    CHECK(SurfaceDebugView(kDebugViewSkyLight) == 1.0f);
    CHECK(SurfaceDebugView(kDebugViewSurfaceSpecular) == 3.0f);
    CHECK(SurfaceDebugView(kDebugViewAlbedo) == 0.0f);
    // Baseline turns it off with every other switch.
    CHECK(TuningForMode(LightingMode::Baseline).features.surfaces == 0.0f);
}

TEST_CASE("Surfaces: the shaders read the slots and the class table the C++ writes", "[SurfaceLight][Shaders]")
{
    const std::string common = gl::RemasterFragmentCommon();
    CHECK(common.find("layout(location = 1) out vec4 fragSurf;") != std::string::npos);
    CHECK(common.find("uniform sampler2D canopyHeight;") != std::string::npos);
    CHECK(common.find("vec4 litSkySh[6];") != std::string::npos);
    CHECK(gl::LitConst::SurfaceShCount == kSkyShCoeffs);
    // Every class has a row in the GLSL table (the default row is the fallback).
    const std::string surface = gl::RemasterFragmentPiece(gl::FragmentPiece::Surface);
    for (int i = 1; i < static_cast<int>(SurfaceClass::Count); ++i)
    {
        CAPTURE(i);
        CHECK(surface.find("if (i == " + std::to_string(i) + ") return vec3(") != std::string::npos);
    }
    // The post pass's horizon occlusion is one of the assembled post shaders.
    const char* gtao = gl::PostShaderSource(gl::PostShader::Gtao);
    REQUIRE(gtao != nullptr);
    CHECK(std::string(gtao).find("float viewZ(vec2 uv)") != std::string::npos);
    CHECK(std::string(gtao).find("const int kSlices = " + std::to_string(kGtaoSlices) + ";") != std::string::npos);
    CHECK(std::string(gtao).find("const int kSteps = " + std::to_string(kGtaoSteps) + ";") != std::string::npos);
    CHECK(std::string(gl::PostShaderName(gl::PostShader::Gtao)) == "psPostGtao");
}

TEST_CASE("Surfaces: the gate holds off once the sky is dark and no lamp is lit", "[SurfaceLight][FeatureGates]")
{
    const LightingTuning t = TuningForMode(LightingMode::Remastered);
    GateInputs day;
    CHECK(ComputeFeatureGates(day, t)[Feature::Surfaces].on);
    GateInputs night;
    night.sunElevationDeg = -20.0f;
    night.nightEffect = 1.0f;
    CHECK_FALSE(ComputeFeatureGates(night, t)[Feature::Surfaces].on);
    // Lamps give the occlusion something to ground.
    night.localLights = 3;
    CHECK(ComputeFeatureGates(night, t)[Feature::Surfaces].on);
    // Dusk keeps it: the sky still lights the shade.
    GateInputs dusk;
    dusk.sunElevationDeg = -4.0f;
    dusk.nightEffect = 0.5f;
    CHECK(ComputeFeatureGates(dusk, t)[Feature::Surfaces].on);
}

TEST_CASE("Surfaces: the shade keeps the knob's share of the fill only under a clear day sky",
          "[SurfaceLight][remaster]")
{
    // A clear sky with the sun high: the knob.
    CHECK(SurfaceShadeFactor(0.45f, 60.0f, 1.0f) == Catch::Approx(0.45f));
    // Night, twilight and a low sun keep the original's fill.
    CHECK(SurfaceShadeFactor(0.45f, -10.0f, 1.0f) == Catch::Approx(1.0f));
    CHECK(SurfaceShadeFactor(0.45f, 3.0f, 1.0f) == Catch::Approx(1.0f));
    const float low = SurfaceShadeFactor(0.45f, 10.0f, 1.0f);
    CHECK(low > 0.45f);
    CHECK(low < 1.0f);
    // An overcast sky lights the shade: the original's fill.
    CHECK(SurfaceShadeFactor(0.45f, 60.0f, 0.2f) == Catch::Approx(1.0f));
    // The knob is clamped to a share.
    CHECK(SurfaceShadeFactor(-1.0f, 60.0f, 1.0f) == Catch::Approx(0.0f));
    CHECK(SurfaceShadeFactor(3.0f, 60.0f, 1.0f) == Catch::Approx(1.0f));

    // The frame constants carry it (surface2.w) and the cavity (surface3.x),
    // and the off state leaves the fill as it was.
    LightingTuning t = TuningForMode(LightingMode::Remastered);
    GateInputs day;
    const FeatureGates gates = ComputeFeatureGates(day, t);
    LightRig rig;
    rig.sunElevationDeg = 60.0f;
    const SurfaceFrameConstants on = ComputeSurfaceFrameConstants(t, gates, rig, SkyLightSH{}, 1.0f);
    CHECK(on.surface2[3] == Catch::Approx(t.surfShade));
    CHECK(on.surface3[0] == Catch::Approx(t.surfCavity));
    t.features.surfaces = 0.0f;
    const SurfaceFrameConstants off =
        ComputeSurfaceFrameConstants(t, ComputeFeatureGates(day, t), rig, SkyLightSH{}, 1.0f);
    CHECK(off.surface2[3] == 1.0f);
    CHECK(off.surface3[0] == 0.0f);
}

TEST_CASE("Surfaces: a vehicle interior gets the occlusion of its own depth", "[SurfaceLight][PostStages]")
{
    PostFrameInputs in;
    in.ambientOcclusion = 1.0f;
    in.worldDepth = false;
    CHECK_FALSE(PlanPostStages(in).Runs(PostStage::Ao));
    in.surfaceInterior = true;
    const PostStagePlan plan = PlanPostStages(in);
    CHECK(plan.Runs(PostStage::Ao));
    CHECK(plan.aoStrength == 1.0f);
    // No TAA, air or shafts there.
    in.temporal = true;
    CHECK_FALSE(PlanPostStages(in).Runs(PostStage::Temporal));
    // The accumulation's weight and slices.
    CHECK(kGtaoTemporalSlices < kGtaoSlices);
    CHECK(kGtaoHistoryWeight > 0.5f);
    CHECK(kGtaoHistoryWeight < 1.0f);
}
