#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <Poseidon/Graphics/Remaster/AirMedium.hpp>
#include <Poseidon/Graphics/Remaster/FeatureGates.hpp>
#include <Poseidon/Graphics/Remaster/FeatureSwitches.hpp>
#include <Poseidon/Graphics/Remaster/PostStages.hpp>
#include <PoseidonGL33/EngineGL33_Lighting.hpp>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

// The air pass (Graphics/Remaster/AirMedium.hpp):
// the medium's closed forms against numeric integration, the valley floor on a
// synthetic valley, the weather and time rules, the classic fog's moved edge,
// and the contracts between the C++ and the shaders (the same constants, the
// same pieces, "off" leaving every program and pass as it was).

using namespace Poseidon::remaster;

namespace
{

namespace gl = Poseidon::gl33lighting;

// Midpoint rule over n steps.
template <typename F>
double Integrate(F f, double a, double b, int n = 20000)
{
    const double h = (b - a) / n;
    double sum = 0.0;
    for (int i = 0; i < n; ++i)
        sum += f(a + (i + 0.5) * h);
    return sum * h;
}

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

int CountOf(const std::string& haystack, const std::string& needle)
{
    int n = 0;
    for (size_t at = haystack.find(needle); at != std::string::npos; at = haystack.find(needle, at + needle.size()))
        ++n;
    return n;
}

// The body of `signature { ... }` in a C++ source (to the closing brace at the
// start of a line).
std::string FunctionBody(const std::string& src, const std::string& signature)
{
    const size_t at = src.find(signature);
    if (at == std::string::npos)
        return {};
    const size_t end = src.find("\n}\n", at);
    return src.substr(at, end == std::string::npos ? std::string::npos : end - at);
}

std::string Num(float v)
{
    std::ostringstream s;
    s << v;
    std::string out = s.str();
    if (out.find('.') == std::string::npos && out.find('e') == std::string::npos)
        out += ".0";
    return out;
}

AirInputs ClearNoon()
{
    AirInputs in;
    in.sunElevationDeg = 65.0f;
    in.sunEast = 0.1f;
    in.fogStart = 1194.0f;
    in.fogEnd = 3980.0f; // view distance 4000
    return in;
}

AirParts All()
{
    AirParts p;
    p.haze = p.valleyMist = p.beams = true;
    return p;
}

} // namespace

TEST_CASE("Air medium: the haze's optical length is the integral of its density", "[remaster][AirMedium]")
{
    const float H = kHazeScaleHeight;
    struct Ray
    {
        float yA, yB, len;
    };
    for (const Ray r : {Ray{0.0f, 0.0f, 3000.0f}, Ray{1.75f, 120.0f, 2500.0f}, Ray{300.0f, 2.0f, 4000.0f},
                        Ray{40.0f, 40.2f, 900.0f}, Ray{-30.0f, 600.0f, 5000.0f}, Ray{800.0f, 820.0f, 150.0f}})
    {
        CAPTURE(r.yA, r.yB, r.len);
        const double numeric = Integrate(
            [&](double t)
            {
                const double y = r.yA + (r.yB - r.yA) * t / r.len;
                return std::exp(-y / H);
            },
            0.0, r.len);
        CHECK(ExpOpticalLength(r.yA, r.yB, r.len, H) == Catch::Approx(numeric).epsilon(1e-4));
    }
    // Low ground is hazier than a ridge: the same 2 km at sea level and at 400 m.
    CHECK(ExpOpticalLength(0.0f, 0.0f, 2000.0f, H) > 2.5f * ExpOpticalLength(400.0f, 400.0f, 2000.0f, H));
}

TEST_CASE("Air medium: the valley mist's optical length is the integral of its flat-topped layer",
          "[remaster][AirMedium]")
{
    const float top = 45.0f, fall = kValleyMistFall;
    struct Ray
    {
        float yA, yB, len;
    };
    // Inside, above, crossing up and down, along the top.
    for (const Ray r : {Ray{5.0f, 12.0f, 1200.0f}, Ray{150.0f, 300.0f, 2000.0f}, Ray{10.0f, 200.0f, 3000.0f},
                        Ray{180.0f, 3.0f, 2500.0f}, Ray{45.2f, 45.0f, 800.0f}, Ray{60.0f, 60.3f, 700.0f}})
    {
        CAPTURE(r.yA, r.yB, r.len);
        const double numeric = Integrate(
            [&](double t)
            {
                const double y = r.yA + (r.yB - r.yA) * t / r.len;
                return std::min(1.0, std::exp(-(y - top) / fall));
            },
            0.0, r.len);
        CHECK(LayerOpticalLength(r.yA, r.yB, r.len, top, fall) == Catch::Approx(numeric).epsilon(2e-3).margin(0.05));
    }
}

