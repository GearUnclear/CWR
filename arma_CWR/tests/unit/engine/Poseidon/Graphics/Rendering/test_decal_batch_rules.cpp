#include <catch2/catch_test_macros.hpp>

#include <Poseidon/Graphics/Rendering/DecalBatchRules.hpp>
#include <PoseidonGL33/EngineGL33_Lighting.hpp>

#include <cstdint>
#include <regex>
#include <set>
#include <string>
#include <vector>

// Batched decals (perf W4-1): the rules EngineGL33_DecalBatch.cpp follows.
// A decal texture takes a layer of the array of its surface shape; consecutive
// decals of one spec, one array and one texture-derived state are one draw;
// the runs of one texture inside a batch are the old draws, in order.

using namespace Poseidon::render::decalbatch;

namespace
{
SurfaceShape MakeShape(int w, int h, int levels, unsigned int format, int bytes = 64 * 64)
{
    SurfaceShape s;
    s.w = w;
    s.h = h;
    s.levels = levels;
    s.internalFormat = format;
    s.bytes = bytes;
    return s;
}

SurfaceStamp MakeStamp(unsigned int handle, int id)
{
    SurfaceStamp s;
    s.handle = handle;
    s.creationId = id;
    return s;
}

constexpr unsigned int kDxt5 = 0x83F3;  // GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
constexpr unsigned int kRgba4 = 0x8056; // GL_RGBA4
} // namespace

TEST_CASE("Decal batch: textures of one shape share an array, a layer each", "[Graphics][DecalBatch]")
{
    LayerTable t;
    const SurfaceShape fire = MakeShape(64, 64, 7, kDxt5);
    // The fire's 48 animation frames: one array, layers 0 to 47 in first-use order.
    for (int i = 0; i < 48; ++i)
    {
        const LayerRef r = t.Acquire(1000 + i, fire, MakeStamp(10 + i, i));
        CAPTURE(i);
        REQUIRE(r.Valid());
        CHECK(r.array == 0);
        CHECK(r.layer == i);
        CHECK(r.copy); // a new layer is filled once
        CHECK(r.created == (i == 0));
    }
    CHECK(t.ArrayCount() == 1);
    CHECK(t.TextureCount() == 48);
    // The same texture on the same surface: the same layer, nothing to copy.
    const LayerRef again = t.Acquire(1007, fire, MakeStamp(17, 7));
    CHECK(again.array == 0);
    CHECK(again.layer == 7);
    CHECK_FALSE(again.copy);
    CHECK(again.growFrom == 0);
    // Another shape (the smoke's frames in another format) gets its own array.
    const LayerRef smoke = t.Acquire(2000, MakeShape(64, 64, 7, kRgba4), MakeStamp(99, 99));
    CHECK(smoke.array == 1);
    CHECK(smoke.layer == 0);
    CHECK(smoke.created);
    // A shape is the size, the mips and the format; any one differing splits.
    CHECK(t.Acquire(3000, MakeShape(32, 64, 7, kDxt5), MakeStamp(1, 1)).array == 2);
    CHECK(t.Acquire(3001, MakeShape(64, 64, 6, kDxt5), MakeStamp(2, 2)).array == 3);
    SurfaceShape ai = fire;
    ai.swizzleAI = true;
    CHECK(t.Acquire(3002, ai, MakeStamp(3, 3)).array == 4);
}

TEST_CASE("Decal batch: an array grows by doubling and keeps its layers", "[Graphics][DecalBatch]")
{
    LayerTable t;
    const SurfaceShape s = MakeShape(64, 64, 7, kDxt5);
    LayerRef first = t.Acquire(1, s, MakeStamp(1, 1));
    CHECK(first.created);
    CHECK(first.capacity == kInitialLayers);
    for (int i = 1; i < kInitialLayers; ++i)
        CHECK(t.Acquire(1 + i, s, MakeStamp(1 + i, i)).growFrom == 0);
    // The 17th texture: the storage grows from 16 to 32 layers.
    const LayerRef grow = t.Acquire(100, s, MakeStamp(100, 100));
    CHECK(grow.layer == kInitialLayers);
    CHECK(grow.growFrom == kInitialLayers);
    CHECK(grow.capacity == 2 * kInitialLayers);
    CHECK(t.ArrayAt(0).capacity == 2 * kInitialLayers);
}

TEST_CASE("Decal batch: a changed surface copies the layer again, in place", "[Graphics][DecalBatch]")
{
    LayerTable t;
    const SurfaceShape s = MakeShape(64, 64, 7, kDxt5);
    REQUIRE(t.Acquire(1, s, MakeStamp(5, 50)).copy);
    // The bank gave the texture another surface (another GL name, or the same
    // name recycled with a new creation id): same layer, copied again.
    const LayerRef moved = t.Acquire(1, s, MakeStamp(6, 51));
    CHECK(moved.layer == 0);
    CHECK(moved.copy);
    const LayerRef recycled = t.Acquire(1, s, MakeStamp(6, 52));
    CHECK(recycled.layer == 0);
    CHECK(recycled.copy);
    CHECK_FALSE(t.Acquire(1, s, MakeStamp(6, 52)).copy);
    // Back at another size: its old layer is freed and it moves to its new shape.
    const LayerRef small = t.Acquire(1, MakeShape(16, 16, 5, kDxt5), MakeStamp(7, 53));
    CHECK(small.array == 1);
    CHECK(small.copy);
    CHECK(t.Acquire(2, s, MakeStamp(8, 54)).layer == 0); // the freed layer is reused
}

