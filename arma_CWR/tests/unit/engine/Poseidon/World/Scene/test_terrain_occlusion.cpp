#include <catch2/catch_test_macros.hpp>

#include <Poseidon/World/Scene/TerrainOcclusion.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <random>
#include <vector>

// Hill culling: the horizon table may call an object hidden only when the
// terrain as the renderer draws it blocks every sight line from the eye to
// every point of the object's enlarged sphere. These tests march sight lines
// over the triangulated grid by brute force and hold the table to that, on
// hilly synthetic terrain, on a notch narrower than one wedge and on an object
// below the eye (two cases an earlier single-ray prototype got wrong; its
// single-ray test is kept here to show that).

using namespace Poseidon::TerrainOcclusion;

namespace
{
constexpr float kOutside = -100.0f; // the engine's YOutsideMap
constexpr float kPi = 3.14159265f;

// A height grid and its triangulated surface, as Landscape::SurfaceY and the
// terrain segments draw it: cells split along the (x+1, z)-(x, z+1) diagonal.
struct Terrain
{
    int size = 0;
    float cell = 0.0f;
    std::vector<float> v;

    Terrain(int n, float cellSize, const std::function<float(float, float)>& height) : size(n), cell(cellSize)
    {
        v.resize(static_cast<size_t>(n) * n);
        for (int z = 0; z < n; ++z)
            for (int x = 0; x < n; ++x)
                v[static_cast<size_t>(z) * n + x] = height(x * cellSize, z * cellSize);
    }
    float Vertex(int x, int z) const
    {
        if (x < 0 || z < 0 || x >= size || z >= size)
            return kOutside;
        return v[static_cast<size_t>(z) * size + x];
    }
    float SurfaceY(float x, float z) const
    {
        const float xr = x / cell, zr = z / cell;
        const int xi = static_cast<int>(std::floor(xr)), zi = static_cast<int>(std::floor(zr));
        const float xin = xr - xi, zin = zr - zi;
        const float y00 = Vertex(xi, zi), y01 = Vertex(xi + 1, zi), y10 = Vertex(xi, zi + 1),
                    y11 = Vertex(xi + 1, zi + 1);
        if (xin <= 1.0f - zin)
            return y00 + (y10 - y00) * zin + (y01 - y00) * xin;
        return y10 + (y01 - y11) - (y10 - y11) * xin - (y01 - y11) * zin;
    }
    HeightGrid Grid() const
    {
        HeightGrid g;
        g.BuildFromVertices(size, cell, kOutside, v);
        return g;
    }
};

// Can the eye see any point of the sphere? Points on the sphere's surface
// (and its centre), each sight line marched at a quarter cell over the
// drawn surface. Conservative the right way round for the check: it may miss
// a sliver of visibility, never invent one.
bool BruteVisible(const Terrain& t, float ex, float ey, float ez, float cx, float cy, float cz, float r)
{
    std::vector<std::array<float, 3>> pts{{cx, cy, cz}};
    for (int i = 0; i <= 12; ++i)
    {
        const float lat = -0.5f * kPi + kPi * i / 12.0f;
        for (int j = 0; j < 24; ++j)
        {
            const float lon = 2.0f * kPi * j / 24.0f;
            pts.push_back({cx + r * std::cos(lat) * std::sin(lon), cy + r * std::sin(lat),
                           cz + r * std::cos(lat) * std::cos(lon)});
        }
    }
    for (const auto& q : pts)
    {
        const float dx = q[0] - ex, dz = q[2] - ez;
        const float len = std::sqrt(dx * dx + dz * dz);
        const int steps = std::max(1, static_cast<int>(len / (0.25f * t.cell)));
        bool clear = true;
        for (int s = 1; s < steps && clear; ++s)
        {
            const float f = static_cast<float>(s) / steps;
            const float lineY = ey + (q[1] - ey) * f;
            if (t.SurfaceY(ex + dx * f, ez + dz * f) >= lineY)
                clear = false;
        }
        if (clear)
            return true;
    }
    return false;
}

// The earlier prototype's test: one ray per bin marched in 25 m steps with the
// terrain lowered 2 m, the sphere's top over its near-edge distance, one extra
// ray each side.
bool PrototypeHidden(const Terrain& t, float ex, float ey, float ez, float range, float px, float py, float pz,
                     float radius)
{
    constexpr int kAz = 1024;
    constexpr float kStep = 25.0f;
    const int steps = static_cast<int>(range / kStep) + 1;
    auto maxTan = [&](int a, int k)
    {
        const float ang = a * (2.0f * kPi / kAz);
        float m = -1e30f;
        for (int i = 0; i <= k; ++i)
        {
            const float d = (i + 1) * kStep;
            m = std::max(m, (t.SurfaceY(ex + std::sin(ang) * d, ez + std::cos(ang) * d) - 2.0f - ey) / d);
        }
        return m;
    };
    const float r = radius * 1.25f + 1.0f;
    const float ox = px - ex, oz = pz - ez;
    const float d = std::sqrt(ox * ox + oz * oz);
    const float dn = d - r;
    if (dn < 3.0f * kStep)
        return false;
    const int k = std::min(static_cast<int>(dn / kStep) - 1, steps - 1);
    const float tObj = (py + r - ey) / dn;
    float ang = std::atan2(ox, oz);
    if (ang < 0)
        ang += 2.0f * kPi;
    const float halfBins = std::asin(std::min(1.0f, r / d)) * (kAz / (2.0f * kPi)) + 1.0f;
    const int b0 = static_cast<int>(std::floor(ang * (kAz / (2.0f * kPi)) - halfBins));
    const int b1 = static_cast<int>(std::ceil(ang * (kAz / (2.0f * kPi)) + halfBins));
    for (int b = b0; b <= b1; ++b)
        if (!(tObj < maxTan(((b % kAz) + kAz) % kAz, k)))
            return false;
    return true;
}

// The enlarged sphere the table tests (Params defaults).
float Enlarged(float radius)
{
    const Params p;
    return radius * p.radiusScale + p.radiusMargin;
}

float Hills(float x, float z)
{
    return 40.0f + 30.0f * std::sin(x * 0.004f) * std::cos(z * 0.0031f) + 18.0f * std::sin(x * 0.011f + 1.3f) +
           12.0f * std::cos(z * 0.013f - 0.4f) + 5.0f * std::sin((x + z) * 0.03f);
}
} // namespace