TEST_CASE("Air medium: the optical depth of a line of sight, for the haze and for AI sight", "[remaster][AirMedium]")
{
    AirMedium m = ComputeAirMedium(ClearNoon(), All(), -1.0f);
    m.valleyMist = kValleyMistSigma;
    const float eye[3] = {6100.0f, 60.0f, 5000.0f};
    const float target[3] = {7300.0f, 8.0f, 6900.0f};
    const float top = 30.0f;
    const double d = std::sqrt(1200.0 * 1200.0 + 52.0 * 52.0 + 1900.0 * 1900.0);
    auto density = [&](double t)
    {
        const double y = eye[1] + (target[1] - eye[1]) * t / d;
        return m.hazeSigma * std::exp(-std::max(y, -30.0) / kHazeScaleHeight) +
               m.valleyMist * std::min(1.0, std::exp(-(y - top) / kValleyMistFall));
    };
    for (float from : {0.0f, kHazeNearDistance, 1000.0f})
    {
        CAPTURE(from);
        CHECK(AirOpticalDepth(m, eye, target, from, top) == Catch::Approx(Integrate(density, from, d)).epsilon(2e-3));
    }
    // Nothing inside the near distance; AI sight starts at the eye.
    const float near[3] = {6150.0f, 59.0f, 5080.0f};
    CHECK(AirOpticalDepth(m, eye, near, kHazeNearDistance, top) == 0.0f);
    CHECK(AirOpticalDepth(m, eye, near, 0.0f, top) > 0.0f);
    CHECK(AirOpticalDepth(m, eye, target, 0.0f, top) > AirOpticalDepth(m, eye, target, kHazeNearDistance, top));
    // No medium, no depth.
    CHECK(AirOpticalDepth(AirMedium{}, eye, target, 0.0f, top) == 0.0f);
}

TEST_CASE("Air medium: the valley floor is the lowest ground within about 850 m", "[remaster][AirMedium]")
{
    // A valley along z at x = 0, its floor at 20 m, walls rising 0.25 m per
    // metre; the sea beyond x = 3000.
    auto height = [](float x, float /*z*/)
    {
        if (x > 3000.0f)
            return -15.0f;
        return 20.0f + 0.25f * std::fabs(x);
    };
    // On the floor: the floor.
    CHECK(LocalLowGround(height, 0.0f, 500.0f) == Catch::Approx(20.0f));
    // 300 m up the wall: a ring tap reaches across the floor (at most 400 m
    // out, so no lower than the floor).
    const float wall = LocalLowGround(height, 300.0f, 500.0f);
    CHECK(wall < height(300.0f, 500.0f));
    CHECK(wall >= 20.0f);
    CHECK(wall < 20.0f + 0.25f * 120.0f);
    // 1200 m up the wall: the valley is out of reach, the lowest tap is the far
    // ring's furthest down the slope (its taps start at 0.5 rad, 60 deg apart).
    float lowest = 1e9f;
    for (int k = 0; k < 6; ++k)
        lowest = std::min(lowest, height(1200.0f + 850.0f * std::cos(0.5f + 1.047198f * k), 0.0f));
    CHECK(LocalLowGround(height, 1200.0f, 500.0f) == Catch::Approx(lowest));
    CHECK(lowest < height(1200.0f - 400.0f, 0.0f));
    // Next to the sea the floor is sea level, never below.
    CHECK(LocalLowGround(height, 2800.0f, 0.0f) == 0.0f);
    // The valley is deep on its floor: full mist there.
    CHECK(LocalValleyAt(height, 0.0f, 500.0f).depth > kValleyDepthFull);
    CHECK(ValleyDepthWeight(LocalValleyAt(height, 0.0f, 500.0f).depth) == 1.0f);
    // A plain and the open sea pool none.
    auto plain = [](float, float) { return 35.0f; };
    CHECK(LocalValleyAt(plain, 100.0f, 100.0f).low == 35.0f);
    CHECK(LocalValleyAt(plain, 100.0f, 100.0f).depth == Catch::Approx(0.0f).margin(1e-4));
    CHECK(ValleyDepthWeight(0.0f) == 0.0f);
    CHECK(ValleyDepthWeight(LocalValleyAt(height, 5000.0f, 0.0f).depth) == 0.0f);
    // A gentle dip of a few metres stays clear; the weight rises to full by
    // kValleyDepthFull.
    CHECK(ValleyDepthWeight(kValleyDepthFrom) == 0.0f);
    CHECK(ValleyDepthWeight(0.5f * (kValleyDepthFrom + kValleyDepthFull)) == Catch::Approx(0.5f));
    // The mist's top sits kValleyMistTop above it, varied by the noise banks.
    CHECK(ValleyMistTop(20.0f, 0.5f, 0.5f) == Catch::Approx(20.0f + 0.9f * kValleyMistTop));
    CHECK(ValleyMistTop(20.0f, 0.0f, 0.5f) == Catch::Approx(20.0f + 0.45f * kValleyMistTop));
    CHECK(ValleyMistTop(20.0f, 1.0f, 1.0f) == Catch::Approx(20.0f + 1.35f * kValleyMistTop + 6.0f));
    CHECK(ValleyMistDensity(0.0f, 0.0f) == Catch::Approx(0.35f));
    CHECK(ValleyMistDensity(1.0f, 1.0f) == Catch::Approx(1.5f));
}

