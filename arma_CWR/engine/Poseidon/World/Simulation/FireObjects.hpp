#pragma once

// Object fires (fire-objects, 2026-11-18): fires that are not grass cells, a
// burning wreck, a fuel pool, a window, a roof, a shed. Two parts, both pure
// (no engine state, so the unit tests drive them):
//   - the shared object fire (ObjectFire, EmberThrow, the draws that light the
//     ground round one): what both builders hand the fire world each frame
//     (FireSpreadWorld.cpp FireWorld::_objectFires), which draws its flames
//     through the fire's flame pass, its smoke through the plume (black and
//     upright, its own point plume), its light through the fire's lights,
//     throws its sparks, hurts the people in and next to it and lights the
//     ground round it;
//   - the wrecks: a destroyed vehicle's fire by its class, on the wildfire
//     consult's tenth answer (2026-11-18, "buildings, wrecks and explosion
//     days"): its timeline (flames, heat, smoke, embers, heat's reach, a
//     tanker's pool, armour's jets and blasts), its fireball's reach and the
//     two fire days as numbers.
//
// The two days (the tenth answer's build-to table; the scenes set them): a
// typical summer afternoon, dead fine fuel moisture kDayTypicalM1h, live 1.00,
// a 4.5 m/s steady sea breeze; the worst fire day, kDayWorstM1h, live 0.65,
// 12.5 m/s gusting. The build's ember ignition chance (EmberPig) is 0.630 and
// 0.956 on them. Every other chance and time of a wreck's here is set on the
// two days and taken between them (and a little beyond) by DayChance and
// DayTime.

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <utility>

