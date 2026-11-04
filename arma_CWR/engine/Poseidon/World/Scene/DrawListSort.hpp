#pragma once

#include <Poseidon/Foundation/Algorithms/Qsort.hpp>
#include <Poseidon/Foundation/Types/Pointers.hpp>
#include <Poseidon/World/Scene/AlphaSortOrder.hpp>
#include <Poseidon/World/Scene/ShapeDrawOrder.hpp>
#include <Poseidon/World/Scene/SurfaceDrawOrder.hpp>

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