TEST_CASE("Air medium: morning mist with an eastern sun, gone by 10 degrees", "[remaster][AirMedium]")
{
    // Full from sunrise to 6 degrees (the ridge at 05:08 in July has the sun
    // at 5.1 degrees, Abel's valley at 05:15 at 6.3), half way at 8, gone by
    // 10 (about an hour after sunrise): a dawn feature. A field at 08:00 in
    // March has the sun at 12.7 degrees in game: no mist there.
    CHECK(MorningWeight(2.0f, 0.9f) == Catch::Approx(1.0f));
    CHECK(MorningWeight(5.1f, 0.6f) == Catch::Approx(1.0f));
    CHECK(MorningWeight(6.3f, 0.9f) > 0.95f);
    CHECK(MorningWeight(8.0f, 0.9f) == Catch::Approx(0.5f));
    CHECK(MorningWeight(9.5f, 0.9f) > 0.0f);
    CHECK(MorningWeight(9.5f, 0.9f) < 0.05f);
    CHECK(MorningWeight(10.0f, 0.9f) == 0.0f);
    CHECK(MorningWeight(12.7f, 0.9f) == 0.0f); // 08:00 in March
    CHECK(MorningWeight(-5.0f, 0.9f) == 0.0f); // still night
    CHECK(MorningWeight(8.0f, -0.6f) == 0.0f); // evening: a western sun
    // The gates use the same window (FeatureGates.hpp).
    CHECK(kMorningMinSunDeg == kGateMorningMinSunDeg);
    CHECK(kMorningGoneSunDeg == kGateMorningMaxSunDeg);
}

