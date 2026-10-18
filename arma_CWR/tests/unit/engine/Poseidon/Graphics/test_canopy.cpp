#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <Poseidon/Graphics/Core/GpuTimings.hpp>
#include <Poseidon/Graphics/Remaster/Canopy.hpp>
#include <Poseidon/Graphics/Remaster/FeatureGates.hpp>
#include <Poseidon/Graphics/Remaster/FeatureSwitches.hpp>
#include <Poseidon/World/MapTypes.hpp>
#include <PoseidonGL33/EngineGL33_Lighting.hpp>

#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

// Long tree shadows (INTEGRATION-PLAN.md step B9, Graphics/Remaster/Canopy.hpp):
// the canopy raster and its mip rules on synthetic crowns, forests and slopes,
// the removal of a destroyed object, the trace's shadow length and its sqrt
// crossing density, what a receiver reads, the sun-visibility map's placement
// and rebuild rule, the strength over the gate's edges, the frame constants,
// and the contracts that tie the shaders to the C++ mirror.

using namespace Poseidon::remaster;
namespace gl = Poseidon::gl33lighting;
using Catch::Approx;

namespace
{

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

constexpr int kRange = 64;     // terrain vertices a side
constexpr float kGrid = 12.5f; // metres between them
constexpr int kK = 2;          // canopy texels per cell: 6.25 m texels, 128 a side
constexpr float kTexel = kGrid / kK;

std::vector<float> FlatHeights(float h)
{
    return std::vector<float>(static_cast<size_t>(kRange) * kRange, h);
}

// Ground rising 0.2 m per metre eastward.
std::vector<float> SlopeHeights()
{
    std::vector<float> h(static_cast<size_t>(kRange) * kRange);
    for (int z = 0; z < kRange; ++z)
        for (int x = 0; x < kRange; ++x)
            h[static_cast<size_t>(z) * kRange + x] = 50.0f + 0.2f * static_cast<float>(x) * kGrid;
    return h;
}

// The centre of level-0 texel j, metres.
float TexelCentre(int j)
{
    return (static_cast<float>(j) + 0.5f) * kTexel;
}

CanopyItem Crown(int id, float x, float z, float radius, float height, float opacity)
{
    CanopyItem it;
    it.id = id;
    it.cx = x;
    it.cz = z;
    it.a = it.b = radius;
    it.height = height;
    it.opacity = opacity;
    it.round = true;
    return it;
}

// An axis-aligned box covering level-0 texels [x0, x1) x [z0, z1) exactly.
CanopyItem Box(int id, int x0, int z0, int x1, int z1, float height, float opacity)
{
    CanopyItem it;
    it.id = id;
    it.cx = 0.5f * static_cast<float>(x0 + x1) * kTexel;
    it.cz = 0.5f * static_cast<float>(z0 + z1) * kTexel;
    it.a = 0.5f * static_cast<float>(x1 - x0) * kTexel;
    it.b = 0.5f * static_cast<float>(z1 - z0) * kTexel;
    it.height = height;
    it.opacity = opacity;
    return it;
}

CanopyRaster Raster(std::vector<CanopyItem> items, std::vector<float> heights = FlatHeights(100.0f))
{
    CanopyRaster r;
    REQUIRE(r.Build(std::move(items), std::move(heights), kRange, kGrid, kK));
    REQUIRE(r.Field());
    return r;
}

// The sun from the east (+x) at `deg` degrees.
CanopySunRay EastSun(float deg)
{
    const float e = deg * 0.017453292f;
    return CanopySunRayToward(std::cos(e), std::sin(e), 0.0f);
}

std::string Fixed(float v, int decimals)
{
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.*f", decimals, static_cast<double>(v));
    return buf;
}

} // namespace

