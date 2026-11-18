#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>
#include <Poseidon/Graphics/Remaster/ShoreRibbons.hpp>
#include <PoseidonGL33/EngineGL33_Lighting.hpp>

#include <cmath>
#include <string>
#include <vector>

// The shore ribbons (SURF-E): the coastline traced from the height map
// (Graphics/Remaster/ShoreRibbons.hpp) and the program that draws them
// (EngineGL33_ShoreRibbons.cpp).

using namespace Poseidon::remaster;
namespace gl = Poseidon::gl33lighting;

namespace
{

constexpr int kN = 64;
constexpr float kCell = 50.0f;

std::vector<float> Heights(float (*f)(float, float))
{
    std::vector<float> h(static_cast<size_t>(kN) * kN);
    for (int z = 0; z < kN; ++z)
        for (int x = 0; x < kN; ++x)
            h[static_cast<size_t>(z) * kN + x] = f(static_cast<float>(x) * kCell, static_cast<float>(z) * kCell);
    return h;
}

struct Pt
{
    float x, z, angle, slope;
    bool last;
};

std::vector<Pt> LevelPoints(const ShoreRibbonSet& s, int level)
{
    std::vector<Pt> out;
    const int tiles = s.tilesPerSide * s.tilesPerSide;
    const std::uint32_t first = s.TileFirst(level, 0, 0);
    const std::uint32_t end = s.tileStart[static_cast<size_t>(level) * (tiles + 1) + tiles];
    for (std::uint32_t i = first; i < end; ++i)
    {
        const float* t = &s.texels[static_cast<size_t>(i) * 4];
        out.push_back(Pt{t[0], t[1], t[2], std::fabs(t[3]), t[3] < 0.0f});
    }
    return out;
}

bool Compiles(const std::string& src, EShLanguage stage, std::string& log)
{
    glslang::TShader shader(stage);
    const char* strings[1] = {src.c_str()};
    shader.setStrings(strings, 1);
    const bool ok = shader.parse(GetDefaultResources(), 330, false, EShMsgDefault);
    log = shader.getInfoLog();
    return ok;
}

} // namespace

TEST_CASE("Shore ribbons: a straight beach is one line with its sea normal and slope", "[Graphics][remaster][shore]")
{
    // The sea west of x = 1000 m, a 1:25 beach east of it.
    const std::vector<float> h = Heights([](float x, float) { return 0.04f * (x - 1000.0f); });
    const ShoreRibbonSet s = BuildShoreRibbons(h, kN, kCell);
    REQUIRE(s.levels == kShoreRibbonLevels);
    REQUIRE(s.tilesPerSide == static_cast<int>(std::ceil((kN - 1) * kCell / kShoreRibbonTile)));
    for (int level : {0, 3})
    {
        CAPTURE(level);
        const float lineX = 1000.0f + (kShoreRibbonLevelBase + kShoreRibbonLevelStep * level) / 0.04f;
        const std::vector<Pt> pts = LevelPoints(s, level);
        REQUIRE(pts.size() > 900); // 3150 m at 3 m
        int last = 0;
        for (const Pt& p : pts)
        {
            CHECK(p.x == Catch::Approx(lineX).margin(0.01));
            // The sea normal points west (the walk goes south, the land on its left).
            CHECK(std::cos(p.angle) == Catch::Approx(-1.0).margin(1e-3));
            CHECK(p.slope == Catch::Approx(0.04).margin(1e-3));
            last += p.last ? 1 : 0;
        }
        // One piece per tile the line crosses, each ended by a last point.
        CHECK(last == s.tilesPerSide);
    }
    CHECK(s.LevelFor(-3.0f) == 0);
    CHECK(s.LevelFor(2.3f) == 5);
    CHECK(s.LevelFor(9.0f) == kShoreRibbonLevels - 1);
}