TEST_CASE("Air medium: clear air keeps the beams narrow, fog and rain make them misty", "[remaster][AirMedium]")
{
    // Clear noon: the haze, no valley mist, a thin beam mist with a strong
    // contrast gain (narrow beams through gaps survive, open ground gets no
    // wash).
    AirMedium m = ComputeAirMedium(ClearNoon(), All(), -1.0f);
    CHECK(m.hazeSigma == Catch::Approx(kHazeSigma));
    CHECK(m.valleyMist == 0.0f);
    CHECK(m.beamMist == Catch::Approx(kBeamMistSigma * kBeamMistClear));
    CHECK(m.beamAir == Catch::Approx(kHazeSigma));
    CHECK(m.beamGain == Catch::Approx(kBeamGainClear));
    CHECK(m.beamHighpass == Catch::Approx(kBeamHighpassClear));
    CHECK(m.beamMistWeight == 0.0f);

    // Sunrise in a clear valley (05:15 in July): the valley mist in full, the
    // beams clear.
    AirInputs in = ClearNoon();
    in.sunElevationDeg = 5.0f;
    in.sunEast = 0.8f;
    m = ComputeAirMedium(in, All(), -1.0f);
    CHECK(m.beamMistWeight == 0.0f);
    CHECK(m.valleyMist == Catch::Approx(kValleyMistSigma));
    // Under an overcast 40% of it.
    in.skyThrough = 0.0f;
    CHECK(ComputeAirMedium(in, All(), -1.0f).valleyMist == Catch::Approx(0.4f * kValleyMistSigma));
    // A clear morning two hours after sunrise (08:00 in March: the
    // sun at 12.7 degrees in game): the mist is gone and the beams stay clear.
    in.skyThrough = 1.0f;
    in.sunElevationDeg = 12.7f;
    m = ComputeAirMedium(in, All(), -1.0f);
    CHECK(m.beamMistWeight == 0.0f);
    CHECK(m.valleyMist == 0.0f);

    // setFog 0.1 (pinewood_morn): misty beams, softer gain, denser air, and
    // mist in the valleys at any hour from 0.3.
    in = ClearNoon();
    in.fog = 0.1f;
    m = ComputeAirMedium(in, All(), -1.0f);
    CHECK(m.beamMistWeight == Catch::Approx(1.0f));
    CHECK(m.beamMist == Catch::Approx(kBeamMistSigma * kBeamMistMisty));
    CHECK(m.beamGain == Catch::Approx(kBeamGainMisty));
    CHECK(m.beamHighpass == Catch::Approx(kBeamHighpassMisty));
    CHECK(m.hazeSigma == Catch::Approx(kHazeSigma * (1.0f + kHazeFogSigmaGain * 0.1f)));
    CHECK(m.valleyMist > 0.0f);
    in.fog = 0.3f;
    CHECK(ComputeAirMedium(in, All(), -1.0f).valleyMist == Catch::Approx(kValleyMistSigma));
    // Heavier fog thickens the beam mist further.
    in.fog = 0.35f;
    CHECK(ComputeAirMedium(in, All(), -1.0f).beamMist > kBeamMistSigma * kBeamMistMisty);

    // Rain.
    in = ClearNoon();
    in.rain = 0.8f;
    CHECK(ComputeAirMedium(in, All(), -1.0f).beamMistWeight == Catch::Approx(1.0f));
    // The sun at the horizon, and not an hour later.
    in = ClearNoon();
    in.sunElevationDeg = 0.0f;
    CHECK(ComputeAirMedium(in, All(), -1.0f).beamMistWeight == Catch::Approx(1.0f));
    in.sunElevationDeg = 5.0f; // a ridge at sunrise
    CHECK(ComputeAirMedium(in, All(), -1.0f).beamMistWeight == 0.0f);
    in.sunElevationDeg = -20.0f; // a clear night: headlights in clear air
    CHECK(ComputeAirMedium(in, All(), -1.0f).beamMistWeight == 0.0f);

    // A gain from the knob is used as given.
    CHECK(ComputeAirMedium(ClearNoon(), All(), 1.0f).beamGain == 1.0f);
    // Parts that do not run this frame leave their densities at zero, and the
    // classic fog unchanged.
    m = ComputeAirMedium(ClearNoon(), AirParts{}, -1.0f);
    CHECK(m.hazeSigma == 0.0f);
    CHECK(m.valleyMist == 0.0f);
    CHECK(m.beamMist == 0.0f);
    CHECK(m.fogEdgeScale == 1.0f);
}

TEST_CASE("Air medium: with the haze on the classic fog keeps only its view-distance edge", "[remaster][AirMedium]")
{
    auto fogFactor = [](double d, double start, double end)
    { return std::clamp(1.0 - (d - start) / (end - start), 0.0, 1.0); };
    // Clear weather at view distances 1500, 2500 and 4000: the start moves out
    // to 55% of the end, and scaling the vertex shader's factor gives exactly
    // the factor of the moved start at every distance.
    for (float end : {1480.0f, 2480.0f, 3980.0f})
    {
        const float start = 0.3f * end;
        const float k = FogEdgeScale(start, end);
        CAPTURE(end, k);
        CHECK(k > 1.0f);
        const double clearW = SmoothStep(kHazeClearFogEndMin, kHazeClearFogEndMax, end);
        const double moved = start + (kHazeEdgeFraction * end - start) * clearW;
        for (double d = 0.0; d < end + 200.0; d += 37.0)
        {
            CAPTURE(d);
            CHECK(std::min(1.0, fogFactor(d, start, end) * k) == Catch::Approx(fogFactor(d, moved, end)).margin(1e-5));
        }
    }
    // The medium hands the apply the moved range (its edge fog).
    CHECK(FogEdgeStart(1194.0f, 3980.0f) == Catch::Approx(kHazeEdgeFraction * 3980.0f));
    // Fog or rain close the range: the classic fog keeps its start.
    CHECK(FogEdgeScale(240.0f, 800.0f) == 1.0f);
    CHECK(FogEdgeStart(240.0f, 800.0f) == 240.0f);
    // No range: nothing to scale.
    CHECK(FogEdgeScale(100.0f, 100.0f) == 1.0f);
    CHECK(FogEdgeScale(0.0f, 0.0f) == 1.0f);
    // The medium carries it only with the haze on.
    AirParts haze;
    haze.haze = true;
    CHECK(ComputeAirMedium(ClearNoon(), haze, -1.0f).fogEdgeScale == FogEdgeScale(1194.0f, 3980.0f));
    CHECK(ComputeAirMedium(ClearNoon(), haze, -1.0f).fogStart == FogEdgeStart(1194.0f, 3980.0f));
    CHECK(ComputeAirMedium(ClearNoon(), haze, -1.0f).fogEnd == 3980.0f);
    AirParts noHaze = All();
    noHaze.haze = false;
    CHECK(ComputeAirMedium(ClearNoon(), noHaze, -1.0f).fogEdgeScale == 1.0f);
    CHECK(ComputeAirMedium(ClearNoon(), noHaze, -1.0f).fogStart == 1194.0f);
    // The apply puts the mist under that edge fog: toward the view distance it
    // converges to the sky limit, as the air does.
    const std::string apply = gl::PostShaderSource(gl::PostShader::AirApply);
    CHECK(apply.find("scat = mix(skyLim * (1.0 - T), scat, fEdge);") != std::string::npos);
}