TEST_CASE("Canopy: trees, bushes, forests and buildings shade the ground, fences and crosses do not",
          "[remaster][Canopy]")
{
    CHECK(CanopyKindOfMapType(Poseidon::MapTree) == CanopyKind::Tree);
    CHECK(CanopyKindOfMapType(Poseidon::MapSmallTree) == CanopyKind::SmallTree);
    CHECK(CanopyKindOfMapType(Poseidon::MapBush) == CanopyKind::Bush);
    for (int t : {Poseidon::MapForestBorder, Poseidon::MapForestTriangle, Poseidon::MapForestSquare})
        CHECK(CanopyKindOfMapType(t) == CanopyKind::Forest);
    for (int t : {Poseidon::MapBuilding, Poseidon::MapHouse, Poseidon::MapChurch, Poseidon::MapChapel,
                  Poseidon::MapBunker, Poseidon::MapFortress, Poseidon::MapViewTower, Poseidon::MapLighthouse,
                  Poseidon::MapFuelstation, Poseidon::MapHospital, Poseidon::MapRock})
        CHECK(CanopyKindOfMapType(t) == CanopyKind::Solid);
    for (int t : {Poseidon::MapCross, Poseidon::MapFountain, Poseidon::MapQuay, Poseidon::MapFence, Poseidon::MapWall,
                  Poseidon::MapHide, Poseidon::MapBusStop})
        CHECK(CanopyKindOfMapType(t) == CanopyKind::None);
    CHECK(CanopyOpacity(CanopyKind::Tree) == Approx(0.8f));
    CHECK(CanopyOpacity(CanopyKind::SmallTree) == Approx(0.65f));
    CHECK(CanopyOpacity(CanopyKind::Bush) == Approx(0.5f));
    CHECK(CanopyOpacity(CanopyKind::Forest) == Approx(0.95f));
    CHECK(CanopyOpacity(CanopyKind::Solid) == Approx(1.0f));
    CHECK(CanopyOpacity(CanopyKind::None) == 0.0f);
    CHECK(CanopyRound(CanopyKind::Tree));
    CHECK(CanopyRound(CanopyKind::Bush));
    CHECK_FALSE(CanopyRound(CanopyKind::Forest));
    CHECK_FALSE(CanopyRound(CanopyKind::Solid));
    // About 6.25 m texels, at most 4096 a side.
    CHECK(CanopyTexelsPerCell(12.5f, 1024) == 2);
    CHECK(CanopyTexelsPerCell(50.0f, 256) == 8);
    CHECK(CanopyTexelsPerCell(12.5f, 4096) == 1);
    CHECK(CanopyTexelsPerCell(6.25f, 512) == 1);
}

TEST_CASE("Canopy: a crown covers its texels by the share of samples inside, at its height above their ground",
          "[remaster][Canopy]")
{
    // A 3 m crown centred on texel (32, 32): the 4x4 samples sit 0.78 and
    // 2.34 m from the texel centre, so the four corner ones (3.31 m) fall out
    // and 12 of 16 cover: density 0.8 * 12 / 16.
    const CanopyRaster r = Raster({Crown(1, TexelCentre(32), TexelCentre(32), 3.0f, 15.0f, 0.8f)});
    const CanopyField& f = *r.Field();
    CHECK(f.size == kRange * kK);
    CHECK(f.texel == Approx(kTexel));
    CHECK(f.Density(0, 32, 32) == Approx(0.6f).margin(1e-4f));
    CHECK(f.Top(0, 32, 32) == Approx(115.0f).margin(0.02f));
    // The neighbour texels the crown does not reach keep the bare ground.
    CHECK(f.Density(0, 33, 32) == 0.0f);
    CHECK(f.Top(0, 33, 32) == Approx(100.0f).margin(0.02f));
    CHECK(f.Density(0, 32, 31) == 0.0f);
    CHECK(f.version == 1u);
}

TEST_CASE("Canopy: overlapping objects multiply their transmission and keep the tallest top", "[remaster][Canopy]")
{
    const CanopyRaster r = Raster({Box(1, 10, 10, 12, 12, 8.0f, 0.5f), Box(2, 10, 10, 12, 12, 20.0f, 0.5f)});
    const CanopyField& f = *r.Field();
    CHECK(f.Density(0, 10, 10) == Approx(0.75f).margin(1e-4f));
    CHECK(f.Top(0, 11, 11) == Approx(120.0f).margin(0.02f));
    // A convex hull footprint: the triangle below its diagonal.
    CanopyItem tri;
    tri.id = 3;
    tri.height = 10.0f;
    tri.opacity = 1.0f;
    tri.hull = CanopyConvexHull(
        {40 * kTexel, 40 * kTexel, 48 * kTexel, 40 * kTexel, 40 * kTexel, 48 * kTexel, 42 * kTexel, 42 * kTexel});
    REQUIRE(tri.hull.size() == 6u); // the interior point is dropped
    const CanopyRaster h = Raster({tri});
    CHECK(h.Field()->Density(0, 41, 41) == Approx(1.0f));
    CHECK(h.Field()->Density(0, 46, 46) == 0.0f);
}

