#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <Poseidon/Graphics/Remaster/RainXp.hpp>

#include <glslang/Public/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>

// Rain you can see (POSEIDON_XP_RAIN, perf-squad lane EX4-RAIN): the switch's
// values, the world-space drops' wrap, which keeps every drop at its own
// place in the world while the camera moves and the box follows it, the
// drops' travel summed frame by frame (round 2: the rain keeps falling when
// the density or the wind changes and when the camera dives), and the smoke
// grid.

using namespace Poseidon::remaster;

TEST_CASE("RainXp: the switch is off unless it names a mode", "[Graphics][RainXp]")
{
    CHECK(RainXpModeFromString(nullptr) == RainXpMode::Off);
    CHECK(RainXpModeFromString("") == RainXpMode::Off);
    CHECK(RainXpModeFromString("0") == RainXpMode::Off);
    CHECK(RainXpModeFromString("off") == RainXpMode::Off);
    CHECK(RainXpModeFromString("banana") == RainXpMode::Off);
    CHECK(RainXpModeFromString("1") == RainXpMode::Late);
    CHECK(RainXpModeFromString("late") == RainXpMode::Late);
    CHECK(RainXpModeFromString("2") == RainXpMode::World);
    CHECK(RainXpModeFromString("world") == RainXpMode::World);
}

TEST_CASE("RainXp: unset, the shipped mode is World and 0 gives today's overlay", "[Graphics][RainXp]")
{
    CHECK(RainXpModeFromEnvValue(nullptr) == RainXpMode::World);
    CHECK(RainXpModeFromEnvValue("") == RainXpMode::World);
    CHECK(RainXpModeFromEnvValue("0") == RainXpMode::Off);
    CHECK(RainXpModeFromEnvValue("late") == RainXpMode::Late);
    CHECK(RainXpModeFromEnvValue("world") == RainXpMode::World);
}

namespace
{
// A drop's camera-relative position as the shader computes it.
void DropRel(const float base[3], const float boxMin[3], const float size[3], const float shift[3], float out[3])
{
    for (int i = 0; i < 3; ++i)
        out[i] = boxMin[i] + std::fmod(base[i] + shift[i], size[i]);
}
} // namespace

TEST_CASE("RainXp: the wrap shift lies in the box and keeps a drop still in the world", "[Graphics][RainXp]")
{
    const float size[3] = {44.0f, 26.0f, 44.0f};
    const float boxMin[3] = {-22.0f, -10.0f, -8.0f};
    const double travel[3] = {1234.5, -3000.25, 77.0};
    // A large world position: the shift stays inside [0, size).
    const double cam[3] = {12345.678, 210.5, 9876.125};
    float shift[3];
    RainWrapShift(cam, travel, boxMin, size, shift);
    for (int i = 0; i < 3; ++i)
    {
        CHECK(shift[i] >= 0.0f);
        CHECK(shift[i] < size[i]);
    }
    // The same travel: a drop seen from a camera moved by 1.5 m east sits
    // 1.5 m further west of it (as long as it does not cross the box's face).
    const float base[3] = {20.0f, 13.0f, 20.0f};
    float a[3], b[3], shift2[3];
    const double cam2[3] = {cam[0] + 1.5, cam[1], cam[2]};
    RainWrapShift(cam2, travel, boxMin, size, shift2);
    DropRel(base, boxMin, size, shift, a);
    DropRel(base, boxMin, size, shift2, b);
    const float dx = std::fmod(a[0] - b[0] + 2.0f * size[0], size[0]);
    CHECK(dx == Catch::Approx(1.5f).margin(1e-3));
    CHECK(std::fmod(b[1] - a[1] + 2.0f * size[1], size[1]) == Catch::Approx(0.0f).margin(1e-3));
    CHECK(std::fmod(b[2] - a[2] + 2.0f * size[2], size[2]) == Catch::Approx(0.0f).margin(1e-3));
    // Travel folded by kRainTravelFold is the same place.
    // (the backend's box, whose edges divide the fold).
    const double folded[3] = {travel[0] + kRainTravelFold, travel[1] - kRainTravelFold, travel[2]};
    float shiftA[3], shiftB[3];
    RainWrapShift(cam, travel, boxMin, kRainBoxSize, shiftA);
    RainWrapShift(cam, folded, boxMin, kRainBoxSize, shiftB);
    for (int i = 0; i < 3; ++i)
    {
        const float d = std::fmod(shiftB[i] - shiftA[i] + 2.0f * kRainBoxSize[i], kRainBoxSize[i]);
        CHECK(std::min(d, kRainBoxSize[i] - d) == Catch::Approx(0.0f).margin(2e-3));
    }
    for (int i = 0; i < 3; ++i)
        CHECK(std::fmod(kRainTravelFold, static_cast<double>(kRainBoxSize[i])) == 0.0);
}