TEST_CASE("Air medium: the shaders use the medium's constants and closed forms", "[remaster][AirMedium][Shaders]")
{
    const std::string march = gl::PostShaderSource(gl::PostShader::AirMarch);
    const std::string apply = gl::PostShaderSource(gl::PostShader::AirApply);
    REQUIRE_FALSE(march.empty());
    REQUIRE_FALSE(apply.empty());
    // The closed forms, as AirMedium.cpp writes them.
    for (const std::string* src : {&march, &apply})
    {
        CHECK(src->find("float f = abs(x) < 1e-3 ? 1.0 - 0.5 * x : (1.0 - exp(-x)) / x;") != std::string::npos);
        CHECK(src->find("return len * exp(-yA / H) * f;") != std::string::npos);
        CHECK(
            src->find("float above = a < hi ? fall * (exp(-(a - top) / fall) - exp(-(hi - top) / fall)) / dy : 0.0;") !=
            std::string::npos);
        CHECK(src->find("const float kAirHazeScaleHeight = " + Num(kHazeScaleHeight) + ";") != std::string::npos);
        CHECK(src->find("const float kAirMistFall = " + Num(kValleyMistFall) + ";") != std::string::npos);
    }
    // The haze's chroma and the sky light of shaded air, the valley floor's
    // rings, the mist's top and banks.
    CHECK(apply.find("vec3(" + Num(kHazeChromaR) + ", 1.0, " + Num(kHazeChromaB) + ")") != std::string::npos);
    CHECK(apply.find("const float kAirHazeShadowAmbient = " + Num(kHazeShadowAmbient) + ";") != std::string::npos);
    CHECK(apply.find("zen * (skyLum * kAirHazeShadowAmbient)") != std::string::npos);
    CHECK(march.find("* " + Num(kValleyNearRadius) + ", uv, onMap);") != std::string::npos);
    CHECK(march.find("* " + Num(kValleyFarRadius) + ", uv, onMap);") != std::string::npos);
    CHECK(march.find("top = airLocalLow(xz, depth) + " + Num(kValleyMistTop) +
                     " * mix(0.45, 1.35, n1) + (n2 - 0.5) * 12.0;") != std::string::npos);
    CHECK(march.find("mistDensity = mix(0.35, 1.5, n1 * 0.7 + n2 * 0.3) * smoothstep(" + Num(kValleyDepthFrom) + ", " +
                     Num(kValleyDepthFull) + ", depth);") != std::string::npos);
    CHECK(march.find("depth = sum / 14.0 - m;") != std::string::npos);
    // The march's loops hold the step counts the engine asks for.
    CHECK(kBeamSteps <= 48);
    CHECK(kHazeVisSteps <= 16);
    CHECK(kAirSpotSteps <= 32);
    CHECK(kAirMaxSpots == 4);
    CHECK(march.find("uniform vec4 airSpotPos[" + std::to_string(kAirMaxSpots) + "];") != std::string::npos);
    // The haze converges to the sky behind the pixel the way the classic fog
    // does for lit geometry (finishFragment): the fog colour toward the sun
    // and, under the sky table, its aerial perspective.
    CHECK(apply.find("vec3 c = fogColorToward(rel);") != std::string::npos);
    CHECK(apply.find("if (litAtmo.z > 0.0) c = atmoFogColor(rel, c);") != std::string::npos);
    // Nothing changes inside the near distance.
    CHECK(apply.find("if (!sky && dist > airMedium.z && (airMedium.x > 0.0 || airMedium.y > 0.0)) {") !=
          std::string::npos);
    // The beams only add light (screen add); the contrast never takes the
    // scattering below (1 - high-pass) of itself.
    // (Under the photoreal camera the frame can hold light above 1: none is
    // added there.)
    CHECK(apply.find("lin += max(1.0 - lin, vec3(0.0)) * (1.0 - exp(-max(glow, vec3(0.0))));") != std::string::npos);
    CHECK(apply.find("shaft = max(shaft - airBeams.y * local, floorAt) + boost2 * max(shaft - local, vec2(0.0));") !=
          std::string::npos);
    // The apply keeps the frame's alpha (the composite's occlusion reads it).
    CHECK(apply.find("fragColor = vec4(col, c.a);") != std::string::npos);
}