TEST_CASE("Canopy: a forest block on a slope follows the slope", "[remaster][Canopy]")
{
    const std::vector<float> slope = SlopeHeights();
    const CanopyRaster r = Raster({Box(1, 20, 20, 36, 28, 18.0f, 0.95f)}, slope);
    const CanopyField& f = *r.Field();
    for (int x : {20, 27, 35})
    {
        CAPTURE(x);
        const float ground = 50.0f + 0.2f * TexelCentre(x);
        CHECK(f.Top(0, x, 24) == Approx(ground + 18.0f).margin(0.03f));
    }
}

TEST_CASE("Canopy: a mip texel keeps its mean ground plus its tallest covered child, and the mean density",
          "[remaster][Canopy]")
{
    // One full texel of a 20 m tree in a 2x2 block on flat ground, then a
    // slope: the parent keeps the child's height above the mean ground.
    const CanopyRaster flat = Raster({Box(1, 10, 10, 11, 11, 20.0f, 1.0f)});
    const CanopyField& f = *flat.Field();
    CHECK(f.Levels() == 8); // 128, 64, ..., 1
    CHECK(f.Density(1, 5, 5) == Approx(0.25f).margin(1e-4f));
    CHECK(f.Top(1, 5, 5) == Approx(120.0f).margin(0.03f));
    CHECK(f.Top(1, 6, 5) == Approx(100.0f).margin(0.03f)); // nothing covered: the ground
    CHECK(f.Density(2, 2, 2) == Approx(0.0625f).margin(1e-4f));
    CHECK(f.Top(2, 2, 2) == Approx(120.0f).margin(0.03f));

    const CanopyRaster sloped = Raster({Box(1, 10, 10, 11, 11, 20.0f, 1.0f)}, SlopeHeights());
    const CanopyField& g = *sloped.Field();
    const float meanGround = 50.0f + 0.2f * 0.5f * (TexelCentre(10) + TexelCentre(11));
    CHECK(g.Top(1, 5, 5) == Approx(meanGround + 20.0f).margin(0.05f));
}

TEST_CASE("Canopy: removing a destroyed object gives the canopy built without it, at every level", "[remaster][Canopy]")
{
    const std::vector<CanopyItem> all = {Crown(7, TexelCentre(30), TexelCentre(30), 4.0f, 14.0f, 0.8f),
                                         Crown(8, TexelCentre(31), TexelCentre(32), 3.0f, 11.0f, 0.65f),
                                         Box(9, 40, 40, 44, 42, 9.0f, 1.0f)};
    CanopyRaster live = Raster(all);
    const std::uint32_t v0 = live.Field()->version;
    const std::vector<CanopyRect> rects = live.Remove(7);
    REQUIRE_FALSE(rects.empty());
    CHECK(live.Field()->version == v0 + 1);
    CHECK(rects.size() == static_cast<size_t>(live.Field()->Levels()));
    CHECK(rects[0].level == 0);
    CHECK(rects[0].x0 <= 30);
    CHECK(rects[0].x1 > 30);
    // Removing it again, or an id the canopy never had, changes nothing.
    CHECK(live.Remove(7).empty());
    CHECK(live.Remove(12345).empty());
    CHECK(live.Field()->version == v0 + 1);

    const CanopyRaster rebuilt = Raster({all[1], all[2]});
    const CanopyField& a = *live.Field();
    const CanopyField& b = *rebuilt.Field();
    REQUIRE(a.Levels() == b.Levels());
    for (int l = 0; l < a.Levels(); ++l)
    {
        CAPTURE(l);
        CHECK(a.levels[static_cast<size_t>(l)] == b.levels[static_cast<size_t>(l)]);
    }
}

TEST_CASE("Canopy: a tree's shadow on flat ground is as long as its height over the sun's tangent",
          "[remaster][Canopy]")
{
    // A 12.5 m square block, 15 m tall, under a 10 degree sun from the east:
    // the shadow falls west and ends near 15 / tan(10 deg) = 85 m.
    const CanopyRaster r = Raster({Box(1, 78, 60, 80, 62, 15.0f, 1.0f)});
    const CanopyField& f = *r.Field();
    const CanopySunRay ray = EastSun(10.0f);
    const float edgeX = 78.0f * kTexel; // the block's west edge
    const float z = 61.0f * kTexel;
    auto ground = [&](float d) { return CanopySunAt(f, edgeX - d, z, 100.0f, ray).ground; };
    CHECK(ground(10.0f) < 0.1f);
    CHECK(ground(40.0f) < 0.1f);
    CHECK(ground(120.0f) > 0.95f);
    // East of the block (between it and the sun) the ground is lit.
    CHECK(CanopySunAt(f, edgeX + 30.0f, z, 100.0f, ray).ground > 0.95f);
    // The half-shadow point lies within one coarse step of the geometric end.
    float half = 0.0f;
    for (float d = 10.0f; d < 160.0f; d += 1.0f)
        if (ground(d) < 0.5f)
            half = d;
    CHECK(half > 65.0f);
    CHECK(half < 105.0f);
    // A higher sun shortens it.
    const CanopySunRay high = EastSun(20.0f);
    CHECK(CanopySunAt(f, edgeX - 60.0f, z, 100.0f, high).ground > 0.95f);
    // No sun, no trace.
    CHECK(CanopySunAt(f, edgeX - 10.0f, z, 100.0f, CanopySunRayToward(1.0f, -0.1f, 0.0f)).ground == 1.0f);
}

