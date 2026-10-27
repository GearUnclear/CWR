#include <Poseidon/World/Terrain/TerrainLighting.hpp>

#include <Poseidon/Graphics/Core/Engine.hpp>
#include <Poseidon/Graphics/Remaster/Canopy.hpp>
#include <Poseidon/Graphics/Remaster/TerrainHorizon.hpp>
#include <Poseidon/Graphics/Remaster/VolumetricFog.hpp>
#include <Poseidon/Graphics/Rendering/Lighting/Lights.hpp>
#include <Poseidon/Core/Config/EngineConfig.hpp>
#include <Poseidon/Core/Global.hpp>
#include <Poseidon/World/Scene/Scene.hpp>
#include <Poseidon/Graphics/Rendering/Shape/Shape.hpp>
#include <Poseidon/Graphics/Textures/TextureBank.hpp>
#include <Poseidon/World/Scene/Object.hpp>
#include <Poseidon/World/Terrain/Landscape.hpp>
#include <Poseidon/UI/Settings/GraphicsApply.hpp>
#include <Poseidon/Foundation/Framework/Log.hpp>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <future>
#include <memory>
#include <unordered_map>
#include <vector>

namespace Poseidon
{

namespace
{
struct BuildResult
{
    std::shared_ptr<const remaster::TerrainHorizonMap> map; // null when cancelled
    // The canopy heightfield of the landscape objects (long tree shadows,
    // canopyShadows), built with the map; null when cancelled or empty.
    std::shared_ptr<remaster::CanopyRaster> canopy;
    long long ms = 0;
    long long canopyMs = 0;
};

// One build in flight: its cancel flag lives with it, so a retired build can
// still be told to stop.
struct Build
{
    std::shared_ptr<std::atomic<bool>> cancel = std::make_shared<std::atomic<bool>>(false);
    std::future<BuildResult> result;
};

struct BuildState
{
    const Landscape* land = nullptr;
    int range = 0;
    float grid = 0.0f;
    std::uint64_t sample = 0;
    Build pending;
    bool handed = true;
    // Builds abandoned for a newer landscape: cancelled, dropped once they stop.
    std::vector<Build> retired;
    // The canopy of the current landscape once handed over (main thread), and
    // the landscape objects destroyed since its objects were gathered: each
    // takes its footprint out of the canopy (TerrainCanopyObjectDestroyed).
    std::shared_ptr<remaster::CanopyRaster> canopy;
    std::vector<int> destroyed;