TEST_CASE("Air medium: the beams' contrast works on the share of the scattering that sees the sun",
          "[remaster][AirMedium][Shaders]")
{
    const float hp = kBeamHighpassClear;
    const float gain = kBeamGainClear;
    // Open ground toward a low sun: every ray sees the sun, but a
    // ray to the far plain runs longer through the ground mist than its
    // depth-matched neighbours (the mean leans to the nearer ground), so its
    // scattering stands above the mean, 0.18 against 0.15. Its share does
    // not: no boost, (1 - high-pass) of it, as anywhere the sun reaches the
    // whole field. A contrast on the scattering itself gave this ray
    // 0.18 - 0.85 * 0.15 + 3 * 0.03 = 0.14, five times as much: the pale band
    // along the plain in an early build.
    CHECK(BeamShaft(0.18f, 0.18f, 0.15f, 0.15f, 1.0f, hp, gain) == Catch::Approx((1.0f - hp) * 0.18f));
    CHECK(BeamShaft(0.05f, 0.05f, 0.15f, 0.15f, 1.0f, hp, gain) == Catch::Approx((1.0f - hp) * 0.05f));
    // A beam through a gap in the crowns: the ray sees the sun, its
    // neighbours mostly do not (the mean share is 0.3). It is boosted above
    // the plain scattering of the ray: a shaft.
    const float beam = BeamShaft(0.18f, 0.18f, 0.054f, 0.18f, 1.0f, hp, gain);
    CHECK(beam == Catch::Approx((1.0f - hp * 0.3f + (gain - 1.0f) * 0.7f) * 0.18f));
    CHECK(beam > 0.18f);
    // A shaded ray among lit ones keeps (1 - high-pass) of its own share.
    CHECK(BeamShaft(0.036f, 0.18f, 0.18f, 0.18f, 1.0f, hp, gain) == Catch::Approx((1.0f - hp) * 0.036f));
    // No depth match (a thin branch against the glow): the share is clamped
    // to the mean share and gets no boost.
    CHECK(BeamShaft(0.18f, 0.18f, 0.054f, 0.18f, 0.0f, hp, gain) == Catch::Approx((1.0f - hp) * 0.3f * 0.18f));
    // In mist the same rule with the misty high-pass and gain.
    CHECK(BeamShaft(0.6f, 0.6f, 0.4f, 0.4f, 1.0f, kBeamHighpassMisty, kBeamGainMisty) ==
          Catch::Approx((1.0f - kBeamHighpassMisty) * 0.6f));
    // Nothing to scatter: nothing added; never below zero.
    CHECK(BeamShaft(0.0f, 0.0f, 0.0f, 0.0f, 1.0f, hp, gain) == 0.0f);
    CHECK(BeamShaft(0.01f, 0.2f, 0.2f, 0.2f, 1.0f, hp, gain) >= 0.0f);

    // The apply runs the same steps (the sun in x, the headlight cones in y).
    const std::string apply = gl::PostShaderSource(gl::PostShader::AirApply);
    CHECK(apply.find("vec2 shaft = vec2(clamp(air.r / max(air.a, 1e-7), 0.0, 1.0), air.g);") != std::string::npos);
    CHECK(apply.find("vec2 local = vec2(clamp(mean.x / max(mean.z, 1e-7), 0.0, 1.0), mean.y);") != std::string::npos);
    CHECK(apply.find("shaft = mix(min(shaft, local), shaft, conf);") != std::string::npos);
    CHECK(apply.find("glow = airSunColor.rgb * (shaft.x * air.a * camBeams) + airSpotTint.rgb * shaft.y;") !=
          std::string::npos);
    // The march hands it both scatterings and the view depth in the second
    // target; the blur and the mean match depths from there, and the mean
    // averages both scatterings.
    const std::string march = gl::PostShaderSource(gl::PostShader::AirMarch);
    CHECK(march.find("sumOpen += w;") != std::string::npos);
    CHECK(march.find("fragColor = vec4(sunBeams, spots, vis, sunOpen);") != std::string::npos);
    CHECK(march.find("fragMist = vec4(top, mistDensity, min(z, 6e4), 1.0);") != std::string::npos);
    const std::string blur = gl::PostShaderSource(gl::PostShader::AirBlur);
    CHECK(blur.find("float zc = texture(airDepthTex, vUV).b;") != std::string::npos);
    CHECK(blur.find("fragColor = sum / wsum;") != std::string::npos);
    const std::string mean = gl::PostShaderSource(gl::PostShader::AirMean);
    CHECK(mean.find("float zc = texture(airDepthTex, vUV).b;") != std::string::npos);
    CHECK(mean.find("mean += texture(airTex, uv).rga * wk;") != std::string::npos);
    CHECK(apply.find("float dn = abs(m.b - z) / (0.05 * z + 0.3);") != std::string::npos);
    // The driver binds the depth on a post unit (never unit 1) for both.
    const std::string air = ReadTextFile(Gl33Dir() / "EngineGL33_PostAir.cpp");
    CHECK(CountOf(air, "\"airDepthTex\", 3);") == 2);
    CHECK(CountOf(air, "BindTex(3, p.airMistTex);") == 2);
}

