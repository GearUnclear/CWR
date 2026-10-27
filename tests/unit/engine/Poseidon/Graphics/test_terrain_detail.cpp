#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <Poseidon/Graphics/Remaster/FeatureGates.hpp>
#include <Poseidon/Graphics/Remaster/FeatureSwitches.hpp>
#include <Poseidon/Graphics/Remaster/TerrainDetail.hpp>
#include <PoseidonGL33/EngineGL33_Lighting.hpp>

#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>
#include <string>

// Terrain detail (Graphics/Remaster/TerrainDetail.hpp): the C++ mirror of the
// terrain main's self-detail (the cell rotation, the locality of each cell's
// patch, the variance-preserving blend, the soft limiter), the detail-kept
// boost and the distance fade, the frame constants that switch it off, and the
// contracts between the mirror, the shader and the backend: the shader's
// constants are the mirror's, and only the land's draws get it.

using namespace Poseidon::remaster;
namespace td = Poseidon::remaster::terrain_detail;
namespace gl = Poseidon::gl33lighting;

namespace
{

constexpr int kPSDetail = 1; // PixelShaderID PSDetail (EngineGL33.hpp): the terrain main

std::string ReadTextFile(const std::filesystem::path& p)
{
    std::ifstream f(p);
    if (!f.is_open())
        return {};
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

std::filesystem::path Gl33Dir()
{
    return std::filesystem::path(TESTS_ROOT_DIR).parent_path() / "engine" / "PoseidonGL33";
}

LightingTuning Apply(const std::string& setting)
{
    LightingTuning t{};
    bool modeSet = false;
    const std::string why = ApplyLightingSetting(t, setting, modeSet);
    INFO(setting << ": " << why);
    REQUIRE(why.empty());
    return t;
}

// A small deterministic generator of unit-variance values (uniform in
// [-sqrt 3, sqrt 3]).
struct UnitNoise
{
    std::uint32_t state = 12345u;
    float operator()()
    {
        state = state * 1664525u + 1013904223u;
        const float u = static_cast<float>(state >> 8) / static_cast<float>(1u << 24);
        return (u * 2.0f - 1.0f) * 1.7320508f;
    }
};

} // namespace

TEST_CASE("Terrain detail: the soft limiter keeps small grain and never passes 0.4", "[remaster][TerrainDetail]")
{
    CHECK(td::SoftLimit(0.0f) == 0.0f);
    CHECK(td::SoftLimit(0.05f) == Catch::Approx(0.05f).epsilon(0.01));
    float last = -1.0f;
    for (float m = -100.0f; m <= 100.0f; m += 0.25f)
    {
        CAPTURE(m);
        const float v = td::SoftLimit(m);
        CHECK(std::fabs(v) < td::kLimit);
        CHECK(td::SoftLimit(-m) == Catch::Approx(-v)); // odd: no brightness bias
        CHECK(v >= last);                              // monotone (flat only where float runs out of precision)
        if (std::fabs(m) <= 4.0f)
            CHECK(v > last);
        last = v;
    }
    CHECK(td::kLimit == 0.4f);
}

TEST_CASE("Terrain detail: four cells blend with bilinear weights that keep the grain's variance",
          "[remaster][TerrainDetail]")
{
    // Across a cell, the four weights stay a bilinear partition of one.
    for (float px = 0.0f; px < 8.0f; px += 0.37f)
        for (float py = 0.0f; py < 8.0f; py += 0.41f)
        {
            CAPTURE(px, py);
            const td::CellBlend b = td::CellsAround(px, py);
            float sum = 0.0f;
            for (float w : b.w)
            {
                CHECK(w >= 0.0f);
                CHECK(w <= 1.0f);
                sum += w;
            }
            CHECK(sum == Catch::Approx(1.0f));
            // The fragment lies between the four cell centres.
            CHECK((b.i0 + 0.5f) * td::kCellTexels <= px + 1e-4f);
            CHECK((b.i0 + 1.5f) * td::kCellTexels >= px - 1e-4f);
        }

    // Independent unit-variance cells: the blend keeps variance 1 anywhere in
    // the cell, where a plain weighted mean drops to 1/4 at the cell corner
    // shared by four (the grain would wash out between cell centres).
    for (const std::array<float, 4>& w :
         {std::array<float, 4>{1.0f, 0.0f, 0.0f, 0.0f}, std::array<float, 4>{0.25f, 0.25f, 0.25f, 0.25f},
          std::array<float, 4>{0.42f, 0.18f, 0.28f, 0.12f}})
    {
        UnitNoise noise;
        double sumBlend = 0.0, sumBlend2 = 0.0, sumMean2 = 0.0;
        constexpr int kSamples = 40000;
        for (int i = 0; i < kSamples; ++i)
        {
            const std::array<float, 4> h = {noise(), noise(), noise(), noise()};
            const float blend = td::VariancePreservingBlend(w, h);
            const float mean = w[0] * h[0] + w[1] * h[1] + w[2] * h[2] + w[3] * h[3];
            sumBlend += blend;
            sumBlend2 += blend * blend;
            sumMean2 += mean * mean;
        }
        const double var = sumBlend2 / kSamples - (sumBlend / kSamples) * (sumBlend / kSamples);
        CAPTURE(w[0], w[1], w[2], w[3], var);
        CHECK(var == Catch::Approx(1.0).epsilon(0.05));
        if (w[0] == 0.25f)
            CHECK(sumMean2 / kSamples == Catch::Approx(0.25).epsilon(0.05));
    }
}

TEST_CASE("Terrain detail: every cell turns its patch by its own angle, from a well spread hash",
          "[remaster][TerrainDetail]")
{
    // Deterministic, in range, and spread evenly over the turn: the unrotated
    // cells of an earlier version printed parallel "mowing stripes" on the Eden meadow.
    constexpr int kBins = 8;
    int bins[kBins] = {};
    int sameAsLeft = 0;
    constexpr int kN = 64;
    for (int j = 0; j < kN; ++j)
        for (int i = 0; i < kN; ++i)
        {
            const float a = td::CellAngle(static_cast<float>(i), static_cast<float>(j));
            CHECK(a == td::CellAngle(static_cast<float>(i), static_cast<float>(j)));
            REQUIRE(a >= 0.0f);
            REQUIRE(a < 6.2831853f);
            ++bins[static_cast<int>(a / 6.2831853f * kBins)];
            if (i > 0 && std::fabs(a - td::CellAngle(static_cast<float>(i - 1), static_cast<float>(j))) < 1e-3f)
                ++sameAsLeft;
            const std::array<float, 2> jit = td::CellJitter(static_cast<float>(i), static_cast<float>(j));
            CHECK(std::fabs(jit[0]) <= td::kJitterTexels * 0.5f);
            CHECK(std::fabs(jit[1]) <= td::kJitterTexels * 0.5f);
        }
    const float expected = static_cast<float>(kN * kN) / kBins;
    for (int b = 0; b < kBins; ++b)
    {
        CAPTURE(b, bins[b]);
        CHECK(bins[b] > expected * 0.75f);
        CHECK(bins[b] < expected * 1.25f);
    }
    CHECK(sameAsLeft < 4);
}

TEST_CASE("Terrain detail: a cell reads its own neighbourhood, six times finer", "[remaster][TerrainDetail]")
{
    // While the fragment is inside a cell's own area, that cell reads the
    // texture within half a patch (plus its jitter) of its centre: grass from
    // grass, rock from rock, a few metres at most (39 cm per base texel).
    const float reach = (td::kPatchTexels * 0.5f + td::kJitterTexels * 0.5f) * std::sqrt(2.0f);
    CHECK(reach * 0.39f < 5.0f);
    for (int cell = 0; cell < 50; ++cell)
    {
        const float ci = static_cast<float>(cell * 7 % 23);
        const float cj = static_cast<float>(cell * 5 % 19);
        const float cx = (ci + 0.5f) * td::kCellTexels;
        const float cy = (cj + 0.5f) * td::kCellTexels;
        for (float dx : {-0.5f, -0.2f, 0.0f, 0.3f, 0.5f})
            for (float dy : {-0.5f, 0.1f, 0.5f})
            {
                const float px = cx + dx * td::kCellTexels;
                const float py = cy + dy * td::kCellTexels;
                const std::array<float, 2> q = td::CellSample(px, py, ci, cj);
                CAPTURE(ci, cj, dx, dy, q[0], q[1]);
                CHECK(std::hypot(q[0] - cx, q[1] - cy) <= reach + 1e-3f);
                // One base texel of fragment motion moves the sample kZoom
                // texels: the patch is the texture zoomed, turned, not stretched.
                const std::array<float, 2> qx = td::CellSample(px + 0.25f, py, ci, cj);
                const std::array<float, 2> qy = td::CellSample(px, py + 0.25f, ci, cj);
                CHECK(std::hypot(qx[0] - q[0], qx[1] - q[1]) == Catch::Approx(0.25f * td::kZoom).epsilon(1e-3));
                CHECK(std::hypot(qy[0] - q[0], qy[1] - q[1]) == Catch::Approx(0.25f * td::kZoom).epsilon(1e-3));
            }
    }
    CHECK(td::kZoom == 6.0f);
    CHECK(td::kCellTexels == 2.0f);
}

TEST_CASE("Terrain detail: everything fades out by 300 m, where the frame gate ends", "[remaster][TerrainDetail]")
{
    CHECK(td::kMaxDistance == 300.0f);
    CHECK(td::DistanceFade(0.0f) == 1.0f);
    CHECK(td::DistanceFade(td::kFadeStart) == 1.0f);
    CHECK(td::DistanceFade(td::kMaxDistance) == 0.0f);
    float last = 1.0f;
    for (float d = td::kFadeStart; d <= td::kMaxDistance; d += 5.0f)
    {
        const float f = td::DistanceFade(d);
        CHECK(f <= last);
        last = f;
    }
    // The gate's reason names the same range.
    const LightingTuning t = Apply("remastered");
    const std::string s = FeatureStateString(t, ComputeFeatureGates(GateInputs{}, t));
    CHECK(s.find("terrainDetail=1:on:per-terrain-fragment-within-300-m") != std::string::npos);

    // Detail kept: no boost where the detail texture is magnified, 2^lod
    // beyond, never above 2x (an earlier 4x painted dark blotches on
    // paths), and back to 1 by the end of the range, so nothing steps there.
    CHECK(td::KeptBoost(-2.0f, 10.0f) == 1.0f);
    CHECK(td::KeptBoost(0.0f, 10.0f) == 1.0f);
    CHECK(td::KeptBoost(0.5f, 10.0f) == Catch::Approx(std::sqrt(2.0f)));
    for (float lod = 0.0f; lod < 10.0f; lod += 0.5f)
    {
        CAPTURE(lod);
        CHECK(td::KeptBoost(lod, 10.0f) <= 2.0f);
        CHECK(td::KeptBoost(lod, td::kMaxDistance) == 1.0f);
        CHECK(td::KeptBoost(lod, td::kMaxDistance - 1.0f) < 1.01f);
    }
    CHECK(td::KeptBoost(4.0f, 10.0f) == 2.0f);
    CHECK(td::kKeptCap == 2.0f);
}

TEST_CASE("Terrain detail: the grain fades in past the near ground, which keeps its authored pebbles",
          "[remaster][TerrainDetail]")
{
    // The GPU's mip level with 16x anisotropy: log2 of the footprint's major
    // axis over min(16, major / minor).
    CHECK(td::FootprintLod(4.0f, 4.0f) == Catch::Approx(2.0f));
    CHECK(td::FootprintLod(0.5f, 0.5f) == Catch::Approx(-1.0f));
    CHECK(td::FootprintLod(1.0f, 32.0f) == Catch::Approx(1.0f));
    CHECK(td::FootprintLod(2.0f, 8.0f) == Catch::Approx(1.0f));

    // A standing eye (1.75 m) over flat ground at 1080p with the capture
    // camera (1920 px across at fov 0.7: focal length 960 / 0.7 px), base
    // texels of 50 m / 128: the zoomed patch's blobs are too coarse near the
    // eye, so the self-detail is off at 5 m and full from 20 m.
    constexpr float kFocal = 960.0f / 0.7f;
    constexpr float kTexel = 50.0f / 128.0f;
    constexpr float kEye = 1.75f;
    auto flatGround = [&](float d)
    {
        const float across = d / (kFocal * kTexel);
        const float along = across * (d * d + kEye * kEye) / (kEye * d);
        return td::NearFade(td::FootprintLod(across, along));
    };
    CHECK(flatGround(3.0f) == 0.0f);
    CHECK(flatGround(5.0f) == 0.0f);
    CHECK(flatGround(20.0f) == 1.0f);
    CHECK(flatGround(100.0f) == 1.0f);
    float last = 0.0f;
    for (float d = 2.0f; d < 40.0f; d += 0.5f)
    {
        const float w = flatGround(d);
        CHECK(w >= last);
        last = w;
    }
    // A slope facing the eye 30 m out (a test view's hillside): full.
    CHECK(td::NearFade(td::FootprintLod(30.0f / (kFocal * kTexel), 30.0f / (kFocal * kTexel))) == 1.0f);
    CHECK(td::NearFade(td::kNearToLod - std::log2(td::kZoom)) == 1.0f);
    CHECK(td::NearFade(td::kNearFromLod - std::log2(td::kZoom)) == 0.0f);

    // The second detail sample only where the detail texture is minified: the
    // magnified detail near the eye stays exactly as authored.
    CHECK(td::AntiTileWeight(-2.0f, 5.0f) == 0.0f);
    CHECK(td::AntiTileWeight(0.0f, 5.0f) == 0.0f);
    CHECK(td::AntiTileWeight(0.5f, 5.0f) == Catch::Approx(0.5f * td::kAntiTile));
    CHECK(td::AntiTileWeight(3.0f, 5.0f) == td::kAntiTile);
    CHECK(td::AntiTileWeight(3.0f, td::kMaxDistance) == 0.0f);
}

TEST_CASE("Terrain detail: its frame constants are zero when the switch or its gate is off",
          "[remaster][TerrainDetail][FeatureGates]")
{
    const LightingTuning on = Apply("remastered");
    CHECK(td::FrameConstants(on, true) == std::array<float, 3>{1.5f, td::kKeptRate, td::kAntiTile});
    CHECK(td::FrameConstants(on, false) == std::array<float, 3>{0.0f, 0.0f, 0.0f});
    CHECK(td::FrameConstants(Apply("remastered,terrainDetail=0"), true) == std::array<float, 3>{0.0f, 0.0f, 0.0f});
    CHECK(td::FrameConstants(Apply("baseline"), true) == std::array<float, 3>{0.0f, 0.0f, 0.0f});
    CHECK(td::FrameConstants(Apply("remastered,detailStrength=0.7"), true)[0] == 0.7f);
    CHECK(td::FrameConstants(Apply("remastered,detailStrength=-1"), true)[0] == 0.0f);

    // The gate: on by day and night in the remastered world, off in classic
    // and under night vision (the world then draws through the classic programs).
    GateInputs in;
    CHECK(ComputeFeatureGates(in, on)[Feature::TerrainDetail].on);
    CHECK_FALSE(ComputeFeatureGates(in, Apply("classic"))[Feature::TerrainDetail].on);
    CHECK_FALSE(ComputeFeatureGates(in, Apply("remastered,terrainDetail=0"))[Feature::TerrainDetail].on);
    in.nightVision = true;
    CHECK_FALSE(ComputeFeatureGates(in, on)[Feature::TerrainDetail].on);
}

TEST_CASE("Terrain detail: the terrain main takes the mirror's constants and runs on the land only",
          "[Graphics][Shaders][remaster][TerrainDetail]")
{
    const std::string main = gl::RemasterFragmentMain(kPSDetail);
    REQUIRE(main.find("tdSelfDetail(") != std::string::npos);

    // Every shape constant of the shader is the mirror's.
    struct Constant
    {
        const char* glsl;
        float value;
    };
    const Constant constants[] = {
        {"kTdZoom", td::kZoom},
        {"kTdCellTexels", td::kCellTexels},
        {"kTdJitterTexels", td::kJitterTexels},
        {"kTdLimit", td::kLimit},
        {"kTdMinLocalMean", td::kMinLocalMean},
        {"kTdFadeStart", td::kFadeStart},
        {"kTdMaxDistance", td::kMaxDistance},
        {"kTdKeptCap", td::kKeptCap},
        {"kTdKeptFadeStart", td::kKeptFadeStart},
        {"kTdNearFromLod", td::kNearFromLod},
        {"kTdNearToLod", td::kNearToLod},
        {"kTdMaxAniso", td::kMaxAniso},
    };
    for (const Constant& c : constants)
    {
        CAPTURE(c.glsl);
        std::smatch m;
        const std::regex re(std::string("const float ") + c.glsl + R"( = (-?[0-9.]+);)");
        REQUIRE(std::regex_search(main, m, re));
        CHECK(std::stof(m[1].str()) == c.value);
    }

    // Off (litDetail.xyz zero) or the sea (litDetail.w zero): the stock terrain.
    CHECK(main.find("if (litDetail.w > 0.5 && (litDetail.x > 0.0 || litDetail.y > 0.0 || litDetail.z > 0.0))") !=
          std::string::npos);
    CHECK(main.find("float detail = t1.a * 2.0;") != std::string::npos);
    CHECK(main.find("vec3 albedo = t0.rgb * constColor.rgb * (detail * selfMod);") != std::string::npos);
    // Nothing beyond the gate's range; derivatives taken before that branch.
    const size_t branch = main.find("if (dist < kTdMaxDistance)");
    REQUIRE(branch != std::string::npos);
    CHECK(main.find("dFdx(vUV0)") < branch);
    CHECK(main.find("dFdx(vUV1)") < branch);
    CHECK(main.find("texture(", branch) == std::string::npos); // only explicit-gradient or -LOD reads inside
    // The other mains do not read it.
    for (int ps : {0, 2, 3})
    {
        CAPTURE(ps);
        const char* other = gl::RemasterFragmentMain(ps);
        REQUIRE(other != nullptr);
        CHECK(std::string(other).find("litDetail") == std::string::npos);
    }

    // The backend: the frame constants from the mirror and the gate, and the
    // land flag from the draw's multitexture stage for every PSDetail draw.
    const std::string lighting = ReadTextFile(Gl33Dir() / "EngineGL33_Lighting.cpp");
    const std::string queue = ReadTextFile(Gl33Dir() / "EngineGL33_Queue.cpp");
    const std::string state = ReadTextFile(Gl33Dir() / "EngineGL33_State.cpp");
    REQUIRE_FALSE(lighting.empty());
    CHECK(
        lighting.find("rm::terrain_detail::FrameConstants(_lighting, _featureGates[rm::Feature::TerrainDetail].on)") !=
        std::string::npos);
    CHECK(queue.find(
              "if (d.shader == Poseidon::render::ShaderFamily::Detail)\n        SelectTerrainDetailLayer(spec);") !=
          std::string::npos);
    // (IsLandLayer is the one reading of the stage; the reflections' tags ask it too.)
    CHECK(state.find("SetLitTerrainDraw(IsLandLayer(spec));") != std::string::npos);
    CHECK(state.find("return Poseidon::render::Has(spec.backend, Poseidon::render::Backend::DetailTexture);") !=
          std::string::npos);
}
