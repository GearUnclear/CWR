#pragma once

// The fire spread's sparks (POSEIDON_XP_FIRESPREAD; FireSpreadWorld.cpp
// EmitSparks feeds them, the renderer draws them, GetSparks): glowing and
// flaming fragments thrown up by the flames, one pool of plain particles
// stepped on the CPU and drawn in one instanced draw (each a streaked point),
// so thousands cost little. Nothing here reads the engine: the laws and the
// pool are pure, so the unit tests can drive them.
//
// The laws are the wildfire consult's (2026-11-17, eighth round, section 3;
// CHECKLIST.md 1.8). A spark rides the flame's gas while it is in the flame:
// it rises at kSparkRiseK sqrt(L) m/s (4.8 m/s in a 2.6 m grass flame, 11 to
// 12 m/s in a 15 m torch) and takes the wind at its own height (SparkWindShare:
// about 0.5 of the 10 m wind within a metre of the ground, 0.8 at 3 m, 1.0 at
// 10 m, 1.2 at 30 m) within a fraction of a second. Above the flame's tip its
// rise dies away over about a second and it sinks at 0.5 to 2 m/s, so grass
// sparks fly 10 to 25 degrees above level and level off, torch sparks go up
// the column and bend flat 30 to 50 m up; none hovers or climbs straight up.
// It glows yellow-orange at birth and cools to a dull red as it dies; one that
// lands goes out. Counts are what is in the air at once:
//   - a grass head: kHeadSparksPerM a metre of front at a kHeadSparkRefM flame
//     (FrontSparksPerM: 5 to 20 a metre seen within 50 m; a flank's 0.65 m
//     flame about 0.8), living 0.5 to 2 s at that flame (FrontSparkLifeSec:
//     0.3 to 1 s on a flank);
//   - a flaring thicket: a burst of 50 to 150 at its peak, then a thin stream,
//     1 to 4 s (a few to 10 s); a reed clump 100 to 300, very brief; a leafy
//     bush a few dozen; an olive's crown between a thicket and a torch
//     (FlareSparkPeak);
//   - a torching pine: 300 to 1000 at its peak (TorchSparkPeak), most living
//     2 to 8 s and a few (cone scales, twig ends) 10 to 30 s, a stream 50 to
//     150 m long, a burst leaving the top with each puff of the column.
// Far off fewer are made (SparkKeep), each standing for the ones left out
// (its light and the area it is spread over that many times), so a far
// spark stream keeps its light without thousands of sub-pixel points; single
// glowing sparks show to 100 to 300 m and flaming fragments to 500 to 700 m,
// so a grass spark is made only within kFrontSparkMaxM, a torch's out to
// kTorchSparkMaxM, where only the stream's faint streak shows.

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <vector>