TEST_CASE("Air pass: the march and the apply paste the remastered prelude's pieces, once",
          "[remaster][AirMedium][Shaders]")
{
    const std::string march = gl::PostShaderSource(gl::PostShader::AirMarch);
    const std::string apply = gl::PostShaderSource(gl::PostShader::AirApply);
    CHECK(CountOf(march, gl::LitConstantsBlock()) == 1);
    CHECK(CountOf(apply, gl::LitConstantsBlock()) == 1);
    // The cloud field and the horizon map the shading reads.
    CHECK(CountOf(march, gl::RemasterFragmentPiece(gl::FragmentPiece::TerrainClouds)) == 1);
    CHECK(march.find("cloudSunVisibility(pa)") != std::string::npos);
    // Its sky-matched cloud shadows read the sky's cloud density: the
    // march binds the world's map on the world's unit.
    const std::string air = ReadTextFile(Gl33Dir() / "EngineGL33_PostAir.cpp");
    CHECK(air.find("SetInt(prog, \"skyDensity\", kSkyDensityUnit);") != std::string::npos);
    CHECK(air.find("BindSkyDensity();") != std::string::npos);
    // The sky table's fog colour.
    CHECK(CountOf(apply, gl::RemasterFragmentPiece(gl::FragmentPiece::Sky)) == 1);
    CHECK(gl::PostShaderName(gl::PostShader::AirMarch) == std::string("psPostAirMarch"));
    CHECK(gl::PostShaderName(gl::PostShader::AirApply) == std::string("psPostAirApply"));
    // The march writes two targets.
    CHECK(march.find("layout(location = 0) out vec4 fragColor;") != std::string::npos);
    CHECK(march.find("layout(location = 1) out vec4 fragMist;") != std::string::npos);
}

TEST_CASE("Air pass: off leaves every draw and the post pass as they were", "[remaster][AirMedium][Shaders][GL33]")
{
    // The classic fog's factor is scaled by 1 + litHaze.x, which the engine
    // leaves at 0 unless the haze runs this frame (UpdateAirMedium), and a
    // zero block (before the first upload) means the classic fog too.
    const std::string finish = gl::RemasterFragmentPiece(gl::FragmentPiece::Finish);
    CHECK(finish.find("r0.rgb = mix(fogC, r0.rgb, min(vFogTC * (1.0 + litHaze.x), 1.0));") != std::string::npos);
    // The sea ends in finishFragment too, so it takes the same edge.
    const std::string water = gl::RemasterFragmentMain(3); // PSWater
    CHECK(water.find("finishFragment(r0, ambientShare, true);") != std::string::npos);
    CHECK(water.find("r0.rgb = mix(fogC, r0.rgb, vFogTC);") == std::string::npos);
    CHECK(static_cast<int>(gl::LitConst::Haze) == 78);
    const std::string air = ReadTextFile(Gl33Dir() / "EngineGL33_PostAir.cpp");
    REQUIRE_FALSE(air.empty());
    const std::string update = FunctionBody(air, "void EngineGL33::UpdateAirMedium()");
    REQUIRE_FALSE(update.empty());
    CHECK(update.find("const float scale = _airFrame == frame ? _airMedium.fogEdgeScale : 1.0f;") != std::string::npos);
    CHECK(update.find("SetLitSlot(LitConst::Haze, scale - 1.0f, 0.0f, 0.0f, 0.0f);") != std::string::npos);
    // The haze's edge moves only where the post pass will run it.
    CHECK(update.find("const bool post = _lighting.postEffects && SSAAActive() && !_post.unavailable") !=
          std::string::npos);
    CHECK(update.find("parts.haze = post && _featureGates[rm::Feature::AirHaze].on;") != std::string::npos);

    // The bloom and the composite read the frame with the air applied only
    // when the Air stage ran; otherwise the frame the stages before it left
    // (the resolved frame, or the reflections' output), as before
    // (PostSceneTex, which picks the volumetric fog's output when the fog ran
    // the stage instead). The air pass applies over that frame.
    const std::string post = ReadTextFile(Gl33Dir() / "EngineGL33_PostFX.cpp");
    REQUIRE_FALSE(post.empty());
    CHECK(CountOf(post, "first ? PostSceneTex(f) : p.bloomTex[i - 1]") == 1);
    CHECK(CountOf(post, "BindTex(0, PostSceneTex(f));") == 1);
    CHECK(CountOf(air, "BindTex(0, f.frameColor);") == 1);
    const std::string scene =
        FunctionBody(ReadTextFile(Gl33Dir() / "EngineGL33_PostFog.cpp"), "unsigned int EngineGL33::PostSceneTex");
    REQUIRE_FALSE(scene.empty());
    CHECK(scene.find("if (!f.plan.Runs(rm::PostStage::Air))\n        return f.frameColor;") != std::string::npos);
    CHECK(scene.find("if (!f.plan.fog)\n        return _post.airOutTex;") != std::string::npos);
    CHECK(CountOf(post, "in.air = airFrame && (_airParts.haze || _airParts.valleyMist || _airParts.beams);") == 1);
    // With every air switch off the plan is the plan of before.
    PostFrameInputs in;
    in.ambientOcclusion = 0.7f;
    in.bloom = 0.35f;
    in.sunShafts = 1.0f;
    in.shaftWeight = 0.5f;
    const std::uint32_t before = PlanPostStages(in).runs;
    in.airBeams = true; // without the pass, the flag alone changes nothing
    CHECK(PlanPostStages(in).runs == before);
}

