#include <catch2/catch_test_macros.hpp>

#include <Poseidon/Foundation/Algorithms/Qsort.hpp>
#include <Poseidon/World/Scene/ShapeDrawOrder.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <random>
#include <set>
#include <utility>
#include <vector>

// W1-7: the Pass1 draw-list comparator (CmpShapeObj in SceneDraw.cpp) ordered shapes by
// `(int)((intptr_t)s2 - (intptr_t)s1)`. On 64-bit builds that truncates the pointer
// difference to its low 32 bits, so the sort was not transitive for shapes 2 GiB or more
// apart and treated shapes a multiple of 4 GiB apart as one shape; QSort could then leave
// one shape's objects in several places (more instanced runs). These tests pin the fixed
// ordering (ShapeDrawOrder.hpp): a strict weak ordering over addresses that differ in the
// high 32 bits, equal keys equal, and the same direction as the old code wherever the
// old code was sound.

using Poseidon::Foundation::QSort;
using Poseidon::ShapeDraw::CompareShapeDraw;
using Poseidon::ShapeDraw::CompareShapeIdentity;

namespace
{
struct FakeShape
{
};
using Key = Poseidon::ShapeDraw::ShapeDrawKey<FakeShape>;

// Fabricated, never dereferenced shape identities. Several differ only in the high 32
// bits; others sit 2 GiB or more apart inside one 4 GiB window.
constexpr std::uintptr_t kAddrs[] = {
    0x0000000000001000ull, 0x0000000080001000ull, 0x00000000F0001000ull, 0x0000000100001000ull,
    0x0000000200001000ull, 0x0000000300001000ull, 0x00007FF600001000ull, 0x00007FF680000000ull,
};
// Face counts per address; equal values force the identity tie-break between shapes.
constexpr int kFaces[] = {12, 12, 40, 12, 40, 12, 7, 12};
constexpr int kNumShapes = static_cast<int>(sizeof(kAddrs) / sizeof(kAddrs[0]));

FakeShape* ShapeAt(std::uintptr_t addr)
{
    return reinterpret_cast<FakeShape*>(addr);
}

int Faces(FakeShape* s)
{
    const std::uintptr_t a = reinterpret_cast<std::uintptr_t>(s);
    for (int i = 0; i < kNumShapes; ++i)
    {
        if (kAddrs[i] == a)
        {
            return kFaces[i];
        }
    }
    return 0;
}

int Cmp(const Key& a, const Key& b)
{
    return CompareShapeDraw(a, b, Faces);
}

int CmpPtr(const Key* a, const Key* b)
{
    return Cmp(*a, *b);
}

int Sign(int v)
{
    return (v > 0) - (v < 0);
}

// The comparator as it was before W1-7 (SceneDraw.cpp at afdb4d60), on the same key.
int LegacyCmp(const Key& o1, const Key& o2)
{
    int sDif = o1.passNum - o2.passNum;
    if (sDif)
    {
        return sDif;
    }
    sDif = static_cast<int>(reinterpret_cast<std::intptr_t>(o2.shape) - reinterpret_cast<std::intptr_t>(o1.shape));
    if (sDif)
    {
        const int cDiff = Faces(o1.shape) - Faces(o2.shape);
        if (cDiff)
        {
            return cDiff;
        }
        return sDif;
    }
    sDif = o2.drawLOD - o1.drawLOD;
    if (sDif)
    {
        return sDif;
    }
    const float fDif = o2.distance2 - o1.distance2;
    if (fDif < 0)
    {
        return -1;
    }
    if (fDif > 0)
    {
        return +1;
    }
    return 0;
}

std::vector<Key> AllKeys()
{
    std::vector<Key> keys;
    const int passes[] = {0, 1};
    const int lods[] = {0, 2};
    const float dists[] = {1.0f, 4.0f, 9.0f};
    for (int p : passes)
    {
        for (std::uintptr_t addr : kAddrs)
        {
            for (int lod : lods)
            {
                for (float d : dists)
                {
                    keys.push_back(Key{p, ShapeAt(addr), lod, d});
                }
            }
        }
    }
    return keys;
}

bool SameKey(const Key& a, const Key& b)
{
    return a.passNum == b.passNum && a.shape == b.shape && a.drawLOD == b.drawLOD && a.distance2 == b.distance2;
}
} // namespace

