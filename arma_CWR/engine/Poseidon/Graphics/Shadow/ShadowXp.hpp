#pragma once

#include <cmath>
#include <cstddef>
#include <cstdlib>

// XP-shadows (graphics-experimental lane): runtime switches for the sun shadow
// maps, read once from the environment. Every switch is off unless its variable
// is set, so an unset environment leaves the tip's shadows exactly as they are.
// Pure: no engine state, no GL.
//
//   POSEIDON_XP_SHADOW_TIER   0 low, 1 medium, 2 high (= the tip's set), 3 ultra,
//                             4 extreme: a whole quality step (TierParams below)
//   POSEIDON_XP_SM_RES        depth-map size per cascade (512..8192)
//   POSEIDON_XP_SM_REACH      shadow-map reach cap in metres: the maps reach
//                             view distance * 5/18 (the OFP ratio) up to this
//                             cap and the object distance, instead of the
//                             shadow distance's fixed 500 m cap
//   POSEIDON_XP_SM_CASCADES   cascade count (1..4)
//   POSEIDON_XP_SM_OMNI       leading camera-centred (omni) tiers (0..cascades)
//   POSEIDON_XP_SM_SPLIT      PSSM blend of the frustum tiers (0 uniform .. 1 log)
//   POSEIDON_XP_SM_TAPS       soft-shadow search and filter taps per TAA frame
//   POSEIDON_XP_SMCULL        1: draw each caster only into the cascades its
//                             bounds overlap (pixel-identical depth maps)
//   POSEIDON_XP_SM_TIGHT      1: minimal bounding sphere for the frustum tiers
//                             (smaller texels, still rotation invariant)
//   POSEIDON_XP_SM_MINPX      n: leave out casters smaller than n screen pixels
//   POSEIDON_XP_SMLOG         n: log the caster census every n frames

