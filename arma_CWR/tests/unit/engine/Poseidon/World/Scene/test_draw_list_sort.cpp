#include <catch2/catch_test_macros.hpp>

#include <Poseidon/World/Scene/DrawListSort.hpp>
#include <Poseidon/Core/TaskPool.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <random>
#include <vector>

// W3-11: the scene's four draw-list sorts (SceneDraw.cpp) moved from QSort over
// Ref<SortObject> with per-entry comparators to DrawListSort::SortByKeys, which
// reads each entry's key once. The draw order must stay exactly the same, ties
// included: equal keys (stacked duplicates of one shape at one spot, entries at
// the same depth) draw in whatever order the unstable quicksort leaves them, and
// that order decides which of two coplanar surfaces wins and how translucent
// entries blend. These tests run the old comparators (copied from SceneDraw.cpp
// before W3-11, on a stand-in for SortObject) through QSort and the new keys
// through SortByKeys on the same random lists, full of duplicates, NaN and
// signed zeros, and compare the two sequences element by element.

using Poseidon::Foundation::QSort;
using Poseidon::Foundation::Ref;
using Poseidon::Foundation::RefCount;
namespace dls = Poseidon::DrawListSort;

namespace
{
struct FakeShape
{
    int faces; // LOD-0 face count
};

// The fields of SortObject the sorts read, plus what Object supplies.
struct FakeEntry : RefCount
{
    signed char passNum = 0;
    signed char drawLOD = 0;
    FakeShape* shape = nullptr; // Object::GetShape()
    bool hasObject = true;      // SortObject::object not null (the surface key tests it)
    int passOrder = 1;          // Object::PassOrder(drawLOD)
    float distance2 = 0.0f;
    float zCoord = 0.0f;
};

using Item = Ref<FakeEntry>;

// ---- The comparators as SceneDraw.cpp had them (afdb4d60 + W1-7). ----

int OldRevDist(const Item* p1, const Item* p0)
{
    const FakeEntry* o0 = *p0;
    const FakeEntry* o1 = *p1;
    float dif = o0->distance2 - o1->distance2;
    if (dif < 0)
    {
        return -1;
    }
    if (dif > 0)
    {
        return +1;
    }
    return 0;
}

int OldAlpha(const Item* p1, const Item* p0)
{
    return Poseidon::AlphaSort::CompareAlphaDepth((*p1)->zCoord, (*p0)->zCoord);
}

int OldShape(const Item* p1, const Item* p2)
{
    const FakeEntry* o1 = *p1;
    const FakeEntry* o2 = *p2;
    const Poseidon::ShapeDraw::ShapeDrawKey<FakeShape> k1{o1->passNum, o1->shape, o1->drawLOD, o1->distance2};
    const Poseidon::ShapeDraw::ShapeDrawKey<FakeShape> k2{o2->passNum, o2->shape, o2->drawLOD, o2->distance2};
    return Poseidon::ShapeDraw::CompareShapeDraw(k1, k2, [](FakeShape* s) { return s->faces; });
}

int OldSurface(const Item* p1, const Item* p2)
{
    const FakeEntry* o1 = *p1;
    const FakeEntry* o2 = *p2;
    const Poseidon::SurfaceDraw::SurfaceDrawKey k1{o1->hasObject ? o1->passOrder : 0,
                                                   o1->hasObject ? static_cast<const void*>(o1->shape) : nullptr,
                                                   o1->distance2};
    const Poseidon::SurfaceDraw::SurfaceDrawKey k2{o2->hasObject ? o2->passOrder : 0,
                                                   o2->hasObject ? static_cast<const void*>(o2->shape) : nullptr,
                                                   o2->distance2};
    return Poseidon::SurfaceDraw::CompareSurfaceDraw(k1, k2);
}

// ---- The new sorts, with the key makers SceneDraw.cpp uses. ----

void NewRevDist(Item* items, int n)
{
    static dls::Scratch<float, FakeEntry> scratch;
    dls::SortByKeys(
        items, n, [](const FakeEntry* o) { return o->distance2; },
        [](float a, float b) { return dls::CompareRevDistance(a, b); }, scratch);
}

void NewAlpha(Item* items, int n)
{
    static dls::Scratch<float, FakeEntry> scratch;
    dls::SortByKeys(
        items, n, [](const FakeEntry* o) { return o->zCoord; },
        [](float a, float b) { return dls::CompareAlphaKeys(a, b); }, scratch);
}

void NewShape(Item* items, int n)
{
    static dls::Scratch<dls::ShapeKey, FakeEntry> scratch;
    dls::SortByKeys(
        items, n, [](const FakeEntry* o)
        { return dls::ShapeKey{{o->passNum, o->shape, o->drawLOD, o->distance2}, o->shape ? o->shape->faces : 0}; },
        [](const dls::ShapeKey& a, const dls::ShapeKey& b) { return dls::CompareShapeKeys(a, b); }, scratch);
}

void NewSurface(Item* items, int n)
{
    static dls::Scratch<Poseidon::SurfaceDraw::SurfaceDrawKey, FakeEntry> scratch;
    dls::SortByKeys(
        items, n,
        [](const FakeEntry* o)
        {
            return Poseidon::SurfaceDraw::SurfaceDrawKey{o->hasObject ? o->passOrder : 0,
                                                         o->hasObject ? static_cast<const void*>(o->shape) : nullptr,
                                                         o->distance2};
        },
        [](const Poseidon::SurfaceDraw::SurfaceDrawKey& a, const Poseidon::SurfaceDraw::SurfaceDrawKey& b)
        { return dls::CompareSurfaceKeys(a, b); }, scratch);
}

struct Fixture
{
    std::vector<FakeShape> shapes;
    std::vector<Ref<FakeEntry>> owners; // keeps every entry alive, as Scene::_drawObjects does
};

float PickFloat(std::mt19937& rng, int mode)
{
    // few distinct values, so most keys tie; plus the awkward ones
    static const float kSmall[] = {0.0f, -0.0f, 1.0f, 4.0f, 9.0f, 100.0f, 2500.0f, 1.0e6f};
    std::uniform_int_distribution<int> pick(0, 99);
    const int p = pick(rng);
    if (mode == 0)
    {
        return kSmall[p % 8];
    }
    if (p < 2)
    {
        return std::numeric_limits<float>::quiet_NaN();
    }
    if (p < 4)
    {
        return std::numeric_limits<float>::infinity();
    }
    if (p < 50)
    {
        return kSmall[p % 8];
    }
    std::uniform_real_distribution<float> any(-50.0f, 4.0e6f);
    return any(rng);
}

std::vector<Item> MakeList(Fixture& w, std::mt19937& rng, int n, int mode)
{
    std::uniform_int_distribution<int> pct(0, 99);
    std::uniform_int_distribution<int> shapeIx(0, static_cast<int>(w.shapes.size()) - 1);
    std::vector<Item> list;
    for (int i = 0; i < n; i++)
    {
        if (!list.empty() && pct(rng) < 15)
        {
            // an exact duplicate entry of the last one (stacked duplicates)
            FakeEntry* dup = new FakeEntry(*list.back().GetRef());
            w.owners.emplace_back(dup);
            list.emplace_back(dup);
            continue;
        }
        FakeEntry* e = new FakeEntry;
        e->passNum = static_cast<signed char>(pct(rng) % 3);
        e->drawLOD = static_cast<signed char>(pct(rng) % 4);
        e->shape = &w.shapes[shapeIx(rng)];
        e->hasObject = pct(rng) < 95;
        e->passOrder = 1 + pct(rng) % 2;
        e->distance2 = PickFloat(rng, mode);
        e->zCoord = PickFloat(rng, mode);
        w.owners.emplace_back(e);
        list.emplace_back(e);
    }
    return list;
}

bool SameSequence(const std::vector<Item>& a, const std::vector<Item>& b)
{
    if (a.size() != b.size())
    {
        return false;
    }
    for (size_t i = 0; i < a.size(); i++)
    {
        if (a[i].GetRef() != b[i].GetRef())
        {
            return false;
        }
    }
    return true;
}

using OldCmp = int (*)(const Item*, const Item*);
using NewSort = void (*)(Item*, int);

// Runs old and new on copies of `list`; checks identical sequences and unchanged
// reference counts.
void CheckSame(const std::vector<Item>& list, OldCmp oldCmp, NewSort newSort)
{
    std::vector<Item> a = list;
    std::vector<Item> b = list;
    std::vector<int> counts;
    for (const Item& it : list)
    {
        counts.push_back(it->RefCounter());
    }
    QSort(a.data(), static_cast<int>(a.size()), oldCmp);
    newSort(b.data(), static_cast<int>(b.size()));
    REQUIRE(SameSequence(a, b));
    for (size_t i = 0; i < list.size(); i++)
    {
        REQUIRE(list[i]->RefCounter() == counts[i]);
    }
}

Fixture MakeFixture()
{
    Fixture w;
    // equal face counts on different shapes exercise the address tiebreak
    w.shapes = {{10}, {10}, {20}, {5}, {10}, {300}, {20}, {7}};
    return w;
}
} // namespace