TEST_CASE("Canopy: a partly covered texel is crossed with the probability sqrt(density)", "[remaster][Canopy]")
{
    // A uniform canopy 5 m tall of density 0.25 under a 45 degree sun: only the
    // first sample (3.125 m out, the ray at 3.4 m) runs under the top.
    CanopyField f;
    f.size = 128;
    f.texel = kTexel;
    f.heightBase = 0.0f;
    f.heightRange = 200.0f;
    for (int s = 128; s >= 1; s /= 2)
    {
        std::vector<std::uint16_t> level(static_cast<size_t>(s) * s * 2);
        for (size_t i = 0; i < level.size(); i += 2)
        {
            level[i] = static_cast<std::uint16_t>(std::lround(5.0 / 200.0 * 65535.0));
            level[i + 1] = static_cast<std::uint16_t>(std::lround(0.25 * 65535.0));
        }
        f.levels.push_back(level);
    }
    const CanopySunRay ray = CanopySunRayToward(1.0f, 1.0f, 0.0f);
    const CanopySunTexel t = CanopySunAt(f, 400.0f, 400.0f, 0.0f, ray);
    const float soft = 0.5f * kTexel * ray.tanSoft + 0.25f * kTexel + 0.5f;
    const float x = (5.0f - (kCanopyGroundLift + 0.5f * kTexel)) / soft;
    const float u = std::fmin(std::fmax((x + 0.5f) / 1.5f, 0.0f), 1.0f);
    const float occ = u * u * (3.0f - 2.0f * u);
    CHECK(t.ground == Approx(1.0f - std::sqrt(0.25f - kCanopyMinDensity) * occ).margin(2e-3f));
    CHECK(t.ground < 1.0f - 0.25f * occ - 0.1f); // not the linear density
}

TEST_CASE("Canopy: a raised receiver never shades itself, and reads a neighbour's shadow edge", "[remaster][Canopy]")
{
    const CanopySunRay ray = EastSun(9.0f);
    // A lone crown one texel wide: the ground under it is in its own shadow
    // (the first sample, half a texel out, sees the crown half filtered
    // away), its crown is not.
    const CanopyRaster lone = Raster({Box(1, 60, 60, 61, 61, 15.0f, 0.95f)});
    const CanopySunTexel under = CanopySunAt(*lone.Field(), TexelCentre(60), TexelCentre(60), 100.0f, ray);
    CHECK(under.ground < 0.5f);
    CHECK(CanopyReceiverVisibility(under, 0.0f) == Approx(under.ground));
    for (float hr : {2.0f, 6.0f, 12.0f})
    {
        CAPTURE(hr);
        CHECK(CanopyReceiverVisibility(under, hr) > 0.95f);
    }
    // Ten metres west of a taller neighbour: a raised receiver low down is in
    // its shadow, one above its shadow edge is lit.
    const CanopyRaster pair = Raster({Box(1, 60, 60, 62, 62, 20.0f, 0.95f)});
    const float x = 60.0f * kTexel - 10.0f;
    const CanopySunTexel west = CanopySunAt(*pair.Field(), x, TexelCentre(61), 100.0f, ray);
    CHECK(west.edge > 15.0f);
    CHECK(west.edge < 20.0f);
    CHECK(CanopyReceiverVisibility(west, 4.0f) < 0.2f);
    CHECK(CanopyReceiverVisibility(west, west.edge + west.soft + 1.0f) > 0.95f);
    // Between the ground and the raised receivers the read blends.
    const float g = CanopyReceiverVisibility(west, 0.0f), r = CanopyReceiverVisibility(west, kCanopyRaisedFrom);
    const float mid = CanopyReceiverVisibility(west, 1.0f);
    CHECK(mid >= std::fmin(g, r) - 1e-5f);
    CHECK(mid <= std::fmax(g, r) + 1e-5f);
    // Open ground: everything lit.
    const CanopySunTexel open = CanopySunAt(*pair.Field(), TexelCentre(20), TexelCentre(20), 100.0f, ray);
    CHECK(open.ground == 1.0f);
    CHECK(open.raisedLow == 1.0f);
    CHECK(open.edge == 0.0f);
    CHECK(CanopyReceiverVisibility(open, 5.0f) == 1.0f);
}

