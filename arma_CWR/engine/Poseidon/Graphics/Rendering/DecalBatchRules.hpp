#pragma once

// Batched decals (perf W4-1): the rules the GL33 backend's decal batch follows,
// kept free of GL so the unit tests can hold them (EngineGL33_DecalBatch.cpp
// drives them).
//
// A decal (Object::DrawDecal: smoke, cloud and fire puffs, a light's flare) is
// one screen quad with its own texture, so every decal used to be its own draw:
// the per-draw cost was about 5 to 7 us of GPU time per puff (perf survey M2
// section 6). The backend now copies each decal texture, once, into a layer of
// a texture array shared by every texture of the same surface shape (size, mip
// count, format), puts the layer in the quad's vertices and draws a run of
// consecutive decals as one draw. Blending is order-dependent, so the batch
// never reorders: it only merges decals that were consecutive in the queue,
// and one draw rasterises and blends its triangles in index order, exactly as
// the separate draws did.

#include <cstdint>
#include <unordered_map>
#include <vector>

namespace Poseidon::render::decalbatch
{

// The shape of a texture's GPU surface. Textures of one shape share an array:
// a layer must have the size, mip chain and texel format of its source for the
// array to sample exactly what the texture would.
struct SurfaceShape
{
    int w = 0;
    int h = 0;
    int levels = 0;
    unsigned int internalFormat = 0;
    bool swizzleAI = false; // the alpha-intensity format's (L, L, L, A) swizzle
    int bytes = 0;          // one layer, every level (sizes the array)

    bool operator==(const SurfaceShape& o) const
    {
        return w == o.w && h == o.h && levels == o.levels && internalFormat == o.internalFormat &&
               swizzleAI == o.swizzleAI;
    }
    bool operator!=(const SurfaceShape& o) const { return !(*this == o); }
    bool Valid() const { return w > 0 && h > 0 && levels > 0 && internalFormat != 0; }
};

// Which GPU surface a layer was copied from (the surface's GL name and its
// creation id; the bank recycles surfaces between textures, and a texture that
// lost its surface has another one). A changed stamp means the layer must be
// copied again.
struct SurfaceStamp
{
    unsigned int handle = 0;
    int creationId = -1;

    bool operator==(const SurfaceStamp& o) const { return handle == o.handle && creationId == o.creationId; }
    bool operator!=(const SurfaceStamp& o) const { return !(*this == o); }
};

// Where a texture lives: its array and layer, and what the caller must do
// before a draw samples it.
struct LayerRef
{
    int array = -1;
    int layer = -1;
    bool copy = false;    // the layer must be filled from the texture's surface
    int growFrom = 0;     // > 0: the array's storage must grow from this many layers ...
    int capacity = 0;     // ... to this many (the array's capacity after the call)
    bool created = false; // the array is new (no storage yet; capacity layers)
    bool Valid() const { return array >= 0 && layer >= 0; }
};

// Layers per array: start small, double on demand, never past the cap or a
// memory budget per array (a large texture gets fewer layers).
constexpr int kInitialLayers = 16;
constexpr int kMaxLayers = 256;
constexpr int kMaxArrays = 16;
constexpr int kArrayBudgetBytes = 32 * 1024 * 1024;

inline int MaxLayersFor(const SurfaceShape& shape)
{
    if (shape.bytes <= 0)
        return kMaxLayers;
    const long long n = kArrayBudgetBytes / static_cast<long long>(shape.bytes);
    return n < 1 ? 1 : (n > kMaxLayers ? kMaxLayers : static_cast<int>(n));
}

// The texture -> (array, layer) table. A texture is identified by an opaque
// key (the backend's texture pointer); Forget drops it when the texture is
// destroyed, so a new texture at the same address starts afresh.
class LayerTable
{
  public:
    struct Array
    {
        SurfaceShape shape;
        int capacity = 0;
        int maxLayers = 0;
        int used = 0;                // layers handed out so far (high water mark)
        std::vector<int> freeLayers; // layers of forgotten textures
    };