namespace Poseidon::firespread
{

// ---- The shared object fire (fire-objects; the wrecks builder owns this block) ----
// One burning part of an object, as its owner (the wrecks here, the buildings)
// sets it every frame: a box's flames over its footprint (a wreck's body, a
// roof, a shed), a pool's on the ground, a face's out of an opening in a wall
// (a window, a door: out along the face, then up the wall), a jet's along its
// direction (an armour's propellant flare from a hatch, seconds long), a
// ball's round its centre (a fuel fireball: (x, y, z) its centre, halfL its
// radius now; its flames billow out and up from its lower part; its light is
// the stock detonation's, its smoke the stock's soot).
enum class ObjectFireShape : std::uint8_t
{
    Box,
    Pool,
    Face,
    Jet,
    Ball,
};
constexpr std::uint8_t kObjectOwnerWreck = 1, kObjectOwnerBuilding = 2;
struct ObjectFire
{
    std::uint64_t id = 0;   // stable for its life (a light keeps its place by it); never 0
    std::uint8_t owner = 0; // kObjectOwnerWreck or kObjectOwnerBuilding
    ObjectFireShape shape = ObjectFireShape::Box;
    float x = 0.0f, y = 0.0f, z = 0.0f; // its base's centre (world m; y the flames' feet)
    float groundY = 0.0f;               // the ground under it (m)
    float ux = 1.0f, uz = 0.0f;         // its long axis (unit, world)
    float halfL = 0.0f, halfW = 0.0f;   // half extents along and across it (m; 0 and 0 a point; a pool's ellipse)
    float faceX = 0.0f, faceZ = 0.0f;   // a face's outward normal (unit; 0 none)
    float jetX = 0.0f, jetY = 1.0f, jetZ = 0.0f; // a jet's direction (unit)
    // Now:
    float flameM = 0.0f; // its flames' length above their feet (m; 0 none; a jet's reach)
    // Its flames' full length (m), fixed for its life (0: flameM): their tempo
    // (FeatureRiseMps, the flipbook's rate, its puff's rate) follows it, never
    // flameM, which grows and falls over minutes (a rate that drifted would
    // race the flipbooks: the same reason a flare's and a torch's tempo
    // lengths are fixed).
    float flameRefM = 0.0f;
    float heat = 0.0f;   // how hard it burns (0..1: the tongues' brightness, its sparks)
    float heatMW = 0.0f; // its heat release (MW: its own plume's rise, its light, its sound)
    float smoke = 0.0f;  // its smoke (plume units: a grass fire makes about 1 a MW)
    float black = 0.8f;  // its smoke's black share (0 grey-white .. 1 black)
    float glow = 0.0f;   // embers or a hot shell glowing with no flame (0..1)
    // Its flames' look:
    float absorb = 6.0f; // visible absorption (1/m: the volume's density)
    float soot = 0.8f;   // sooty tips (0..1)
    float tempK = 0.0f;  // temperature offset (K: + a propellant jet's white-yellow)
    // What it lights round it (the shared draws, ObjectFireDraws, every gameplay tick):
    float heatReachM = 0.0f,
          heatMeanSec =
              0.0f; // cured grass out to this from its footprint's edge, each point in this mean time (0: never)
    float emberRate = 0.0f; // embers a second, each landing downwind off its edge ...
    float emberMedianM = 3.0f, emberSigma = 0.4f, emberMaxM = 6.0f; // ... a lognormal distance ...
    float emberChance = 0.0f;                                       // ... and lighting cured grass with this chance ...
    float emberDelaySec = 0.0f; // ... after smouldering about this long (0 to 2 times it; 0 at once)
    float sparkRate = 0.0f;     // sparks a second in the air at it (looks)
    float hurtM = 0.5f;         // people within this of its footprint burn (the grass fire's rate)
};
// A burst of burning brands (a car fireball's debris, a tanker's fuel rain and
// torn tank, a roof's collapse, an armour's blast): count of them leave (x, z)
// (off its edge: the owner's point) and land a lognormal distance off it
// (median medianM, log sigma sigma, within minM..maxM), downwind (dirX, dirZ;
// 0 and 0 all round) within spreadRad either side; each lights cured grass
// where it lands with `chance` (< 0: the build's pig, EmberPig) after a delay
// of delayMinSec to delayMaxSec. source names it in the logs.
struct EmberThrow
{
    float x = 0.0f, z = 0.0f;
    int count = 0;
    float medianM = 10.0f, sigma = 0.6f, minM = 0.0f, maxM = 300.0f;
    float dirX = 0.0f, dirZ = 0.0f, spreadRad = 0.35f;
    float chance = -1.0f;
    float delayMinSec = 0.0f, delayMaxSec = 0.0f;
    std::uint64_t source = 0;
    std::uint8_t kind = 0; // the draw's kind for the logs and stats (ObjectIgniteKind)
};
// What lit a point (the logs and stats line).
enum class ObjectIgniteKind : std::uint8_t
{
    Heat = 0,     // its flames' heat and drips (heatReachM), a pool under it
    Ember = 1,    // its embers on the wind
    Fireball = 2, // a fuel fireball's flame and heat
    Debris = 3,   // burning pieces a fireball throws
    Rain = 4,     // a tanker's burning fuel rain
    Fragment = 5, // a torn tank's or an armour's blast's fragments
    Brand = 6,    // anything else thrown (a roof's collapse)
};
constexpr int kObjectIgniteKinds = 7;
// Its light (FireSpreadWorld.cpp UpdateLights): its flames' luminance over
// them as drawn, kObjectLightNits (a sooty fuel and plastics fire: dimmer
// than a grass flame's body, its smoke shrouding its top), a jet's
// kJetLightNits (a propellant charge's white-yellow flare); a hot shell's
// glow kObjectGlowCd a metre of it. Its heat's radiant share
// (ObjectFluxAt: a sooty pool fire's 0.2 to 0.35).
constexpr float kObjectLightNits = 4000.0f, kJetLightNits = 20000.0f, kObjectGlowCd = 150.0f;
constexpr float kObjectRadiantShare = 0.3f;
// The build's ember ignition chance (FireSpreadWorld.cpp NotifyBurning's,
// the consult's "pig"): none at 20% dead fine fuel moisture, all but certain
// at 3%.
inline float EmberPig(float m1h)
{
    return std::pow(std::clamp((0.20f - m1h) / 0.17f, 0.0f, 1.0f), 1.5f);
}
// A small random source for the draws (xorshift32), seeded per object fire.
struct ObjRand
{
    std::uint32_t s = 0x9e3779b9U;
    explicit ObjRand(std::uint32_t seed = 0x9e3779b9U) : s(seed ? seed : 0x9e3779b9U) {}
    float U()
    {
        s ^= s << 13;
        s ^= s >> 17;
        s ^= s << 5;
        return static_cast<float>(s >> 8) / 16777216.0f;
    }
    float N() // a standard normal (Box-Muller)
    {
        const float a = std::max(U(), 1e-7f), b = U();
        return std::sqrt(-2.0f * std::log(a)) * std::cos(6.2831853f * b);
    }
    int Poisson(float mean) // small means only (Knuth)
    {
        if (mean <= 0.0f)
            return 0;
        const float l = std::exp(-std::min(mean, 30.0f));
        int k = 0;
        float p = 1.0f;
        do
        {
            ++k;
            p *= U();
        } while (p > l && k < 64);
        return k - 1;
    }
};
// The distance from a footprint's centre to its edge along the unit
// direction (dx, dz): a box's, a pool's ellipse's (0: a point).
inline float ObjectEdgeAlong(const ObjectFire& f, float dx, float dz)
{
    const float a = std::fabs(dx * f.ux + dz * f.uz), c = std::fabs(-dx * f.uz + dz * f.ux);
    if (f.halfL <= 0.0f && f.halfW <= 0.0f)
        return 0.0f;
    if (f.shape == ObjectFireShape::Pool)
    {
        const float q = (a * a) / std::max(f.halfL * f.halfL, 1e-4f) + (c * c) / std::max(f.halfW * f.halfW, 1e-4f);
        return 1.0f / std::sqrt(std::max(q, 1e-8f));
    }
    const float ta = a > 1e-5f ? f.halfL / a : 1e9f, tc = c > 1e-5f ? f.halfW / c : 1e9f;
    return std::min(ta, tc);
}
// How far (m) a point lies outside a footprint (0 inside): a box's, a pool's.
inline float ObjectOutside(const ObjectFire& f, float px, float pz)
{
    const float dx = px - f.x, dz = pz - f.z;
    const float a = dx * f.ux + dz * f.uz, c = -dx * f.uz + dz * f.ux;
    if (f.shape == ObjectFireShape::Pool && f.halfL > 0.0f && f.halfW > 0.0f)
    {
        const float d = std::sqrt(dx * dx + dz * dz);
        if (d < 1e-4f)
            return 0.0f;
        return std::max(d - ObjectEdgeAlong(f, dx / d, dz / d), 0.0f);
    }
    const float ea = std::max(std::fabs(a) - f.halfL, 0.0f), ec = std::max(std::fabs(c) - f.halfW, 0.0f);
    return std::sqrt(ea * ea + ec * ec);
}
// A point drawn evenly over a footprint grown by `grow` metres all round (a
// box's corners rounded, a pool's ellipse grown): (px, pz).
inline void ObjectFootPoint(const ObjectFire& f, float grow, ObjRand& r, float& px, float& pz)
{
    const float g = std::max(grow, 0.0f);
    const float L = std::max(f.halfL, 0.0f) + g, W = std::max(f.halfW, 0.0f) + g;
    for (int tries = 0; tries < 8; ++tries)
    {
        const float a = (2.0f * r.U() - 1.0f) * L, c = (2.0f * r.U() - 1.0f) * W;
        px = f.x + a * f.ux - c * f.uz;
        pz = f.z + a * f.uz + c * f.ux;
        if (ObjectOutside(f, px, pz) <= g)
            return;
    }
    px = f.x;
    pz = f.z;
}
// A point of the ground in a footprint's heat (its reach `reach` off its edge,
// and its outside distance `off`): a pool's under it and round it (evenly
// over its grown ellipse); a box's or a point's round it, a point of its
// edge (evenly along its perimeter) stepped out an even share of the reach
// (so a thin strip of grass along one side is found as readily as a wide
// field: the heat lights the nearest fuel, not the most of it).
inline void ObjectHeatPoint(const ObjectFire& f, float reach, ObjRand& r, float& px, float& pz, float& off)
{
    reach = std::max(reach, 0.0f);
    if (f.shape == ObjectFireShape::Pool)
    {
        ObjectFootPoint(f, reach, r, px, pz);
        off = ObjectOutside(f, px, pz);
        return;
    }
    const float L = std::max(f.halfL, 0.0f), W = std::max(f.halfW, 0.0f);
    const float d = reach * r.U();
    off = d;
    if (L + W < 1e-3f || r.U() < 0.35f)
    {
        // (a point's, and a share of a box's: out along a direction from its
        // middle, so its corners' ground is tried too)
        const float a = 6.2831853f * r.U();
        const float cx = std::cos(a), cz = std::sin(a);
        const float e = ObjectEdgeAlong(f, cx, cz);
        px = f.x + cx * (e + d);
        pz = f.z + cz * (e + d);
        off = ObjectOutside(f, px, pz);
        return;
    }
    // a point of the perimeter: the long sides (2L each), then the ends (2W)
    const float s = r.U() * 4.0f * (L + W);
    float a = 0.0f, c = 0.0f, na = 0.0f, nc = 0.0f;
    if (s < 4.0f * L)
    {
        a = (s < 2.0f * L ? s : s - 2.0f * L) - L;
        c = s < 2.0f * L ? W : -W;
        nc = s < 2.0f * L ? 1.0f : -1.0f;
    }
    else
    {
        const float q = s - 4.0f * L;
        c = (q < 2.0f * W ? q : q - 2.0f * W) - W;
        a = q < 2.0f * W ? L : -L;
        na = q < 2.0f * W ? 1.0f : -1.0f;
    }
    a += na * d;
    c += nc * d;
    px = f.x + a * f.ux - c * f.uz;
    pz = f.z + a * f.uz + c * f.ux;
}
// The shared draws that light the ground round an object fire, for dt
// seconds in the 10 m wind (windX, windZ): its heat (with the chance 1 -
// exp(-dt / heatMeanSec) a tick, kHeatTryPoints points of the ground in its
// reach (ObjectHeatPoint) are tried nearest its edge first until one lights:
// the hottest ground; so any cured grass within its reach lights in about
// heatMeanSec, as the consult has it, "certain inside that distance within 1
// to 3 min") and its embers (a Poisson count of emberRate x dt, each landing
// downwind off its edge a lognormal distance, lighting with emberChance).
// `light(x, z, chance, delaySec, kind)` is called for each try and returns
// true when the ground there takes fire or could (the heat's try stops
// there); the caller checks the ground (bare, wet, burnt) and rolls the
// chance.
constexpr int kHeatTryPoints = 24;
constexpr float kEmberSpreadCalmRad = 1.2f, kEmberSpreadWindRad = 0.35f, kEmberSpreadWindMps = 6.0f;
// Embers leave downwind, within this many radians either side (a calm air
// spreads them all round: kEmberSpreadCalmRad at no wind, narrowing to
// kEmberSpreadWindRad by kEmberSpreadWindMps).
inline float EmberSpreadRad(float wind10)
{
    const float k = std::clamp(wind10 / kEmberSpreadWindMps, 0.0f, 1.0f);
    return kEmberSpreadCalmRad + (kEmberSpreadWindRad - kEmberSpreadCalmRad) * k;
}
// Where one brand lands: off (x, z) by a lognormal distance (median medianM,
// log sigma sigma, within minM..maxM), its way turned from (dirX, dirZ) by a
// normal spread (its standard deviation half of spreadRad, clamped there),
// all round when (dirX, dirZ) is 0.
inline void BrandLanding(float x, float z, float dirX, float dirZ, float spreadRad, float medianM, float sigma,
                         float minM, float maxM, ObjRand& r, float& lx, float& lz)
{
    const bool any = std::fabs(dirX) + std::fabs(dirZ) > 1e-4f;
    const float ang =
        any ? std::atan2(dirZ, dirX) + std::clamp(0.5f * spreadRad * r.N(), -spreadRad, spreadRad) : 6.2831853f * r.U();
    const float d = std::clamp(medianM * std::exp(sigma * r.N()), minM, std::max(maxM, minM));
    lx = x + std::cos(ang) * d;
    lz = z + std::sin(ang) * d;
}
template <class Light>
inline void ObjectFireDraws(const ObjectFire& f, float dt, float windX, float windZ, ObjRand& r, Light&& light)
{
    if (dt <= 0.0f)
        return;
    if (f.heatMeanSec > 0.0f && (f.heatReachM > 0.0f || f.halfL > 0.0f || f.halfW > 0.0f))
    {
        const float p = 1.0f - std::exp(-dt / f.heatMeanSec);
        if (r.U() < p)
        {
            float px[kHeatTryPoints], pz[kHeatTryPoints], out[kHeatTryPoints];
            int order[kHeatTryPoints];
            for (int i = 0; i < kHeatTryPoints; ++i)
            {
                ObjectHeatPoint(f, f.heatReachM, r, px[i], pz[i], out[i]);
                order[i] = i;
            }
            std::sort(order, order + kHeatTryPoints, [&](int a, int b) { return out[a] < out[b]; });
            for (int i = 0; i < kHeatTryPoints; ++i)
                if (light(px[order[i]], pz[order[i]], 1.0f, 0.0f, ObjectIgniteKind::Heat))
                    break;
        }
    }
    if (f.emberRate > 0.0f && f.emberChance > 0.0f)
    {
        const int n = r.Poisson(f.emberRate * dt);
        const float w = std::sqrt(windX * windX + windZ * windZ);
        const float dx = w > 0.2f ? windX / w : 0.0f, dz = w > 0.2f ? windZ / w : 0.0f;
        const float spread = EmberSpreadRad(w);
        for (int i = 0; i < n; ++i)
        {
            // its way first (as BrandLanding turns it), then its edge along that way
            const float ang = (std::fabs(dx) + std::fabs(dz) > 0.0f)
                                  ? std::atan2(dz, dx) + std::clamp(0.5f * spread * r.N(), -spread, spread)
                                  : 6.2831853f * r.U();
            const float cx = std::cos(ang), cz = std::sin(ang);
            const float edge = ObjectEdgeAlong(f, cx, cz);
            const float d =
                std::clamp(f.emberMedianM * std::exp(f.emberSigma * r.N()), 0.0f, std::max(f.emberMaxM, 0.0f));
            const float delay = f.emberDelaySec > 0.0f ? f.emberDelaySec * 2.0f * r.U() : 0.0f;
            light(f.x + cx * (edge + d), f.z + cz * (edge + d), f.emberChance, delay, ObjectIgniteKind::Ember);
        }
    }
}

// ---- The two days (the tenth answer) ----
constexpr float kDayTypicalM1h = 0.075f, kDayWorstM1h = 0.035f;
// Where a dead fine fuel moisture stands between the two days: 0 the typical,
// 1 the worst; beyond them a little (kDayShareMin..Max), so a damper day
// keeps falling and a drier one stays near the worst.
constexpr float kDayShareMin = -2.0f, kDayShareMax = 1.25f;
inline float DayShare(float m1h)
{
    return std::clamp((kDayTypicalM1h - m1h) / (kDayTypicalM1h - kDayWorstM1h), kDayShareMin, kDayShareMax);
}
// A chance set on both days, between them log-linear in the share (a chance
// at most kDayChanceMax beyond the worst).
constexpr float kDayChanceMax = 0.995f;
inline float DayChance(float typical, float worst, float m1h)
{
    const float s = DayShare(m1h);
    const float a = std::log(std::max(typical, 1e-6f)), b = std::log(std::max(worst, 1e-6f));
    return std::clamp(std::exp(a + (b - a) * s), 0.0f, kDayChanceMax);
}
// A time (s) set on both days, log-linear in the share.
inline float DayTime(float typical, float worst, float m1h)
{
    const float s = DayShare(m1h);
    const float a = std::log(std::max(typical, 1e-3f)), b = std::log(std::max(worst, 1e-3f));
    return std::exp(a + (b - a) * s);
}

// ---- Wrecks ----
// A wreck's class (WreckClassOf, from its config): a car or jeep; a truck (or
// an aircraft's wreck); a fuel tanker with fuel in its tank; armour (tracked,
// or armoured), with or without ammunition (WreckConfig::ammo).
enum class WreckClass : std::uint8_t
{
    Car = 0,
    Truck = 1,
    Tanker = 2,
    Armour = 3,
};
constexpr int kWreckClasses = 4;
// The thresholds (CfgVehicles: transportFuel, armor, the shape's length, the
// mass): a tanker carries kTankerFromL or more fuel; armour is tracked or
// kArmourFromArmor or more; a truck is kTruckFromM long or more, or
// kTruckFromKg heavy.
constexpr float kTankerFromL = 1000.0f, kArmourFromArmor = 100.0f, kTruckFromM = 6.0f, kTruckFromKg = 4500.0f;
inline WreckClass WreckClassOf(float fuelCargoL, bool tracked, float armor, float lengthM, float massKg)
{
    if (fuelCargoL >= kTankerFromL)
        return WreckClass::Tanker;
    if (tracked || armor >= kArmourFromArmor)
        return WreckClass::Armour;
    if (lengthM >= kTruckFromM || massKg >= kTruckFromKg)
        return WreckClass::Truck;
    return WreckClass::Car;
}
const char* WreckClassName(WreckClass c);
// What the wreck code reads off the vehicle as it dies.
struct WreckConfig
{
    WreckClass cls = WreckClass::Car;
    bool fuelBurning =
        true;          // its fuel was in its tank (a car whose tank was empty burns slowly: its peak 10 to 20 min on)
    bool ammo = false; // armour carrying ammunition: jets and blasts
    float fuelCargoL = 0.0f; // a tanker's load (l)
    float sizeK = 1.0f;      // a motorcycle's 0.5, the rest 1 (its flames and heat)
};
// A fire's heat release (MW) from its flame's height L over a fire D metres
// across (Heskestad 1983, L = 0.235 Q^(2/5) - 1.02 D, Q in kW): a car's 4 m
// flames over 2.5 m about 4 MW, its 2.5 m about 2, a tanker's 22 m pool
// flames over 11.3 m about 240 MW (the consult: 3 to 6, 1.5 to 3 and 240).
inline float HeskestadMW(float flameM, float diameterM)
{
    const float q = (std::max(flameM, 0.0f) + 1.02f * std::max(diameterM, 0.1f)) / 0.235f;
    return 0.001f * std::pow(q, 2.5f);
}
// Its timeline, drawn once from the tenth answer's ranges by its seed (all
// times in seconds after it was destroyed):
//   - a car or jeep: its fuel burning from the first second (40 to 80 l):
//     flames fuelFlameM (3 to 5 m) for fuelSec (2 to 5 min), a pool of
//     poolAreaM2 (5 to 15 m2) under and round it; then seats, trim and wiring,
//     bodyFlameM (2 to 3 m) from every opening; from tyreSec (about 20 min)
//     the tyres, under 1.5 m at the wheels and the densest black smoke; flaming
//     over at flameEndSec (30 to 45 min); small flames at the tyres a while
//     after; smoking to smokeEndSec (1 to 2 h), the shell hot to hotEndSec (2
//     to 4 h), its thin bodywork glowing dull red by night for glowEndSec (20
//     to 30 min). Without its fuel: no fuel phase, the body at its peak at
//     bodyPeakSec (10 to 20 min);
//   - a truck: its fuel phase (4 to 6 m), then the body to its peak (bodyFlameM
//     4 to 7 m) at 10 to 20 min, flaming for 1 to 1.5 h, six to ten tyres an
//     hour of black smoke, smoking 2 to 4 h; its embers twice a car's;
//   - a fuel tanker (its tank opened): a pool of poolAreaM2 (100 m2 for a
//     full 5 m3 load) burning poolFlameM (20 to 25 m) for poolSec (5 to 15 min),
//     then as a truck (the truck burns beside it from the start);
//   - armour: its fuel (500 to 1000 l of diesel) and rubber, bodyFlameM (3 to 6
//     m) steady from its hatches and engine deck, flaming 1 to 3 h, the hull hot
//     6 to 12 h; with ammunition, propellant jets (5 to 20 m for 2 to 10 s,
//     several, from 2 to 30 min) and now and then a high-explosive blast
//     throwing hot fragments 50 to 200 m.
// Its embers: kEmberFuelRate a second (times emberK) through emberFuelSec (the
// fuel phase's 2 to 3 min), then kEmberAfterRate until flaming ends, then none
// (the consult: "otherwise a 40 min wreck would light the grass for certain
// even on a damp day"), each lighting with the build's pig.
constexpr int kWreckJetsMax = 8, kWreckBlastsMax = 2;
constexpr float kEmberFuelRate = 0.07f, kEmberAfterRate = 0.005f;
// (followups) Its char (WreckStateAt charK) follows the dose of flame it has
// taken (its flames over their peak, a second at a time; a tanker's pool
// engulfs it), kept at kWreckCharSteps points to its flames' end: 1 -
// exp(-dose / tau), tau its whole dose over kWreckCharK, so a car is about
// half charred by the end of its fuel phase and burnt out when its flames end.
constexpr int kWreckCharSteps = 32;
constexpr float kWreckCharK = 3.0f;
struct WreckTimeline
{
    WreckClass cls = WreckClass::Car;
    bool fuelBurning = true, ammo = false;
    float sizeK = 1.0f;
    float fuelSec = 0.0f, fuelFlameM = 0.0f; // the fuel phase (0: none)
    float emberFuelSec = 0.0f;               // its embers' fuel phase
    float bodyPeakSec = 0.0f, bodyFlameM = 0.0f;
    float bodyHoldSec = 0.0f;                // the body holds its peak to here, then falls toward the tyres'
    float tyreSec = 0.0f, tyreFlameM = 0.0f; // the tyres' and what is left's flames from here
    float flameEndSec = 0.0f;                // flaming over
    float smoulderEndSec = 0.0f;             // small flames at the tyres to here
    float smokeEndSec = 0.0f, hotEndSec = 0.0f, glowEndSec = 0.0f;
    float poolAreaM2 = 0.0f, poolFlameM = 0.0f, poolSec = 0.0f; // its pool (a car's under it, a tanker's)
    float emberK = 1.0f;                                        // its embers' rates times this
    float heatReachM = 0.0f;                                    // its heat's reach at its peak (m off its edge)
    float poolReachM = 0.0f;                                    // a tanker's pool's heat reach (m off the pool's edge)
    int jets = 0;
    float jetAt[kWreckJetsMax] = {}, jetSec[kWreckJetsMax] = {}, jetM[kWreckJetsMax] = {};
    int blasts = 0;
    float blastAt[kWreckBlastsMax] = {};
    // (followups) its tyres (and a tracked hull's rubber) catch at
    // tyreCatchSec and burn on to its flames' end, their smoke at full
    // tyreSmokeShare of its flames' peak smoke; its char's dose to its
    // flames' end (kWreckCharSteps + 1 points, the last its whole dose)
    float tyreCatchSec = 0.0f, tyreSmokeShare = 0.0f;
    float charDose[kWreckCharSteps + 1] = {};
};
WreckTimeline WreckTimelineOf(const WreckConfig& c, std::uint32_t seed);
// Its body's flames t seconds on (m), before their slow wander and with no
// smouldering at its tyres (WreckStateAt adds both).
float WreckBodyFlameAt(const WreckTimeline& w, float t);
// Its state t seconds after it was destroyed (diameterM its body's width
// across, for its heat).
struct WreckState
{
    float bodyFlameM = 0.0f, bodyHeat = 0.0f, bodyMW = 0.0f;
    float poolFlameM = 0.0f, poolMW = 0.0f, poolAreaM2 = 0.0f;
    float smoke = 0.0f, black = 0.0f; // its smoke (plume units) and how black
    float emberRate = 0.0f;           // embers a second before the chance (kEmberFuelRate ...)
    bool emberFuel = false;           // ... the fuel phase's (close in) or the body's
    float heatShare = 0.0f;           // its heat over its peak's (its heat's reach and pace)
    float glow = 0.0f;                // its glow points' and its light's (0..1: bodywork, then embers at its tyres)
    // (followups) for the renderer (EngineGL33 burnWreck): how far its paint,
    // glass, canvas and tyres have burnt (0..1, 1 from its flames' end on),
    // and its shell's heat glow by night (0..1, to hotEndSec)
    float charK = 0.0f, shellGlow = 0.0f;
    float jetM = 0.0f; // a jet's reach now (0: none)
    int jet = -1;      // which
    bool flaming = false, smoking = false, done = false;
};
WreckState WreckStateAt(const WreckTimeline& w, float t, float diameterM);
// Smoke per MW (plume units) by what burns: petrol and diesel, the body's
// trim and plastics, tyres (the soot yields: grass about 0.01 g/g, petrol 0.06
// to 0.1, rubber 0.1 to 0.15, so per MW several times a grass fire's).
constexpr float kWreckSmokeFuel = 5.0f, kWreckSmokeBody = 6.0f, kWreckSmokeTyre = 12.0f, kWreckSmokePool = 3.0f;
// (followups) The smoke over its timeline (the consult's tenth answer: a car's
// fuel fire 0 to 5 min, black; seats, trim, wiring and the first tyres 5 to 20
// min; tyres and what is left 20 to 45 min, under 1.5 m of flame at the wheels
// and "the densest black smoke of the whole fire"; a smoking shell to 1 to 2
// h; a truck's six to ten tyres an hour of black smoke). Its tyres' smoke is
// its own: catching kTyreCatch{Min,Max}Sec after its fuel phase (a truck's
// and armour's by their own), rising to full by its tyre phase and holding
// to its flames' end at a share of its flames' peak smoke (kTyreSmoke*: a
// car's so its tyre phase, with its low plume, is the densest of its fire),
// then kTyreSmoulderShare of it with the small flames at its tyres; after its
// flames its shell smokes kShellSmokeShare of its tyres' full smoke, fading
// to its smoke's end. Rubber is several times as sooty as petrol a megawatt
// (kWreckSmokeTyre), so its few hundred kilowatts make the most smoke.
constexpr float kTyreCatchMinSec = 180.0f, kTyreCatchMaxSec = 420.0f;
constexpr float kTyreSmokeCar = 0.75f, kTyreSmokeTruck = 0.4f, kTyreSmokeArmour = 0.3f;
constexpr float kTyreSmoulderShare = 0.4f, kShellSmokeShare = 0.3f;
// (followups) Its shell's heat glow (WreckState shellGlow): while it flames
// the thin bodywork in its flames glows dull red (by night; the consult: the
// first 20 to 30 min of a car), kShellGlowFlaming plus the rest by its heat;
// after its flames the shell cools over its hot time (hotEndSec), its glow
// from kShellGlowEnd falling as exp(-t / (kShellGlowFall x its cooling span))
// to nothing at its end: a clear dull red for the first half hour or so,
// then embers glowing low in its floor and wheels.
constexpr float kShellGlowFlaming = 0.35f, kShellGlowEnd = 0.55f, kShellGlowFall = 0.3f, kShellGlowInSec = 60.0f;

// ---- A wreck's reach on the two days (calibrated: WreckDayOf) ----
// The build-to table: grass within 3 m of a wreck lights 85 to 95% of the time
// within 1 to 3 min on the typical day, for certain within seconds on the
// worst; from the middle of an 8 m bare road 20 to 40% within 2 to 15 min
// (typical) and 80 to 95% within 10 to 60 s (worst). The sources:
//   - its heat and drips: cured grass within heatReachM of its edge (a car 1.5
//     to 2.5 m, a truck 3 to 4 m, a tanker's pool 15 to 25 m off the pool),
//     each point in a mean time of kHeatMean* at its peak (longer as its heat
//     falls); grass under a pool kPoolMean*;
//   - its embers (above), landing downwind off its edge a lognormal distance:
//     through the fuel phase burning fuel and drips close in (median
//     kFuelEmberMedian*), then the interior's lofted brands farther (median
//     kBodyEmberMedian*), together the consult's 2 to 6 m (typical) and 3 to
//     15 m (worst) from the wreck;
//   - its fuel fireball (a car's 10 to 12 m across for about 1 s): the cured
//     grass under it (its ground disc kFireballGroundShare of its radius)
//     lights, as a whole, with kFireballFull* (the consult: about half the
//     time on the typical day, nearly always on the worst), less toward its
//     edge (each point weighted 1 - (r / R)^2);
//   - the burning pieces it throws (3 to 10 of a car's, 5 to 30 m, all round),
//     each lighting with kDebris* after a smoulder of kDebrisDelay*;
//   - a tanker's fireball (500 to 1000 kg: 46 to 58 m across, 4 s) lights all
//     the ground out to kTankerRingTyp (typical) to kTankerRingWorst (worst)
//     radii at once, rains burning fuel over one to two radii (kRainDrops,
//     each kRain*) and throws 1 to 5 torn pieces 100 to 200 m (each kFrag*);
//   - armour's blasts throw kBlastFragsMin..Max fragments 50 to 200 m (each
//     kBlastFrag*: one or two spot fires a blast on the worst day).
// Where the consult's per-source numbers disagree with its own outcomes the
// outcomes win (NOTES.md): its 0.4 to 0.6 a piece of debris would light the
// road case nine times in ten within seconds, since pieces thrown 5 to 30 m
// from the middle of an 8 m road land on the verges four times in five; the
// typical day's piece is kDebrisTyp, its worst kDebrisWorst.
constexpr float kHeatMeanTypSec = 80.0f, kHeatMeanWorstSec = 4.0f;
constexpr float kPoolMeanTypSec = 40.0f, kPoolMeanWorstSec = 2.0f;
// The two days differ in how soon, not only how often (the wrecks shooter's
// before: its embers lit the verge in about 14 s on both days): on the
// typical day (7.5% dead fuel, 40 to 45% air) a brand, the fireball's touch or
// a fuel spill smoulders in the grass before it flames, about
// kSmoulderTypSec (0 to 2 times it); on the worst day (3.5%, 10 to 15%) at
// once, kSmoulderWorstSec.
constexpr float kSmoulderTypSec = 45.0f, kSmoulderWorstSec = 1.0f;
constexpr float kFuelEmberMedianTypM = 1.0f, kFuelEmberMedianWorstM = 3.6f, kFuelEmberMaxTypM = 4.0f;
constexpr float kBodyEmberMedianTypM = 1.9f, kBodyEmberMedianWorstM = 4.5f, kBodyEmberMaxTypM = 6.0f;
constexpr float kEmberSigmaTyp = 0.45f, kEmberSigmaWorst = 0.5f, kEmberMaxWorstM = 15.0f;
constexpr float kFireballGroundShare = 0.9f, kFireballFullTyp = 0.5f, kFireballFullWorst = 0.97f;
constexpr int kDebrisMin = 3, kDebrisMax = 10;
constexpr float kDebrisMinM = 5.0f, kDebrisMaxM = 30.0f, kDebrisMedianM = 11.0f, kDebrisSigma = 0.5f;
constexpr float kDebrisTyp = 0.02f, kDebrisWorst = 0.12f;
constexpr float kDebrisDelayTypSec = 40.0f, kDebrisDelayWorstSec = 8.0f;
constexpr float kTankerRingTyp = 1.6f, kTankerRingWorst = 2.0f;
constexpr int kRainDrops = 24;
constexpr float kRainTyp = 0.5f, kRainWorst = 0.95f;
constexpr int kFragsMin = 1, kFragsMax = 5;
constexpr float kFragMinM = 100.0f, kFragMaxM = 200.0f, kFragTyp = 0.5f, kFragWorst = 0.99f;
constexpr int kBlastFragsMin = 2, kBlastFragsMax = 4;
constexpr float kBlastFragMinM = 50.0f, kBlastFragMaxM = 200.0f, kBlastFragTyp = 0.15f, kBlastFragWorst = 0.5f;
// The tanker's fireball (the fuel in it, kg: 500 to 1000 of a full 5 m3 load,
// less from a smaller one) and a petrol pool's area (m2: 100 for 1.5 m3 or more
// poured out, kPoolAreaMinM2 at least).
constexpr float kTankerBallMinKg = 500.0f, kTankerBallMaxKg = 1000.0f, kPoolFullM3 = 1.5f, kPoolAreaMinM2 = 30.0f;
struct WreckDay
{
    float pig = 0.0f; // each ember's chance
    float heatMeanSec = 0.0f, poolMeanSec = 0.0f;
    float fuelEmberMedianM = 0.0f, fuelEmberMaxM = 0.0f; // the fuel phase's embers
    float bodyEmberMedianM = 0.0f, bodyEmberMaxM = 0.0f; // the body's after it
    float emberSigma = 0.0f;
    float fireballFull = 0.0f, debris = 0.0f, debrisDelaySec = 0.0f;
    float smoulderSec = 0.0f; // a brand's or a touch's smoulder before it flames (mean)
    float tankerRing = 0.0f, rain = 0.0f, frag = 0.0f, blastFrag = 0.0f;
};
WreckDay WreckDayOf(float m1h);
// A fireball's flame contact (the draws at its birth): kFireballPoints points
// over its ground disc (radius groundR round (x, z)), each tried with 1 -
// exp(-h w), w = 1 - (r / groundR)^2 and h set so the whole disc lights with
// `full`; `light(x, z, chance, delay, kind)` for each (chance 1: already
// rolled).
constexpr int kFireballPoints = 32;
template <class Light>
inline void FireballContact(float x, float z, float groundR, float full, ObjRand& r, Light&& light,
                            float smoulderSec = 0.0f)
{
    if (groundR <= 0.0f || full <= 0.0f)
        return;
    // the weights' sum over the disc's points (a disc's 1 - r^2 averages 1/2)
    const float h = -std::log(std::max(1.0f - std::min(full, 0.9999f), 1e-6f)) / (0.5f * kFireballPoints);
    for (int i = 0; i < kFireballPoints; ++i)
    {
        // stratified over the disc (the R2 sequence turned by the seed)
        const float qa = std::fmod(0.5f + 0.7548777f * static_cast<float>(i) + r.U() * 0.05f, 1.0f);
        const float qb = std::fmod(0.5f + 0.5698403f * static_cast<float>(i) + r.U() * 0.05f, 1.0f);
        const float rr = groundR * std::sqrt(qa), ang = 6.2831853f * qb;
        const float w = 1.0f - (rr / groundR) * (rr / groundR);
        if (r.U() < 1.0f - std::exp(-h * w))
            light(x + rr * std::cos(ang), z + rr * std::sin(ang), 1.0f, smoulderSec * 2.0f * r.U(),
                  ObjectIgniteKind::Fireball);
    }
}

// A throw's brands (EmberThrow), each `light(x, z, chance, delaySec, kind)`
// (the throw's chance, or `pig` for < 0).
template <class Light>
inline void EmberThrowDraws(const EmberThrow& e, float pig, ObjRand& r, Light&& light)
{
    const float chance = e.chance < 0.0f ? pig : e.chance;
    for (int i = 0; i < e.count; ++i)
    {
        float lx = 0.0f, lz = 0.0f;
        BrandLanding(e.x, e.z, e.dirX, e.dirZ, e.spreadRad, e.medianM, e.sigma, e.minM, e.maxM, r, lx, lz);
        const float delay = e.delayMinSec + (e.delayMaxSec - e.delayMinSec) * r.U();
        light(lx, lz, chance, delay, static_cast<ObjectIgniteKind>(e.kind));
    }
}

// ---- A wreck's object fires (WreckBodyFire, WreckPoolFire) and its fireball's draws ----
// Its body (a box: the shape's plan, its flames' feet kWreckFootShare of its
// height up) and its pool (an ellipse round it of the pool's area; a tanker's
// a disc), their ground-lighting fields for its state now on the day: the
// heat's reach falls as the square root of its heat over its peak's and its
// pace with the heat (a dying fire lights nothing new).
constexpr float kWreckFootShare = 0.3f;
// Its flames stand over kWreckFlameShare of its plan, their bodies dense and
// sooty (kWreckAbsorb, 1/m), a little deeper orange than a grass flame
// (kWreckTempK); sparks kWreckSparksPerM a metre of flame (burning trim and
// upholstery; half that through the fuel phase: petrol makes soot, not
// sparks). A propellant jet releases kWreckJetMW, burns kWreckJetTempK hotter
// (white-yellow) and throws kWreckJetSparks a second; a high-explosive blast
// flashes kWreckFlashM high for kWreckFlashSec. At most kWrecksMax wrecks burn
// at once (the oldest goes first); one whose vehicle is gone fades out over
// kWreckGoneSec.
constexpr float kWreckFlameShare = 0.85f, kWreckAbsorb = 4.5f, kWreckTempK = -40.0f, kWreckSparksPerM = 6.0f;
constexpr float kWreckJetMW = 20.0f, kWreckJetTempK = 350.0f, kWreckJetSparks = 250.0f;
constexpr float kWreckFlashM = 9.0f, kWreckFlashSec = 0.6f, kWreckGoneSec = 10.0f, kWreckBallTempK = 80.0f;
constexpr int kWrecksMax = 48;
// Its fuel fireball (radius R, CCPS: its duration 0.45 M^(1/3) s, so 0.155 R
// s), drawn through the flame pass: it swells from kBallStartShare of R to R
// over kBallSwellShare of its life, rising from kBallRise0 to kBallRise1 of R
// above the wreck, bright to kBallFadeShare of its life, then dulling and
// darkening into its soot (the stock's black smoke takes over).
constexpr float kBallSecPerM = 0.155f, kBallStartShare = 0.35f, kBallSwellShare = 0.35f;
constexpr float kBallRise0 = 0.4f, kBallRise1 = 0.9f, kBallFadeShare = 0.6f;
struct BallState
{
    float radiusM = 0.0f, riseM = 0.0f, heat = 0.0f, soot = 0.0f;
    bool on = false;
};
inline BallState FireballAt(float R, float age)
{
    BallState b;
    const float T = std::max(kBallSecPerM * R, 0.3f);
    if (R <= 0.0f || age < 0.0f || age > T)
        return b;
    const float x = age / T;
    auto ss = [](float e0, float e1, float v)
    {
        const float k = std::clamp((v - e0) / (e1 - e0), 0.0f, 1.0f);
        return k * k * (3.0f - 2.0f * k);
    };
    b.on = true;
    b.radiusM = R * (kBallStartShare + (1.0f - kBallStartShare) * ss(0.0f, kBallSwellShare, x));
    b.riseM = R * (kBallRise0 + (kBallRise1 - kBallRise0) * x);
    b.heat = 1.0f - ss(kBallFadeShare, 1.0f, x);
    b.soot = 0.3f + 0.7f * ss(0.3f, 1.0f, x);
    return b;
}
inline void WreckBodyFire(const WreckTimeline& w, const WreckState& s, const WreckDay& d, ObjectFire& f)
{
    f.flameM = s.bodyFlameM;
    f.heat = s.bodyHeat;
    f.heatMW = s.bodyMW;
    f.smoke = s.smoke - s.poolMW * kWreckSmokePool;
    f.black = s.black;
    f.glow = s.glow;
    const bool hot = s.bodyFlameM > 0.3f && s.heatShare > 0.02f;
    f.heatReachM = hot ? w.heatReachM * std::sqrt(s.heatShare) : 0.0f;
    f.heatMeanSec = hot && d.heatMeanSec > 0.0f ? d.heatMeanSec / std::max(s.heatShare, 0.05f) : 0.0f;
    f.emberRate = s.emberRate;
    f.emberMedianM = s.emberFuel ? d.fuelEmberMedianM : d.bodyEmberMedianM;
    f.emberSigma = d.emberSigma;
    f.emberMaxM = s.emberFuel ? d.fuelEmberMaxM : d.bodyEmberMaxM;
    f.emberChance = d.pig;
    f.emberDelaySec = d.smoulderSec;
    f.hurtM = 0.5f + 0.2f * s.bodyFlameM;
}
// The pool round a body (bodyHalfL, bodyHalfW, its axis in f already): its
// ellipse of the pool's area now (a car's thin spill under and round it; a
// tanker's a disc), its flames, heat and smoke; the ground under it lights in
// the pool's mean time, a tanker's out to its heat's reach too.
inline void WreckPoolFire(const WreckTimeline& w, const WreckState& s, const WreckDay& d, float bodyHalfL,
                          float bodyHalfW, ObjectFire& f)
{
    f.shape = ObjectFireShape::Pool;
    f.flameM = s.poolFlameM;
    f.heat = s.poolFlameM > 0.0f ? 0.8f + 0.2f * std::min(s.poolFlameM / std::max(w.poolFlameM, 0.1f), 1.0f) : 0.0f;
    f.heatMW = s.poolMW;
    f.smoke = s.poolMW * kWreckSmokePool;
    f.black = 1.0f;
    const float a = std::max(s.poolAreaM2, 0.0f);
    if (w.cls == WreckClass::Tanker)
    {
        f.halfL = f.halfW = std::sqrt(a / 3.14159265f);
        const float share = std::clamp(s.poolMW / std::max(2.4f * w.poolAreaM2, 1.0f), 0.0f, 1.0f);
        f.heatReachM = w.poolReachM * std::sqrt(share);
        f.heatMeanSec = d.heatMeanSec > 0.0f && share > 0.02f ? d.heatMeanSec / std::max(share, 0.05f) : 0.0f;
    }
    else
    {
        // pi (L + e)(W + e) = a, e >= 0
        const float L = std::max(bodyHalfL, 0.1f), W = std::max(bodyHalfW, 0.1f);
        const float bq = L + W, cq = L * W - a / 3.14159265f;
        const float e = std::max(0.0f, 0.5f * (-bq + std::sqrt(std::max(bq * bq - 4.0f * cq, 0.0f))));
        f.halfL = L + e;
        f.halfW = W + e;
        f.heatReachM = 0.0f;
        f.heatMeanSec = s.poolAreaM2 > 0.0f ? d.poolMeanSec : 0.0f;
    }
    f.emberRate = 0.0f;
    f.hurtM = 0.3f;
}
// The fireball's draws at its birth (radius ballR m, its centre over the
// body's): a car's, a truck's or armour's flame contact under it and its
// burning debris (kDebrisMin..Max pieces all round); a tanker's ring of fire
// (area(x, z, r): every cell within r lit at once), its fuel rain and its
// torn tank's pieces. `light` as ObjectFireDraws'.
template <class Light, class Area>
inline void WreckFireballDraws(const WreckTimeline& w, const WreckDay& d, const ObjectFire& body, float ballR,
                               ObjRand& r, Light&& light, Area&& area)
{
    if (ballR <= 0.0f)
        return;
    if (w.cls == WreckClass::Tanker)
    {
        if (d.tankerRing > 0.0f)
            area(body.x, body.z, d.tankerRing * ballR);
        EmberThrow rain;
        rain.x = body.x;
        rain.z = body.z;
        rain.count = kRainDrops;
        rain.medianM = 1.45f * ballR;
        rain.sigma = 0.2f;
        rain.minM = ballR;
        rain.maxM = 2.0f * ballR;
        rain.chance = d.rain;
        rain.delayMaxSec = 3.0f;
        rain.kind = static_cast<std::uint8_t>(ObjectIgniteKind::Rain);
        EmberThrowDraws(rain, d.pig, r, light);
        EmberThrow frag;
        frag.x = body.x;
        frag.z = body.z;
        frag.count = kFragsMin + static_cast<int>(r.U() * static_cast<float>(kFragsMax - kFragsMin + 1));
        frag.medianM = 140.0f;
        frag.sigma = 0.2f;
        frag.minM = kFragMinM;
        frag.maxM = kFragMaxM;
        frag.chance = d.frag;
        frag.delayMinSec = 5.0f;
        frag.delayMaxSec = 30.0f;
        frag.kind = static_cast<std::uint8_t>(ObjectIgniteKind::Fragment);
        EmberThrowDraws(frag, d.pig, r, light);
        return;
    }
    FireballContact(body.x, body.z, kFireballGroundShare * ballR, d.fireballFull, r, light, d.smoulderSec);
    EmberThrow deb;
    deb.x = body.x;
    deb.z = body.z;
    deb.count = kDebrisMin + static_cast<int>(r.U() * static_cast<float>(kDebrisMax - kDebrisMin + 1));
    deb.medianM = kDebrisMedianM;
    deb.sigma = kDebrisSigma;
    deb.minM = kDebrisMinM;
    deb.maxM = kDebrisMaxM;
    deb.chance = d.debris;
    deb.delayMinSec = 0.3f * d.debrisDelaySec;
    deb.delayMaxSec = 1.7f * d.debrisDelaySec;
    deb.kind = static_cast<std::uint8_t>(ObjectIgniteKind::Debris);
    EmberThrowDraws(deb, d.pig, r, light);
}
// An armour's high-explosive blast's fragments (kBlastFragsMin..Max, 50 to 200 m, all round).
template <class Light>
inline void WreckBlastDraws(const WreckDay& d, float x, float z, ObjRand& r, Light&& light)
{
    EmberThrow e;
    e.x = x;
    e.z = z;
    e.count = kBlastFragsMin + static_cast<int>(r.U() * static_cast<float>(kBlastFragsMax - kBlastFragsMin + 1));
    e.medianM = 100.0f;
    e.sigma = 0.4f;
    e.minM = kBlastFragMinM;
    e.maxM = kBlastFragMaxM;
    e.chance = d.blastFrag;
    e.delayMinSec = 2.0f;
    e.delayMaxSec = 20.0f;
    e.kind = static_cast<std::uint8_t>(ObjectIgniteKind::Fragment);
    EmberThrowDraws(e, d.pig, r, light);
}
// The fuel in a fireball (kg) with the fire lane on, by the wreck's class
// (the consult: a car's 5 to 10 kg, 10 to 12 m across for about 1 s; a
// tanker's 500 to 1000 kg, 46 to 58 m for about 4 s, less from a smaller
// load): stockKg the stock's (3.7 kg a hit of its FuelExplosion, at most 150;
// armour keeps it), u a draw (0..1), fuelL what was in its tank, loadL a
// tanker's load.
inline float WreckFireballKg(WreckClass c, float stockKg, float u, float fuelL, float loadL)
{
    switch (c)
    {
        case WreckClass::Car:
            return fuelL > 1.0f ? (5.0f + 5.0f * u) * std::clamp(fuelL / 40.0f, 0.3f, 1.0f) : 0.0f;
        case WreckClass::Truck:
            return fuelL > 1.0f ? (8.0f + 12.0f * u) * std::clamp(fuelL / 100.0f, 0.3f, 1.0f) : 0.0f;
        case WreckClass::Tanker:
            return (kTankerBallMinKg + (kTankerBallMaxKg - kTankerBallMinKg) * u) *
                   std::clamp(loadL / 5000.0f, 0.2f, 1.0f);
        default:
            return stockKg;
    }
}

} // namespace Poseidon::firespread