TEST_CASE("CompareShapeIdentity: total order over addresses that differ in the high 32 bits", "[scene][shapesort]")
{
    for (std::uintptr_t a : kAddrs)
    {
        REQUIRE(CompareShapeIdentity(ShapeAt(a), ShapeAt(a)) == 0);
        for (std::uintptr_t b : kAddrs)
        {
            const int ab = CompareShapeIdentity(ShapeAt(a), ShapeAt(b));
            REQUIRE(Sign(ab) == -Sign(CompareShapeIdentity(ShapeAt(b), ShapeAt(a))));
            REQUIRE((ab == 0) == (a == b));
            // higher address first, the direction the old difference meant
            if (a != b)
            {
                REQUIRE((ab < 0) == (a > b));
            }
            for (std::uintptr_t c : kAddrs)
            {
                if (ab < 0 && CompareShapeIdentity(ShapeAt(b), ShapeAt(c)) < 0)
                {
                    REQUIRE(CompareShapeIdentity(ShapeAt(a), ShapeAt(c)) < 0);
                }
            }
        }
    }
}

TEST_CASE("Legacy truncated comparator was broken on these addresses", "[scene][shapesort]")
{
    // Documents the defect the fix removes (no engine code runs the legacy version).
    const Key a{0, ShapeAt(0x0000000100001000ull), 0, 1.0f}; // 12 faces
    const Key c{0, ShapeAt(0x0000000300001000ull), 0, 5.0f}; // 12 faces
    // Two different shapes exactly 8 GiB apart: the truncated difference is 0, so the
    // legacy comparator treated them as one shape and fell through to the distance key.
    REQUIRE(LegacyCmp(a, c) > 0);
    REQUIRE(Cmp(a, c) > 0); // same sign here by luck of the distance key
    const Key c2{0, ShapeAt(0x0000000300001000ull), 0, 0.5f};
    REQUIRE(LegacyCmp(a, c2) < 0); // legacy: a before c2 because a is farther
    REQUIRE(Cmp(a, c2) > 0);       // fixed: the higher address (c2's shape) first

    // Non-transitive cycle inside one 4 GiB window (equal face counts, so the identity
    // key decides): x = 0x00001000, y = 0x80001000 (2 GiB above x), z = 0x100001000.
    const Key x{0, ShapeAt(0x0000000000001000ull), 0, 1.0f};
    const Key y{0, ShapeAt(0x0000000080001000ull), 0, 1.0f};
    const Key z{0, ShapeAt(0x0000000100001000ull), 0, 1.0f};
    // y - x = 0x80000000 truncates to INT_MIN: legacy says x before y (wrong direction).
    REQUIRE(LegacyCmp(x, y) < 0);
    // z - y = 0x80000000 truncates to INT_MIN: legacy says y before z.
    REQUIRE(LegacyCmp(y, z) < 0);
    // z - x = 0x100000000 truncates to 0: legacy calls x and z the same shape.
    REQUIRE(LegacyCmp(x, z) == 0);
    // The fixed comparator orders them by address, highest first.
    REQUIRE(Cmp(z, y) < 0);
    REQUIRE(Cmp(y, x) < 0);
    REQUIRE(Cmp(z, x) < 0);
}

TEST_CASE("CompareShapeDraw: strict weak ordering over fabricated draw lists", "[scene][shapesort]")
{
    // Counted in plain loops (about 900k triples), asserted once per property.
    const std::vector<Key> keys = AllKeys();
    int irreflexive = 0, antisymmetric = 0, equalMeansSame = 0, transitiveLess = 0, transitiveEqual = 0;
    for (const Key& a : keys)
    {
        irreflexive += Cmp(a, a) != 0;
        for (const Key& b : keys)
        {
            const int ab = Cmp(a, b);
            antisymmetric += Sign(ab) != -Sign(Cmp(b, a));
            // every field is part of the key, so equal means equal keys
            equalMeansSame += (ab == 0) != SameKey(a, b);
            for (const Key& c : keys)
            {
                const int bc = Cmp(b, c);
                if (ab < 0 && bc < 0)
                {
                    transitiveLess += !(Cmp(a, c) < 0);
                }
                if (ab == 0 && bc == 0)
                {
                    transitiveEqual += Cmp(a, c) != 0;
                }
            }
        }
    }
    REQUIRE(keys.size() == 96u);
    REQUIRE(irreflexive == 0);
    REQUIRE(antisymmetric == 0);
    REQUIRE(equalMeansSame == 0);
    REQUIRE(transitiveLess == 0);
    REQUIRE(transitiveEqual == 0);
}

