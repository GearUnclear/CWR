#pragma once

#include <cstdint>

namespace Poseidon::ShapeDraw
{

// Sort key for the Pass1 draw list (Scene::DrawObjectsAndShadowsPass1, CmpShapeObj in
// SceneDraw.cpp). POD; no engine dependencies, so the ordering can be unit-tested with
// fabricated shape addresses that are never dereferenced.
template <class ShapeT>
struct ShapeDrawKey
{
    int passNum;     // primary key, ascending
    ShapeT* shape;   // LODShape identity: groups identical shapes into one run
    int drawLOD;     // same shape: coarse LODs (large numbers) first
    float distance2; // same shape and LOD: back to front
};

// Shape identity order: higher address first, the direction the original
// `(int)((intptr_t)s2 - (intptr_t)s1)` meant. That expression truncated a 64-bit
// difference to int (its low 32 bits): two shapes 2 GiB or more apart could get a sign
// that did not follow their addresses (a non-transitive sort that can split a shape into
// several runs), and two shapes a multiple of 4 GiB apart compared as the same shape.
// Compare the addresses instead.
template <class ShapeT>
inline int CompareShapeIdentity(const ShapeT* a, const ShapeT* b)
{
    const std::uintptr_t ua = reinterpret_cast<std::uintptr_t>(a);
    const std::uintptr_t ub = reinterpret_cast<std::uintptr_t>(b);
    if (ua == ub)
    {
        return 0;
    }
    return ua > ub ? -1 : +1;
}

// Strict weak ordering for the Pass1 draw list: pass ascending; then, between different
// shapes, simpler shapes first (complexity(shape) = face count of LOD 0, evaluated only
// when the shapes differ) and shape identity; within one shape, coarse LODs first, then
// far to near. Returns <0 when a draws first, 0 for equal keys, >0 when b draws first.
template <class ShapeT, class Complexity>
inline int CompareShapeDraw(const ShapeDrawKey<ShapeT>& a, const ShapeDrawKey<ShapeT>& b, Complexity complexity)
{
    if (a.passNum != b.passNum)
    {
        return a.passNum < b.passNum ? -1 : +1;
    }
    if (a.shape != b.shape)
    {
        const int ca = complexity(a.shape);
        const int cb = complexity(b.shape);
        if (ca != cb)
        {
            return ca < cb ? -1 : +1;
        }
        return CompareShapeIdentity(a.shape, b.shape);
    }
    if (a.drawLOD != b.drawLOD)
    {
        return a.drawLOD > b.drawLOD ? -1 : +1;
    }
    if (a.distance2 > b.distance2)
    {
        return -1;
    }
    if (a.distance2 < b.distance2)
    {
        return +1;
    }
    return 0;
}

} // namespace Poseidon::ShapeDraw