    void Retire(Build& b)
    {
        if (!b.result.valid())
            return;
        b.cancel->store(true, std::memory_order_relaxed);
        retired.push_back(std::move(b));
        b = Build{};
    }
    void DropFinished()
    {
        retired.erase(std::remove_if(retired.begin(), retired.end(),
                                     [](Build& b)
                                     {
                                         return !b.result.valid() ||
                                                b.result.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
                                     }),
                      retired.end());
    }
    // Cancels and joins everything (a cancelled build stops within a row).
    void Stop()
    {
        Retire(pending);
        for (Build& b : retired)
            if (b.result.valid())
                b.result.wait();
        retired.clear();
        handed = true;
        canopy.reset();
        destroyed.clear();
    }
    ~BuildState() { Stop(); }
};
BuildState s_state;

// Cheap per-frame change detector: grid dimensions plus a strided sample of
// heights. A full hash of a 1024x1024 grid every frame would cost more than the
// lighting it serves; the strided sample still notices a new island or an
// editor reshaping the ground.
std::uint64_t SampleHeights(const Landscape& land, int range, float grid)
{
    std::uint64_t h = 1469598103934665603ull;
    auto mix = [&h](std::uint32_t v)
    {
        h ^= v;
        h *= 1099511628211ull;
    };
    mix(static_cast<std::uint32_t>(range));
    std::uint32_t bits = 0;
    std::memcpy(&bits, &grid, sizeof(bits));
    mix(bits);
    const int step = range > 64 ? range / 64 : 1;
    for (int z = 0; z < range; z += step)
    {
        for (int x = (z / step) % step; x < range; x += step)
        {
            const float v = land.ClippedData(z, x);
            std::memcpy(&bits, &v, sizeof(bits));
            mix(bits);
        }
    }
    return h;
}

// Ground colour per landscape texture cell (display space): the average colour
// of the texture covering it, looked up once per texture. The texture grid can
// be coarser than the terrain grid; the worker expands it (ExpandGroundColors).
struct GroundCells
{
    int landRange = 0;
    float landGrid = 0.0f;
    std::vector<float> rgb; // landRange * landRange * 3
};

GroundCells GroundColors(const Landscape& land)
{
    GroundCells cells;
    cells.landRange = land.GetLandRange();
    cells.landGrid = land.GetLandGrid();
    if (cells.landRange < 1 || !(cells.landGrid > 0.0f))
        return cells;
    const int n = cells.landRange;
    cells.rgb.assign(static_cast<size_t>(n) * n * 3, 0.5f);
    std::unordered_map<int, Color> byIndex;
    const int textures = land.TextureCount();
    for (int z = 0; z < n; ++z)
    {
        for (int x = 0; x < n; ++x)
        {
            const int index = land.ClippedTextureIndex(z, x);
            auto it = byIndex.find(index);
            if (it == byIndex.end())
            {
                Texture* tex = index >= 0 && index < textures ? land.ClippedTexture(z, x) : nullptr;
                it = byIndex.emplace(index, tex ? tex->GetColor() : Color(0.5f, 0.5f, 0.5f, 1.0f)).first;
            }
            float* o = cells.rgb.data() + (static_cast<size_t>(z) * n + x) * 3;
            o[0] = it->second.R();
            o[1] = it->second.G();
            o[2] = it->second.B();
        }
    }
    return cells;
}

// Per terrain vertex, from the texture cells (empty when there are none). A
// cell covers the grid square to its +x/+z, so its colour belongs to the
// square's centre: each vertex samples the cells bilinearly around its own
// position (the four cells sharing it when the grids match), which keeps
// colour edges on the texture edges instead of half a square off.
std::vector<float> ExpandGroundColors(const GroundCells& cells, int range, float grid)
{
    std::vector<float> rgb;
    if (cells.rgb.empty())
        return rgb;
    rgb.resize(static_cast<size_t>(range) * range * 3);
    const int n = cells.landRange;
    auto axis = [&](int v, int& i0, int& i1, float& t)
    {
        const float f = static_cast<float>(v) * grid / cells.landGrid - 0.5f;
        const float fl = std::floor(f);
        t = f - fl;
        i0 = std::min(std::max(static_cast<int>(fl), 0), n - 1);
        i1 = std::min(std::max(static_cast<int>(fl) + 1, 0), n - 1);
    };
    for (int z = 0; z < range; ++z)
    {
        int z0, z1;
        float tz;
        axis(z, z0, z1, tz);
        for (int x = 0; x < range; ++x)
        {
            int x0, x1;
            float tx;
            axis(x, x0, x1, tx);
            const float* c00 = cells.rgb.data() + (static_cast<size_t>(z0) * n + x0) * 3;
            const float* c01 = cells.rgb.data() + (static_cast<size_t>(z0) * n + x1) * 3;
            const float* c10 = cells.rgb.data() + (static_cast<size_t>(z1) * n + x0) * 3;
            const float* c11 = cells.rgb.data() + (static_cast<size_t>(z1) * n + x1) * 3;
            float* o = rgb.data() + (static_cast<size_t>(z) * range + x) * 3;
            for (int c = 0; c < 3; ++c)
            {
                const float a = c00[c] + (c01[c] - c00[c]) * tx;
                const float b = c10[c] + (c11[c] - c10[c]) * tx;
                o[c] = a + (b - a) * tz;
            }
        }
    }
    return rgb;
}

// The canopy (Graphics/Remaster/Canopy.hpp): one footprint per landscape
// object that shades the ground at a distance, gathered on the main thread (the
// object lists belong to it) and rasterised by the worker. Destroyed objects
// are left out; ones destroyed later are taken out by Remove.
std::vector<remaster::CanopyItem> GatherCanopy(const Landscape& land, int& considered)
{
    std::vector<remaster::CanopyItem> items;
    considered = 0;
    const int n = land.GetLandRange();
    for (int z = 0; z < n; ++z)
    {
        for (int x = 0; x < n; ++x)
        {
            const ObjectList& list = land.GetObjects(z, x);
            for (int i = 0; i < list.Size(); ++i)
            {
                Object* obj = list[i];
                if (!obj || obj->GetType() != Primary || obj->IsDestroyed())
                    continue;
                LODShapeWithShadow* shape = obj->GetShape();
                if (!shape)
                    continue;
                ++considered;
                const remaster::CanopyKind kind = remaster::CanopyKindOfMapType(shape->GetMapType());
                const float opacity = remaster::CanopyOpacity(kind);
                if (opacity <= 0.0f)
                    continue;
                const Vector3 mn = shape->Min();
                const Vector3 mx = shape->Max();
                const Vector3 c = (mn + mx) * 0.5f;
                const Vector3 wc = obj->PositionModelToWorld(c);
                const Vector3 wu = obj->PositionModelToWorld(Vector3(mx.X(), c.Y(), c.Z())) -
                                   obj->PositionModelToWorld(Vector3(mn.X(), c.Y(), c.Z()));
                const Vector3 wv = obj->PositionModelToWorld(Vector3(c.X(), c.Y(), mx.Z())) -
                                   obj->PositionModelToWorld(Vector3(c.X(), c.Y(), mn.Z()));
                const float lu = std::sqrt(wu.X() * wu.X() + wu.Z() * wu.Z());
                const float lv = std::sqrt(wv.X() * wv.X() + wv.Z() * wv.Z());
                if (lu < 0.2f || lv < 0.2f)
                    continue;
                float top = -1e9f;
                for (int k = 0; k < 8; ++k)
                {
                    const Vector3 corner((k & 1) ? mx.X() : mn.X(), (k & 2) ? mx.Y() : mn.Y(),
                                         (k & 4) ? mx.Z() : mn.Z());
                    top = std::max(top, obj->PositionModelToWorld(corner).Y());
                }
                remaster::CanopyItem it;
                it.id = obj->ID();
                it.cx = wc.X();
                it.cz = wc.Z();
                it.ux = wu.X() / lu;
                it.uz = wu.Z() / lu;
                it.vx = wv.X() / lv;
                it.vz = wv.Z() / lv;
                it.round = remaster::CanopyRound(kind);
                const float shrink = it.round ? remaster::kCanopyCrownShrink : 1.0f;
                it.a = 0.5f * lu * shrink;
                it.b = 0.5f * lv * shrink;
                // The absolute top for now; the worker turns it into the height
                // above the ground at the centre.
                it.height = top;
                it.opacity = opacity;
                if (!it.round)
                {
                    // Forest triangles and odd footprints: the hull of the
                    // geometry level when it is loaded, else the box.
                    const Shape* geo = shape->GeometryLevel();
                    if (geo && geo->NPos() >= 3)
                    {
                        std::vector<float> pts;
                        pts.reserve(static_cast<size_t>(geo->NPos()) * 2);
                        for (int k = 0; k < geo->NPos(); ++k)
                        {
                            const Vector3 w = obj->PositionModelToWorld(geo->Pos(k));
                            pts.push_back(w.X());
                            pts.push_back(w.Z());
                        }
                        it.hull = remaster::CanopyConvexHull(std::move(pts));
                    }
                }
                items.push_back(std::move(it));
            }
        }
    }
    return items;
}

// Worker: each footprint's height above the ground at its centre, then the
// raster (null when cancelled).
std::shared_ptr<remaster::CanopyRaster> BuildCanopy(std::vector<remaster::CanopyItem> items,
                                                    const std::vector<float>& heights, int range, float grid,
                                                    const std::atomic<bool>* cancel)
{
    auto ground = [&](float x, float z)
    {
        const float fx = std::clamp(x / grid, 0.0f, static_cast<float>(range - 1));
        const float fz = std::clamp(z / grid, 0.0f, static_cast<float>(range - 1));
        const int x0 = std::min(static_cast<int>(fx), range - 2), z0 = std::min(static_cast<int>(fz), range - 2);
        const float tx = fx - static_cast<float>(x0), tz = fz - static_cast<float>(z0);
        auto at = [&](int xx, int zz) { return heights[static_cast<size_t>(zz) * range + xx]; };
        return (at(x0, z0) * (1.0f - tx) + at(x0 + 1, z0) * tx) * (1.0f - tz) +
               (at(x0, z0 + 1) * (1.0f - tx) + at(x0 + 1, z0 + 1) * tx) * tz;
    };
    std::vector<remaster::CanopyItem> kept;
    kept.reserve(items.size());
    for (remaster::CanopyItem& it : items)
    {
        const float rel = it.height - ground(it.cx, it.cz);
        if (rel < remaster::kCanopyMinRelTop)
            continue;
        it.height = std::min(rel, remaster::kCanopyMaxRelTop);
        kept.push_back(std::move(it));
    }
    auto raster = std::make_shared<remaster::CanopyRaster>();
    if (!raster->Build(std::move(kept), heights, range, grid, remaster::CanopyTexelsPerCell(grid, range), cancel))
        return nullptr;
    return raster;
}
} // namespace

void UpdateTerrainLighting(Landscape* land)
{
    s_state.DropFinished();
    if (!GEngine || !land || !GEngine->WantsTerrainHorizonMap())
        return;
    const int range = land->GetTerrainRange();
    const float grid = land->GetTerrainGrid();
    if (range < 2 || !(grid > 0.0f))
        return;

    // Hand over a finished build (logged here, on the main thread: the worker
    // never touches the logger, which may be gone when a build ends at exit).
    if (!s_state.handed && s_state.pending.result.valid() &&
        s_state.pending.result.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
    {
        BuildResult r = s_state.pending.result.get();
        s_state.pending = Build{};
        s_state.handed = true;
        if (r.map)
        {
            LOG_INFO(Graphics, "Terrain horizon map {}x{} (from {}x{} at {} m) built in {} ms", r.map->size,
                     r.map->size, s_state.range, s_state.range, s_state.grid, r.ms);
            GEngine->SetTerrainHorizonMap(std::move(r.map));
        }
        if (r.canopy && r.canopy->Field())
        {
            const remaster::CanopyField& f = *r.canopy->Field();
            LOG_INFO(Graphics, "Canopy {}x{} at {} m ({} levels) from {} objects built in {} ms", f.size, f.size,
                     f.texel, f.Levels(), r.canopy->ItemCount(), r.canopyMs);
            s_state.canopy = std::move(r.canopy);
            GEngine->SetTerrainCanopy(s_state.canopy->Field());
        }
    }

    // Objects destroyed since the canopy's objects were gathered take their
    // footprints out of it (a few texels each), once it is here.
    if (s_state.canopy && !s_state.destroyed.empty())
    {
        std::vector<remaster::CanopyRect> rects;
        for (int id : s_state.destroyed)
        {
            const std::vector<remaster::CanopyRect> r = s_state.canopy->Remove(id);
            rects.insert(rects.end(), r.begin(), r.end());
        }
        s_state.destroyed.clear();
        if (!rects.empty())
            GEngine->TerrainCanopyChanged(rects.data(), static_cast<int>(rects.size()));
    }

    const std::uint64_t sample = SampleHeights(*land, range, grid);
    if (land == s_state.land && range == s_state.range && grid == s_state.grid && sample == s_state.sample)
        return;
    // A build still running for the previous landscape is cancelled and left
    // to stop on its own; the main thread never waits for it.
    s_state.Retire(s_state.pending);

    s_state.land = land;
    s_state.range = range;
    s_state.grid = grid;
    s_state.sample = sample;
    s_state.handed = false;
    GEngine->SetTerrainHorizonMap(nullptr); // no stale map over the new ground
    GEngine->SetTerrainCanopy(nullptr);
    s_state.canopy.reset();
    // The gather below leaves out what is destroyed by now.
    s_state.destroyed.clear();

    std::vector<float> heights(static_cast<size_t>(range) * range);
    for (int z = 0; z < range; ++z)
        for (int x = 0; x < range; ++x)
            heights[static_cast<size_t>(z) * range + x] = land->ClippedData(z, x); // (z, x) order
    // Texture headers load on this (the main) thread, once per texture cell;
    // the worker expands the cells to the terrain grid.
    GroundCells cells = GroundColors(*land);
    const auto gather0 = std::chrono::steady_clock::now();
    int considered = 0;
    std::vector<remaster::CanopyItem> canopyItems = GatherCanopy(*land, considered);
    LOG_INFO(Graphics, "Canopy: {} occluders of {} landscape objects gathered in {} ms", canopyItems.size(), considered,
             std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - gather0).count());

    Build build;
    std::shared_ptr<std::atomic<bool>> cancel = build.cancel;
    build.result = std::async(std::launch::async,
                              [heights = std::move(heights), cells = std::move(cells),
                               canopyItems = std::move(canopyItems), cancel, range, grid]() mutable
                              {
                                  const auto t0 = std::chrono::steady_clock::now();
                                  const std::vector<float> ground = ExpandGroundColors(cells, range, grid);
                                  auto map = std::make_shared<remaster::TerrainHorizonMap>();
                                  remaster::TerrainHorizonOptions options;
                                  options.cancel = cancel.get();
                                  BuildResult r;
                                  if (remaster::BuildTerrainHorizonMap(heights.data(), range, grid, options, *map,
                                                                       ground.empty() ? nullptr : ground.data()))
                                      r.map = std::move(map);
                                  const auto t1 = std::chrono::steady_clock::now();
                                  r.ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
                                  if (r.map)
                                  {
                                      r.canopy =
                                          BuildCanopy(std::move(canopyItems), heights, range, grid, cancel.get());
                                      r.canopyMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                                                       std::chrono::steady_clock::now() - t1)
                                                       .count();
                                  }
                                  return r;
                              });
    s_state.pending = std::move(build);
}