    LayerRef Acquire(std::uintptr_t tex, const SurfaceShape& shape, const SurfaceStamp& stamp)
    {
        LayerRef r;
        if (!shape.Valid())
            return r;
        auto it = _entries.find(tex);
        if (it != _entries.end())
        {
            Entry& e = it->second;
            if (_arrays[e.array].shape == shape)
            {
                r.array = e.array;
                r.layer = e.layer;
                r.capacity = _arrays[e.array].capacity;
                r.copy = e.stamp != stamp;
                e.stamp = stamp;
                return r;
            }
            // The texture came back at another size or format: its old layer
            // is free again and it takes one in the array of its new shape.
            Release(e);
            _entries.erase(it);
        }
        int a = FindArray(shape);
        if (a < 0)
        {
            if (static_cast<int>(_arrays.size()) >= kMaxArrays)
                return r;
            Array arr;
            arr.shape = shape;
            arr.maxLayers = MaxLayersFor(shape);
            arr.capacity = arr.maxLayers < kInitialLayers ? arr.maxLayers : kInitialLayers;
            _arrays.push_back(arr);
            a = static_cast<int>(_arrays.size()) - 1;
            r.created = true;
        }
        Array& arr = _arrays[a];
        int layer = -1;
        if (!arr.freeLayers.empty())
        {
            layer = arr.freeLayers.back();
            arr.freeLayers.pop_back();
        }
        else if (arr.used < arr.maxLayers)
        {
            layer = arr.used++;
        }
        if (layer < 0)
            return r; // full: the texture draws on its own, as before
        if (layer >= arr.capacity)
        {
            int grown = arr.capacity * 2;
            while (grown <= layer)
                grown *= 2;
            if (grown > arr.maxLayers)
                grown = arr.maxLayers;
            if (!r.created)
                r.growFrom = arr.capacity;
            arr.capacity = grown;
        }
        _entries[tex] = Entry{a, layer, stamp};
        r.array = a;
        r.layer = layer;
        r.copy = true;
        r.capacity = arr.capacity;
        return r;
    }

    void Forget(std::uintptr_t tex)
    {
        auto it = _entries.find(tex);
        if (it == _entries.end())
            return;
        Release(it->second);
        _entries.erase(it);
    }

    void Clear()
    {
        _entries.clear();
        _arrays.clear();
    }

    int ArrayCount() const { return static_cast<int>(_arrays.size()); }
    const Array& ArrayAt(int a) const { return _arrays[a]; }
    int TextureCount() const { return static_cast<int>(_entries.size()); }

  private:
    struct Entry
    {
        int array = -1;
        int layer = -1;
        SurfaceStamp stamp;
    };

    int FindArray(const SurfaceShape& shape) const
    {
        for (int i = 0; i < static_cast<int>(_arrays.size()); ++i)
            if (_arrays[i].shape == shape)
                return i;
        return -1;
    }

    void Release(const Entry& e)
    {
        if (e.array >= 0 && e.array < static_cast<int>(_arrays.size()))
            _arrays[e.array].freeLayers.push_back(e.layer);
    }

    std::unordered_map<std::uintptr_t, Entry> _entries;
    std::vector<Array> _arrays;
};

// What makes two consecutive decals one draw: the same spec flags (so the same
// pass state), the same array, and the same texture-derived state (the wet
// weather's surface class and a vehicle interior's glass mark, both read from
// the batch's first texture; -1 while neither is in use, which makes every
// texture equal).
struct BatchKey
{
    int spec = 0;
    int array = -1;
    int stateClass = -1;

    bool operator==(const BatchKey& o) const
    {
        return spec == o.spec && array == o.array && stateClass == o.stateClass;
    }
};

inline bool Joins(const BatchKey& open, const BatchKey& next)
{
    return open.array >= 0 && open == next;
}

// Runs of consecutive decals with one texture inside a batch, in queue order:
// the fallback when the bound program cannot read the array (the classic
// lighting set) draws one run per draw, the draws the decals had before.
class RunList
{
  public:
    struct Run
    {
        std::uintptr_t tex = 0;
        int first = 0; // first index in the batch's index queue
        int count = 0; // indices
    };

    void Add(std::uintptr_t tex, int first, int count)
    {
        if (!_runs.empty() && _runs.back().tex == tex && _runs.back().first + _runs.back().count == first)
        {
            _runs.back().count += count;
            return;
        }
        _runs.push_back(Run{tex, first, count});
    }
    void Clear() { _runs.clear(); }
    int Size() const { return static_cast<int>(_runs.size()); }
    const Run& operator[](int i) const { return _runs[i]; }
    bool Empty() const { return _runs.empty(); }

  private:
    std::vector<Run> _runs;
};

} // namespace Poseidon::render::decalbatch
