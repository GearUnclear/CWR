#include <Poseidon/World/Terrain/TerrainLighting.hpp>

#include <Poseidon/Graphics/Core/Engine.hpp>
#include <Poseidon/Graphics/Remaster/TerrainHorizon.hpp>
#include <Poseidon/World/Terrain/Landscape.hpp>
#include <Poseidon/Foundation/Framework/Log.hpp>

#include <chrono>
#include <cstdint>
#include <cstring>
#include <future>
#include <memory>
#include <vector>

namespace Poseidon
{

namespace
{
struct BuildState
{
    const Landscape* land = nullptr;
    int range = 0;
    float grid = 0.0f;
    std::uint64_t sample = 0;
    std::future<std::shared_ptr<const remaster::TerrainHorizonMap>> pending;
    bool handed = true;
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
} // namespace

void UpdateTerrainLighting(Landscape* land)
{
    if (!GEngine || !land || !GEngine->WantsTerrainHorizonMap())
        return;
    const int range = land->GetTerrainRange();
    const float grid = land->GetTerrainGrid();
    if (range < 2 || !(grid > 0.0f))
        return;

    // Hand over a finished build.
    if (!s_state.handed && s_state.pending.valid() &&
        s_state.pending.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
    {
        GEngine->SetTerrainHorizonMap(s_state.pending.get());
        s_state.handed = true;
    }

    const std::uint64_t sample = SampleHeights(*land, range, grid);
    if (land == s_state.land && range == s_state.range && grid == s_state.grid && sample == s_state.sample)
        return;
    // A build still running for the previous landscape finishes first (the
    // future's destructor waits); a new island is rare enough for that.
    if (s_state.pending.valid())
        s_state.pending.wait();

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

    s_state.pending = std::async(
        std::launch::async,
        [heights = std::move(heights), range, grid]()
        {
            const auto t0 = std::chrono::steady_clock::now();
            auto map = std::make_shared<remaster::TerrainHorizonMap>();
            remaster::BuildTerrainHorizonMap(heights.data(), range, grid, remaster::TerrainHorizonOptions{}, *map);
            const auto ms =
                std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0).count();
            LOG_INFO(Graphics, "Terrain horizon map {}x{} (from {}x{} at {} m) built in {} ms", map->size, map->size,
                     range, range, grid, ms);
            return std::shared_ptr<const remaster::TerrainHorizonMap>(std::move(map));
        });
}

} // namespace Poseidon
