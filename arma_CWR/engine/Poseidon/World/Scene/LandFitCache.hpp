#pragma once

#include <cstdint>
#include <cstring>
#include <unordered_map>
#include <utility>
#include <vector>

// Per-object fitted geometry (perf plan W1-9, survey S2-1(a)).
//
// A land-following shape (ClipLandKeep / ClipLandOn vertices: bushes, trees
// fitted to the slope) is shared by every object that uses it, and
// Object::Animate refits its vertices to the terrain under the object being
// drawn, every draw. The shared vertex buffer then had to be re-uploaded per
// draw, which serialised the GPU behind each upload (about 3,100 uploads and
// 22 ms of pass1 at the valley at a 4 km view distance, measurements-2 1b).
//
// Instead, an object of the plain classes (Object, ObjectPlain: they use
// Object::Animate unchanged) is fitted once per (object, LOD level): the fitted
// vertices go into a vertex buffer the cache owns, and the bounds the fit
// produces are kept with it. The entry is valid while everything the fit reads
// is unchanged, which the key below holds: the level, the object's world
// matrix (a move, a rotation or a fall changes it), the terrain height version
// and the object's damage. Anything else (a destroyed object, a shape whose
// every vertex is ClipLandOn, the software path) keeps today's per-draw fit.
//
// This header holds the engine-free core (the key and the table), so the unit
// tests can pin the invalidation rules, plus the declarations of the engine
// side (Object.cpp, ShapeDraw.cpp).

namespace Poseidon
{
class Shape;
class VertexBuffer;
class Landscape;
} // namespace Poseidon

namespace Poseidon::LandFit
{

// Everything a land fit reads, as values. Two keys are equal only when every
// float is bit-identical, so a fit is reused only for exactly the same input.
struct FitKey
{
    const void* level = nullptr; // the Shape of the LOD level (its vertices never change after load)
    int levelIndex = -1;         // the LOD index (an object holds one entry per level in use)
    int nPos = 0;                // vertex count of that level (guards a reloaded level at the same address)
    float transform[12] = {};    // the object's world matrix: aside, up, direction, position
    std::uint64_t terrain = 0;   // terrain height version (TerrainVersion)
    float damage = 0.0f;         // Object::GetTotalDammage

    bool operator==(const FitKey& o) const
    {
        return level == o.level && levelIndex == o.levelIndex && nPos == o.nPos && terrain == o.terrain &&
               std::memcmp(transform, o.transform, sizeof(transform)) == 0 &&
               std::memcmp(&damage, &o.damage, sizeof(damage)) == 0;
    }
    bool operator!=(const FitKey& o) const { return !(*this == o); }
};

// The key of one fit. `matrix` is the object's world matrix as rows: aside,
// up, direction, position (12 floats).
inline FitKey MakeFitKey(const void* level, int levelIndex, int nPos, const float matrix[12], std::uint64_t terrain,
                         float damage)
{
    FitKey key;
    key.level = level;
    key.levelIndex = levelIndex;
    key.nPos = nPos;
    std::memcpy(key.transform, matrix, sizeof(key.transform));
    key.terrain = terrain;
    key.damage = damage;
    return key;
}

// One fitted entry per (owner, LOD level). The owner is an object address: the
// key alone decides validity (an entry built for a dead object is reused only
// by an object at the same address with the same level, matrix, terrain and
// damage, whose fit is the same), and entries nobody draws age out.
template <class Payload>
class FitTable
{
  public:
    struct Entry
    {
        FitKey key;
        Payload payload;
        std::uint32_t lastUsed = 0;
    };

    // The entry of (owner, key.levelIndex) when its key equals `key`; nullptr
    // when there is none or it is stale (a stale entry stays until Store
    // replaces it or Sweep drops it).
    Entry* Find(const void* owner, const FitKey& key, std::uint32_t frame)
    {
        auto it = _owners.find(owner);
        if (it == _owners.end())
            return nullptr;
        for (Entry& e : it->second)
        {
            if (e.key.levelIndex != key.levelIndex)
                continue;
            if (e.key != key)
                return nullptr;
            e.lastUsed = frame;
            return &e;
        }
        return nullptr;
    }