TEST_CASE("Shore ribbons: an island is a closed loop, normals outward, points in their tiles",
          "[Graphics][remaster][shore]")
{
    // A cone 20 m high at (1600, 1600), the waterline at r = 400 m at sea level 0.
    const std::vector<float> h =
        Heights([](float x, float z) { return 20.0f - 0.05f * std::hypot(x - 1600.0f, z - 1600.0f); });
    const ShoreRibbonSet s = BuildShoreRibbons(h, kN, kCell);
    const std::vector<Pt> pts = LevelPoints(s, 0);
    REQUIRE_FALSE(pts.empty());
    CHECK(s.contours >= kShoreRibbonLevels);
    double len = 0.0;
    int lastPts = 0;
    for (size_t i = 0; i < pts.size(); ++i)
    {
        const Pt& p = pts[i];
        const float r = std::hypot(p.x - 1600.0f, p.z - 1600.0f);
        CHECK(r == Catch::Approx(400.0).margin(4.0)); // the triangles' chords lie inside the circle
        const float ox = (p.x - 1600.0f) / r, oz = (p.z - 1600.0f) / r;
        CHECK(std::cos(p.angle) * ox + std::sin(p.angle) * oz > 0.98f);
        if (p.last)
            ++lastPts;
        else
        {
            // Every point that starts a segment sits in the tile it was sorted to:
            // the segment's next point is the next in the list.
            REQUIRE(i + 1 < pts.size());
            len += std::hypot(pts[i + 1].x - p.x, pts[i + 1].z - p.z);
            CHECK(std::hypot(pts[i + 1].x - p.x, pts[i + 1].z - p.z) < kShoreRibbonSpacing * 1.5f);
        }
    }
    CHECK(lastPts > 4);
    CHECK(len == Catch::Approx(2.0 * 3.14159265 * 400.0).epsilon(0.02));
    for (int tz = 0; tz < s.tilesPerSide; ++tz)
        for (int tx = 0; tx < s.tilesPerSide; ++tx)
        {
            const std::uint32_t a = s.TileFirst(0, tx, tz);
            const std::uint32_t b = tx + 1 < s.tilesPerSide ? s.TileFirst(0, tx + 1, tz) : s.TileFirst(0, 0, tz + 1);
            for (std::uint32_t i = a; i < b; ++i)
            {
                const float* t = &s.texels[static_cast<size_t>(i) * 4];
                if (t[3] < 0.0f)
                    continue; // a piece's last point may sit over the tile's edge
                CHECK(static_cast<int>(t[0] / s.tile) == tx);
                CHECK(static_cast<int>(t[1] / s.tile) == tz);
            }
        }
}

TEST_CASE("Shore ribbons: nothing to trace on a map without a shore", "[Graphics][remaster][shore]")
{
    const std::vector<float> h = Heights([](float, float) { return 40.0f; });
    const ShoreRibbonSet s = BuildShoreRibbons(h, kN, kCell);
    CHECK(s.Empty());
    CHECK(s.contours == 0);
    CHECK(BuildShoreRibbons({}, 0, kCell).Empty());
}

TEST_CASE("Shore ribbons: the program compiles under glslang", "[Graphics][Shaders][remaster][shore]")
{
    glslang::InitializeProcess();
    std::string log;
    const bool vs = Compiles(gl::ShoreRibbonVertexSource(), EShLangVertex, log);
    CAPTURE(log);
    CHECK(vs);
    const bool fs = Compiles(gl::ShoreRibbonFragmentSource(), EShLangFragment, log);
    CAPTURE(log);
    CHECK(fs);
    // The vertex shader feeds the prelude's varyings and its own two.
    const std::string vsrc = gl::ShoreRibbonVertexSource();
    for (const char* v : {"out vec3 vWorldRel;", "out float vFogTC;", "flat out float vLitMode;", "out vec4 vCrown;",
                          "out vec4 vRibbon;", "out vec2 vFoamUV;"})
    {
        CAPTURE(v);
        CHECK(vsrc.find(v) != std::string::npos);
    }
    glslang::FinalizeProcess();
}