TEST_CASE("RainXp: drops fall at the fall speed", "[Graphics][RainXp]")
{
    CHECK(RainFallSpeed(0.0f) == Catch::Approx(kRainFallSlow));
    CHECK(RainFallSpeed(2.0f) == Catch::Approx(kRainFallFast));
    // A still camera, steady rain, no wind: every layer travels down at its
    // share of the fall speed.
    RainXpDrift drift;
    RainXpFrame out;
    const float calm[3] = {0.0f, 0.0f, 0.0f};
    const double cam[3] = {5000.0, 100.0, 5000.0};
    RainDriftStep(drift, 1000.0, 1.0f, calm, cam, out);
    double y0[kRainLayers];
    for (int l = 0; l < kRainLayers; ++l)
        y0[l] = out.travel[l][1];
    RainDriftStep(drift, 1000.1, 1.0f, calm, cam, out);
    for (int l = 0; l < kRainLayers; ++l)
    {
        CHECK(y0[l] - out.travel[l][1] == Catch::Approx(0.1 * kRainFallFast * kRainLayerFall[l]).margin(1e-4));
        CHECK(out.relVel[l][1] == Catch::Approx(-kRainFallFast * kRainLayerFall[l]).margin(1e-4));
        CHECK(out.travel[l][0] == Catch::Approx(0.0).margin(1e-9));
    }
}

TEST_CASE("RainXp: the rain keeps falling while the density and the wind change (round 1 rose)", "[Graphics][RainXp]")
{
    // As in the owner's clips: rain with the density wandering down by 0.01 a
    // second (Weather::MoveClouds), 60 fps, here with the mission clock at 400 s.
    RainXpDrift drift;
    RainXpFrame out;
    const double cam[3] = {5000.0, 100.0, 5000.0};
    float wind[3] = {3.6f, 0.0f, 1.8f};
    const double step = 1.0 / 60.0;
    double prev[kRainLayers][3] = {};
    bool round1Rose = false;
    double maxSide = 0.0;
    for (int k = 0; k < 600; ++k)
    {
        const double t = 400.0 + k * step;
        const float density = 1.0f - 0.01f * static_cast<float>(k * step);
        // The wind steps by a metre a second half way and a gust comes and goes.
        if (k == 300)
            wind[0] += 1.0f;
        const float gust = (k >= 120 && k < 240) ? 3.0f : 0.0f;
        const float w[3] = {wind[0] + gust, 0.0f, wind[2]};
        RainDriftStep(drift, t, density, w, cam, out);
        if (k > 0)
        {
            for (int l = 0; l < kRainLayers; ++l)
            {
                // Down every frame, at no less than the slowest layer of the lightest rain ...
                const double fell = prev[l][1] - out.travel[l][1];
                CHECK(fell > 0.8 * kRainFallSlow * step);
                CHECK(fell < 1.2 * kRainFallFast * step);
                // ... and never thrown aside: no faster than the wind.
                const double side = std::hypot(out.travel[l][0] - prev[l][0], out.travel[l][2] - prev[l][2]) / step;
                maxSide = std::max(maxSide, side);
            }
            // Round 1's placement (velocity times the clock) at the same moment.
            if (drift.r1Speed[1] > 0.0f)
                round1Rose = true;
        }
        for (int l = 0; l < kRainLayers; ++l)
            for (int i = 0; i < 3; ++i)
                prev[l][i] = out.travel[l][i];
    }
    CHECK(maxSide < 9.0);
    // Round 1: the fall speed dropped by 0.025 m/s a second; times a clock of
    // 400 s and more that is 10 m/s and more of rise against 8.5 m/s and less of fall.
    CHECK(round1Rose);
    CHECK(drift.r1Speed[1] > 1.0f);
}

TEST_CASE("RainXp: the rain keeps falling on screen when the camera dives or races", "[Graphics][RainXp]")
{
    RainXpDrift drift;
    RainXpFrame out;
    const float calm[3] = {0.0f, 0.0f, 0.0f};
    double cam[3] = {5000.0, 300.0, 5000.0};
    const double step = 1.0 / 60.0;
    // Diving at 12 m/s (faster than the rain falls) and racing at 40 m/s.
    for (int k = 0; k < 240; ++k)
    {
        RainDriftStep(drift, 50.0 + k * step, 1.0f, calm, cam, out);
        cam[1] -= 12.0 * step;
        cam[0] += 40.0 * step;
    }
    for (int l = 0; l < kRainLayers; ++l)
    {
        const float fall = kRainFallFast * kRainLayerFall[l];
        CHECK(out.relVel[l][1] <= -kRainKeepFall * fall + 1e-3f);
        CHECK(std::hypot(out.relVel[l][0], out.relVel[l][2]) <= kRainMaxSlant * fall + 1e-3f);
    }
    // A slow camera is left alone: the drops stay put in the world.
    RainXpDrift walk;
    double cam2[3] = {5000.0, 100.0, 5000.0};
    for (int k = 0; k < 240; ++k)
    {
        RainDriftStep(walk, 50.0 + k * step, 1.0f, calm, cam2, out);
        cam2[0] += 2.0 * step;
        cam2[1] -= 1.0 * step;
    }
    for (int l = 0; l < kRainLayers; ++l)
    {
        CHECK(out.relVel[l][0] == Catch::Approx(-2.0f).margin(0.02));
        CHECK(out.relVel[l][1] == Catch::Approx(-kRainFallFast * kRainLayerFall[l] + 1.0f).margin(0.02));
    }
    // A cut (the camera jumps) and a clock set back move nothing.
    double before[3];
    for (int i = 0; i < 3; ++i)
        before[i] = out.travel[0][i];
    cam2[0] += 5000.0;
    RainDriftStep(walk, 10.0, 1.0f, calm, cam2, out);
    for (int i = 0; i < 3; ++i)
        CHECK(out.travel[0][i] == before[i]);
}

