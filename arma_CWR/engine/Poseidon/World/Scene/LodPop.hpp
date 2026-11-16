#pragma once

#include <cmath>
#include <cstdlib>

// Experimental switches of the EX2-POP lane (PERF-PLAN R-18, R-19, EXP-L14b).
//
//   POSEIDON_XP_LODHYST=<percent>  LOD hysteresis: an object leaves its level
//       only once it is <percent> past the threshold, and once culled as too
//       small it comes back at 1.35 px, not 1.25. 0 or unset: off, stateless.
//   POSEIDON_XP_LODFADE=<ms>       LOD cross-fade: a static object that changes
//       level draws both levels for <ms>, each on its own share of the MSAA
//       samples (Engine::SetDrawSampleMask), so the resolve blends them with no
//       dither and no shader change. Needs MSAA. 0 or unset: off. With it on, a
//       camera at rest lets go of the hysteresis, so the settled picture is the
//       stateless one, reached by a fade.
//   POSEIDON_XP_LODLOG=<path>      appends per-frame pop counters (CSV) while
//       a triCamPath runs, and events to <path>.events (round 2: every kind of
//       appearing and vanishing, shadow casters included, with where on screen).
//   POSEIDON_XP_SHADOWFADE=<ms>    round 2: a static object whose shadow-map
//       caster changes level dissolves from the old shape to the new one over
//       <ms>, face by face, instead of swapping in one frame. No shader change.
//   POSEIDON_XP_SHADOWKEEP         round 2, on unless 0: an object casts into the sun shadow
//       maps whenever its shadow can reach ground the maps cover, whether or
//       not the camera sees the object itself. Without it an object outside
//       the screen, hidden behind another object, or in a screen corner past
//       the shadow distance casts nothing, and its shadow appears in one frame
//       when the object comes into view. Needs the shadow maps.
//
// LODBAND (10) and SHADOWBAND (150) below are on unless set to 0, as is
// SHADOWKEEP (see kShippedLodBandPercent); the rest are off unless set.
//
// Round 3 (the owner: "a super quick fade in instead of a pop. But still
// bad"): arrival by distance instead of by time.
//   POSEIDON_XP_LODBAND=<percent>  an object's level switch is a cross-fade
//       over a band of <percent> of its distance on each side of the switch,
//       read from the distance every frame (no timer, no history); so is its
//       arrival at the object distance. Only objects of kBandMinPx and more on
//       screen (smaller ones switch at once, unseen, and stay instanced). With
//       POSEIDON_XP_SHADOWKEEP, a static caster's level switch in the shadow
//       maps is a dissolve over the same band, per fragment of the depth map
//       (a world-anchored hash, DissolveCellHash). Needs MSAA for the
//       objects. Replaces POSEIDON_XP_LODFADE and _SHADOWFADE when set.
//   POSEIDON_XP_SHADOWBAND=<metres>  the sun shadows keep full strength out
//       to the shadow distance and fade out over the next <metres> by distance
//       from the eye (as a sphere, so turning the head changes nothing), along
//       a smoothstep; the maps reach that much further. With
//       POSEIDON_XP_SHADOWKEEP the casters are cut by the same distance.