TEST_CASE("Canopy: what a raised receiver reads changes smoothly with an occluder's density", "[remaster][Canopy]")
{
    // A tall neighbour 10 m toward the sun, its opacity swept in small steps
    // across the edge rule's crossing band: no read jumps (a threshold on the
    // density made the edge, and with it a crown's light, flip between texels).
    const CanopySunRay ray = EastSun(9.0f);
    const float x = 60.0f * kTexel - 10.0f;
    float prev[3] = {-1.0f, -1.0f, -1.0f};
    float worst = 0.0f;
    for (int i = 1; i <= 500; ++i)
    {
        const float opacity = 0.001f * static_cast<float>(i);
        const CanopyRaster r = Raster({Box(1, 60, 60, 62, 62, 20.0f, opacity)});
        const CanopySunTexel t = CanopySunAt(*r.Field(), x, TexelCentre(61), 100.0f, ray);
        const float reads[3] = {CanopyReceiverVisibility(t, 3.0f), CanopyReceiverVisibility(t, 10.0f),
                                CanopyReceiverVisibility(t, 16.0f)};
        for (int k = 0; k < 3; ++k)
        {
            if (prev[k] >= 0.0f)
                worst = std::fmax(worst, std::fabs(reads[k] - prev[k]));
            prev[k] = reads[k];
        }
    }
    // A threshold on the density jumps by most of (1 - raisedLow) in one step.
    CHECK(worst < 0.1f);
}

TEST_CASE("Canopy: the sun-visibility map is texel-aligned and rebuilt only when the sun or the camera moved",
          "[remaster][Canopy]")
{
    CHECK(CanopySunTexelFor(800.0f) == Approx(kCanopySunMinTexel));
    CHECK(CanopySunTexelFor(2000.0f) == Approx(2.21875f));
    CHECK(CanopySunTexelFor(4000.0f) == Approx(kCanopySunMaxTexel));
    float ox = 0.0f, oz = 0.0f;
    CanopySunOrigin(5501.3f, 6248.7f, 2.0f, ox, oz);
    CHECK(std::fmod(ox, 2.0f) == Approx(0.0f).margin(1e-3f));
    CHECK(5501.3f - ox == Approx(0.5f * kCanopySunSize * 2.0f + 1.3f).margin(1e-2f));

    CanopySunKey built;
    CHECK(CanopySunNeedsRebuild(built, built, 0.0f, 0.0f)); // none yet
    built.valid = true;
    built.texel = 2.0f;
    built.canopyVersion = 3;
    const float e = 9.0f * 0.017453292f;
    built.sun[0] = std::cos(e);
    built.sun[1] = std::sin(e);
    built.sun[2] = 0.0f;
    CanopySunOrigin(1000.0f, 1000.0f, 2.0f, built.originX, built.originZ);
    CanopySunKey want = built;
    CHECK_FALSE(CanopySunNeedsRebuild(built, want, 1000.0f, 1000.0f));
    auto turned = [&](float deg)
    {
        CanopySunKey k = built;
        const float a = e + deg * 0.017453292f;
        k.sun[0] = std::cos(a);
        k.sun[1] = std::sin(a);
        return k;
    };
    CHECK_FALSE(CanopySunNeedsRebuild(built, turned(0.04f), 1000.0f, 1000.0f));
    CHECK(CanopySunNeedsRebuild(built, turned(0.06f), 1000.0f, 1000.0f));
    const float extent = kCanopySunSize * 2.0f;
    CHECK_FALSE(CanopySunNeedsRebuild(built, want, 1000.0f + 0.12f * extent, 1000.0f));
    CHECK(CanopySunNeedsRebuild(built, want, 1000.0f, 1000.0f - 0.13f * extent));
    want.canopyVersion = 4; // a destroyed tree
    CHECK(CanopySunNeedsRebuild(built, want, 1000.0f, 1000.0f));
    want = built;
    want.texel = 2.5f; // a new view distance
    CHECK(CanopySunNeedsRebuild(built, want, 1000.0f, 1000.0f));
}