namespace Poseidon::firespread
{

// ---- The laws ----
constexpr float kSparkRiseK = 3.0f;
inline float SparkRiseMps(float flameM)
{
    return kSparkRiseK * std::sqrt(std::max(flameM, 0.05f));
}
// The wind at a height over the ground as a share of the 10 m wind: a log
// profile of roughness kSparkZ0 (from kSparkZMin up, at most kSparkWindMax).
constexpr float kSparkZ0 = 0.05f, kSparkZMin = 0.3f, kSparkWindMax = 1.3f;
inline float SparkWindShare(float aglM)
{
    const float z = std::max(aglM, kSparkZMin);
    return std::min(kSparkWindMax, std::log(z / kSparkZ0) / std::log(10.0f / kSparkZ0));
}
// Over the flame's tip the rise relaxes to the sink over SparkRiseDecaySec
// (mostly gone within about a second: kSparkRiseDecayBase plus
// kSparkRiseDecayPerM a metre of flame, a big torch's plume carrying it a
// little longer, at most kSparkRiseDecayMax); inside the flame the spark
// takes the gas's rise over kSparkRiseTakeSec and, everywhere, the wind's
// over kSparkWindTakeSec. The air's eddies shake it by kSparkTurb of the wind
// at its height (plus kSparkTurbMin m/s), and inside the flame by
// kSparkTurbFlame of the rise.
constexpr float kSparkRiseDecayBase = 0.6f, kSparkRiseDecayPerM = 0.03f, kSparkRiseDecayMax = 1.4f;
constexpr float kSparkRiseTakeSec = 0.2f, kSparkWindTakeSec = 0.25f;
constexpr float kSparkTurb = 0.22f, kSparkTurbMin = 0.3f, kSparkTurbFlame = 0.2f;
inline float SparkRiseDecaySec(float flameM)
{
    return std::clamp(kSparkRiseDecayBase + kSparkRiseDecayPerM * std::max(flameM, 0.0f), kSparkRiseDecayBase,
                      kSparkRiseDecayMax);
}

// A grass head's sparks in the air at once, a metre of front: kHeadSparksPerM
// at a kHeadSparkRefM flame (the consult's 2.6 m head), as (L / ref) to
// kHeadSparkPow (a flank's 0.65 m flame about 0.8 a metre, its 0.5 to 2), at
// most kHeadSparksMaxPerM.
constexpr float kHeadSparksPerM = 8.0f, kHeadSparkRefM = 2.6f, kHeadSparkPow = 1.3f, kHeadSparksMaxPerM = 20.0f;
inline float FrontSparksPerM(float flameM)
{
    if (flameM <= 0.05f)
        return 0.0f;
    return std::min(kHeadSparksMaxPerM, kHeadSparksPerM * std::pow(flameM / kHeadSparkRefM, kHeadSparkPow));
}
// A grass spark's life (s): kFrontLifeMin to Max at the reference flame,
// scaled by sqrt(L / ref) (a flank's 0.3 to 1 s), within kFrontLifeScaleMin..Max.
constexpr float kFrontLifeMin = 0.5f, kFrontLifeMax = 2.0f, kFrontLifeScaleMin = 0.4f, kFrontLifeScaleMax = 1.3f;
inline float FrontSparkLifeScale(float flameM)
{
    return std::clamp(std::sqrt(std::max(flameM, 0.0f) / kHeadSparkRefM), kFrontLifeScaleMin, kFrontLifeScaleMax);
}
inline float FrontSparkLifeSec(float flameM, float u)
{
    return (kFrontLifeMin + (kFrontLifeMax - kFrontLifeMin) * std::clamp(u, 0.0f, 1.0f)) * FrontSparkLifeScale(flameM);
}
// Near the eye the small dim ones show too (the consult's 5 to 20 a metre
// seen close): up to kNearBoost times more within kNearBoostFromM, none
// extra past kNearBoostToM.
constexpr float kNearBoost = 0.7f, kNearBoostFromM = 15.0f, kNearBoostToM = 40.0f;
inline float SparkNearBoost(float distM)
{
    const float k = std::clamp((kNearBoostToM - distM) / (kNearBoostToM - kNearBoostFromM), 0.0f, 1.0f);
    return 1.0f + kNearBoost * k * k * (3.0f - 2.0f * k);
}
// The share of a source's sparks made at distance d (m): all within fullM,
// (fullM / d)^2 beyond, fading to none from 0.8 of maxM to maxM.
constexpr float kFrontSparkFullM = 100.0f, kFrontSparkMaxM = 300.0f;
// At night a far spark is drawn at least kSparkGlareM across (SparkView's
// glare: past kSparkGlareFromM, fully past kSparkGlareToM), by night's share.
constexpr float kSparkGlareM = 0.08f, kSparkGlareFromM = 20.0f, kSparkGlareToM = 40.0f;
constexpr float kTorchSparkFullM = 150.0f, kTorchSparkMaxM = 2000.0f;
inline float SparkKeep(float distM, float fullM, float maxM)
{
    if (distM >= maxM)
        return 0.0f;
    const float far = distM > fullM ? (fullM / distM) * (fullM / distM) : 1.0f;
    const float edge = std::clamp((maxM - distM) / (0.2f * maxM), 0.0f, 1.0f);
    return far * edge;
}

// What throws them (the kinds' shapes: SparkKindShapeOf).
enum class SparkKind : std::uint8_t
{
    Front = 0,   // a grass or litter front's flames
    Thicket = 1, // a flaring thicket
    Leafy = 2,   // a leafy bush's weak flare
    Reed = 3,    // a reed clump's flash
    Olive = 4,   // an olive's crown
    Torch = 5,   // a torching tree
};
constexpr int kSparkKinds = 6;
struct SparkKindShape
{
    float lifeMin = 1.0f, lifeMax = 2.0f;                    // most of them (s)
    float longShare = 0.0f, longMin = 0.0f, longMax = 0.0f;  // a few living longer (s)
    float sizeMin = 0.02f, sizeMax = 0.04f, longSize = 1.5f; // across (m), the long-lived this much bigger
    float kMin = 1350.0f, kMax = 1550.0f, kEnd = 1050.0f;    // the glow's temperature at birth and death (K)
    float sinkMin = 0.5f, sinkMax = 2.0f, longSink = 0.4f;   // the sink over the tip (m/s), the long-lived's
    float burstSec = 1.5f, streamShare = 0.15f;              // a flare's burst (s), then its stream's share
};
inline SparkKindShape SparkKindShapeOf(SparkKind k)
{
    SparkKindShape s;
    switch (k)
    {
        case SparkKind::Front: // seed heads, bits of stalk
            s.lifeMin = kFrontLifeMin;
            s.lifeMax = kFrontLifeMax;
            s.sizeMin = 0.015f;
            s.sizeMax = 0.035f;
            s.kMin = 1400.0f;
            s.kMax = 1600.0f;
            s.sinkMin = 0.8f;
            s.sinkMax = 2.0f;
            break;
        case SparkKind::Thicket:
            s.lifeMin = 1.0f;
            s.lifeMax = 4.0f;
            s.longShare = 0.06f;
            s.longMin = 5.0f;
            s.longMax = 10.0f;
            s.sizeMin = 0.02f;
            s.sizeMax = 0.04f;
            s.kMin = 1350.0f;
            s.kMax = 1600.0f;
            break;
        case SparkKind::Leafy:
            s.lifeMin = 0.5f;
            s.lifeMax = 2.0f;
            s.sizeMin = 0.015f;
            s.sizeMax = 0.03f;
            s.burstSec = 1.0f;
            s.streamShare = 0.1f;
            break;
        case SparkKind::Reed: // very brief
            s.lifeMin = 0.5f;
            s.lifeMax = 2.0f;
            s.sizeMin = 0.015f;
            s.sizeMax = 0.03f;
            s.kMin = 1350.0f;
            s.kMax = 1550.0f;
            s.burstSec = 1.0f;
            s.streamShare = 0.1f;
            break;
        case SparkKind::Olive:
            s.lifeMin = 1.0f;
            s.lifeMax = 5.0f;
            s.longShare = 0.05f;
            s.longMin = 6.0f;
            s.longMax = 15.0f;
            s.sizeMin = 0.02f;
            s.sizeMax = 0.045f;
            s.kMin = 1350.0f;
            s.kMax = 1600.0f;
            s.burstSec = 3.0f;
            s.streamShare = 0.3f;
            break;
        case SparkKind::Torch: // bark flakes, needles, twig ends, cone scales
            s.lifeMin = 2.0f;
            s.lifeMax = 8.0f;
            s.longShare = 0.05f;
            s.longMin = 10.0f;
            s.longMax = 30.0f;
            s.sizeMin = 0.02f;
            s.sizeMax = 0.05f;
            s.longSize = 1.6f;
            s.kMin = 1450.0f;
            s.kMax = 1700.0f;
            s.sinkMin = 0.6f;
            s.sinkMax = 1.8f;
            s.longSink = 0.35f;
            break;
    }
    return s;
}
inline float SparkMeanLifeSec(const SparkKindShape& s)
{
    return (1.0f - s.longShare) * 0.5f * (s.lifeMin + s.lifeMax) + s.longShare * 0.5f * (s.longMin + s.longMax);
}
// A flare's sparks in the air at its peak (its full flame L m long): a
// thicket kThicketSparksPerM a metre of flame (a 2.4 m bush's 4.8 m flame
// about 120), a reed clump kReedSparksPerM, a leafy bush kLeafySparksPerM, an
// olive kOliveSparksPerM, each within its class's range.
constexpr float kThicketSparksPerM = 25.0f, kReedSparksPerM = 80.0f, kLeafySparksPerM = 10.0f;
constexpr float kOliveSparksPerM = 30.0f;
inline float FlareSparkPeak(SparkKind k, float flameM)
{
    const float L = std::max(flameM, 0.0f);
    switch (k)
    {
        case SparkKind::Reed:
            return std::clamp(kReedSparksPerM * L, 60.0f, 300.0f);
        case SparkKind::Leafy:
            return std::clamp(kLeafySparksPerM * L, 5.0f, 40.0f);
        case SparkKind::Olive:
            return std::clamp(kOliveSparksPerM * L, 60.0f, 250.0f);
        default:
            return std::clamp(kThicketSparksPerM * L, 20.0f, 150.0f);
    }
}
// A torching tree's sparks in the air at its peak: kTorchSparksPerM a metre
// of its peak flame (a lone 11 m pine's 14 m column about 600), within
// kTorchSparksMin..Max.
constexpr float kTorchSparksPerM = 45.0f, kTorchSparksMin = 300.0f, kTorchSparksMax = 1000.0f;
inline float TorchSparkPeak(float peakFlameM)
{
    return std::clamp(kTorchSparksPerM * std::max(peakFlameM, 0.0f), kTorchSparksMin, kTorchSparksMax);
}

// ---- The pool ----
// Where to make some: around (x, y, z) within spreadXZ across and spreadY up
// (uniform), its initial velocity, the flame's top it rides the gas to
// (world y), the gas's rise inside, the ground under it, the light each
// stands for (1 / SparkKeep: also the area it is spread over).
struct SparkBirth
{
    SparkKind kind = SparkKind::Front;
    float x = 0.0f, y = 0.0f, z = 0.0f;
    float spreadXZ = 0.0f, spreadY = 0.0f;
    float vx = 0.0f, vy = 0.0f, vz = 0.0f;
    float tipY = 0.0f, riseMps = 0.0f;
    float groundY = 0.0f;
    float weight = 1.0f;
    float lifeScale = 1.0f; // the kind's lives times this (a grass flame's FrontSparkLifeScale)
};
// One spark for the renderer: where it is (world m) and how fast it moves
// (m/s: it is drawn as the streak a camera's shutter would see), its size
// across (m), its glow's temperature (K), its light's weight now (its LOD
// weight times its fade and twinkle) and the area its light is spread over
// (a multiple of its own: its LOD weight, at least 1).
struct SparkPoint
{
    float x = 0.0f, y = 0.0f, z = 0.0f;
    float vx = 0.0f, vy = 0.0f, vz = 0.0f;
    float size = 0.03f, kelvin = 1400.0f;
    float light = 1.0f, area = 1.0f;
};
// The counts for the stats line.
struct SparkCounts
{
    int alive = 0, byKind[kSparkKinds] = {};
    long long made = 0, dropped = 0, landed = 0;
};

class SparkPool
{
  public:
    // At most kMaxSparks alive (more births are dropped and counted).
    static constexpr int kMaxSparks = 16000;
    using GroundFn = std::function<float(float, float)>;