TEST_CASE("Decal batch: a texture that cannot join gets no layer", "[Graphics][DecalBatch]")
{
    LayerTable t;
    // No surface shape (no GPU surface yet): no layer.
    CHECK_FALSE(t.Acquire(1, SurfaceShape{}, MakeStamp(0, -1)).Valid());
    // A full array: the texture keeps its own draw.
    const SurfaceShape big = MakeShape(1024, 1024, 11, kDxt5, kArrayBudgetBytes / 2);
    CHECK(MaxLayersFor(big) == 2);
    CHECK(t.Acquire(10, big, MakeStamp(1, 1)).Valid());
    CHECK(t.Acquire(11, big, MakeStamp(2, 2)).Valid());
    CHECK_FALSE(t.Acquire(12, big, MakeStamp(3, 3)).Valid());
    // A destroyed texture's layer is free for the next one.
    t.Forget(10);
    const LayerRef reuse = t.Acquire(12, big, MakeStamp(3, 3));
    CHECK(reuse.Valid());
    CHECK(reuse.layer == 0);
    CHECK(reuse.copy);
    // At most kMaxArrays shapes.
    LayerTable many;
    for (int i = 0; i < kMaxArrays; ++i)
        CHECK(many.Acquire(100 + i, MakeShape(8 + i, 8, 3, kDxt5), MakeStamp(i + 1, i)).Valid());
    CHECK_FALSE(many.Acquire(999, MakeShape(500, 8, 3, kDxt5), MakeStamp(77, 77)).Valid());
    // A forgotten texture at a reused address starts afresh.
    many.Forget(100);
    const LayerRef fresh = many.Acquire(100, MakeShape(8, 8, 3, kDxt5), MakeStamp(1, 0));
    CHECK(fresh.copy);
}

TEST_CASE("Decal batch: only consecutive decals of one spec, array and state join", "[Graphics][DecalBatch]")
{
    BatchKey open;
    open.spec = 0x1234;
    open.array = 0;
    open.stateClass = -1;
    BatchKey next = open;
    CHECK(Joins(open, next));
    next.spec = 0x1235;
    CHECK_FALSE(Joins(open, next)); // another pass state
    next = open;
    next.array = 1;
    CHECK_FALSE(Joins(open, next)); // another array: another texture binding
    next = open;
    next.stateClass = 3;
    CHECK_FALSE(Joins(open, next)); // the wet weather's or an interior's surface class differs
    BatchKey none;
    CHECK_FALSE(Joins(none, none)); // no batch is open
}

TEST_CASE("Decal batch: runs keep the queue order and merge only neighbours", "[Graphics][DecalBatch]")
{
    // Textures A A B A C C in queue order, 6 indices each.
    RunList runs;
    const std::uintptr_t order[] = {1, 1, 2, 1, 3, 3};
    int first = 0;
    for (std::uintptr_t tex : order)
    {
        runs.Add(tex, first, 6);
        first += 6;
    }
    REQUIRE(runs.Size() == 4);
    // A's two visits stay apart: the B between them is drawn between them.
    CHECK(runs[0].tex == 1);
    CHECK(runs[0].first == 0);
    CHECK(runs[0].count == 12);
    CHECK(runs[1].tex == 2);
    CHECK(runs[1].first == 12);
    CHECK(runs[2].tex == 1);
    CHECK(runs[2].first == 18);
    CHECK(runs[3].tex == 3);
    CHECK(runs[3].first == 24);
    CHECK(runs[3].count == 12);
    // The runs tile the batch's indices from 0 to the end without a gap.
    int next = 0;
    for (int i = 0; i < runs.Size(); ++i)
    {
        CHECK(runs[i].first == next);
        next += runs[i].count;
    }
    CHECK(next == first);
    // After a flush the index queue restarts at 0: a new run, even for the
    // same texture.
    runs.Clear();
    runs.Add(3, 0, 6);
    CHECK(runs.Size() == 1);
    runs.Add(3, 12, 6); // not adjacent: kept apart
    CHECK(runs.Size() == 2);
}

TEST_CASE("Decal batch: the array has its own unit and the object program reads it only on demand",
          "[Graphics][DecalBatch][remaster]")
{
    namespace gl = Poseidon::gl33lighting;
    bool row = false;
    for (const auto& s : gl::kLitSamplers)
    {
        if (std::string(s.name) == "decalLayers")
            row = s.unit == gl::kDecalLayerUnit;
        else
            CHECK(s.unit != gl::kDecalLayerUnit);
    }
    CHECK(row);
    // psNormalRemaster's texture read: the material texture unless a batched
    // decal draw sets decalLayerMode, then its layer of the array.
    const std::string normal = std::string(gl::RemasterFragmentCommon()) + gl::RemasterFragmentMain(0);
    CHECK(normal.find("uniform sampler2DArray decalLayers;") != std::string::npos);
    CHECK(normal.find("uniform float decalLayerMode;") != std::string::npos);
    CHECK(normal.find("vec4 t0 = decalLayerMode > 0.5 ? texture(decalLayers, vec3(vUV0, vUV1.x)) : "
                      "texture(tex0, vUV0);") != std::string::npos);
    // The fragment stage keeps within the 16 samplers GL 3.3 guarantees.
    std::set<std::string> declared;
    static const std::regex sampler(R"(uniform\s+[iu]?sampler\w+\s+(\w+)\s*;)");
    for (auto it = std::sregex_iterator(normal.begin(), normal.end(), sampler); it != std::sregex_iterator(); ++it)
        declared.insert((*it)[1].str());
    CAPTURE(declared.size());
    CHECK(declared.count("decalLayers") == 1);
    CHECK(declared.size() <= 16);
}