TEST_CASE("Canopy: the shadows fade in and out over the gate's edges and are on exactly where the gate is",
          "[remaster][Canopy][FeatureGates]")
{
    CHECK(CanopyStrength(2.0f, 1.0f) == 0.0f);
    CHECK(CanopyStrength(3.0f, 1.0f) == Approx(0.5f));
    CHECK(CanopyStrength(4.0f, 1.0f) == Approx(1.0f));
    CHECK(CanopyStrength(15.0f, 1.0f) == Approx(1.0f));
    CHECK(CanopyStrength(23.0f, 1.0f) == Approx(0.5f));
    CHECK(CanopyStrength(25.0f, 1.0f) == 0.0f);
    CHECK(CanopyStrength(10.0f, 0.55f) == Approx(0.5f));
    CHECK(CanopyStrength(10.0f, 0.5f) == 0.0f);
    const LightingTuning t = Apply("remastered");
    for (float sun = -5.0f; sun <= 40.0f; sun += 0.5f)
        for (float sky = 0.3f; sky <= 1.0f; sky += 0.05f)
        {
            GateInputs in;
            in.sunElevationDeg = sun;
            in.skyThrough = sky;
            const bool on = ComputeFeatureGates(in, t)[Feature::CanopyShadows].on;
            CAPTURE(sun, sky);
            CHECK(on == (CanopyStrength(sun, sky) > 0.0f));
        }
}

TEST_CASE("Canopy: the frame constants are zero while the switch, its gate or the map is off", "[remaster][Canopy]")
{
    GateInputs low;
    low.sunElevationDeg = 9.0f;
    const LightingTuning on = Apply("remastered");
    const FeatureGates g = ComputeFeatureGates(low, on);
    REQUIRE(g[Feature::CanopyShadows].on);
    const CanopyFrameConstants c = ComputeCanopyFrameConstants(on, g, true, 9.0f, 1.0f, 100.0f, 200.0f, 2.0f);
    CHECK(c.canopy[0] == Approx(1.0f));
    CHECK(c.canopy[1] == 100.0f);
    CHECK(c.canopy[2] == 200.0f);
    CHECK(c.canopy[3] == Approx(1.0f / (kCanopySunSize * 2.0f)));
    // No map yet.
    CHECK(ComputeCanopyFrameConstants(on, g, false, 9.0f, 1.0f, 100.0f, 200.0f, 2.0f).canopy[0] == 0.0f);
    // The switch off: the look before this step.
    const LightingTuning off = Apply("remastered,canopyShadows=0");
    const FeatureGates go = ComputeFeatureGates(low, off);
    CHECK_FALSE(go[Feature::CanopyShadows].on);
    const CanopyFrameConstants co = ComputeCanopyFrameConstants(off, go, true, 9.0f, 1.0f, 100.0f, 200.0f, 2.0f);
    for (float v : co.canopy)
        CHECK(v == 0.0f);
    // Baseline turns it off with every other switch.
    const LightingTuning base = Apply("baseline");
    CHECK(ComputeCanopyFrameConstants(base, ComputeFeatureGates(low, base), true, 9.0f, 1.0f, 0, 0, 2.0f).canopy[0] ==
          0.0f);
    // Noon: the gate holds it off.
    GateInputs noon;
    noon.sunElevationDeg = 55.0f;
    CHECK(ComputeCanopyFrameConstants(on, ComputeFeatureGates(noon, on), true, 55.0f, 1.0f, 0, 0, 2.0f).canopy[0] ==
          0.0f);
    // Gates off (debugging): full strength while the sun is up.
    const LightingTuning forced = Apply("remastered,gates=0");
    CHECK(ComputeCanopyFrameConstants(forced, ComputeFeatureGates(noon, forced), true, 55.0f, 1.0f, 0, 0, 2.0f)
              .canopy[0] == 1.0f);
}

TEST_CASE("Canopy: the switch has landed with its own GPU section", "[remaster][Canopy][FeatureSwitches]")
{
    const FeatureInfo& info = GetFeatureInfo(Feature::CanopyShadows);
    CHECK(info.landed);
    CHECK(std::string(info.step) == "B9");
    CHECK(std::string(info.gpuSection) == "canopy");
    CHECK(std::string(Poseidon::gpu::SectionName(Poseidon::gpu::Section::Canopy)) == "canopy");
    // The map is world-space: no frame-relative pass resolution.
    GateInputs low;
    low.sunElevationDeg = 9.0f;
    CHECK(ComputeFeatureGates(low, Apply("remastered"))[Feature::CanopyShadows].scale == 1.0f);
}