TEST_CASE("DrawListSort: the four keyed sorts give QSort's order exactly, ties included", "[drawsort]")
{
    Fixture w = MakeFixture();
    std::mt19937 rng(0x5013u);
    const int sizes[] = {0, 1, 2, 3, 5, 8, 9, 10, 16, 17, 31, 100, 257, 1000, 4096};
    int lists = 0;
    for (int round = 0; round < 6; round++)
    {
        for (int n : sizes)
        {
            for (int mode = 0; mode < 2; mode++)
            {
                const std::vector<Item> list = MakeList(w, rng, n, mode);
                CheckSame(list, OldRevDist, NewRevDist);
                CheckSame(list, OldAlpha, NewAlpha);
                CheckSame(list, OldShape, NewShape);
                CheckSame(list, OldSurface, NewSurface);
                lists++;
            }
        }
    }
    CHECK(lists == 6 * 15 * 2);
}

TEST_CASE("DrawListSort: the test lists are sensitive to tie order", "[drawsort]")
{
    // Control: a stable sort with the same comparator is a correct sort too, but
    // it orders ties differently from the quicksort. If it never differed here,
    // the identity tests above would prove nothing about ties.
    Fixture w = MakeFixture();
    std::mt19937 rng(0x7135u);
    int differ = 0;
    for (int i = 0; i < 20; i++)
    {
        const std::vector<Item> list = MakeList(w, rng, 500, 0);
        std::vector<Item> a = list;
        std::vector<Item> b = list;
        QSort(a.data(), static_cast<int>(a.size()), OldShape);
        std::stable_sort(b.begin(), b.end(), [](const Item& x, const Item& y) { return OldShape(&x, &y) < 0; });
        differ += SameSequence(a, b) ? 0 : 1;
    }
    CHECK(differ > 0);
}