    explicit SparkPool(std::uint32_t seed = 0x9e3779b9U) : _rng(seed ? seed : 1U) {}
    void Clear()
    {
        _sparks.clear();
        _points.clear();
    }
    // n of them as `b` says (each its own place, life, size and glow).
    void Emit(const SparkBirth& b, int n);
    // A fractional count: floor(n), and one more with the chance of what is left.
    void EmitExpected(const SparkBirth& b, float expected);
    // dt seconds in the 10 m wind (wx, wz); ground(x, z) the land's height,
    // read only for sparks sinking near it (those under it go out).
    void Step(float dt, float wind10X, float wind10Z, const GroundFn& ground);
    // The renderer's list (Step's state): BuildPoints makes it, once a frame
    // after Step; LastPoints is the last one made.
    const std::vector<SparkPoint>& BuildPoints();
    const std::vector<SparkPoint>& LastPoints() const { return _points; }
    int Alive() const { return static_cast<int>(_sparks.size()); }
    long long Made() const { return _made; }
    SparkCounts Counts() const;
    // Alive sparks of a kind within r metres (horizontally) of (x, z).
    int CountNear(SparkKind kind, float x, float z, float r) const;
    float Uniform() { return Next01(); }

    // A spark's light over its life (0..1, at age a of life L): in over
    // kSparkFadeInSec, out over its last kSparkFadeOutShare as it burns out,
    // and its glow's temperature from kStart to kEnd as (a / L)^kSparkCoolPow.
    static constexpr float kSparkFadeInSec = 0.06f, kSparkFadeOutShare = 0.25f, kSparkCoolPow = 1.5f;
    static float LightOverLife(float a, float life);
    static float KelvinOverLife(float a, float life, float kStart, float kEnd);

  private:
    struct Spark
    {
        float x, y, z, vx, vy, vz;
        float age, life;
        float tipY, rise, sink, decay;
        float k0, kEnd, size, weight, gy;
        float twinkleHz, twinklePh;
        std::uint8_t kind;
        bool out; // above its flame's tip (once out, never back)
    };
    float Next01()
    {
        _rng ^= _rng << 13;
        _rng ^= _rng >> 17;
        _rng ^= _rng << 5;
        return static_cast<float>(_rng & 0xffffffU) / 16777216.0f;
    }
    // A sum of three uniforms: about a normal of mean 0 and spread 1.
    float Gauss() { return (Next01() + Next01() + Next01() - 1.5f) * 2.0f; }
    std::vector<Spark> _sparks;
    std::vector<SparkPoint> _points;
    std::uint32_t _rng;
    std::uint32_t _frame = 0;
    long long _made = 0, _dropped = 0, _landed = 0;
    double _time = 0.0;
};

} // namespace Poseidon::firespread