namespace
{
// AI sight's map and the landscape it was built for.
struct FogSightBuilt
{
    const Landscape* land = nullptr;
    int range = 0;
    float grid = 0.0f;
    std::uint64_t sample = 0;
    std::shared_ptr<const remaster::FogSightMap> map;
};
FogSightBuilt s_sight;
} // namespace

remaster::FogInputs WorldFogInputs(const Landscape* land)
{
    remaster::FogInputs in;
    constexpr float kDeg = 57.29578f;
    auto clampUnit = [](float v) { return v < -1.0f ? -1.0f : (v > 1.0f ? 1.0f : v); };
    const LightSun* sun = GScene ? GScene->MainLight() : nullptr;
    if (sun)
    {
        const Vector3 toSun = -sun->Direction();
        in.sunElevationDeg = std::asin(clampUnit(toSun.Y())) * kDeg;
        const float azimuth = std::atan2(toSun.X(), toSun.Z()) * kDeg;
        in.sunAzimuthDeg = azimuth < 0.0f ? azimuth + 360.0f : azimuth;
        in.nightEffect = sun->NightEffect();
    }
    if (land)
    {
        in.skyThrough = land->SkyThrough();
        in.fog = land->GetFog();
        in.rain = land->GetRainDensity();
    }
    in.viewDistance = ENGINE_CONFIG.tacticalZ;
    if (GScene)
    {
        in.fogStart = GScene->GetFogMinRange();
        in.fogEnd = GScene->GetFogMaxRange();
    }
    in.year = Glob.clock.GetYear();
    in.dayOfYear = std::clamp(static_cast<int>(std::floor(Glob.clock.GetTimeInYear() / OneDay)), 0, 365);
    // The island's aridity (the dust) from AI sight's map, so the renderer
    // draws the dust AI sight takes.
    if (const std::shared_ptr<const remaster::FogSightMap> map = remaster::FogSightMapInUse())
        in.aridity = map->aridity;
    return in;
}

