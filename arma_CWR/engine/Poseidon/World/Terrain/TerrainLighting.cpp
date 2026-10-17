#include <Poseidon/World/Terrain/TerrainLighting.hpp>

#include <Poseidon/Graphics/Core/Engine.hpp>
#include <Poseidon/Graphics/Remaster/TerrainHorizon.hpp>
#include <Poseidon/Graphics/Textures/TextureBank.hpp>
#include <Poseidon/World/Terrain/Landscape.hpp>
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
    long long ms = 0;
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
                                     [](Build& b) {
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
    for (int z = 0; z < n; ++z)
    {
        for (int x = 0; x < n; ++x)
        {
            const int index = land.ClippedTextureIndex(z, x);
            auto it = byIndex.find(index);
            if (it == byIndex.end())
            {
                Texture* tex = land.ClippedTexture(z, x);
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

    std::vector<float> heights(static_cast<size_t>(range) * range);
    for (int z = 0; z < range; ++z)
        for (int x = 0; x < range; ++x)
            heights[static_cast<size_t>(z) * range + x] = land->ClippedData(z, x); // (z, x) order
    // Texture headers load on this (the main) thread, once per texture cell;
    // the worker expands the cells to the terrain grid.
    GroundCells cells = GroundColors(*land);

    Build build;
    std::shared_ptr<std::atomic<bool>> cancel = build.cancel;
    build.result = std::async(
        std::launch::async,
        [heights = std::move(heights), cells = std::move(cells), cancel, range, grid]()
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
            r.ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0).count();
            return r;
        });
    s_state.pending = std::move(build);
}

void ShutdownTerrainLighting()
{
    s_state.Stop();
    s_state.land = nullptr;
    s_state.range = 0;
    s_state.sample = 0;
}

} // namespace Poseidon