namespace Poseidon::shadow::xp
{

struct TierParams
{
    int resolution;
    int cascades;
    int omni;
    float reach; // 0 = the shadow distance as today
    int taaTaps; // 0 = the stock 4
    bool cull;   // per-cascade caster culling
    bool tight;  // minimal frustum-tier spheres
};

// Tier 2 is the tip exactly (2048, 4 cascades, 2 omni, 500 m cap, 4 taps).
inline TierParams Tier(int tier)
{
    switch (tier)
    {
        case 0:
            return {1024, 3, 1, 300.0f, 0, true, true};
        case 1:
            return {2048, 3, 1, 500.0f, 0, true, true};
        case 3:
            return {4096, 4, 2, 1000.0f, 0, true, true};
        case 4:
            return {4096, 4, 2, 1600.0f, 0, true, true};
        default:
            return {2048, 4, 2, 0.0f, 0, false, false};
    }
}

inline const char* Env(const char* name)
{
    const char* v = std::getenv(name);
    return (v && *v) ? v : nullptr;
}

inline bool EnvInt(const char* name, int& out)
{
    const char* v = Env(name);
    if (!v)
        return false;
    char* end = nullptr;
    const long n = std::strtol(v, &end, 10);
    if (end == v)
        return false;
    out = static_cast<int>(n);
    return true;
}

inline bool EnvFloat(const char* name, float& out)
{
    const char* v = Env(name);
    if (!v)
        return false;
    char* end = nullptr;
    const float f = std::strtof(v, &end);
    if (end == v || !(f == f))
        return false;
    out = f;
    return true;
}

// The switches as set in this process; `any` false = none set.
struct Overrides
{
    bool any = false;
    int tier = -1;
    int resolution = 0;
    int cascades = 0;
    int omni = -1;
    float split = -1.0f;
    float reach = 0.0f;
    int taaTaps = 0;
    int cull = -1;  // -1 unset, 0 off, 1 on
    int tight = -1; // -1 unset, 0 off, 1 on
    int logEvery = 0;
    float minPx = -1.0f;
};

inline Overrides ReadOverrides()
{
    Overrides o;
    int i = 0;
    float f = 0.0f;
    if (EnvInt("POSEIDON_XP_SHADOW_TIER", i) && i >= 0 && i <= 4)
    {
        o.tier = i;
        o.any = true;
    }
    if (EnvInt("POSEIDON_XP_SM_RES", i) && i >= 512 && i <= 8192)
    {
        o.resolution = i;
        o.any = true;
    }
    if (EnvInt("POSEIDON_XP_SM_CASCADES", i) && i >= 1 && i <= 4)
    {
        o.cascades = i;
        o.any = true;
    }
    if (EnvInt("POSEIDON_XP_SM_OMNI", i) && i >= 0 && i <= 4)
    {
        o.omni = i;
        o.any = true;
    }
    if (EnvFloat("POSEIDON_XP_SM_SPLIT", f) && f >= 0.0f && f <= 1.0f)
    {
        o.split = f;
        o.any = true;
    }
    if (EnvFloat("POSEIDON_XP_SM_REACH", f) && f >= 50.0f && f <= 5000.0f)
    {
        o.reach = f;
        o.any = true;
    }
    if (EnvInt("POSEIDON_XP_SM_TAPS", i) && i >= 1 && i <= 16)
    {
        o.taaTaps = i;
        o.any = true;
    }
    if (EnvInt("POSEIDON_XP_SMCULL", i))
    {
        o.cull = i != 0 ? 1 : 0;
        o.any = true;
    }
    if (EnvInt("POSEIDON_XP_SM_TIGHT", i))
    {
        o.tight = i != 0 ? 1 : 0;
        o.any = true;
    }
    if (EnvFloat("POSEIDON_XP_SM_MINPX", f) && f >= 0.0f && f <= 16.0f)
    {
        o.minPx = f;
        o.any = true;
    }
    if (EnvInt("POSEIDON_XP_SMLOG", i) && i > 0)
        o.logEvery = i;
    return o;
}

// Shadow-map far distance (metres). reach 0 keeps the shadow distance as it is
// (the tip). Otherwise the maps reach the OFP ratio of the view distance
// (5/18), capped by `reach` and the object distance, and never less than the
// shadow distance itself (or the cap, when the cap is the lower of the two).
inline float MapFar(float shadowsZ, float viewZ, float objectsZ, float reach)
{
    if (!(reach > 0.0f))
        return shadowsZ;
    float want = viewZ * (250.0f / 900.0f);
    if (want > reach)
        want = reach;
    if (want > objectsZ)
        want = objectsZ;
    const float floorZ = shadowsZ < reach ? shadowsZ : reach;
    return want > floorZ ? want : floorZ;
}

// The static caster cache's entry cap for a shadow-map far distance: the tip's
// 1024 at 500 m, growing with the covered area, so a long reach does not clear
// (and rebuild) the whole cache every frame.
inline size_t CasterCacheCap(float mapFar)
{
    const float s = mapFar / 500.0f;
    const float cap = 1024.0f * s * s;
    if (cap <= 1024.0f)
        return 1024;
    return cap >= 12288.0f ? 12288 : static_cast<size_t>(cap);
}

// Conservative overlap of a camera-relative sphere with a cascade's light
// view-projection (column-major, orthographic: w = 1): true unless the sphere
// lies wholly outside the map's x or y range. Never tests z (depth clamp
// flattens near casters onto the near plane).
inline bool SphereInCascade(const float* vp, const float* sphere)
{
    const float cx = sphere[0], cy = sphere[1], cz = sphere[2], r = sphere[3];
    const float x = vp[0] * cx + vp[4] * cy + vp[8] * cz + vp[12];
    const float y = vp[1] * cx + vp[5] * cy + vp[9] * cz + vp[13];
    const float sx = r * std::sqrt(vp[0] * vp[0] + vp[4] * vp[4] + vp[8] * vp[8]);
    const float sy = r * std::sqrt(vp[1] * vp[1] + vp[5] * vp[5] + vp[9] * vp[9]);
    const float m = 1.0f + 1e-3f;
    return (x - sx) <= m && (x + sx) >= -m && (y - sy) <= m && (y + sy) >= -m;
}

} // namespace Poseidon::shadow::xp