TEST_CASE("DrawListSort: Pass2 re-sorts Pass1's output exactly as before", "[drawsort]")
{
    // The surface sort runs on the shape-sorted list and the alpha sort on the
    // surface-sorted one, in place: the chain must match step by step.
    Fixture w = MakeFixture();
    std::mt19937 rng(77u);
    for (int i = 0; i < 40; i++)
    {
        const std::vector<Item> list = MakeList(w, rng, 50 + i * 97, i % 2);
        std::vector<Item> a = list;
        std::vector<Item> b = list;
        QSort(a.data(), static_cast<int>(a.size()), OldShape);
        NewShape(b.data(), static_cast<int>(b.size()));
        REQUIRE(SameSequence(a, b));
        QSort(a.data(), static_cast<int>(a.size()), OldSurface);
        NewSurface(b.data(), static_cast<int>(b.size()));
        REQUIRE(SameSequence(a, b));
        QSort(a.data(), static_cast<int>(a.size()), OldAlpha);
        NewAlpha(b.data(), static_cast<int>(b.size()));
        REQUIRE(SameSequence(a, b));
    }
}

TEST_CASE("DrawListSort: sorted, reversed and all-equal input", "[drawsort]")
{
    Fixture w = MakeFixture();
    std::mt19937 rng(3u);
    for (int n : {9, 64, 1000})
    {
        std::vector<Item> list = MakeList(w, rng, n, 1);
        // already in the shape order, and reversed
        QSort(list.data(), n, OldShape);
        CheckSame(list, OldShape, NewShape);
        CheckSame(list, OldRevDist, NewRevDist);
        std::vector<Item> rev(list.rbegin(), list.rend());
        CheckSame(rev, OldShape, NewShape);
        CheckSame(rev, OldAlpha, NewAlpha);
        // every key equal: the order is the quicksort's swap path alone
        std::vector<Item> same;
        for (int i = 0; i < n; i++)
        {
            FakeEntry* e = new FakeEntry;
            e->shape = &w.shapes[0];
            e->distance2 = 25.0f;
            e->zCoord = 25.0f;
            w.owners.emplace_back(e);
            same.emplace_back(e);
        }
        CheckSame(same, OldShape, NewShape);
        CheckSame(same, OldSurface, NewSurface);
        CheckSame(same, OldAlpha, NewAlpha);
        CheckSame(same, OldRevDist, NewRevDist);
    }
}