namespace Poseidon::LodPop
{

constexpr signed char kNoLevel = -2; // no level remembered yet

// The size at which a culled object comes back, against the size at which it
// is culled: 1.35 px against 1.25 px.
constexpr float kCullLeaveFactor = 1.35f / 1.25f;

// A camera that jumps this far in a frame made a cut: the levels held for the
// old place mean nothing at the new one.
constexpr float kCutDistance = 30.0f;

// A camera still for this many frames is at rest.
constexpr int kRestFrames = 30;

inline float EnvMilliseconds(const char* name)
{
    const char* v = std::getenv(name);
    const float ms = v ? static_cast<float>(std::atof(v)) : 0.0f;
    return ms > 0.0f ? ms * 0.001f : 0.0f;
}

inline bool EnvFlag(const char* name)
{
    const char* v = std::getenv(name);
    return v && *v && std::atoi(v) != 0;
}

// The shipped settings (owner verdict "much better", 2026-11-13): SHADOWKEEP on,
// LODBAND 10 percent, SHADOWBAND 150 m (shadow::xp::RadialFadeBand). Unset
// means these; POSEIDON_XP_<NAME>=0 turns each one off again (today's picture,
// for the identity gates).
constexpr float kShippedLodBandPercent = 10.0f;

// On unless the variable is set to a number that reads as 0.
inline bool EnvFlagDefaultOn(const char* name)
{
    const char* v = std::getenv(name);
    return !v || !*v || std::atoi(v) != 0;
}

// POSEIDON_XP_SHADOWFADE: is face number `face` of a caster drawn into the
// shadow maps, a fraction t of the way through a dissolve? The level coming in
// gains faces as t grows, the level going out loses them; the golden-ratio
// sequence spreads the faces evenly over the shape at every t.
inline float FaceDissolveKey(int face)
{
    const float v = static_cast<float>(face) * 0.6180339887f;
    return v - static_cast<float>(static_cast<int>(v));
}

inline bool FaceInDissolve(int face, float t, bool comingIn)
{
    const bool below = FaceDissolveKey(face) < t;
    return comingIn ? below : !below;
}

// POSEIDON_XP_SHADOWKEEP. A shadow is followed along the sun ray for at most
// this far (the old off-screen test's 300 m), and the sun counts as at least 4
// degrees up (the shadow maps' own floor, SceneShadowPass.cpp).
constexpr float kKeepMaxShadow = 300.0f;
constexpr float kKeepMinSunDown = 0.0698f;
// The tallest thing a grid cell of objects is assumed to hold (metres).
constexpr float kKeepCellHeight = 40.0f;

// How far along the sun ray the shadow of something `height` tall can land on
// flat ground: height / sin(sun elevation), capped. sunDown = -sunDir.y.
inline float ShadowLength(float height, float sunDown)
{
    const float s = sunDown > kKeepMinSunDown ? sunDown : kKeepMinSunDown;
    const float len = height / s;
    return len > kKeepMaxShadow ? kKeepMaxShadow : len;
}

// The smallest eye depth any part of an object's shadow can have: the object
// sits at eye depth `depth`, its shadow runs `len` along the sun ray, and
// sunDotForward is the sun ray against the camera's forward axis. The shadow
// maps shade receivers by eye depth, so a caster matters exactly when this is
// inside their far distance.
inline float MinShadowEyeDepth(float depth, float len, float sunDotForward, float radius)
{
    const float end = depth + len * sunDotForward;
    return (end < depth ? end : depth) - radius;
}

inline float EnvPercentBand(const char* name)
{
    const char* v = std::getenv(name);
    const float percent = v ? static_cast<float>(std::atof(v)) : 0.0f;
    return percent > 0.0f ? 1.0f + percent * 0.01f : 0.0f;
}

// As EnvPercentBand, but unset (or empty) means `defaultPercent`.
inline float EnvPercentBandOr(const char* name, float defaultPercent)
{
    const char* v = std::getenv(name);
    const float percent = (v && *v) ? static_cast<float>(std::atof(v)) : defaultPercent;
    return percent > 0.0f ? 1.0f + percent * 0.01f : 0.0f;
}

// POSEIDON_XP_LODBAND: the distance band of a level switch as a factor (1.10
// for 10 percent), 0 when off. Read once.
inline float LodDistBand()
{
    static const float s_band = EnvPercentBandOr("POSEIDON_XP_LODBAND", kShippedLodBandPercent);
    return s_band;
}

// The level of an object that drew `held` last frame. find(resol2) is the
// stateless choice, never finer for a larger resol2. The object goes coarser
// only when the choice is coarser still at a resolution `band` lower, and
// finer only when it is finer still at one `band` higher, so inside the band
// around a threshold it stays where it was, and the answer is always a level
// the stateless choice gives somewhere in the band.
template <class FindLevel>
int HeldLevel(int held, float resol2, float band, FindLevel find)
{
    const float band2 = band * band;
    const int coarser = find(resol2 / band2);
    if (coarser > held)
    {
        return coarser;
    }
    const int finer = find(resol2 * band2);
    if (finer < held)
    {
        return finer;
    }
    return held;
}

// POSEIDON_XP_LODBAND (round 3): where an object stands in a level switch is
// read from its distance, not from a timer. Over the last `band` (a factor on
// the distance, 1.1 = 10 %) BEFORE a switch both levels draw, `t` the coarser
// one's share: 0 where the band starts (nearer), 1 at the switch itself, so
// past the switch the object draws exactly what it drew before (the finer,
// dearer level is never drawn further out than today; only the coarser one
// is drawn a little nearer). Stateless, so a camera at rest holds the same
// picture whatever came before, and walking toward an object spreads its
// switch over metres.
struct LevelBlend
{
    int fine;   // the finer level (what the object is nearer than the band)
    int coarse; // the coarser level (at and past the switch); == fine outside a band
    float t;    // the coarser level's share, 0..1
};

// find(resol2) is the stateless choice (never finer for a larger resol2);
// resol2 grows with the square of the distance.
template <class FindLevel>
LevelBlend BandBlend(float resol2, float band, FindLevel find)
{
    const int now = find(resol2);
    if (!(band > 1.0f) || !(resol2 > 0.0f))
    {
        return LevelBlend{now, now, 0.0f};
    }
    const float band2 = band * band;
    const int coarse = find(resol2 * band2);
    if (coarse == now)
    {
        return LevelBlend{now, now, 0.0f};
    }
    // The switch out of `now`: the nearest resol2 at which the choice is
    // coarser (bisection in log space).
    float lo = resol2;
    float hi = resol2 * band2;
    for (int i = 0; i < 14; i++)
    {
        const float mid = std::sqrt(lo * hi);
        if (find(mid) > now)
        {
            hi = mid;
        }
        else
        {
            lo = mid;
        }
    }
    const float edge = std::sqrt(lo * hi);
    // resol2 runs from edge / band2 (t 0) to edge (t 1).
    float t = 1.0f + std::log(resol2 / edge) / (2.0f * std::log(band));
    t = t < 0.0f ? 0.0f : t > 1.0f ? 1.0f : t;
    return LevelBlend{now, coarse, t};
}

// POSEIDON_XP_LODBAND: objects smaller than this on screen (pixels) switch
// level and arrive at once, as before: a change that small cannot be seen, and
// a blended object costs a draw of its own (round 3: blending every object in
// a band put 700 to 1,500 objects a frame in a blend on the test paths).
constexpr float kBandMinPx = 8.0f;

// How many of `samples` MSAA samples a share `t` gets (0..samples).
inline int ShareSamples(float t, int samples)
{
    int k = static_cast<int>(t * static_cast<float>(samples) + 0.5f);
    return k < 0 ? 0 : k > samples ? samples : k;
}

// An object's visibility near an edge where it used to vanish in one frame:
// 1 well inside, 0 at the edge, over the last `band` factor before it.
// With `below` the object is past the edge when value < limit (its size on
// screen), otherwise when value > limit (its distance).
inline float EdgeShare(float value, float limit, float band, bool below)
{
    if (!(band > 1.0f) || !(limit > 0.0f) || !(value > 0.0f))
    {
        return 1.0f;
    }
    const float r = below ? value / limit : limit / value; // 1 at the edge, band well inside
    const float s = std::log(r) / std::log(band);
    return s < 0.0f ? 0.0f : s > 1.0f ? 1.0f : s;
}

// POSEIDON_XP_SHADOWBAND (round 3): the receivers fade by distance from the
// eye, not by depth along the view, so the caster cut uses the same measure:
// the nearest distance from the eye to any point of the object's shadow
// (the segment from the object along the sun ray for `len`), less its radius.
// rel = object - eye, sun a unit vector.
inline float MinShadowRadialDistance(float relX, float relY, float relZ, float sunX, float sunY, float sunZ, float len,
                                     float radius)
{
    float s = -(relX * sunX + relY * sunY + relZ * sunZ);
    s = s < 0.0f ? 0.0f : s > len ? len : s;
    const float x = relX + sunX * s, y = relY + sunY * s, z = relZ + sunZ * s;
    return std::sqrt(x * x + y * y + z * z) - radius;
}

// POSEIDON_XP_LODBAND with SHADOWKEEP (round 3, fix round): a caster in the
// band of a level switch is dissolved per fragment in the shadow depth pass,
// not per face (the faces of a coarse forest block are tens of metres of
// shadow each, so a face dissolve still stepped in one frame). Each fragment
// is kept or dropped by a hash of its world cell (1/16 m) against a key. The
// hash position is the world position modulo kDissolveWrap, so it stays small
// and exact in float; the shadow maps' texel grid is anchored to the world, so
// a texel samples the same point, and the same hash, every frame.
constexpr float kDissolveWrap = 256.0f;
constexpr float kDissolveCellsPerMetre = 16.0f;

inline float DissolveHashCoord(float world)
{
    float v = std::fmod(world, kDissolveWrap);
    return v < 0.0f ? v + kDissolveWrap : v;
}

// The key a level's fragments carry: the coarser level keeps the fragments
// whose hash is under its share t (key t), the finer one the rest (key 2 + t).
inline float DissolveKey(float t, bool coarser)
{
    const float c = t < 0.0f ? 0.0f : t > 1.0f ? 1.0f : t;
    return coarser ? c : 2.0f + c;
}

// The depth shader's hash (EngineGL33_ShadowDepth.cpp kDissolveFS), for tests.
inline float DissolveCellHash(float hx, float hy, float hz)
{
    auto cell = [](float v)
    { return static_cast<unsigned>(std::floor((v > 0.0f ? v : 0.0f) * kDissolveCellsPerMetre)); };
    unsigned h = (cell(hx) * 73856093u) ^ (cell(hy) * 19349663u) ^ (cell(hz) * 83492791u);
    h ^= h >> 13;
    h *= 0x5bd1e995u;
    h ^= h >> 15;
    return static_cast<float>(h & 0xFFFFFFu) * (1.0f / 16777216.0f);
}

inline bool DissolveKeeps(float hash, float key)
{
    return key < 1.5f ? hash < key : hash >= key - 2.0f;
}

// Which of `samples` MSAA samples the new and the old level draw on, a fraction
// t of the way through a fade: complementary, and both have some until it ends.
struct FadeMasks
{
    unsigned now;
    unsigned was;
};

inline FadeMasks FadeSampleMasks(float t, int samples)
{
    const unsigned all = (1u << samples) - 1u;
    int k = static_cast<int>(t * samples + 0.5f);
    k = k < 1 ? 1 : k > samples - 1 ? samples - 1 : k;
    const unsigned now = (1u << k) - 1u;
    return FadeMasks{now, all & ~now};
}

} // namespace Poseidon::LodPop