TEST_CASE("Air pass: unit 2 keeps a complete depth array and gets its post sampler back",
          "[remaster][AirMedium][GL33][ShadowSampler]")
{
    // A shader must never sample an incomplete texture: unit 2 never holds texture 0 for a depth array, and the
    // comparison sampler the march reads it through goes away after the draw
    // (the post pass and the HUD after it run with sampler 0 there).
    const std::string air = ReadTextFile(Gl33Dir() / "EngineGL33_PostAir.cpp");
    const std::string body = FunctionBody(air, "void EngineGL33::PostAir(const PostFrame& f)");
    REQUIRE_FALSE(body.empty());
    const size_t unit = body.find("glActiveTexture(GL_TEXTURE0 + kShadowMapUnit);");
    REQUIRE(unit != std::string::npos);
    const size_t bind =
        body.find("glBindTexture(GL_TEXTURE_2D_ARRAY, shadowMap ? _shadowMapTex : _dummyDepthArray);", unit);
    REQUIRE(bind != std::string::npos);
    CHECK(body.substr(unit, bind - unit).find("glActiveTexture(", 10) == std::string::npos);
    const size_t sampler = body.find("glBindSampler(kShadowMapUnit, _shadowCompareSampler);", bind);
    REQUIRE(sampler != std::string::npos);
    const size_t draw = body.find("DrawTo(p.airMarchFbo, f.hw, f.hh);", sampler);
    REQUIRE(draw != std::string::npos);
    const size_t restore = body.find("glBindSampler(kShadowMapUnit, 0);", draw);
    REQUIRE(restore != std::string::npos);
    // Nothing draws between the march and the sampler's restore.
    CHECK(body.substr(draw + 1, restore - draw - 1).find("DrawTo(") == std::string::npos);
    CHECK(CountOf(body, "glBindSampler(") == 2);
    // The horizon unit keeps a complete array too.
    CHECK(body.find("glBindTexture(GL_TEXTURE_2D_ARRAY, terrain ? _terrainHorizonTex : _dummyHorizonTex);") !=
          std::string::npos);
    // The terrain maps are bound whenever the shading's litTerrain says they
    // are valid (terrain shadows or terrain occlusion on): airGround reads
    // the heights then, so the stand-in never flattens the valley mist.
    const size_t terrain = body.find("const bool terrain = ");
    REQUIRE(terrain != std::string::npos);
    const std::string terrainRule = body.substr(terrain, body.find(';', terrain) - terrain);
    CHECK(terrainRule.find("_terrainMap->Valid()") != std::string::npos);
    CHECK(terrainRule.find("(_lighting.terrainShadows > 0.0f || _lighting.terrainOcclusion > 0.0f)") !=
          std::string::npos);
    CHECK(body.find("BindTex(kTerrainHeightUnit, terrain ? _terrainHeightTex : _dummyHeightTex);") !=
          std::string::npos);
}