TEST_CASE("Canopy: the GPU pass and the world read follow the C++ mirror", "[remaster][Canopy][Shaders]")
{
    const std::string pass = gl::PostShaderSource(gl::PostShader::CanopySun);
    REQUIRE_FALSE(pass.empty());
    CHECK(std::string(gl::PostShaderName(gl::PostShader::CanopySun)) == "psCanopySun");
    const std::pair<const char*, std::string> constants[] = {
        {"kMaxSteps", std::to_string(kCanopyMaxSteps)}, {"kGrowth", Fixed(kCanopyGrowth, 2)},
        {"kMinDensity", Fixed(kCanopyMinDensity, 3)},   {"kMinTransmit", Fixed(kCanopyMinTransmit, 2)},
        {"kGroundLift", Fixed(kCanopyGroundLift, 1)},   {"kRaisedLift", Fixed(kCanopyRaisedLift, 1)},
        {"kRaisedSkip", Fixed(kCanopyRaisedSkip, 1)},   {"kEdgeCrossFrom", Fixed(kCanopyEdgeCrossFrom, 1)},
        {"kEdgeCrossTo", Fixed(kCanopyEdgeCrossTo, 1)}, {"kEdgeRange", Fixed(kCanopyEdgeRange, 1)},
        {"kSoftRange", Fixed(kCanopySoftRange, 1)},     {"kMinSoft", Fixed(kCanopyMinSoft, 1)},
    };
    for (const auto& [name, value] : constants)
    {
        const std::string decl = std::string(name) + " = " + value + ";";
        CAPTURE(decl);
        CHECK(pass.find(decl) != std::string::npos);
    }
    // The sqrt crossing density and the step rule.
    CHECK(pass.find("float crossing = sqrt(c.y - kMinDensity);") != std::string::npos);
    CHECK(pass.find("float stepLen = max(c0, kGrowth * t);") != std::string::npos);
    CHECK(pass.find("float t = 0.5 * c0;") != std::string::npos);
    // 40 steps cover the longest trace (900 m, a sun at 2 deg).
    {
        float t = 0.5f * kTexel;
        int steps = 0;
        while (t <= kCanopyMaxTrace)
        {
            t += std::fmax(kTexel, kCanopyGrowth * t);
            ++steps;
        }
        CHECK(steps <= kCanopyMaxSteps);
    }

    // The world read (the TerrainClouds piece): the stored ranges, the
    // no-shadow-map reach, the map's edge fade, the read's minimum softness and
    // the ground-to-raised blend are the mirror's.
    const std::string read = gl::RemasterFragmentPiece(gl::FragmentPiece::TerrainClouds);
    CHECK(read.find("float canopySunVisibility(vec3 posAbs) {") != std::string::npos);
    CHECK(read.find("m.b * " + Fixed(kCanopyEdgeRange, 1)) != std::string::npos);
    CHECK(read.find("m.a * " + Fixed(kCanopySoftRange, 1) + ", " + Fixed(kCanopyMinReadSoft, 2)) != std::string::npos);
    CHECK(read.find(": " + Fixed(kCanopyNoMapReach, 1) + ";") != std::string::npos);
    CHECK(read.find(": " + Fixed(kCanopyNoMapFade, 1) + ";") != std::string::npos);
    CHECK(read.find("smoothstep(" + Fixed(kCanopySunEdgeFrom, 2) + ", " + Fixed(kCanopySunEdgeTo, 2)) !=
          std::string::npos);
    CHECK(read.find("max(hr, " + Fixed(kCanopyRaisedFrom, 1) + ")") != std::string::npos);
    // It takes over where the shadow maps fade out: over their far fade, from
    // the last tier's reach back by the fade (shadowMapVisibility's fade).
    CHECK(read.find("float reach = maps ? cascadeSplits[nC - 1] :") != std::string::npos);
    CHECK(read.find("float w = clamp((depth - (reach - fade)) / fade, 0.0, 1.0);") != std::string::npos);
    const std::string shadows = gl::RemasterFragmentPiece(gl::FragmentPiece::Shadows);
    CHECK(shadows.find("float fade = clamp((lastSplit - eyeDepth) / max(cascadeCtl.y, 0.001), 0.0, 1.0);") !=
          std::string::npos);
    CHECK(read.find("smoothstep(" + Fixed(kCanopyGroundTo, 1) + ", " + Fixed(kCanopyRaisedFrom, 1) + ", hr)") !=
          std::string::npos);
    // It multiplies the sun visibility once, after the terrain lookup that
    // sets the ground height it reads.
    const std::string shade = gl::RemasterFragmentPiece(gl::FragmentPiece::Shade);
    const size_t terrain = shade.find("terrainSunVisibility(posAbs, skyVisibility)");
    const size_t canopy = shade.find("sunVisibility *= canopySunVisibility(posAbs);");
    REQUIRE(terrain != std::string::npos);
    REQUIRE(canopy != std::string::npos);
    CHECK(terrain < canopy);
    const std::string statement = "sunVisibility *= canopySunVisibility(posAbs);";
    CHECK(shade.find("canopySunVisibility", canopy + statement.size()) == std::string::npos);
    // While off (litCanopy.x 0) it returns before reading anything.
    CHECK(read.find("if (litCanopy.x <= 0.0 || litFlags.y < 0.5) return 1.0;") != std::string::npos);
}