TEST_CASE("TerrainOcclusion: the grid draws Landscape::SurfaceY's triangles and finds a segment's lowest point",
          "[scene][occlusion]")
{
    const Terrain t(128, 12.5f, Hills);
    const HeightGrid g = t.Grid();
    REQUIRE(g.Valid());
    std::mt19937 rng(7);
    std::uniform_real_distribution<float> pos(-100.0f, 1700.0f), len(0.0f, 80.0f), dir(0.0f, 2.0f * kPi);
    for (int i = 0; i < 400; ++i)
    {
        const float x0 = pos(rng), z0 = pos(rng);
        CHECK(std::fabs(g.SurfaceY(x0, z0) - t.SurfaceY(x0, z0)) < 1e-2f); // x / cell against x * (1 / cell)
        const float a = dir(rng), l = len(rng) * (i % 4 == 0 ? 0.05f : 1.0f);
        const float x1 = x0 + std::sin(a) * l, z1 = z0 + std::cos(a) * l;
        const float low = g.SegmentMin(x0, z0, x1, z1);
        // Never above the surface anywhere along the segment, and reached by
        // it (within float rounding) at some point: the exact minimum.
        float brute = 1e30f;
        for (int s = 0; s <= 2000; ++s)
        {
            const float f = s / 2000.0f;
            const float y = t.SurfaceY(x0 + (x1 - x0) * f, z0 + (z1 - z0) * f);
            CAPTURE(x0, z0, x1, z1, f);
            CHECK(low <= y + 1e-2f);
            brute = std::min(brute, y);
        }
        CHECK(low >= brute - 0.05f);
    }
    // Off the grid the drawn terrain sinks to the outside height.
    CHECK(g.SegmentMin(-50.0f, 100.0f, -40.0f, 110.0f) == kOutside);
}

TEST_CASE("TerrainOcclusion: nothing the table hides can be seen", "[scene][occlusion]")
{
    // Hilly ground, many eyes and objects: every Hidden answer is checked
    // against brute-force sight lines to the enlarged sphere.
    const Terrain t(512, 12.5f, Hills);
    const HeightGrid p = t.Grid();
    std::mt19937 rng(11);
    std::uniform_real_distribution<float> where(800.0f, 5600.0f), unit(0.0f, 1.0f);
    int hidden = 0, visible = 0, tested = 0;
    for (int e = 0; e < 12; ++e)
    {
        const float ex = where(rng), ez = where(rng);
        const float ey = t.SurfaceY(ex, ez) + (e % 3 == 0 ? 40.0f : 1.75f);
        HorizonTable table;
        table.Build(p, ex, ey, ez, 2500.0f, Params());
        REQUIRE(table.Valid());
        for (int o = 0; o < 150; ++o)
        {
            const float ang = unit(rng) * 2.0f * kPi;
            const float dist = 60.0f + unit(rng) * 2300.0f;
            const float px = ex + std::sin(ang) * dist, pz = ez + std::cos(ang) * dist;
            const float radius = 0.5f + unit(rng) * 12.0f;
            const float py = t.SurfaceY(px, pz) + radius * (0.2f + unit(rng));
            const Result r = table.Test(px, py, pz, radius);
            if (r == Result::NotTested)
                continue;
            ++tested;
            if (r == Result::Hidden)
            {
                ++hidden;
                INFO("eye " << ex << "," << ey << "," << ez << " object " << px << "," << py << "," << pz << " r "
                            << radius);
                CHECK_FALSE(BruteVisible(t, ex, ey, ez, px, py, pz, Enlarged(radius)));
            }
            else
            {
                ++visible;
            }
        }
    }
    INFO("tested " << tested << " hidden " << hidden << " visible " << visible);
    CHECK(tested > 1500);
    CHECK(hidden > 200); // the hills hide a good share: the test is not empty
    CHECK(visible > 200);
}

