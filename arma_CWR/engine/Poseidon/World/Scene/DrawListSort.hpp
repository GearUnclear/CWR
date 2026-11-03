#pragma once

#include <Poseidon/Foundation/Algorithms/Qsort.hpp>
#include <Poseidon/Foundation/Types/Pointers.hpp>
#include <Poseidon/World/Scene/AlphaSortOrder.hpp>
#include <Poseidon/World/Scene/ShapeDrawOrder.hpp>
#include <Poseidon/World/Scene/SurfaceDrawOrder.hpp>

#include <algorithm>
#include <cstdint>
#include <vector>

namespace Poseidon::DrawListSort
{

// W3-11: the scene sorts its draw list four times a frame (SceneDraw.cpp: the
// occluder list by distance, Pass1 by shape, Pass2 by surface order and by alpha
// depth), each with Foundation::QSort over Ref<SortObject> and a comparator that
// reads the SortObject, its Object and, for the shape order, the LOD-0 face count
// of both shapes on every comparison. With 4,000 to 8,000 entries that is about
// 100,000 comparisons per sort, each a few dependent loads from objects spread
// over the heap, plus a std::swap of Ref<> per exchange (Ref has no move, so a
// swap is three AddRef and three Release, atomic, on two more cache lines):
// 1.6 to 1.9 ms per sort at the long views (M2 3a).
//
// SortByKeys reads each entry's key once into a contiguous array, sorts that
// array with the same quicksort (QSortWithContext is QSort's algorithm line for
// line: middle pivot, the same partition loop, insertion sort at 8 or fewer, the
// same stack order) and then puts the entries in the order the keys ended in.
// The quicksort's moves depend only on the comparison results, and a key
// comparison returns what the entry comparison returned (the same pure function
// of the same values), so the result is the order QSort gives, ties included,
// for any input, NaN keys included. The entries are moved as raw pointers
// (Ref::SetRef): the list holds the same references afterwards, so no count
// changes.

template <class Key>
struct Entry
{
    Key key;
    int index; // position in the list before the sort
};

// Reusable buffers, so a frame's sorts allocate nothing once warm.
template <class Key, class T>
struct Scratch
{
    std::vector<Entry<Key>> keys;
    std::vector<T*> items;
};

// Sorts items[0, n) into exactly the order
//   QSort(items, n, [](a, b) { return compare(makeKey(*a), makeKey(*b)); })
// gives. makeKey(T*) -> Key; compare(const Key&, const Key&) -> int (only its
// sign is used, as QSort uses it).
template <class T, class Key, class MakeKey, class Compare>
void SortByKeys(Foundation::Ref<T>* items, int n, MakeKey makeKey, Compare compare, Scratch<Key, T>& scratch)
{
    if (n < 2)
    {
        return;
    }
    scratch.keys.resize(n);
    Entry<Key>* keys = scratch.keys.data();
    for (int i = 0; i < n; i++)
    {
        keys[i].key = makeKey(items[i].GetRef());
        keys[i].index = i;
    }
    Foundation::QSortWithContext(keys, n, &compare, [](const Entry<Key>* a, const Entry<Key>* b, const Compare* c)
                                 { return (*c)(a->key, b->key); });
    scratch.items.resize(n);
    T** sorted = scratch.items.data();
    for (int i = 0; i < n; i++)
    {
        sorted[i] = items[keys[i].index].GetRef();
    }
    for (int i = 0; i < n; i++)
    {
        items[i].SetRef(sorted[i]);
    }
}

// ---- XP-cputhreads (POSEIDON_XP_CPUTHREADS): the same sort on worker threads ----
//
// QSortWithContext partitions a range around its middle element and then sorts
// the two sides; each side's sort touches only that side's elements and its
// moves depend only on that side's contents. So partitioning the top levels
// exactly as QSortWithContext does (PartitionStep is its partition loop line for
// line) and then running QSortWithContext on every remaining side, in any order
// and on any thread, leaves the array exactly as one QSortWithContext call
// does, ties and NaN keys included. The key reads and the final gather are pure
// per entry and run on the workers too. The pool needs
// ParallelFor(uint32_t count, f(start, end)) (Poseidon::TaskPool).

struct SortRange
{
    int lo; // first index
    int hi; // last index (inclusive)
};

// One partition step of QSortWithContext on [lo, hi] (more than QSortCutoff
// entries). The sides it would go on to sort are returned in side[0] and side[1];
// a side it would not sort (fewer than two entries) has hi < lo.
template <class Type, class ContextType, class Compare>
inline void PartitionStep(Type* base, SortRange r, ContextType context, Compare comp, SortRange (&side)[2])
{
    namespace qd = Foundation::PoseidonBase::Algorithms::Detail;
    Type* lo = base + r.lo;
    Type* hi = base + r.hi;
    const unsigned size = static_cast<unsigned>((hi - lo) + 1);
    Type* mid = lo + (size / 2);
    qd::Swap(mid, lo);
    Type* loguy = lo;
    Type* higuy = hi + 1;
    for (;;)
    {
        do
        {
            loguy++;
        } while (loguy <= hi && comp(loguy, lo, context) <= 0);
        do
        {
            higuy--;
        } while (higuy > lo && comp(higuy, lo, context) >= 0);
        if (higuy < loguy)
        {
            break;
        }
        qd::Swap(loguy, higuy);
    }
    qd::Swap(lo, higuy);
    side[0] = SortRange{r.lo, -1};
    side[1] = SortRange{r.lo, -1};
    if (lo + 1 < higuy)
    {
        side[0] = SortRange{r.lo, static_cast<int>(higuy - 1 - base)};
    }
    if (loguy < hi)
    {
        side[1] = SortRange{static_cast<int>(loguy - base), r.hi};
    }
}

struct ParallelScratch
{
    std::vector<SortRange> cur, next;
};

// QSortWithContext(base, num, context, comp), with up to `levels` partition
// levels split over the pool and every remaining side sorted as one task,
// largest first. A range is split further only while it holds more than
// leafMin entries and more than num / 32 (a middle pivot can leave one side
// much larger than the other, and that side would bound the whole sort).
template <class Pool, class Type, class ContextType, class Compare>
void ParallelQSortWithContext(Type* base, int num, ContextType context, Compare comp, Pool& pool,
                              ParallelScratch& scratch, int levels = 10, int leafMin = 256)
{
    if (num < 2)
    {
        return;
    }
    const int cutoff = static_cast<int>(Foundation::PoseidonBase::Algorithms::Detail::QSortCutoff);
    int keep = leafMin > cutoff ? leafMin : cutoff;
    keep = keep > num / 32 ? keep : num / 32;
    std::vector<SortRange>& cur = scratch.cur;
    std::vector<SortRange>& next = scratch.next;
    cur.assign(1, SortRange{0, num - 1});
    for (int level = 0; level < levels; ++level)
    {
        bool split = false;
        for (const SortRange& r : cur)
        {
            split = split || (r.hi - r.lo + 1) > keep;
        }
        if (!split)
        {
            break;
        }
        next.assign(cur.size() * 2, SortRange{0, -1});
        auto step = [&](uint32_t i)
        {
            const SortRange r = cur[i];
            if (r.hi - r.lo + 1 <= keep)
            {
                next[2 * i] = r; // a leaf: sorted whole below
                return;
            }
            SortRange side[2];
            PartitionStep(base, r, context, comp, side);
            next[2 * i] = side[0];
            next[2 * i + 1] = side[1];
        };
        if (cur.size() == 1)
        {
            step(0);
        }
        else
        {
            pool.ParallelFor(static_cast<uint32_t>(cur.size()),
                             [&](uint32_t s, uint32_t e)
                             {
                                 for (uint32_t i = s; i < e; ++i)
                                 {
                                     step(i);
                                 }
                             });
        }
        cur.clear();
        for (const SortRange& r : next)
        {
            if (r.hi > r.lo)
            {
                cur.push_back(r);
            }
        }
    }
    // Largest first, so the long sides start at once (the order the sides are
    // sorted in does not change the result).
    std::sort(cur.begin(), cur.end(),
              [](const SortRange& a, const SortRange& b) { return (a.hi - a.lo) > (b.hi - b.lo); });
    pool.ParallelFor(static_cast<uint32_t>(cur.size()),
                     [&](uint32_t s, uint32_t e)
                     {
                         for (uint32_t i = s; i < e; ++i)
                         {
                             Foundation::QSortWithContext(base + cur[i].lo, cur[i].hi - cur[i].lo + 1, context, comp);
                         }
                     });
}

// Reusable buffers of SortByKeysParallel.
template <class Key, class T>
struct ParallelSortScratch
{
    Scratch<Key, T> main;
    ParallelScratch ranges;
    std::vector<Entry<Key>> verify; // the serial sort's keys, POSEIDON_XP_CPUTHREADS_VERIFY only
};

// Exactly SortByKeys (same final order), with the key reads, the quicksort and
// the gather on the pool. Lists shorter than `minParallel` sort serially. When
// `mismatches` is not null the serial sort runs too, on a copy of the keys, and
// the number of positions whose entry differs is added to it.
template <class Pool, class T, class Key, class MakeKey, class Compare>
void SortByKeysParallel(Foundation::Ref<T>* items, int n, MakeKey makeKey, Compare compare,
                        ParallelSortScratch<Key, T>& scratch, Pool& pool, int minParallel = 1024,
                        int* mismatches = nullptr)
{
    if (n < 2)
    {
        return;
    }
    if (n < minParallel)
    {
        SortByKeys(items, n, makeKey, compare, scratch.main);
        return;
    }
    scratch.main.keys.resize(n);
    Entry<Key>* keys = scratch.main.keys.data();
    pool.ParallelFor(static_cast<uint32_t>(n),
                     [&](uint32_t s, uint32_t e)
                     {
                         for (uint32_t i = s; i < e; ++i)
                         {
                             keys[i].key = makeKey(items[i].GetRef());
                             keys[i].index = static_cast<int>(i);
                         }
                     });
    auto cmp = [](const Entry<Key>* a, const Entry<Key>* b, const Compare* c) { return (*c)(a->key, b->key); };
    if (mismatches)
    {
        scratch.verify.assign(keys, keys + n);
    }
    ParallelQSortWithContext(keys, n, &compare, cmp, pool, scratch.ranges);
    if (mismatches)
    {
        Entry<Key>* ref = scratch.verify.data();
        Foundation::QSortWithContext(ref, n, &compare, cmp);
        int bad = 0;
        for (int i = 0; i < n; i++)
        {
            bad += ref[i].index != keys[i].index ? 1 : 0;
        }
        *mismatches += bad;
    }
    scratch.main.items.resize(n);
    T** sorted = scratch.main.items.data();
    pool.ParallelFor(static_cast<uint32_t>(n),
                     [&](uint32_t s, uint32_t e)
                     {
                         for (uint32_t i = s; i < e; ++i)
                         {
                             sorted[i] = items[keys[i].index].GetRef();
                         }
                     });
    for (int i = 0; i < n; i++)
    {
        items[i].SetRef(sorted[i]);
    }
}

// ---- The four keys and their comparisons. Each returns exactly what the entry
// comparator it replaces returned (SceneDraw.cpp before W3-11). ----

// CmpRevDistObj(p1, p0): the occluder list, nearest first, by the sign of a
// float difference (second minus first; a NaN difference compares equal).
inline int CompareRevDistance(float first, float second)
{
    float dif = second - first;
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

// CmpShapeObj: ShapeDraw::CompareShapeDraw with the LOD-0 face count read once
// per entry instead of once per comparison. The face count is used only when the
// shapes differ, and then it is each side's own.
struct ShapeKey
{
    ShapeDraw::ShapeDrawKey<const void> draw;
    int complexity; // LOD-0 face count of draw.shape
};

inline int CompareShapeKeys(const ShapeKey& a, const ShapeKey& b)
{
    return ShapeDraw::CompareShapeDraw(a.draw, b.draw, [&a, &b](const void* shape)
                                       { return shape == a.draw.shape ? a.complexity : b.complexity; });
}

// CmpSurfaceObj: SurfaceDraw::CompareSurfaceDraw on the key the comparator built
// (its PassOrder is a virtual call on the object; now once per entry).
inline int CompareSurfaceKeys(const SurfaceDraw::SurfaceDrawKey& a, const SurfaceDraw::SurfaceDrawKey& b)
{
    return SurfaceDraw::CompareSurfaceDraw(a, b);
}

// CmpRevAlphaSortObj(p1, p0): AlphaSort::CompareAlphaDepth(p1->zCoord, p0->zCoord).
inline int CompareAlphaKeys(float first, float second)
{
    return AlphaSort::CompareAlphaDepth(first, second);
}

} // namespace Poseidon::DrawListSort