TEST_CASE("CompareShapeDraw: equal keys compare equal, including duplicates", "[scene][shapesort]")
{
    const Key k{1, ShapeAt(0x00007FF600001000ull), 2, 42.0f};
    const Key copy = k;
    REQUIRE(Cmp(k, copy) == 0);
    REQUIRE(Cmp(copy, k) == 0);
    const Key otherDist{1, ShapeAt(0x00007FF600001000ull), 2, 42.5f};
    REQUIRE(Cmp(k, otherDist) != 0);
}

TEST_CASE("CompareShapeDraw: keeps the legacy direction wherever the legacy code was sound", "[scene][shapesort]")
{
    // Pass ascending, simpler shapes first, higher address first, coarse LOD first, far
    // first: the same signs the old comparator gave when shapes were under 2 GiB apart.
    const std::vector<Key> keys = AllKeys();
    int compared = 0, disagreements = 0;
    for (const Key& a : keys)
    {
        for (const Key& b : keys)
        {
            const std::intptr_t gap =
                reinterpret_cast<std::intptr_t>(a.shape) - reinterpret_cast<std::intptr_t>(b.shape);
            if (gap > -0x80000000ll && gap < 0x80000000ll)
            {
                ++compared;
                disagreements += Sign(Cmp(a, b)) != Sign(LegacyCmp(a, b));
            }
        }
    }
    REQUIRE(compared > 0);
    REQUIRE(disagreements == 0);
    REQUIRE(Cmp(Key{0, ShapeAt(kAddrs[2]), 0, 1.0f}, Key{1, ShapeAt(kAddrs[0]), 0, 1.0f}) < 0); // pass
    REQUIRE(Cmp(Key{0, ShapeAt(kAddrs[6]), 0, 1.0f}, Key{0, ShapeAt(kAddrs[0]), 0, 1.0f}) < 0); // 7 < 12 faces
    REQUIRE(Cmp(Key{0, ShapeAt(kAddrs[0]), 2, 1.0f}, Key{0, ShapeAt(kAddrs[0]), 0, 1.0f}) < 0); // coarse LOD
    REQUIRE(Cmp(Key{0, ShapeAt(kAddrs[0]), 0, 9.0f}, Key{0, ShapeAt(kAddrs[0]), 0, 1.0f}) < 0); // far first
}

TEST_CASE("QSort with CompareShapeDraw keeps each shape in one contiguous run", "[scene][shapesort]")
{
    std::vector<Key> keys = AllKeys();
    // duplicates: stacked copies of one object (same shape, LOD and distance)
    const std::vector<Key> base = keys;
    keys.insert(keys.end(), base.begin(), base.end());
    std::mt19937 rng(20261031u);
    for (int round = 0; round < 16; ++round)
    {
        std::shuffle(keys.begin(), keys.end(), rng);
        QSort(keys.data(), static_cast<int>(keys.size()), CmpPtr);
        for (std::size_t i = 1; i < keys.size(); ++i)
        {
            REQUIRE(Cmp(keys[i - 1], keys[i]) <= 0);
        }
        std::set<std::pair<int, FakeShape*>> seen;
        int runs = 0;
        for (std::size_t i = 0; i < keys.size(); ++i)
        {
            if (i == 0 || keys[i].passNum != keys[i - 1].passNum || keys[i].shape != keys[i - 1].shape)
            {
                ++runs;
                REQUIRE(seen.insert({keys[i].passNum, keys[i].shape}).second);
            }
        }
        REQUIRE(runs == 2 * kNumShapes);
    }
}