TEST_CASE("DrawListSort: the shape key compares like CmpShapeObj on every pair", "[drawsort]")
{
    Fixture w = MakeFixture();
    std::mt19937 rng(11u);
    const std::vector<Item> list = MakeList(w, rng, 300, 1);
    for (const Item& x : list)
    {
        for (const Item& y : list)
        {
            const dls::ShapeKey kx{{x->passNum, x->shape, x->drawLOD, x->distance2}, x->shape->faces};
            const dls::ShapeKey ky{{y->passNum, y->shape, y->drawLOD, y->distance2}, y->shape->faces};
            const int oldR = OldShape(&x, &y);
            const int newR = dls::CompareShapeKeys(kx, ky);
            REQUIRE((oldR > 0) == (newR > 0));
            REQUIRE((oldR < 0) == (newR < 0));
            REQUIRE(OldRevDist(&x, &y) == dls::CompareRevDistance(x->distance2, y->distance2));
            REQUIRE(OldAlpha(&x, &y) == dls::CompareAlphaKeys(x->zCoord, y->zCoord));
        }
    }
}

// ---- XP-cputhreads: SortByKeysParallel (the same order on worker threads) ----

namespace
{
// The four keyed sorts again, through SortByKeysParallel on a real pool, with the
// serial check on as well (it must count no difference).
struct ParallelSorts
{
    Poseidon::TaskPool& pool;
    int minParallel;
    int mismatches = 0;

    void RevDist(Item* items, int n)
    {
        static dls::ParallelSortScratch<float, FakeEntry> scratch;
        dls::SortByKeysParallel(
            items, n, [](const FakeEntry* o) { return o->distance2; },
            [](float a, float b) { return dls::CompareRevDistance(a, b); }, scratch, pool, minParallel, &mismatches);
    }
    void Alpha(Item* items, int n)
    {
        static dls::ParallelSortScratch<float, FakeEntry> scratch;
        dls::SortByKeysParallel(
            items, n, [](const FakeEntry* o) { return o->zCoord; },
            [](float a, float b) { return dls::CompareAlphaKeys(a, b); }, scratch, pool, minParallel, &mismatches);
    }
    void Shape(Item* items, int n)
    {
        static dls::ParallelSortScratch<dls::ShapeKey, FakeEntry> scratch;
        dls::SortByKeysParallel(
            items, n, [](const FakeEntry* o)
            { return dls::ShapeKey{{o->passNum, o->shape, o->drawLOD, o->distance2}, o->shape ? o->shape->faces : 0}; },
            [](const dls::ShapeKey& a, const dls::ShapeKey& b) { return dls::CompareShapeKeys(a, b); }, scratch, pool,
            minParallel, &mismatches);
    }
    void Surface(Item* items, int n)
    {
        static dls::ParallelSortScratch<Poseidon::SurfaceDraw::SurfaceDrawKey, FakeEntry> scratch;
        dls::SortByKeysParallel(
            items, n,
            [](const FakeEntry* o)
            {
                return Poseidon::SurfaceDraw::SurfaceDrawKey{
                    o->hasObject ? o->passOrder : 0, o->hasObject ? static_cast<const void*>(o->shape) : nullptr,
                    o->distance2};
            },
            [](const Poseidon::SurfaceDraw::SurfaceDrawKey& a, const Poseidon::SurfaceDraw::SurfaceDrawKey& b)
            { return dls::CompareSurfaceKeys(a, b); }, scratch, pool, minParallel, &mismatches);
    }
};

template <class F>
void CheckParallelSame(const std::vector<Item>& list, OldCmp oldCmp, F parallelSort)
{
    std::vector<Item> a = list;
    std::vector<Item> b = list;
    std::vector<int> counts;
    for (const Item& it : list)
    {
        counts.push_back(it->RefCounter());
    }
    QSort(a.data(), static_cast<int>(a.size()), oldCmp);
    parallelSort(b.data(), static_cast<int>(b.size()));
    REQUIRE(SameSequence(a, b));
    for (size_t i = 0; i < list.size(); i++)
    {
        REQUIRE(list[i]->RefCounter() == counts[i]);
    }
}
} // namespace