void SimulateFogSight(Landscape* land)
{
    const int range = land ? land->GetTerrainRange() : 0;
    const float grid = land ? land->GetTerrainGrid() : 0.0f;
    if (range < 2 || !(grid > 0.0f))
    {
        ShutdownFogSight();
        return;
    }
    const std::uint64_t sample = SampleHeights(*land, range, grid);
    if (land != s_sight.land || range != s_sight.range || grid != s_sight.grid || sample != s_sight.sample)
    {
        // A new island (or reshaped ground): the map is built here, at once,
        // so AI sight never runs a step without it.
        const auto t0 = std::chrono::steady_clock::now();
        std::vector<float> heights(static_cast<size_t>(range) * range);
        for (int z = 0; z < range; ++z)
            for (int x = 0; x < range; ++x)
                heights[static_cast<size_t>(z) * range + x] = land->ClippedData(z, x); // (z, x) order
        // The ground's colours give the island's aridity (the dust), as the
        // terrain horizon map's tints give it to the renderer.
        const std::vector<float> rgb = ExpandGroundColors(GroundColors(*land), range, grid);
        s_sight.land = land;
        s_sight.range = range;
        s_sight.grid = grid;
        s_sight.sample = sample;
        s_sight.map = remaster::BuildFogSightMap(heights.data(), range, grid, remaster::TerrainHorizonOptions{}.maxSize,
                                                 rgb.empty() ? nullptr : rgb.data());
        if (s_sight.map)
            LOG_INFO(
                Graphics, "AI fog sight: map {}x{} at {} m (from {}x{} at {} m), aridity {:.2f}, built in {} ms",
                s_sight.map->size, s_sight.map->size, s_sight.map->cellSize, range, range, grid, s_sight.map->aridity,
                std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0).count());
    }
    remaster::PublishFogSight(remaster::MakeFogSightSnapshot(WorldFogInputs(land), FogSightClassic(), s_sight.map));
}

bool FogSightClassic()
{
    // The Lighting row as last applied (UI/Settings/GraphicsApply.hpp): a
    // dedicated server or a headless client never applies one and keeps the
    // default, Remastered.
    const GraphicsConfig* cfg = AppliedGraphicsConfig();
    return cfg && cfg->lighting == GraphicsConfig::LightingClassic;
}

void ShutdownFogSight()
{
    s_sight = FogSightBuilt{};
    remaster::PublishFogSight(nullptr);
}

void ShutdownTerrainLighting()
{
    s_state.Stop();
    s_state.land = nullptr;
    s_state.range = 0;
    s_state.sample = 0;
}

bool TerrainLightingPending()
{
    return GEngine && GEngine->WantsTerrainHorizonMap() && !s_state.handed;
}

void TerrainCanopyObjectDestroyed(int objectId)
{
    // Only while a canopy is here or on its way: otherwise there is nothing
    // to take the object out of (classic lighting builds none).
    if (objectId < 0 || (!s_state.canopy && s_state.handed))
        return;
    s_state.destroyed.push_back(objectId);
}

} // namespace Poseidon