TEST_CASE("TerrainOcclusion: a notch narrower than a wedge shows what is behind it", "[scene][occlusion]")
{
    // A 30 m wall 3 km north of the eye, three vertex rows deep, with one
    // vertex column cut to the ground: a V notch 25 m wide at the top. A
    // tower 4.5 km out stands in line with the notch. At 3 km the notch is
    // narrower than one of the table's wedges (37 m) and sits between two of
    // the prototype's rays (18 m apart), which pass it 6 m and 12.5 m off its
    // centre line, where the V is 14 m and 30 m high.
    const float ex = 3200.0f, ez = 500.0f, ey = 1.75f;
    const float notchX = 3212.5f; // a vertex column
    auto wall = [&](float x, float z, bool notch)
    {
        const bool onWall = z >= ez + 3000.0f - 0.1f && z <= ez + 3025.0f + 0.1f;
        const bool inNotch = notch && std::fabs(x - notchX) < 1.0f;
        return onWall && !inNotch ? 30.0f : 0.0f;
    };
    const Terrain t(512, 12.5f, [&](float x, float z) { return wall(x, z, true); });
    const HeightGrid g = t.Grid();
    const float tx = ex + 4500.0f * (notchX - ex) / 3000.0f, tz = ez + 4500.0f;
    const float tr = 3.0f, ty = 4.0f;
    REQUIRE(BruteVisible(t, ex, ey, ez, tx, ty, tz, tr));
    HorizonTable table;
    table.Build(g, ex, ey, ez, 5000.0f, Params());
    CHECK(table.Test(tx, ty, tz, tr) == Result::Visible);
    // The prototype's rays straddle the notch: it called the tower hidden.
    CHECK(PrototypeHidden(t, ex, ey, ez, 5000.0f, tx, ty, tz, tr));

    // Without the notch the same tower is hidden.
    const Terrain solid(512, 12.5f, [&](float x, float z) { return wall(x, z, false); });
    const HeightGrid gs = solid.Grid();
    HorizonTable table2;
    table2.Build(gs, ex, ey, ez, 5000.0f, Params());
    CHECK(table2.Test(tx, ty, tz, tr) == Result::Hidden);
    CHECK_FALSE(BruteVisible(solid, ex, ey, ez, tx, ty, tz, Enlarged(tr)));
}

TEST_CASE("TerrainOcclusion: an object below the eye is judged by its far side", "[scene][occlusion]")
{
    // The eye on a knoll 100 m up looks north, down over a 63 m terrace from
    // 150 m to 200 m, into a valley. A sphere 330 m out, its top 40 m up (60 m
    // under the eye), shows its top over the terrace's far edge by about half
    // a metre. For a sphere below the eye the prototype took the top over the
    // NEAR distance, the steepest-looking slope only for a sphere above the
    // eye, and hid it.
    auto profile = [](float, float z)
    {
        const float d = z - 1000.0f;
        if (std::fabs(d) < 1.0f)
            return 98.25f;
        if (d >= 149.0f && d <= 201.0f)
            return 63.0f;
        return 0.0f;
    };
    const Terrain t(256, 12.5f, profile);
    const HeightGrid g = t.Grid();
    const float ex = 1600.0f, ey = 100.0f, ez = 1000.0f;
    const float radius = (30.0f - 1.0f) / 1.25f; // enlarged to 30 m
    const float px = ex, pz = ez + 330.0f, py = 40.0f - 30.0f;
    REQUIRE(BruteVisible(t, ex, ey, ez, px, py, pz, Enlarged(radius)));
    CHECK(PrototypeHidden(t, ex, ey, ez, 1500.0f, px, py, pz, radius));
    HorizonTable table;
    table.Build(g, ex, ey, ez, 1500.0f, Params());
    CHECK(table.Test(px, py, pz, radius) == Result::Visible);
    // Lower it 10 m and the terrace hides it from every point.
    CHECK(table.Test(px, py - 10.0f, pz, radius) == Result::Hidden);
    CHECK_FALSE(BruteVisible(t, ex, ey, ez, px, py - 10.0f, pz, Enlarged(radius)));
}

