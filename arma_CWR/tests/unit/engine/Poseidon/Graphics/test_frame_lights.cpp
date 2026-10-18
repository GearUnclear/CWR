#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <Poseidon/Graphics/Rendering/Lighting/FrameLights.hpp>
#include <Poseidon/Graphics/Remaster/FeatureSwitches.hpp>
#include <PoseidonGL33/EngineGL33_Lighting.hpp>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <vector>

// The frame's tiled light list (INTEGRATION-PLAN.md section 5 A6,
// Graphics/Rendering/Lighting/FrameLights.hpp): binning must list every light
// whose reach can touch a tile (a missed tile is a hard seam in a lamp's pool),
// ranking keeps the lights strongest at the camera, a full tile keeps the
// strongest, and the buffer and the shaders agree on the layout and the model.

using namespace Poseidon::lighting;

namespace
{

constexpr float kPi = 3.14159265f;

struct Vec3
{
    float x, y, z;
};

Vec3 Cross(const Vec3& a, const Vec3& b)
{
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

Vec3 Normalized(const Vec3& v)
{
    const float l = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    return {v.x / l, v.y / l, v.z / l};
}

// A camera looking along heading h (0 = +z, clockwise toward +x) and pitch p
// (degrees), a 1920x1080 target with the 3D viewport `vp` (x, y, w, h) and the
// capture rig's horizontal field of view.
TileView MakeView(float headingDeg, float pitchDeg, const float vp[4] = nullptr)
{
    const float h = headingDeg * kPi / 180.0f, p = pitchDeg * kPi / 180.0f;
    const Vec3 f{std::sin(h) * std::cos(p), std::sin(p), std::cos(h) * std::cos(p)};
    const Vec3 r = Normalized(Cross({0.0f, 1.0f, 0.0f}, f));
    const Vec3 u = Cross(f, r);
    TileView v;
    const Vec3 rows[3] = {r, u, f};
    for (int i = 0; i < 3; ++i)
    {
        v.rot[i][0] = rows[i].x;
        v.rot[i][1] = rows[i].y;
        v.rot[i][2] = rows[i].z;
    }
    v.p11 = 1.0f / std::tan(0.7f);
    v.p22 = v.p11 * 1920.0f / 1080.0f;
    const float full[4] = {0.0f, 0.0f, 1920.0f, 1080.0f};
    for (int i = 0; i < 4; ++i)
        v.viewport[i] = vp ? vp[i] : full[i];
    return v;
}

// Where a camera-relative point lands: frame-target pixel, false when it is
// behind the near distance or off the viewport.
bool Project(const TileView& v, const Vec3& p, float& px, float& py)
{
    const float x = v.rot[0][0] * p.x + v.rot[0][1] * p.y + v.rot[0][2] * p.z;
    const float y = v.rot[1][0] * p.x + v.rot[1][1] * p.y + v.rot[1][2] * p.z;
    const float z = v.rot[2][0] * p.x + v.rot[2][1] * p.y + v.rot[2][2] * p.z;
    if (z <= 0.1f)
        return false;
    px = v.viewport[0] + (v.p11 * x / z * 0.5f + 0.5f) * v.viewport[2];
    py = v.viewport[1] + (v.p22 * y / z * 0.5f + 0.5f) * v.viewport[3];
    return px >= v.viewport[0] && py >= v.viewport[1] && px < v.viewport[0] + v.viewport[2] &&
           py < v.viewport[1] + v.viewport[3];
}

// Points the shaders light (localLight): inside the reach, and for a
// spotlight in front of the lamp and inside the cone edge. Half of them on the
// reach's boundary, where a missed tile would show first.
std::vector<Vec3> LitPoints(const FrameLight& l, int n, std::mt19937& rng)
{
    std::uniform_real_distribution<float> u01(0.0f, 1.0f);
    const float reach = kRangeFactor * l.start * 0.9995f;
    std::vector<Vec3> out;
    const Vec3 d = Normalized({l.dir[0], l.dir[1], l.dir[2]});
    const Vec3 side = Normalized(std::fabs(d.y) < 0.9f ? Cross(d, {0.0f, 1.0f, 0.0f}) : Cross(d, {1.0f, 0.0f, 0.0f}));
    const Vec3 up = Cross(side, d);
    const float cosEdge = std::sqrt(kSpotOuterCos2) * 1.0005f;
    for (int i = 0; i < n; ++i)
    {
        const float dist = (i % 2 == 0) ? reach : reach * std::cbrt(u01(rng));
        Vec3 dir;
        if (l.IsSpot())
        {
            const float c = (i % 4 < 2) ? cosEdge : cosEdge + (1.0f - cosEdge) * u01(rng);
            const float s = std::sqrt(std::max(0.0f, 1.0f - c * c));
            const float phi = 2.0f * kPi * u01(rng);
            dir = {d.x * c + (side.x * std::cos(phi) + up.x * std::sin(phi)) * s,
                   d.y * c + (side.y * std::cos(phi) + up.y * std::sin(phi)) * s,
                   d.z * c + (side.z * std::cos(phi) + up.z * std::sin(phi)) * s};
        }
        else
        {
            const float z = 2.0f * u01(rng) - 1.0f;
            const float s = std::sqrt(std::max(0.0f, 1.0f - z * z));
            const float phi = 2.0f * kPi * u01(rng);
            dir = {s * std::cos(phi), z, s * std::sin(phi)};
        }
        out.push_back({l.pos[0] + dir.x * dist, l.pos[1] + dir.y * dist, l.pos[2] + dir.z * dist});
    }
    return out;
}

bool TileLists(const TileGrid& g, int tx, int ty, int light)
{
    int ids[kMaxLightsPerTile];
    const int n = TileLightsAt(g, tx * g.tileSize, ty * g.tileSize, ids);
    return std::find(ids, ids + n, light) != ids + n;
}

FrameLight Point(float x, float y, float z, float start, float brightness = 1.0f)
{
    FrameLight l;
    l.kind = FrameLightKind::Lamp;
    l.pos[0] = x;
    l.pos[1] = y;
    l.pos[2] = z;
    l.start = start;
    l.diffuse[0] = l.diffuse[1] = l.diffuse[2] = 0.8f;
    l.sortBrightness = brightness;
    return l;
}

FrameLight Spot(float x, float y, float z, float start, const Vec3& dir)
{
    FrameLight l = Point(x, y, z, start);
    l.kind = FrameLightKind::Spot;
    const Vec3 d = Normalized(dir);
    l.dir[0] = d.x;
    l.dir[1] = d.y;
    l.dir[2] = d.z;
    return l;
}

std::string ReadTextFile(const std::filesystem::path& p)
{
    std::ifstream f(p);
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

std::filesystem::path EngineDir()
{
    return std::filesystem::path(TESTS_ROOT_DIR).parent_path() / "engine";
}

std::string FunctionBody(const std::string& source, const std::string& signature)
{
    const size_t start = source.find(signature);
    if (start == std::string::npos)
        return {};
    const size_t end = source.find("\n}\n", start);
    return source.substr(start, end == std::string::npos ? std::string::npos : end - start);
}

int CountOf(const std::string& haystack, const std::string& needle)
{
    int n = 0;
    for (size_t at = haystack.find(needle); at != std::string::npos; at = haystack.find(needle, at + needle.size()))
        ++n;
    return n;
}

} // namespace

TEST_CASE("Frame lights: every tile a light can reach lists it, and few others do", "[Graphics][remaster][FrameLights]")
{
    std::mt19937 rng(20261018);
    std::uniform_real_distribution<float> pos(-120.0f, 120.0f);
    std::uniform_real_distribution<float> height(-3.0f, 12.0f);
    std::uniform_real_distribution<float> start(0.5f, 8.0f);
    std::uniform_real_distribution<float> angle(0.0f, 360.0f);
    std::uniform_real_distribution<float> pitch(-25.0f, 25.0f);
    const float pillarbox[4] = {240.0f, 0.0f, 1440.0f, 1080.0f};
    int checked = 0, fullScreen = 0, tight = 0;
    for (int round = 0; round < 40; ++round)
    {
        const TileView view = MakeView(angle(rng), pitch(rng), round % 4 == 3 ? pillarbox : nullptr);
        std::vector<FrameLight> lights;
        for (int i = 0; i < 12; ++i)
        {
            const float a = angle(rng) * kPi / 180.0f;
            const Vec3 beam{std::sin(a), -0.15f, std::cos(a)};
            lights.push_back(i % 3 == 0 ? Spot(pos(rng), height(rng), pos(rng), start(rng), beam)
                                        : Point(pos(rng), height(rng), pos(rng), start(rng)));
        }
        TileGrid grid;
        ResizeTileGrid(grid, 1920, 1080);
        BinLightsToTiles(lights, view, grid);
        CHECK(grid.dropped == 0); // 12 lights never fill a 16-entry tile
        for (int i = 0; i < static_cast<int>(lights.size()); ++i)
        {
            CAPTURE(round, i);
            const FrameLight& l = lights[static_cast<size_t>(i)];
            int minX = 1 << 30, minY = 1 << 30, maxX = -1, maxY = -1;
            for (const Vec3& p : LitPoints(l, 3000, rng))
            {
                float px, py;
                if (!Project(view, p, px, py))
                    continue;
                const int tx = static_cast<int>(px) / kTileSize, ty = static_cast<int>(py) / kTileSize;
                CAPTURE(px, py);
                REQUIRE(TileLists(grid, tx, ty, i));
                minX = std::min(minX, tx);
                maxX = std::max(maxX, tx);
                minY = std::min(minY, ty);
                maxY = std::max(maxY, ty);
                ++checked;
            }
            // Tight for a point light well in front of the eye: its rectangle is
            // the box around the projected sphere, so no listed tile lies more
            // than a few tiles outside what the samples reach. (A spotlight is
            // binned by its whole reach, which its cone fills only in part.)
            float rect[4];
            const LightSphere s = LightBounds(l);
            const float z = view.rot[2][0] * s.c[0] + view.rot[2][1] * s.c[1] + view.rot[2][2] * s.c[2];
            if (!LightScreenRect(l, view, rect))
            {
                CHECK(maxX < 0);
                continue;
            }
            if (z - s.r < view.nearZ)
            {
                ++fullScreen;
                continue;
            }
            // The box of a projected sphere that crosses the viewport's edge
            // keeps its full height (or width) there: skip those too.
            const bool clipped = rect[0] <= view.viewport[0] || rect[1] <= view.viewport[1] ||
                                 rect[2] >= view.viewport[0] + view.viewport[2] - 1.0f ||
                                 rect[3] >= view.viewport[1] + view.viewport[3] - 1.0f;
            if (maxX < 0 || l.IsSpot() || z < 2.0f * s.r || clipped)
                continue;
            ++tight;
            const int slack = 3;
            for (int ty = 0; ty < grid.tilesY; ++ty)
                for (int tx = 0; tx < grid.tilesX; ++tx)
                    if (TileLists(grid, tx, ty, i))
                    {
                        CAPTURE(tx, ty, minX, maxX, minY, maxY);
                        CHECK(tx >= minX - slack);
                        CHECK(tx <= maxX + slack);
                        CHECK(ty >= minY - slack);
                        CHECK(ty <= maxY + slack);
                    }
        }
        // Nothing lands outside the viewport.
        for (int ty = 0; ty < grid.tilesY; ++ty)
            for (int tx = 0; tx < grid.tilesX; ++tx)
            {
                const bool outside =
                    (tx + 1) * kTileSize <= view.viewport[0] || tx * kTileSize >= view.viewport[0] + view.viewport[2];
                if (outside)
                {
                    int ids[kMaxLightsPerTile];
                    CHECK(TileLightsAt(grid, tx * kTileSize, ty * kTileSize, ids) == 0);
                }
            }
    }
    CHECK(checked > 100000);
    CHECK(fullScreen > 0); // some reaches cross the eye plane
    CHECK(tight > 10);     // and the tightness was checked on enough of them
    INFO("tight-checked lights: " << tight);
}

TEST_CASE("Frame lights: every kind is binned by its whole reach, a spotlight too", "[Graphics][remaster][FrameLights]")
{
    // The classic budget ranks a spotlight wherever it is in reach, inside its
    // cone or not, so the tiles must list it over the whole sphere, not only
    // over its cone (which would be about half as large).
    std::mt19937 rng(7);
    const FrameLight spot = Spot(3.0f, 1.0f, -2.0f, 20.0f, {0.3f, -0.1f, 1.0f});
    const LightSphere s = LightBounds(spot);
    CHECK(s.r == Catch::Approx(kRangeFactor * 20.0f));
    CHECK(s.c[0] == 3.0f);
    CHECK(s.c[1] == 1.0f);
    CHECK(s.c[2] == -2.0f);
    for (const Vec3& p : LitPoints(spot, 20000, rng))
    {
        const float dx = p.x - s.c[0], dy = p.y - s.c[1], dz = p.z - s.c[2];
        REQUIRE(std::sqrt(dx * dx + dy * dy + dz * dz) <= s.r * 1.0001f);
    }
    // Pointing away from the eye 40 m ahead, its reach (100 m) holds the eye:
    // listed over the whole viewport, so it ranks there as Scene::SelectLights
    // would rank it, though it lights none of it.
    const TileView view = MakeView(0.0f, 0.0f);
    const FrameLight away = Spot(0.0f, 0.0f, 40.0f, 10.0f, {0.0f, 0.0f, 1.0f});
    float rect[4];
    REQUIRE(LightScreenRect(away, view, rect));
    CHECK(rect[0] == 0.0f);
    CHECK(rect[2] == 1919.0f);
    const LightSphere ps = LightBounds(Point(1.0f, 2.0f, 3.0f, 5.0f));
    CHECK(ps.r == Catch::Approx(50.0f));
    CHECK(ps.c[0] == 1.0f);
}

TEST_CASE("Frame lights: ranking keeps the lights strongest at the camera", "[Graphics][remaster][FrameLights]")
{
    std::vector<FrameLight> lights;
    lights.push_back(Point(0.0f, 0.0f, 100.0f, 5.0f, 1000.0f)); // 1000 / 10000 = 0.1
    lights.push_back(Point(0.0f, 0.0f, 10.0f, 5.0f, 100.0f));   // 100 / 100 = 1
    lights.push_back(Point(0.0f, 0.0f, 20.0f, 5.0f, 100.0f));   // 0.25
    lights.push_back(Point(0.0f, 0.0f, 10.0f, 5.0f, 100.0f));   // 1, after the first of its kind
    FrameLight dark = Point(0.0f, 0.0f, 1.0f, 5.0f, 1e6f);
    dark.diffuse[0] = dark.diffuse[1] = dark.diffuse[2] = 0.0f;
    lights.push_back(dark); // strongest weight, but adds nothing
    lights[3].owner = 42;
    RankFrameLights(lights);
    REQUIRE(lights.size() == 4);
    CHECK(lights[0].pos[2] == 10.0f);
    CHECK(lights[0].owner == 0);
    CHECK(lights[1].owner == 42); // ties keep their order
    CHECK(lights[2].pos[2] == 20.0f);
    CHECK(lights[3].pos[2] == 100.0f);
    // An ambient-only light still counts.
    FrameLight glow = dark;
    glow.ambient[1] = 0.1f;
    CHECK_FALSE(LightIsDark(glow));
    CHECK(LightIsDark(dark));

    // Over capacity: the strongest stay.
    std::vector<FrameLight> many;
    for (int i = 0; i < kMaxFrameLights + 10; ++i)
        many.push_back(Point(0.0f, 0.0f, 5.0f + static_cast<float>(i), 1.0f, 50.0f));
    std::reverse(many.begin(), many.end());
    RankFrameLights(many);
    REQUIRE(many.size() == static_cast<size_t>(kMaxFrameLights));
    for (int i = 0; i < kMaxFrameLights; ++i)
        CHECK(many[static_cast<size_t>(i)].pos[2] == 5.0f + static_cast<float>(i));
}

TEST_CASE("Frame lights: a full tile keeps the strongest lights, in rank order", "[Graphics][remaster][FrameLights]")
{
    const TileView view = MakeView(0.0f, 0.0f);
    std::vector<FrameLight> lights;
    for (int i = 0; i < 20; ++i)
        lights.push_back(Point(static_cast<float>(i % 5) - 2.0f, 1.0f, 3.0f, 10.0f, 100.0f - static_cast<float>(i)));
    TileGrid grid;
    ResizeTileGrid(grid, 1920, 1080);
    BinLightsToTiles(lights, view, grid);
    // Every reach holds the eye: every tile of the viewport lists the first 16.
    REQUIRE(grid.tilesX == 60);
    REQUIRE(grid.tilesY == 34);
    CHECK(grid.litTiles == 60 * 34);
    CHECK(grid.refs == 60 * 34 * kMaxLightsPerTile);
    CHECK(grid.dropped == 60 * 34 * (20 - kMaxLightsPerTile));
    for (int py : {0, 540, 1079})
        for (int px : {0, 960, 1919})
        {
            int ids[kMaxLightsPerTile];
            REQUIRE(TileLightsAt(grid, px, py, ids) == kMaxLightsPerTile);
            for (int k = 0; k < kMaxLightsPerTile; ++k)
                CHECK(ids[k] == k);
        }
    // Past the grid there is nothing.
    int ids[kMaxLightsPerTile];
    CHECK(TileLightsAt(grid, 1920 + kTileSize * 2, 0, ids) == 0);
    CHECK(TileLightsAt(grid, -1, 5, ids) == 0);
}

TEST_CASE("Frame lights: reaches behind the eye or beside the view are not binned", "[Graphics][remaster][FrameLights]")
{
    const TileView view = MakeView(0.0f, 0.0f); // looking along +z
    float rect[4];
    // 100 m behind, reach 30 m.
    CHECK_FALSE(LightScreenRect(Point(0.0f, 0.0f, -100.0f, 3.0f), view, rect));
    // Far to the side of the view (90 deg off axis), reach 30 m.
    CHECK_FALSE(LightScreenRect(Point(200.0f, 0.0f, 10.0f, 3.0f), view, rect));
    // A headlight 300 m behind the eye (reach 200 m), whichever way it points.
    CHECK_FALSE(LightScreenRect(Spot(0.0f, 0.0f, -300.0f, 20.0f, {0.0f, 0.0f, 1.0f}), view, rect));
    // Pointing into the view, it covers the screen around its beam.
    REQUIRE(LightScreenRect(Spot(0.0f, -1.0f, -5.0f, 20.0f, {0.0f, -0.05f, 1.0f}), view, rect));
    // A lamp straight ahead (reach 10 m at 100 m) covers a box around the
    // screen centre.
    REQUIRE(LightScreenRect(Point(0.0f, 0.0f, 100.0f, 1.0f), view, rect));
    CHECK(rect[0] < 960.0f);
    CHECK(rect[2] > 960.0f);
    CHECK(rect[1] < 540.0f);
    CHECK(rect[3] > 540.0f);
    CHECK(rect[2] - rect[0] < 400.0f);
    // A reach around the eye covers the whole viewport.
    REQUIRE(LightScreenRect(Point(1.0f, 0.0f, 1.0f, 2.0f), view, rect));
    CHECK(rect[0] == 0.0f);
    CHECK(rect[1] == 0.0f);
    CHECK(rect[2] == 1919.0f);
    CHECK(rect[3] == 1079.0f);
    // An empty viewport bins nothing.
    TileView none = view;
    none.viewport[2] = 0.0f;
    CHECK_FALSE(LightScreenRect(Point(0.0f, 0.0f, 10.0f, 2.0f), none, rect));
}

TEST_CASE("Frame lights: the light buffer holds four texels per light", "[Graphics][remaster][FrameLights]")
{
    std::vector<FrameLight> lights;
    lights.push_back(Point(1.0f, 2.0f, 3.0f, 4.0f, 250.0f));
    lights.push_back(Spot(-1.0f, 0.5f, 7.0f, 25.0f, {0.0f, 0.0f, 1.0f}));
    lights[1].ambient[0] = 0.1f;
    lights[1].sortBrightness = 10000.0f;
    std::vector<float> t;
    PackFrameLights(lights, t);
    REQUIRE(t.size() == 2u * kTexelsPerLight * 4);
    const float a[16] = {1, 2, 3, 4,   0.8f, 0.8f, 0.8f, 0,
                         0, 0, 0, 250, 0,    0,    1,    static_cast<float>(FrameLightKind::Lamp)};
    for (int i = 0; i < 16; ++i)
        CHECK(t[static_cast<size_t>(i)] == a[i]);
    const float* b = t.data() + 16;
    CHECK(b[0] == -1.0f);
    CHECK(b[3] == 25.0f);
    CHECK(b[7] == 1.0f); // spotlight
    CHECK(b[8] == 0.1f);
    CHECK(b[11] == 10000.0f); // the ranking weight
    CHECK(b[14] == 1.0f);
    CHECK(b[15] == static_cast<float>(FrameLightKind::Spot));
    CHECK(std::string(FrameLightKindName(FrameLightKind::Fire)) == "fire");
}

TEST_CASE("Frame lights: a light ranks only inside the sphere the tiles bin it by", "[Graphics][remaster][FrameLights]")
{
    // Where no tile lists a light, it must not take a place in the budget:
    // its importance is 0 outside LightBounds' sphere, the classic sort
    // brightness over squared distance well inside it, and continuous between.
    for (const FrameLight& l :
         {Point(2.0f, 3.0f, 40.0f, 5.0f, 100.0f), Spot(-3.0f, 0.5f, 10.0f, 20.0f, {0.2f, -0.1f, 1.0f})})
    {
        CAPTURE(l.IsSpot());
        const LightSphere s = LightBounds(l);
        const float c[3] = {s.c[0], s.c[1], s.c[2]};
        // At the sphere's centre: the classic value.
        const float d2 = (c[0] - l.pos[0]) * (c[0] - l.pos[0]) + (c[1] - l.pos[1]) * (c[1] - l.pos[1]) +
                         (c[2] - l.pos[2]) * (c[2] - l.pos[2]);
        CHECK(ClassicImportance(l, c) == Catch::Approx(l.sortBrightness / std::max(d2, 1e-4f)));
        // Along a ray out of the sphere: falls to 0 at the edge, smoothly.
        float prev = ClassicImportance(l, c);
        for (int i = 1; i <= 400; ++i)
        {
            const float t = s.r * 1.2f * static_cast<float>(i) / 400.0f;
            const float p[3] = {c[0] + t * 0.6f, c[1] + t * 0.48f, c[2] + t * 0.64f};
            const float v = ClassicImportance(l, p);
            if (t >= s.r)
                CHECK(v == 0.0f);
            CHECK(v >= 0.0f);
            prev = v;
        }
        CHECK(prev == 0.0f);
    }
}

TEST_CASE("Frame lights: the classic budget keeps the 7 strongest at a fragment, blending at its edge",
          "[Graphics][remaster][FrameLights]")
{
    float w[kMaxLightsPerTile];
    // Up to the budget, every light counts.
    const float few[7] = {5, 1, 0.1f, 3, 2, 0.01f, 7};
    ClassicBudgetWeights(few, 7, w);
    for (int i = 0; i < 7; ++i)
        CHECK(w[i] == 1.0f);
    // Clearly separated: exactly the 7 strongest, as Scene::SelectLights keeps.
    const float ten[10] = {10, 0.5f, 9, 8, 0.3f, 7, 6, 5, 0.1f, 4};
    ClassicBudgetWeights(ten, 10, w);
    for (int i = 0; i < 10; ++i)
    {
        CAPTURE(i);
        CHECK(w[i] == (ten[i] >= 4.0f ? 1.0f : 0.0f));
    }
    // The 7th and 8th close together share a blend; the sum of weights stays
    // at the budget.
    const float close[9] = {10, 9, 8, 7, 6, 5, 4.0f, 3.9f, 1};
    ClassicBudgetWeights(close, 9, w);
    CHECK(w[6] > 0.5f);
    CHECK(w[6] < 1.0f);
    CHECK(w[7] > 0.0f);
    CHECK(w[7] < 0.5f);
    CHECK(w[6] + w[7] == Catch::Approx(1.0f));
    CHECK(w[8] == 0.0f);
    CHECK(w[5] == 1.0f);
    // Continuous: sweeping one light's importance across the edge moves the
    // weighted sum of fixed contributions without a jump.
    float values[9] = {10, 9, 8, 7, 6, 5, 4, 0, 1};
    const float contribution[9] = {0.1f, 0.2f, 0.1f, 0.3f, 0.1f, 0.2f, 0.4f, 0.9f, 0.05f};
    float prev = -1.0f, maxStep = 0.0f;
    for (int s = 0; s <= 2000; ++s)
    {
        values[7] = 2.0f + 4.0f * static_cast<float>(s) / 2000.0f; // from below the 8th to above the 6th
        ClassicBudgetWeights(values, 9, w);
        float sum = 0.0f;
        for (int i = 0; i < 9; ++i)
            sum += w[i] * contribution[i];
        if (prev >= 0.0f)
            maxStep = std::max(maxStep, std::fabs(sum - prev));
        prev = sum;
    }
    CHECK(maxStep < 0.005f); // a hard cut would jump by 0.5 (0.9 in, 0.4 out)
}

TEST_CASE("Frame lights: the shaders read the list with the model and sizes of FrameLights.hpp",
          "[Graphics][Shaders][remaster][FrameLights]")
{
    namespace gl = Poseidon::gl33lighting;
    const std::string lights = gl::RemasterFragmentPiece(gl::FragmentPiece::Lights);
    REQUIRE_FALSE(lights.empty());
    // Capacities and layout.
    CHECK(lights.find("const int kTexelsPerLight = " + std::to_string(kTexelsPerLight) + ";") != std::string::npos);
    CHECK(lights.find("const int kTileTexels = " + std::to_string(kTileTexels) + ";") != std::string::npos);
    CHECK(kIndicesPerTexel == 4); // RGBA8UI: the shader reads ids[0..3] of each texel
    CHECK(kTileTexels == 4);      // loadTileList keeps a tile's texels in four registers
    CHECK(lights.find("int count = loadTileList();") != std::string::npos);
    CHECK(lights.find("layout(std140) uniform FrameLights {") != std::string::npos);
    CHECK(lights.find("vec4 frameLights[" + std::to_string(kMaxFrameLights * kTexelsPerLight) + "];") !=
          std::string::npos);
    CHECK(lights.find("uniform usampler2D lightTiles;") != std::string::npos);
    // The model the binning bounds: the reach ends at kRangeFactor start
    // distances and the cone edge is kSpotOuterCos2.
    CHECK(lights.find("float end2 = start2 * " + std::to_string(static_cast<int>(kRangeFactor * kRangeFactor)) +
                      ".0;") != std::string::npos);
    char cone[64];
    std::snprintf(cone, sizeof(cone), "smoothstep(%.4f, %.4f, cos2)", kSpotOuterCos2, kSpotInnerCos2);
    CHECK(lights.find(cone) != std::string::npos);
    // One model for both sources: the per-draw slots and the tiles.
    CHECK(CountOf(lights, "vec3 localLight(") == 1);
    CHECK(lights.find("sum += localLight(litLightPos[i], litLightColor[i]") != std::string::npos);
    CHECK(lights.find("return localLight(frameLights[at], frameLights[at + 1], frameLights[at + 2].rgb, "
                      "frameLights[at + 3].xyz, n, kd, ka);") != std::string::npos);
    CHECK(lights.find("if (litLightCount.y > 0.5) return localLightsTiled(n, kd, ka);") != std::string::npos);
    // A tile is found from the pixel alone; the night factor scales the sum.
    CHECK(lights.find("ivec2 tile = ivec2(gl_FragCoord.xy) / int(litLightCount.y);") != std::string::npos);
    CHECK(CountOf(lights, "return sum * litLightCount.z;") == 1);
    // The classic budget per surface, ranked at the fragment as
    // ClassicBudgetWeights does: sort brightness (texel 2 w) over the squared
    // distance, the 7th and 8th blended within the softness.
    CHECK(lights.find("const int kMaxTileLights = " + std::to_string(kMaxLightsPerTile) + ";") != std::string::npos);
    CHECK(lights.find("const int kClassicBudget = " + std::to_string(kClassicLightsPerSurface) + ";") !=
          std::string::npos);
    CHECK(lights.find("return fade * frameLights[at + 2].w / max(d2, 1e-4);") != std::string::npos);
    CHECK(lights.find("float fade = 1.0 - smoothstep(0.5 * r2, r2, d2);") != std::string::npos);
    // The sphere the importance fades with is the reach LightBounds bins by.
    CHECK(lights.find("const float kReachFactor = " + std::to_string(static_cast<int>(kRangeFactor)) + ".0;") !=
          std::string::npos);
    CHECK(lights.find("float r2 = kReachFactor * kReachFactor * pos.w * pos.w;") != std::string::npos);
    char band[96];
    std::snprintf(band, sizeof(band), "float band = max(edgeIn - edgeOut, %.2f * edgeIn);", kClassicBudgetSoftness);
    CHECK(lights.find(band) != std::string::npos);
    CHECK(lights.find("float w = count > kClassicBudget ? clamp(0.5 + (lightImportance(id) - mid) / band, 0.0, 1.0) : "
                      "1.0;") != std::string::npos);
    // The 7th and 8th strongest: the last two of the eight kept sorted.
    CHECK(kClassicLightsPerSurface == 7);
    CHECK(lights.find("edgeIn = t6;") != std::string::npos);
    CHECK(lights.find("edgeOut = t7;") != std::string::npos);
    CHECK(CountOf(lights, "m = max(t") == kClassicLightsPerSurface);
    // The per-object selection's cap is the same number.
    const std::string scene = ReadTextFile(EngineDir() / "Poseidon" / "World" / "Scene" / "Scene.cpp");
    CHECK(scene.find("const int maxLightsPerObject = Poseidon::lighting::kClassicLightsPerSurface;") !=
          std::string::npos);
    // The debug view.
    CHECK(kDebugViewLightTiles == 6);
    const std::string shade = gl::RemasterFragmentPiece(gl::FragmentPiece::Shade);
    CHECK(shade.find("litFlags.z > 5.5 ? lightTileView(display) : display") != std::string::npos);
    // The tile texture's unit, and the block's binding point, which every
    // program gets when it is linked.
    bool tiles = false;
    for (const auto& s : gl::kLitSamplers)
        tiles = tiles || (std::string(s.name) == "lightTiles" && s.unit == gl::kLightTileUnit);
    CHECK(tiles);
    CHECK(gl::kLightListBinding != gl::kLitBinding);
    const std::string shaders = ReadTextFile(EngineDir() / "PoseidonGL33" / "EngineGL33_Shaders.cpp");
    CHECK(shaders.find("glUniformBlockBinding(prog, lightBlock, Poseidon::gl33lighting::kLightListBinding);") !=
          std::string::npos);
    const std::string lighting = ReadTextFile(EngineDir() / "PoseidonGL33" / "EngineGL33_Lighting.cpp");
    CHECK(lighting.find("glBindBufferBase(GL_UNIFORM_BUFFER, kLightListBinding, t.dataBuf);") != std::string::npos);
}

TEST_CASE("Frame lights: the backend lights from the tiles only while they are on", "[Graphics][remaster][FrameLights]")
{
    const std::string lighting = ReadTextFile(EngineDir() / "PoseidonGL33" / "EngineGL33_Lighting.cpp");
    REQUIRE_FALSE(lighting.empty());
    // Tile mode skips the per-draw light slots and sends the draw's night factor.
    const std::string upload = FunctionBody(lighting, "void EngineGL33::UploadLitLights(");
    REQUIRE_FALSE(upload.empty());
    const size_t tiled = upload.find("if (_lightTilesActive)");
    const size_t perDraw = upload.find("SetSlot(LitConst::LightPos + n");
    REQUIRE(tiled != std::string::npos);
    REQUIRE(perDraw != std::string::npos);
    CHECK(tiled < perDraw);
    CHECK(upload.find("nightEffect > 0.0f ? nightEffect : 0.0f") != std::string::npos);
    // The frame decides from the gate, and the slot says so.
    const std::string frame = FunctionBody(lighting, "void EngineGL33::UploadLitFrameConstants()");
    CHECK(frame.find("_featureGates[rm::Feature::LightTiles].on") != std::string::npos);
    CHECK(frame.find("lightCount[1] = _lightTilesActive ? static_cast<float>(fl::kTileSize) : 0.0f;") !=
          std::string::npos);
    // Passes after the world light each draw from its own list again.
    const std::string post = ReadTextFile(EngineDir() / "PoseidonGL33" / "EngineGL33_PostFX.cpp");
    CHECK(FunctionBody(post, "void EngineGL33::EndWorldScene()").find("_lightTilesActive = false;") !=
          std::string::npos);
    // The scene keeps lit objects out of instanced runs only while draws read
    // their own lists.
    const std::string scene = ReadTextFile(EngineDir() / "Poseidon" / "World" / "Scene" / "SceneDraw.cpp");
    CHECK(scene.find("drawGrouping && !noLocalLights && !GEngine->LocalLightsFromTiles() ? this : nullptr") !=
          std::string::npos);
}

TEST_CASE("Frame lights: the light list switch has landed", "[Graphics][remaster][FrameLights]")
{
    const auto& info = Poseidon::remaster::GetFeatureInfo(Poseidon::remaster::Feature::LightTiles);
    CHECK(info.landed);
    CHECK(std::string(info.step) == "A6");
    CHECK(std::string(info.key) == "lightTiles");
}