TEST_CASE("DrawListSort: the parallel sorts give QSort's order exactly, ties included", "[drawsort][xp-cputhreads]")
{
    Fixture w = MakeFixture();
    Poseidon::TaskPool pool(4);
    // minParallel 2 forces the parallel path on every list, small ones included.
    for (int minParallel : {2, 1024})
    {
        ParallelSorts ps{pool, minParallel};
        std::mt19937 rng(0x9a11u + static_cast<unsigned>(minParallel));
        const int sizes[] = {0, 1, 2, 3, 8, 9, 17, 100, 257, 258, 1000, 1023, 1024, 4096, 9000};
        for (int round = 0; round < 4; round++)
        {
            for (int n : sizes)
            {
                for (int mode = 0; mode < 2; mode++)
                {
                    const std::vector<Item> list = MakeList(w, rng, n, mode);
                    CheckParallelSame(list, OldRevDist, [&](Item* it, int k) { ps.RevDist(it, k); });
                    CheckParallelSame(list, OldAlpha, [&](Item* it, int k) { ps.Alpha(it, k); });
                    CheckParallelSame(list, OldShape, [&](Item* it, int k) { ps.Shape(it, k); });
                    CheckParallelSame(list, OldSurface, [&](Item* it, int k) { ps.Surface(it, k); });
                }
            }
        }
        CHECK(ps.mismatches == 0);
    }
}

TEST_CASE("DrawListSort: the parallel sort of sorted, reversed and all-equal input", "[drawsort][xp-cputhreads]")
{
    Fixture w = MakeFixture();
    Poseidon::TaskPool pool(8);
    ParallelSorts ps{pool, 2};
    std::mt19937 rng(5u);
    for (int n : {300, 5000})
    {
        std::vector<Item> list = MakeList(w, rng, n, 1);
        QSort(list.data(), n, OldShape);
        CheckParallelSame(list, OldShape, [&](Item* it, int k) { ps.Shape(it, k); });
        std::vector<Item> rev(list.rbegin(), list.rend());
        CheckParallelSame(rev, OldShape, [&](Item* it, int k) { ps.Shape(it, k); });
        CheckParallelSame(rev, OldAlpha, [&](Item* it, int k) { ps.Alpha(it, k); });
        std::vector<Item> same;
        for (int i = 0; i < n; i++)
        {
            FakeEntry* e = new FakeEntry;
            e->shape = &w.shapes[0];
            e->distance2 = 25.0f;
            e->zCoord = 25.0f;
            w.owners.emplace_back(e);
            same.emplace_back(e);
        }
        CheckParallelSame(same, OldShape, [&](Item* it, int k) { ps.Shape(it, k); });
        CheckParallelSame(same, OldRevDist, [&](Item* it, int k) { ps.RevDist(it, k); });
    }
    CHECK(ps.mismatches == 0);
}

TEST_CASE("DrawListSort: ParallelQSortWithContext matches QSortWithContext on ints", "[drawsort][xp-cputhreads]")
{
    // Equal values carry their original position, so any tie order change shows.
    struct V
    {
        int key;
        int pos;
    };
    Poseidon::TaskPool pool(6);
    dls::ParallelScratch scratch;
    std::mt19937 rng(1234u);
    auto cmp = [](const V* a, const V* b, const int*) { return a->key < b->key ? -1 : (a->key > b->key ? 1 : 0); };
    const int ctx = 0;
    for (int n : {2, 9, 300, 2049, 20000})
    {
        for (int range : {3, 50, 1000000})
        {
            for (int levels : {1, 2, 4, 6})
            {
                std::uniform_int_distribution<int> pick(0, range);
                std::vector<V> a(static_cast<size_t>(n));
                for (int i = 0; i < n; i++)
                {
                    a[static_cast<size_t>(i)] = V{pick(rng), i};
                }
                std::vector<V> b = a;
                Poseidon::Foundation::QSortWithContext(a.data(), n, &ctx, cmp);
                dls::ParallelQSortWithContext(b.data(), n, &ctx, cmp, pool, scratch, levels, 16);
                bool same = true;
                for (int i = 0; i < n; i++)
                {
                    same = same && a[static_cast<size_t>(i)].key == b[static_cast<size_t>(i)].key &&
                           a[static_cast<size_t>(i)].pos == b[static_cast<size_t>(i)].pos;
                }
                REQUIRE(same);
            }
        }
    }
}