TEST_CASE("RainXp: a smoke puff covers the cells under it at its depth", "[Graphics][RainXp]")
{
    auto grid = std::make_unique<RainCoverGrid>();
    grid->Clear();
    float cover = 0.0f, depth = 0.0f;
    grid->Cell(0, cover, depth);
    CHECK(cover == 0.0f);
    // A puff over the middle of a 1920 x 1080 screen, 200 px half size, 9 m off.
    grid->Add(960.0f, 540.0f, 200.0f, 200.0f, 1920.0f, 1080.0f, 0.8f, 9.0f);
    CHECK(grid->puffs == 1);
    const int mid = (kRainCoverH / 2) * kRainCoverW + kRainCoverW / 2;
    grid->Cell(mid, cover, depth);
    CHECK(cover > 0.4f);
    CHECK(cover < 0.8f);
    CHECK(depth == Catch::Approx(9.0f));
    // A second, nearer puff: more cover, a depth between the two.
    grid->Add(960.0f, 540.0f, 200.0f, 200.0f, 1920.0f, 1080.0f, 0.8f, 5.0f);
    float cover2 = 0.0f;
    grid->Cell(mid, cover2, depth);
    CHECK(cover2 > cover);
    CHECK(cover2 < 1.0f);
    CHECK(depth > 5.0f);
    CHECK(depth < 9.0f);
    // Nothing far from it, nothing from a puff off screen or with no opacity.
    grid->Cell(0, cover, depth);
    CHECK(cover == 0.0f);
    grid->Add(-900.0f, 540.0f, 100.0f, 100.0f, 1920.0f, 1080.0f, 0.8f, 5.0f);
    grid->Add(960.0f, 540.0f, 100.0f, 100.0f, 1920.0f, 1080.0f, 0.0f, 5.0f);
    CHECK(grid->puffs == 2);
}

// The world-space rain's shaders (EngineGL33_XpRain.cpp) compile under glslang
// as GLSL 330, as I-28 checks the shipped shaders (test_shader_named_outputs.cpp).
namespace
{
std::string RainXpShaderBlock(const std::string& src, const std::string& symbol)
{
    const std::string head = "const char " + symbol + "[] = R\"(";
    const size_t at = src.find(head);
    if (at == std::string::npos)
        return {};
    const size_t start = at + head.size();
    const size_t close = src.find(")\"", start);
    return close == std::string::npos ? std::string() : src.substr(start, close - start);
}

bool RainXpCompiles(const std::string& src, EShLanguage stage, std::string& log)
{
    glslang::TShader shader(stage);
    const char* strings[1] = {src.c_str()};
    shader.setStrings(strings, 1);
    // Plain GLSL 330 as the driver takes it (no SPIR-V client: its rules ask
    // a location of every loose uniform, which the GL path does not need).
    shader.setEnvInput(glslang::EShSourceGlsl, stage, glslang::EShClientNone, 0);
    const bool ok = shader.parse(GetDefaultResources(), 330, false, EShMsgDefault);
    log = shader.getInfoLog();
    return ok;
}
} // namespace

TEST_CASE("RainXp: the world-space rain's shaders compile under glslang", "[Graphics][RainXp][Shaders]")
{
    std::ifstream f(std::filesystem::path(TESTS_ROOT_DIR).parent_path() / "engine" / "PoseidonGL33" /
                    "EngineGL33_XpRain.cpp");
    REQUIRE(f.is_open());
    std::stringstream ss;
    ss << f.rdbuf();
    const std::string body = ss.str();
    const std::string vs = RainXpShaderBlock(body, "kXpRainVS");
    const std::string fs = RainXpShaderBlock(body, "kXpRainFS");
    REQUIRE_FALSE(vs.empty());
    REQUIRE_FALSE(fs.empty());
    glslang::InitializeProcess();
    std::string log;
    const bool okV = RainXpCompiles(vs, EShLangVertex, log);
    CAPTURE(log);
    CHECK(okV);
    const bool okF = RainXpCompiles(fs, EShLangFragment, log);
    CAPTURE(log);
    CHECK(okF);
    glslang::FinalizeProcess();
}