    // Stores (or replaces) the entry of (owner, key.levelIndex).
    Entry& Store(const void* owner, const FitKey& key, Payload&& payload, std::uint32_t frame)
    {
        std::vector<Entry>& levels = _owners[owner];
        for (Entry& e : levels)
        {
            if (e.key.levelIndex == key.levelIndex)
            {
                e.key = key;
                e.payload = std::move(payload);
                e.lastUsed = frame;
                return e;
            }
        }
        levels.push_back(Entry{key, std::move(payload), frame});
        ++_entries;
        return levels.back();
    }

    // Drops every entry of an owner.
    void Forget(const void* owner)
    {
        auto it = _owners.find(owner);
        if (it == _owners.end())
            return;
        _entries -= it->second.size();
        _owners.erase(it);
    }

    // Drops the entries not used in the last `maxAge` frames; returns how many.
    std::size_t Sweep(std::uint32_t frame, std::uint32_t maxAge)
    {
        std::size_t dropped = 0;
        for (auto it = _owners.begin(); it != _owners.end();)
        {
            std::vector<Entry>& levels = it->second;
            for (std::size_t i = levels.size(); i-- > 0;)
            {
                if (frame - levels[i].lastUsed > maxAge)
                {
                    levels.erase(levels.begin() + static_cast<std::ptrdiff_t>(i));
                    ++dropped;
                }
            }
            it = levels.empty() ? _owners.erase(it) : std::next(it);
        }
        _entries -= dropped;
        return dropped;
    }

    void Clear()
    {
        _owners.clear();
        _entries = 0;
    }
    std::size_t Size() const { return _entries; }

  private:
    std::unordered_map<const void*, std::vector<Entry>> _owners;
    std::size_t _entries = 0;
};

// Folds a terrain description into a version: changes whenever the landscape,
// its grid, a strided sample of its heights or the edit count changes.
inline std::uint64_t MixVersion(std::uint64_t h, std::uint64_t v)
{
    h ^= v;
    h *= 1099511628211ull;
    return h;
}

// Counters of one frame (perf plan P0-15(e) `landFitDraws`, M2's dynamic
// upload counter). Until INFRA-C1's PerfCounters API carries them, they live
// here; LandFit::Stats() exposes them and POSEIDON_LANDFIT_STATS=1 logs a
// window average every 256 frames.
struct FrameStats
{
    int fits = 0;          // land fits run (Object::Animate's land branch, uncached draws included)
    int cacheBuilds = 0;   // fitted entries built (a first draw, or a stale key)
    int cacheDraws = 0;    // draws from an object's fitted buffer
    int cacheBounds = 0;   // AnimatedMinMax answered from the cache
    int dynUploads = 0;    // dynamic vertex buffer uploads (VertexBufferGL33)
    int dynUploadKB = 0;   // their size
    int dynSkipped = 0;    // dynamic buffer updates skipped because the vertices were unchanged
    int staticUploads = 0; // static vertex buffer re-uploads (dirty)
    int entries = 0;       // cache size at the end of the frame
};

// --- Engine side (Object.cpp) ---

// The frame's counters (the GL33 backend adds its uploads here).
FrameStats& Stats();
// Called once per frame by Scene::BeginObjects: closes the counters, refreshes
// the terrain version and ages out unused entries.
void BeginFrame(const Landscape* land);
// The current terrain height version.
std::uint64_t TerrainVersion();
// Landscape::HeightChange calls this (an editor reshaping the ground).
void NoteTerrainEdit();
// Drops every entry (Scene teardown); their buffers are freed here, on the
// render thread, while the context is alive.
void ClearAll();

// --- Engine side (ShapeDraw.cpp) ---

// True when Shape::Draw would draw `shape` with these clip flags and special
// flags from its vertex buffer (the TL path); the software path reads the CPU
// vertices instead, so a fitted copy is of no use there.
bool DrawsFromVertexBuffer(const Shape& shape, unsigned int clip, int spec);

// While alive, the next Shape::Draw draws from `buffer` instead of the shape's
// own vertex buffer (the object's fitted copy); the shape's buffer is restored
// when that draw returns.
class ScopedDrawBuffer
{
  public:
    explicit ScopedDrawBuffer(VertexBuffer* buffer);
    ~ScopedDrawBuffer();
    ScopedDrawBuffer(const ScopedDrawBuffer&) = delete;
    ScopedDrawBuffer& operator=(const ScopedDrawBuffer&) = delete;

  private:
    VertexBuffer* _previous;
};

} // namespace Poseidon::LandFit
