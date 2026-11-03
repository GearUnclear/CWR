#pragma once

// Experimental view-distance policy (perf squad lane xp-viewdist). Every switch
// is off by default, so the default frame is unchanged; the variables are read
// once, at the first query.
//
// POSEIDON_XP_VD_EARLYCULL=1: objects that the LOD selection would hide anyway
// because they lie beyond the object draw distance (objectsZ, the shadow range
// when that is larger, or the far ring's size limit below) are dropped while
// the world enumerates them, before a draw-list entry exists, and whole object
// grid cells beyond that distance are skipped. Today every object out to the
// fog end (the view distance) gets a draw-list entry, a frustum test and a LOD
// selection, and at 4 to 5 km more than half of them are discarded. Pure CPU:
// the drawn set is exactly the same (the size test uses the finest detail
// width, which never culls an object a later detail would draw).
//
// POSEIDON_XP_VD_FAR=<px>: the far ring. Beyond objectsZ (2/3 of the view
// distance, at most 3 km), objects are drawn out to the fog end when their
// projected diameter is at least <px> pixels at 1080p (scaled by the scene's
// detail width like the stock 1.25 px tiny-object cull). "1" means the default
// of 4 px. Big things (forest blocks, large buildings, tall trees) then fill
// the hills that stay bare between 2/3 of the view distance and the horizon,
// while small clutter keeps the stock limit. A look change: a quality tier.
//
// POSEIDON_XP_VD_SMALLPX=<px>: the tiny-object cull at <px> pixels instead of
// the stock 1.25 px, without coarsening any LOD (the stock detail pin raises
// both). For lower presets: distant scrub thins out, draws drop.
//
// POSEIDON_XP_VD_FORCE=<m> (GameStateExtUi.cpp SetVisibility): measurement
// only, replaces every requested view distance.

#include <cstdlib>

namespace Poseidon::xp
{
struct ViewDistPolicy
{
    bool earlyCull = false;
    float farPx = 0.0f;   // 0 = off
    float smallPx = 0.0f; // 0 = the stock 1.25 px

    // The stock tiny-object cull is diameter < 0.125 * Left * dist * lodInvWidth,
    // which is 1.25 px at the finest width (lodInvWidth = 20 / screen width), so a
    // limit of p pixels is a coefficient of 0.1 * p.
    float FarPixelCoef() const { return 0.1f * farPx; }
    // The tiny-object cull coefficient: the stock 0.125 unless SMALLPX is set.
    float SmallPixelCoef() const { return smallPx > 0.0f ? 0.1f * smallPx : 0.125f; }

    static ViewDistPolicy FromEnv()
    {
        ViewDistPolicy p;
        if (const char* v = std::getenv("POSEIDON_XP_VD_EARLYCULL"))
            p.earlyCull = v[0] != '\0' && v[0] != '0';
        if (const char* v = std::getenv("POSEIDON_XP_VD_FAR"))
        {
            const float f = static_cast<float>(std::atof(v));
            if (f > 1.0f)
                p.farPx = f > 64.0f ? 64.0f : f;
            else if (f > 0.0f)
                p.farPx = 4.0f;
        }
        if (const char* v = std::getenv("POSEIDON_XP_VD_SMALLPX"))
        {
            const float f = static_cast<float>(std::atof(v));
            if (f > 0.0f)
                p.smallPx = f > 64.0f ? 64.0f : f;
        }
        return p;
    }
};

// Scene::LevelFromDistance2's visibility test (not its level choice), as the
// stock code and the far ring compute it: distance2 is the LOD distance
// squared, scale the camera's Left(), invWidth the scene's detail width,
// smallCoef the tiny-object cull (0.125 = the stock 1.25 px).
inline bool LodVisible(float distance2, float diameter, float objectsZ, float scale, float invWidth, float farCoef,
                       float smallCoef = 0.125f)
{
    const float detail2 = distance2 * (invWidth * invWidth); // the stock order: distance2 * Square(w)
    if (distance2 > objectsZ * objectsZ)
    {
        if (farCoef <= 0.0f || diameter * diameter < (farCoef * scale) * (farCoef * scale) * detail2)
            return false;
    }
    return !(diameter * diameter < (smallCoef * scale) * (smallCoef * scale) * detail2);
}

// EARLYCULL's object test: true only when LodVisible is false at every detail
// width from invWidthMin up and no shadow path can reach the object. centreDist2
// is the camera to centre distance squared; an object whose radius exceeds a
// quarter of that distance is drawn by its near-edge distance and is never
// decided here; inside shadowRange the shadow LOD may still draw it.
inline bool BeyondDrawRange(float centreDist2, float radius, float diameter, float objectsZ, float shadowRange,
                            float scale, float invWidthMin, float farCoef, float smallCoef = 0.125f)
{
    if (radius * radius > centreDist2 * (0.25f * 0.25f))
        return false;
    if (centreDist2 < shadowRange * shadowRange)
        return false;
    return !LodVisible(centreDist2, diameter, objectsZ, scale, invWidthMin, farCoef, smallCoef);
}

// EARLYCULL's cell test: a cell whose bounding sphere (centre at camToCentre,
// radius cellRadius) holds every static object's sphere can be skipped when
// every centre lies beyond range and every radius under a quarter of its distance.
inline bool CellBeyond(float camToCentre, float cellRadius, float range)
{
    const float dc = camToCentre - cellRadius;
    return range > 0.0f && dc > range && dc > 4.0f * cellRadius;
}

inline const ViewDistPolicy& GViewDistPolicy()
{
    static const ViewDistPolicy s_policy = ViewDistPolicy::FromEnv();
    return s_policy;
}
} // namespace Poseidon::xp