TEST_CASE("Canopy: units 10 and 15 hold stand-ins from init on, and the map is never sampled while it is drawn",
          "[remaster][Canopy][ShadowSampler]")
{
    CHECK(gl::kCanopyUnit == 10);
    CHECK(gl::kCanopyVisUnit == 15);
    const std::string lighting = ReadTextFile(Gl33Dir() / "EngineGL33_Lighting.cpp");
    const std::string canopy = ReadTextFile(Gl33Dir() / "EngineGL33_Canopy.cpp");
    REQUIRE_FALSE(lighting.empty());
    REQUIRE_FALSE(canopy.empty());
    CHECK(lighting.find("InitCanopy();") != std::string::npos);
    CHECK(lighting.find("DeinitCanopy();") != std::string::npos);
    CHECK(lighting.find("BindCanopy();") != std::string::npos);
    const size_t init = canopy.find("void EngineGL33::InitCanopy()");
    REQUIRE(init != std::string::npos);
    const size_t initEnd = canopy.find("\n}\n", init);
    const std::string initBody = canopy.substr(init, initEnd - init);
    CHECK(initBody.find("BindUnit(kCanopyUnit, s_canopy.fieldStandIn);") != std::string::npos);
    CHECK(initBody.find("BindUnit(kCanopyVisUnit, s_canopy.sunStandIn);") != std::string::npos);
    // The build binds the stand-in on unit 15 before it draws into the map,
    // and the map back after.
    const size_t build = canopy.find("bool EngineGL33::BuildCanopySunGpu(");
    REQUIRE(build != std::string::npos);
    const size_t standIn = canopy.find("BindUnit(kCanopyVisUnit, s_canopy.sunStandIn);", build);
    const size_t draw = canopy.find("glDrawArrays(GL_TRIANGLES, 0, 3);", build);
    const size_t back = canopy.find("BindUnit(kCanopyVisUnit, s_canopy.sunTex);", build);
    REQUIRE(standIn != std::string::npos);
    REQUIRE(draw != std::string::npos);
    REQUIRE(back != std::string::npos);
    CHECK(standIn < draw);
    CHECK(draw < back);
}

TEST_CASE("GpuTimings: the canopy map's build has its own section on the frames that rebuild it",
          "[Graphics][gpu][Canopy]")
{
    using namespace Poseidon::gpu;
    std::uint64_t stamps[kMarkCount] = {};
    std::uint32_t recorded = 0;
    auto set = [&](Mark m, double ms)
    {
        stamps[static_cast<int>(m)] = static_cast<std::uint64_t>(ms * 1e6);
        recorded |= 1u << static_cast<int>(m);
    };
    set(Mark::FrameBegin, 0.0);
    set(Mark::CanopyBegin, 0.1);
    set(Mark::CanopyEnd, 0.9);
    set(Mark::WorldBegin, 1.0);
    set(Mark::WorldEnd, 4.0);
    set(Mark::FrameEnd, 5.0);
    GpuTimings t;
    t.Submit(stamps, recorded);
    CHECK(t.LastMs(Section::Canopy) == Approx(0.8).margin(1e-3));
    CHECK(t.LastMs(Section::World) == Approx(3.0).margin(1e-3));
    CHECK(t.Summary().find(" | canopy 0.80") != std::string::npos);
    // A frame that does not rebuild records no canopy section.
    recorded &= ~(1u << static_cast<int>(Mark::CanopyBegin));
    recorded &= ~(1u << static_cast<int>(Mark::CanopyEnd));
    t.Submit(stamps, recorded);
    CHECK(t.LastMs(Section::Canopy) < 0.0f);
    CHECK(kMarkCount <= 32);
}