TEST_CASE("TerrainOcclusion: ridges hide what is below their line, never what is near", "[scene][occlusion]")
{
    // A 50 m ridge 400 m east of the eye.
    const Terrain t(512, 12.5f, [](float x, float) { return x >= 2400.0f && x <= 2450.0f ? 50.0f : 0.0f; });
    const HeightGrid p = t.Grid();
    const float ex = 2000.0f, ey = 1.75f, ez = 3000.0f;
    HorizonTable table;
    table.Build(p, ex, ey, ez, 4000.0f, Params());
    CHECK(table.Test(3200.0f, 10.0f, ez, 8.0f) == Result::Hidden);      // a tree 1.2 km out, far below the ridge line
    CHECK(table.Test(3200.0f, 150.0f, ez, 8.0f) == Result::Visible);    // a hill top above it
    CHECK(table.Test(2300.0f, 10.0f, ez, 8.0f) == Result::Visible);     // in front of the ridge
    CHECK(table.Test(2060.0f, 2.0f, ez, 2.0f) == Result::NotTested);    // closer than 75 m
    CHECK(table.Test(ex, 2.0f, ez - 1200.0f, 8.0f) == Result::Visible); // the other way: open ground
    // The table answers only for the eye it was built for.
    CHECK(table.Matches(ex, ey, ez, 4000.0f));
    CHECK_FALSE(table.Matches(ex + 0.01f, ey, ez, 4000.0f));
    CHECK_FALSE(table.Matches(ex, ey, ez, 3000.0f));
    // Only the wedges asked about were built.
    CHECK(table.WedgesBuilt() > 0);
    CHECK(table.WedgesBuilt() < 40);
}

TEST_CASE("TerrainOcclusion: chords grow from 25 m to 100 m apart", "[scene][occlusion]")
{
    const Terrain t(64, 12.5f, [](float, float) { return 0.0f; });
    const HeightGrid p = t.Grid();
    HorizonTable table;
    table.Build(p, 400.0f, 1.75f, 400.0f, 4000.0f, Params());
    REQUIRE(table.Chords() > 2);
    CHECK(table.ChordDistance(0) == 25.0f);
    CHECK(table.ChordDistance(1) == 50.0f);
    CHECK(table.ChordDistance(table.Chords() - 1) <= 4000.0f);
    // 20 near chords to 500 m, 20 to 1,500 m, 25 to 4,000 m.
    CHECK(table.Chords() == 65);
    float last = 0.0f;
    for (int k = 0; k < table.Chords(); ++k)
    {
        const float d = table.ChordDistance(k);
        const float step = d - last;
        CAPTURE(k, d);
        CHECK(step <= (d <= 500.0f ? 25.0f : (d <= 1500.0f ? 50.0f : 100.0f)) + 1e-3f);
        last = d;
    }
}

TEST_CASE("TerrainOcclusion: the governor idles on views the cull does not pay for", "[scene][occlusion]")
{
    CullGovernor g;
    // A crowded view with a good share behind hills: it keeps running.
    for (int i = 0; i < 100; ++i)
    {
        REQUIRE(g.ShouldRun());
        g.Report(7000, 6000);
    }
    CHECK_FALSE(g.Idle());

    // An open view: after a full window it idles, then probes once every
    // window.
    CullGovernor open;
    for (int i = 0; i < CullGovernor::kWindow; ++i)
    {
        REQUIRE(open.ShouldRun());
        open.Report(800, 5);
    }
    CHECK(open.Idle());
    int runs = 0;
    for (int i = 0; i < 3 * CullGovernor::kWindow; ++i)
    {
        if (open.ShouldRun())
        {
            ++runs;
            open.Report(800, 5);
        }
    }
    CHECK(runs == 3);
    CHECK(open.Idle());
    // A probe that finds a ridge in view starts it again.
    while (!open.ShouldRun())
    {
    }
    open.Report(5000, 2500);
    CHECK_FALSE(open.Idle());
    CHECK(open.ShouldRun());

    // Too few objects to bother, however many are hidden.
    CullGovernor sparse;
    for (int i = 0; i < CullGovernor::kWindow; ++i)
    {
        sparse.ShouldRun();
        sparse.Report(150, 150);
    }
    CHECK(sparse.Idle());
}
