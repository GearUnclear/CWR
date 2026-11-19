#pragma once

// Spreading wildland fire (FIRE-SPREAD explore lane, behind
// POSEIDON_XP_FIRESPREAD, off by default).
//
// The pure core: no engine state, no GL, so unit tests can run it.
//
// Physics, all from the published fire behaviour literature:
//  * Surface rate of spread: Rothermel (1972) with Albini's (1976)
//    corrections, the model behind BEHAVE, BehavePlus, FARSITE and FlamMap.
//  * Fuels: Anderson's (1982) 13 standard fire behaviour fuel models (loads,
//    surface-to-volume ratios, depths, moisture of extinction).
//  * Wind and slope: Rothermel's wind and slope factors combined as vectors
//    (the BEHAVE method); midflame wind from the 10 m wind by a wind
//    adjustment factor per fuel model (Baughman and Albini 1980, sheltered
//    under a canopy).
//  * Fire shape: an ellipse with the ignition at its rear focus, length to
//    breadth from the 10 m wind (Alexander 1985; the effective midflame wind
//    taken back to 10 m by the fuel's wind adjustment factor), so flank and
//    backing rates follow from the head rate.
//  * Build-up: a fire from a point (an ignition, a spot fire) does not run
//    at its equilibrium rate at once: R(t) = R_eq (1 - exp(-a t)) with t the
//    fire's age (the Canadian FBP System, Forestry Canada Fire Danger Group
//    1992, ST-X-3; see kBuildUpPerMin). A line ignition kMatureLineM or
//    wider starts at its full rate (Cheney and Gould 1995).
//  * Intensity and flames: Byram's fireline intensity from Rothermel's
//    reaction intensity and Anderson's (1969) flame residence time
//    (384 / sigma minutes); flame length from Byram (1959).
//  * Crowns: Van Wagner's (1977) critical surface intensity for torching
//    from canopy base height and foliar moisture; active crown spread by
//    Rothermel's (1991) 3.34 x fuel model 10 rule with Van Wagner's
//    critical rate (3 / canopy bulk density).
//  * Torching only where a tree stands in the cell, and only a conifer (Van
//    Wagner's formula was fitted to conifer stands; summer broadleaf foliage
//    at 150 to 250% moisture scorches rather than torches); active crown fire
//    only in conifer forest.
//  * Spotting: embers only from a torching tree, while it torches; each
//    ember's distance a lognormal draw (Sardoy et al. 2008), most landing a
//    few tens of metres out, never past Albini's (1979) maximum spot distance
//    from a single torching tree (AlbiniTorchSpotMaxM); the ignition chance
//    from the fine fuel moisture.
//
// The grid: a sparse grid of square cells (8 m by default). Each cell holds a
// fuel model, a canopy flag, its ground height, its state and the fire's
// anchor that reached it. The front is Huygens' principle on the model's own
// ellipse, not a lattice (the realism audit's C3): the ellipse's travel time
// from a point is a gauge, g(p) = (|p| - e p.h) / (R (1 - e)) (Gauge), so
// every direction is exact. Each caught cell carries an anchor (Anchor: an
// ignition point, a spot fire's landing point, the ignition line itself, or
// a short piece of front) and each step every unburnt neighbour of the front
// (the 8 adjacent cells; no knight's moves) takes, over the front cells round
// it, the soonest arrival from their anchors: the travel time integrated
// along the straight ray through each cell's own ellipse (its own fuel,
// slope and noise), through its build-up. An anchor counts only if that ray
// crosses burnable cells and no road; otherwise the giver's own piece of
// front (a short segment through its centre, square to its front) does, so a
// fire entering a new fuel restarts there as a straight front. Past
// kAnchorMaxM from a cell a new anchor kAnchorSlideM behind it on the same
// ray takes over (its time from the old one), so the rays stay short and a
// wind shift turns the front where it is. In uniform fuel and a steady wind
// a point fire is the model's ellipse, a line fire the line and its ends'
// half ellipses: sides diverging atan(b / c) (9.6 deg for a length to
// breadth of 6), corners rounded over the run, the head at the model's rate.
//
// A cell catches when the front first enters its square (its centre a little
// later), flames until its far corner has burnt for the residence time, and
// its views carry the exact front (FrontAgeAt), so nothing is drawn in cell
// squares. Intensity follows the front's normal speed (FrontSpeed: the head's
// at the head, b / t on a flank). Spread and flames are separate: the front
// cells keep spreading for as long as they have unburnt neighbours.
//
// Roads are barriers between cells, not fuel-less margins (the fire science
// consult's ninth answer): only a cell whose centre lies on a road's bare
// running surface is bare; every cell near one keeps its line (Fuel::road*).
// A step across the bare surface needs flame contact (the gap along the wind
// within L sin(tilt) + 0.4 L), else the cell throws embers across once
// (RoadEmberChance), lighting a spot fire beyond the far edge.
//
// Positions are world metres, x east and z north. The core steps at a fixed
// rate and touches at most a fixed number of cells per step, so its cost has
// a ceiling whatever the fire does.
//
// What it hands the drawing code moves smoothly, not in cells and steps: a
// cell catches at the moment within its step its progress reached 1 (not at
// the step's end), remembers the direction the front came from and how long
// it takes to cross the cell, and the views run on a display clock one step
// behind the simulation, interpolated between steps. PartStrength says how
// hard any point inside a burning cell flames now, so a front sweeps across
// the cell instead of a whole 8 m square lighting at once.

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <unordered_map>
#include <vector>

namespace Poseidon::firespread
{

// Anderson (1982) fuel model numbers 1 to 13; 0 = does not burn.
constexpr int kFuelModels = 14;

struct FuelModel
{
    const char* name;
    float w1h, w10h, w100h, wLive; // fuel loads, tons per acre
    float sav1h;                   // 1 h dead surface-to-volume ratio (1/ft); 10 h 109, 100 h 30, live 1500
    float depthFt;                 // fuel bed depth (ft)
    float mxDead;                  // dead fuel moisture of extinction (fraction)
    float waf;                     // wind adjustment factor, 10 m wind to midflame, unsheltered
};

const FuelModel& Model(int fm);

// Fuel moisture (fractions of dry weight).
struct Moisture
{
    float m1h = 0.06f; // fine dead fuel: dry summer afternoon 0.03 to 0.06, night 0.10 to 0.14, after rain 0.25+
    float m10h = 0.07f;
    float m100h = 0.09f;
    float mLive = 1.00f;  // live herbaceous and woody (cured grass 0.3, green 1.2 to 2.5)
    float foliar = 1.00f; // canopy foliar moisture (Van Wagner), typical 0.9 to 1.2
};

// What the caller says grows at a point.
struct Fuel
{
    std::uint8_t model = 0;   // Anderson model, 0 = none
    bool canopy = false;      // trees over it (crown fuel)
    bool sheltered = false;   // under a closed canopy: the midflame wind is sheltered (WAF 0.15)
    float canopyBaseM = 4.0f; // canopy base height (m)
    float canopyTopM = 14.0f; // canopy top (m)
    float canopyBulkKgM3 = 0.12f;
    // The tallest tree standing in the cell (its top above the ground, m; 0
    // none) and where (m off the cell's centre): a torching crown's flame
    // stands on it. The tallest shrub's height (m; 0 none).
    float treeHeightM = 0.0f;
    float treeOffX = 0.0f, treeOffZ = 0.0f;
    float shrubHeightM = 0.0f;
    // That tree (or the forest block) is a conifer: only conifers torch and
    // carry a crown fire (TorchingIntensityKwM was fitted to conifers).
    bool conifer = true;
    // It is an olive (partly torches: kOliveTorchKwM).
    bool olive = false;
    // The nearest road's bare running surface within reach (roadHalfM 0:
    // none): a point on its centre line (m off the query point), its
    // direction (unit) and the bare surface's half width (m).
    float roadOffX = 0.0f, roadOffZ = 0.0f, roadDirX = 0.0f, roadDirZ = 1.0f, roadHalfM = 0.0f;
    // (buildings builder) Bare because a building's footprint and apron
    // cover the query point: the burning cells beside it draw their ground on
    // up to that apron (BurningView::builtSides).
    bool built = false;
};

// The Rothermel no-wind, no-slope solution of a fuel model at a moisture,
// plus the coefficients for its wind and slope factors.
struct SurfaceFire
{
    bool burns = false;
    float r0Ftmin = 0.0f; // rate of spread with no wind and no slope (ft/min)
    float irBtu = 0.0f;   // reaction intensity (Btu/ft2/min)
    float sigma = 0.0f;   // characteristic surface-to-volume ratio (1/ft)
    float beta = 0.0f;    // packing ratio
    float betaOp = 0.0f;  // optimum packing ratio
    float windC = 0.0f, windB = 0.0f, windE = 0.0f;
    float slopeK = 0.0f; // phi_s = slopeK * tan^2(slope)
    float residenceSec = 0.0f;
};

SurfaceFire SolveSurface(int fm, const Moisture& m);

// The head fire of a surface fire in a midflame wind (ft/min) on a slope (as
// a tan) with an angle between the upslope and the wind; the result.
struct HeadFire
{
    float rateMps = 0.0f;           // head rate of spread (m/s)
    float dirX = 1.0f, dirZ = 0.0f; // head direction (unit, world)
    float lengthToBreadth = 1.0f;
    float eccentricity = 0.0f;
    float intensityKwM = 0.0f; // Byram fireline intensity at the head
    float flameM = 0.0f;       // Byram flame length at the head
};

// windX/windZ: midflame wind (m/s, world). slopeX/slopeZ: the ground's
// gradient (rise per run, world). waf: the wind adjustment factor that made
// the midflame wind from the 10 m wind (the length to breadth reads the 10 m wind).
// grass: a grass fuel model (its length to breadth; its rate at most kGrassMaxRateMps).
HeadFire SolveHead(const SurfaceFire& s, float windX, float windZ, float slopeX, float slopeZ, float waf = 1.0f,
                   bool grass = false);
// Rate in a direction (unit) from the elliptical shape.
float RateToward(const HeadFire& h, float dirX, float dirZ);
// Byram intensity (kW/m) and flame length (m) of a front moving at rate (m/s).
float IntensityAt(const SurfaceFire& s, float rateMps);
float FlameLengthM(float intensityKwM);
// The fire ellipse's length-to-breadth ratio for a 10 m open wind (m/s), U
// in km/h: forest and shrub fuels the Canadian FBP System's (Forestry Canada
// Fire Danger Group 1992, ST-X-3, eq. 79, fitted to Alexander's 1985 data),
// LB = 1 + 8.729 (1 - exp(-0.030 U))^2.155 (4.99 at 40 km/h); grass the FBP
// grass form (eq. 80, as corrected by Wotton, Alexander and Taylor 2009),
// LB = 1.1 U^0.464 (6.06 at 40 km/h), at most kGrassMaxLB (the data end).
// A young fire's grows from 1 with its build-up (FBP eq. 81, Wotton et al.
// 2009): LB(t) = (LB - 1) BuildUp(t) + 1 (FireGrid::Step).
constexpr float kGrassMaxLB = 8.0f;
float LengthToBreadth(float wind10Mps, bool grass = false);
// A grass head never runs faster than this (m/s): the fastest grass fires
// measured ran 4.7 to 6.3 m/s (the fire science consult, 2026-11-16).
constexpr float kGrassMaxRateMps = 6.5f;
// Van Wagner (1977): the surface intensity (kW/m) that starts torching.
float TorchingIntensityKwM(float canopyBaseM, float foliarMoisture);

// ---- The flames' shape, for the drawing (all from the fire model) ----
// A flame's tilt from the vertical (radians) in a wind of windMps at
// midflame for a flame flameM long: the wind's push against the flame's
// buoyancy, tan(tilt) = kFlameTiltK U / sqrt(g L) (the tilt follows the
// square root of the flame Froude number U^2 / (g L), Weise and Biging 1996),
// at most kFlameTiltMax.
constexpr float kFlameTiltK = 1.4f;
constexpr float kFlameTiltMax = 1.31f; // 75 degrees
float FlameTiltRad(float windMps, float flameM);
// A torching tree (Van Wagner's crown ignition from below): its crown flames
// for kTorchSec, starting kTorchDelaySec after the surface front reaches it,
// building over kTorchRiseSec and dying down over the last kTorchFallSec.
// Its flame stands on the crown (its foot kTorchFootShare of the tree's
// height up) and reaches, in still air, kTorchRatio times the tree's height:
// Albini's (1979) steady flame of one 12 m pine is 11.4 m standing on the
// crown's middle, a tip at about 1.5 tree heights (the fire science consult,
// 2026-11-16, from BehavePlus's spot.cpp). Trees torching together flame
// higher, as N^0.4 of their number (Albini's steady flame height), at most
// kTorchRatioMax (a stand: 2 to 2.5 tree heights): TorchRatio.
constexpr float kTorchRatio = 1.5f;
constexpr float kTorchRatioMax = 2.5f;
constexpr float kTorchFootShare = 0.25f;
float TorchRatio(int treesTorching);
// The olive, the one broadleaf that can partly torch (sclerophyll, oily
// leaves): only from kOliveTorchKwM, about kOliveTorchShare of the trees, a
// shorter and briefer flame (kOliveTorchRatio, over kOliveTorchSec instead
// of kTorchSec). The consult's judgement.
constexpr float kOliveTorchKwM = 1000.0f;
constexpr float kOliveTorchShare = 0.5f;
constexpr float kOliveTorchRatio = 1.2f;
constexpr float kOliveTorchSec = 15.0f;
constexpr float kTorchDelaySec = 2.0f;
constexpr float kTorchSec = 30.0f;
constexpr float kTorchRiseSec = 5.0f;
constexpr float kTorchFallSec = 12.0f;
// How hard (0..1) a tree torches t seconds after the surface front reached it.
float TorchEnvelope(float t);
// A forest block whose shape has no height torches as trees this tall (m).
constexpr float kTorchDefaultTreeM = 12.0f;

// ---- Build-up from a point (Forestry Canada Fire Danger Group 1992) ----
// The FBP System's acceleration for open fuels is 0.115 per minute (63% of
// the equilibrium rate after 8.7 min). McAlpine and Wakimoto (1991) measured
// faster build-up in small fires. This uses 0.33 per minute (63% after 3
// min), chosen to match the Cheney and Gould (1995) grass head-width law
// under a realistically swinging wind at 5 to 10 minutes (the fire science
// consult, 2026-11-16); FBP's 0.115 is a forest-litter value, about half as
// fast at 5 minutes in grass. Params::buildUpPerMin (0: none, every fire at
// its equilibrium rate at once).
constexpr float kBuildUpPerMin = 0.33f;
// A line ignition this long or longer starts at its equilibrium rate (Cheney
// and Gould 1995: heads over about 100 m wide run at the full rate).
constexpr float kMatureLineM = 100.0f;
// A mature fire's age (s): its build-up is complete.
constexpr float kMatureAgeSec = 1.0e5f;
// The share (0..1) of the equilibrium rate a fire ageSec old runs at.
float BuildUp(float ageSec, float perMin);
// The distance a fire runs over elapsedSec from the age age0Sec, in seconds
// at its equilibrium rate (the integral of BuildUp).
float BuildUpRun(float age0Sec, float elapsedSec, float perMin);

// ---- Spotting from torching trees ----
// Albini's (1979, INT-56) maximum spot distance (m) from a single torching
// tree treeM tall in a 10 m wind of wind10Mps over flat, open ground (the
// downwind cover half the tree's height, as BehavePlus takes an open stand):
// the steady flame's height and duration from the tree's diameter, the
// firebrand's lofted height from them, and its drift down a logarithmic wind
// profile (BehavePlus's Spot module).
float AlbiniTorchSpotMaxM(float treeM, float wind10Mps);
// One ember's landing distance (m): a lognormal draw (Sardoy et al. 2008,
// Combustion and Flame 154: firebrand landing distances are close to
// lognormal) with its median kSpotMedianShare of the cap and log spread
// kSpotLogSigma, from two uniform numbers in (0, 1]; never past the cap.
// (sigma 0.8: one throw in ten past 2.8 medians, one in a hundred past 6.4;
// the consult's judgement, between 0.55 and FlamMap's exponential draw with
// its mean at 0.2 of the cap)
constexpr float kSpotMedianShare = 0.1f;
constexpr float kSpotLogSigma = 0.8f;
float SpotDistanceM(float capM, float u1, float u2);
// Embers each torching tree throws over its torch (each lights a spot fire
// with the fine fuel's ignition chance, about 0.95 at 4%): about 0.5 spot
// fires a tree in open grass in a strong wind. A judgement inside what the
// literature allows (the realism audit: 0.3 to 1), not a published figure;
// tunable.
constexpr float kSpotEmbersPerTree = 0.55f;
// No spotting below this 10 m wind (m/s).
constexpr float kSpotMinWindMps = 3.0f;

// ---- The front: Huygens on the model's own ellipse (the audit's C3) ----
// Where a fire spreads from: a point, or a segment centred at (cx, cz) along
// (ux, uz), halfM to each side; the time it stood on the front and the
// fire's age then (its build-up).
struct Anchor
{
    float cx = 0.0f, cz = 0.0f;
    float ux = 1.0f, uz = 0.0f;
    float halfM = 0.0f;
    float t = 0.0f;
    float age = 0.0f;
    std::uint32_t id = 0;
    // A seed (an ignition, a spot fire): the cells near it draw its own field
    // (kSeedExactM). An area's radius (m): its edge burning at t.
    bool seed = false;
    float radiusM = 0.0f;
    // The straight front it is a piece of (along u, off its centre: an
    // ignition line's whole length carried on), so a piece made from it keeps
    // the line's span however far the front has run.
    float spanLo = 0.0f, spanHi = 0.0f;
};

// The rear-focus ellipse's travel time (equilibrium seconds) to the
// displacement (px, pz) from where the fire stood: (|p| - e p.h) / (R (1 - e)),
// rate R (m/s) at the head along (hx, hz), eccentricity e.
float Gauge(float px, float pz, float rate, float hx, float hz, float ecc);
// The same from a segment (a point when halfM is 0) centred at the origin
// along (ux, uz) (unit): its nearest point in travel time (the closed form of
// the minimum along it) and that time; (qx, qz) is the point.
float SegmentGauge(float wx, float wz, float ux, float uz, float halfM, float rate, float hx, float hz, float ecc,
                   float* qx = nullptr, float* qz = nullptr);
// From an anchor (its centre at the origin): its segment's, less an area's
// radius toward the point.
float AnchorGauge(const Anchor& a, float wx, float wz, float rate, float hx, float hz, float ecc, float* qx = nullptr,
                  float* qz = nullptr);
// A cell draws its seed's own field within this of the seed (FrontAgeAt);
// elsewhere the arrival field between its and its neighbours' centres, and
// its own source's where that is narrower than kNarrowSourceM to each side (a
// narrow head the field between centres 8 m apart would blunt).
constexpr float kSeedExactM = 16.0f, kNarrowSourceM = 4.0f;
// The normal speed (m/s at equilibrium) of a front facing (nx, nz) (unit):
// the ellipse's support function, R at the head, b / t square to it, the
// backing rate at the back.
float FrontSpeed(float rate, float hx, float hz, float ecc, float nx, float nz);
// The eccentricity of an ellipse of this length to breadth.
inline float EccentricityOf(float lb)
{
    return lb > 1.0f ? std::sqrt(lb * lb - 1.0f) / lb : 0.0f;
}
// The time (s) a fire aged age0Sec takes to run runSec (equilibrium seconds):
// BuildUpRun's inverse.
float BuildUpTime(float age0Sec, float runSec, float perMin);
// Anchors: an anchor counts for a cell within kAnchorMaxM of it; past that a
// new one kAnchorSlideM behind the cell takes over, kAnchorSlideHalfM to each
// side square to the front (wide enough that a wind turned by up to about 20
// degrees still finds the front's head on it). A giver whose anchor does not
// count offers from its own piece of front, kLocalHalfM to each side.
constexpr float kAnchorMaxM = 40.0f, kAnchorSlideM = 20.0f, kAnchorSlideHalfM = 12.0f, kLocalHalfM = 4.0f;
// A new anchor is a piece of the front where it is made (FrontPiece): the
// old anchor carried on where the front is straight, else square to the
// front and only as wide as its curvature lets the piece stay within
// kAnchorBulgeM of the true front (a young fire's narrow head: a point).
constexpr float kAnchorBulgeM = 0.3f;
// When a front reaches a cell's square (equilibrium s from the source): the
// sooner of its corners from the whole source and of its edges from the
// source's point (qx, qz) (off the source's centre) nearest its centre
// (cx, cz), half wide; (ex, ez) the point of entry off the centre.
float SquareEntryGauge(const Anchor& src, float cx, float cz, float qx, float qz, float half, float rate, float hx,
                       float hz, float ecc, float& ex, float& ez);
// A cell catches this much before the front reaches its square at most, so
// the edge noise's points near its edge never pop in (the noise's reach over
// the front's speed there).
constexpr float kEntryNoiseMaxSec = 4.0f;
// The front's radius of curvature (m) at the displacement (px, pz) from a
// point source in an ellipse (h, e): the scaled ellipse through it.
float FrontRadiusM(float px, float pz, float hx, float hz, float ecc);

// The piece of front through (px, pz) (world) at time t (the fire aged age)
// made from the anchor old whose nearest point to it is (qx, qz) off old's
// centre: old carried on over its span when that point is inside it, else
// square to the normal (nx, nz), its width from the front's curvature in the
// ellipse (h, e); at most maxHalf to each side.
Anchor FrontPiece(const Anchor& old, float qx, float qz, float px, float pz, float nx, float nz, float hx, float hz,
                  float ecc, float t, float age, float maxHalf);
// The drawn front's edge noise (FrontAgeAt): each point read up to this far
// off (m), over features about kEdgeNoiseCellM across.
constexpr float kEdgeNoiseM = 1.6f, kEdgeNoiseCellM = 7.0f;
// The edge noise's offset (m) at a world point: smooth, deterministic.
void EdgeWarp(float x, float z, float& wx, float& wz);
// The wind's wander (the fire science consult's Q2: a real wind swings 10 to
// 15 degrees, standard deviation, over minutes; a frozen wind makes needle
// heads and ruler-straight flanks): the turn (radians) at time t, three slow
// swings of periods kWanderPeriodsSec with phases from seed, sigmaDeg their
// combined standard deviation.
constexpr float kWindWanderDeg = 12.0f;
constexpr float kWanderPeriodsSec[3] = {97.0f, 223.0f, 541.0f};
float WindWanderRad(double t, std::uint32_t seed, float sigmaDeg);
// Per-cell fuel noise (patchy load and continuity): a cell's rates times this
// factor, a smooth field over kFuelNoiseCells cells and a cell's own share,
// log standard deviation about 0.12 at scale 1 (Params::fuelNoise; 0 none).
constexpr float kFuelNoiseCells = 3.0f;
float FuelNoiseFactor(std::int32_t i, std::int32_t j, std::uint32_t seed, float scale);
// Roads (the fire science consult's ninth answer): an ember crossing's
// expected far-side ignitions per metre of front burning out against a bare
// gap of gapEffM along the wind, flames reaching reachM across, a front of
// intensityKwM, a 10 m wind of wind10Mps and the fine fuel's ignition chance
// pIgnite: 0.01 (I / 2000) (U10 / 11)^1.5 exp(-(g_eff - reach) / 3) p, none
// under kRoadEmberMinWindMps. A cell draws once: 1 - exp(-kRoadFrontM n).
constexpr float kRoadEmberK = 0.01f, kRoadEmberDecayM = 3.0f, kRoadEmberMinWindMps = 5.0f, kRoadFrontM = 8.0f;
float RoadEmberRate(float intensityKwM, float wind10Mps, float gapEffM, float reachM, float pIgnite);
// A gap is crossed by flame contact when its width along the wind is within
// the flame's reach: L sin(tilt) + 0.4 L.
inline float FlameReachM(float flameM, float tiltRad)
{
    return flameM * std::sin(tiltRad) + 0.4f * flameM;
}
// The gap along the wind: the bare width over the cosine between the wind
// and the road's normal, at most 1 / 0.3 times it.
inline float GapAlongWindM(float gapM, float cosWindNormal)
{
    return gapM / std::max(std::fabs(cosWindNormal), 0.3f);
}

// (the dance fix) Cell::drawnArrival before any front was offered to it.
constexpr float kNoDrawnArrival = 1e30f;

enum class CellState : std::uint8_t
{
    Unknown, // not looked at yet
    Unburnt,
    Burning, // flaming (drawn, hurts)
    Burnt,   // flaming over; may still be on the spreading front
};

struct Cell
{
    // The planar front the views draw across the cell: igniteTime is when it
    // passes the cell's near edge (the edge square to it), so a point u of the
    // way across is reached u crossSec later (the cell catches a little
    // before, when the front first enters its square: entryTime).
    float igniteTime = 0.0f;
    float entryTime = 0.0f;
    float centreTime = 0.0f; // when the front reaches its centre
    // The fire's age when it reached this cell (s): its build-up (0 for an
    // ignition or a spot fire, kMatureAgeSec for a long line).
    float fireAge0 = 0.0f;
    float flameSec = 0.0f; // how long it flames, from igniteTime
    float intensityKwM = 0.0f;
    float flameM = 0.0f;
    float heightM = 0.0f;       // ground height at its centre
    float gx = 0.0f, gz = 0.0f; // ground gradient (rise per run)
    // Cached head fire of this cell's fuel for the current wind (recomputed when the wind moves).
    float hRate = 0.0f, hDirX = 1.0f, hDirZ = 0.0f, hEcc = 0.0f, hCrown = 0.0f;
    float hLB = 1.0f; // its equilibrium length to breadth (a young fire's grows to it)
    // (the dance fix) The head its views draw the front with (FillFront): its
    // head as it caught. The head above follows the wind while it burns (the
    // spread from it); a drawn front that followed it ran back.
    float drawRate = 0.0f, drawDirX = 1.0f, drawDirZ = 0.0f, drawLB = 1.0f;
    std::uint32_t windStamp = 0;
    float canopyBaseM = 4.0f;
    float fuelK = 1.0f; // FuelNoiseFactor: its rates' factor
    // The front's direction across the cell (unit, world: its normal at the
    // centre; 0 for a point ignition or a spot fire, which lights its cell
    // from a point) and the time it takes to cross it (s).
    float frontX = 0.0f, frontZ = 0.0f;
    float crossSec = 0.0f;
    float rateMps = 0.0f; // the front's normal speed through it (m/s), when it caught
    // Where its fire came from (FrontAgeAt draws from it; its neighbours
    // spread from it while it counts for them).
    Anchor anchor;
    // An ignition line's cell has a second, backing phase once its head
    // phase is over: the front backing from the line (its normal, crossing,
    // near-edge time and window from it, and its fire).
    bool hasBack = false;
    float backX = 0.0f, backZ = 0.0f, backCross = 0.0f, backIgnite = 0.0f, backFlameSec = 0.0f;
    float backRate = 0.0f, backIntensityKwM = 0.0f, backFlameM = 0.0f;
    // The nearest road (Fuel::road*): a point on its centre line (world), its
    // direction, the bare surface's half width (0: none).
    float roadX = 0.0f, roadZ = 0.0f, roadDX = 0.0f, roadDZ = 1.0f, roadHalfM = 0.0f;
    std::uint8_t roadSides = 0;  // its 4 sides (+x, -x, +z, -z) that border a bare road cell
    std::uint8_t bareSides = 0;  // and those that border other bare ground
    std::uint8_t builtSides = 0; // (buildings builder) and those that border a building's bare cell
    bool built = false;          // (buildings builder) Fuel::built
    bool gapDrawn = false;       // it has drawn its embers across a road
    // (the dance fix) Its centre's arrival as its neighbours' views draw it
    // (FillFront): offered, the sooner of each step's predicted arrival and
    // the last one run on by the step (the predictions move both ways as
    // offers come and go and the wind turns, and a front drawn through them
    // ran back); caught, the sooner of that and its centre's time, from
    // drawnCaughtT (sim s) on run on with the clock toward its centre's time.
    float drawnArrival = kNoDrawnArrival;
    float drawnCaughtT = 0.0f;
    float treeH = 0.0f, treeOX = 0.0f, treeOZ = 0.0f; // Fuel::treeHeightM, treeOffX, treeOffZ
    float shrubH = 0.0f;                              // Fuel::shrubHeightM
    std::int32_t offer = -1;                          // its slot in this step's offers, scratch
    std::uint8_t model = 0;
    bool canopy = false;
    bool sheltered = false;
    bool conifer = false; // Fuel::conifer
    bool olive = false;   // Fuel::olive
    bool torching = false;
    bool front = false;   // on the spreading front list
    bool offered = false; // on this step's offer list
    CellState state = CellState::Unknown;
};

struct Params
{
    float cellM = 8.0f;
    float stepSec = 0.25f;
    int maxStepsPerTick = 2;    // the most steps one Tick runs (a long frame drops time)
    int maxFront = 20000;       // front cells at most; cells that would join past it wait
    int maxFrontPerStep = 8000; // front cells processed per step (round robin past it, with the missed time)
    Moisture moisture;
    bool spotting = true;
    float buildUpPerMin = kBuildUpPerMin; // 0: every fire at its equilibrium rate at once
    float fuelNoise = 1.0f;               // FuelNoiseFactor's scale (0: every cell its fuel model's own)
    std::uint32_t seed = 0x5eed1234u;
    // The wind's wander (WindWanderRad: deg standard deviation, 0 none; its
    // seed): each step turns the wind Tick is given by the wander at that
    // step's own sim time, so the spread never depends on the frame rate.
    float windWanderDeg = 0.0f;
    std::uint32_t windWanderSeed = 0x77a2d1c5u;
};

// A flaming cell as the drawing and gameplay code see it (on the display
// clock: FireGrid::DisplayTime).
struct BurningView
{
    float x = 0.0f, z = 0.0f, y = 0.0f; // cell centre (world m), y its ground
    float strength = 0.0f;              // 0..1 envelope over its flaming time
    float flameM = 0.0f;                // flame length now (m)
    float flameMaxM = 0.0f;             // its flame length at full strength (m)
    float intensityKwM = 0.0f;
    float age = 0.0f;      // display seconds since it caught, when its flames start across it (< 0: not yet)
    float life = 0.0f;     // display seconds the cell shows flames in all (its window ends at age = life)
    float crossSec = 0.0f; // seconds the front takes to cross the cell (0: all of it at once)
    float frontX = 0.0f, frontZ = 0.0f; // the front's direction across it (unit; 0 with crossSec 0)
    float rateMps = 0.0f;               // the front's rate of spread through it (m/s)
    float residenceSec = 0.0f;          // the fuel's flame residence time (s): each point flames this long
    // Its tree (treeH 0: none standing in it) and shrub, as Fuel gave them.
    float treeH = 0.0f, treeOX = 0.0f, treeOZ = 0.0f;
    float shrubH = 0.0f;
    float torchM = 0.0f;    // the torching crown's flame length now (m; 0 when not torching)
    float torchMaxM = 0.0f; // and at its peak
    float torchAge = 0.0f;  // seconds since the surface front reached the tree
    std::uint8_t model = 0;
    bool torching = false;
    bool sheltered = false;
    // The exact front (FrontAgeAt; exact false: the planar front above): the
    // cell's anchor (its centre off the cell's centre, its direction and half
    // length, 0 a point), the cell's ellipse (head rate at equilibrium and
    // direction as it caught, eccentricity now), the build-up's share behind
    // its centre (gGrown: when the front passed it) and ahead of it
    // (gGrownAhead: now; 0 the same as behind), and the anchor's run
    // (equilibrium seconds) to where the front stands now. (The dance fix:
    // a point's age never goes back.)
    bool exact = false;
    float ancX = 0.0f, ancZ = 0.0f, ancUX = 1.0f, ancUZ = 0.0f, ancHalf = 0.0f, ancRadius = 0.0f;
    // Elsewhere the arrival field between the cell's and its 8 neighbours'
    // centres (grid: nbAge the seconds since the front reached each, row by
    // row from the south-west, the cell's own in the middle; a neighbour
    // caught or offered its drawn arrival (Cell::drawnArrival), bare ground
    // the cell's own), so neighbouring cells draw one continuous front; else
    // the planar front through the centre (planar: ageC since it passed the
    // centre, square to frontX/Z at rateMps).
    bool grid = false;
    float nbAge[9] = {};
    bool planar = false;
    float ageC = 0.0f;
    float gRate = 0.0f, gDirX = 1.0f, gDirZ = 0.0f, gEcc = 0.0f, gGrown = 1.0f, gRun = 0.0f;
    float gGrownAhead = 0.0f;
    // The nearest road's bare surface (roadHalf 0: none): a point on its
    // centre line off the cell's centre, its direction, the half width, the
    // side of it the cell's centre is on (+1 or -1), and the cell's sides
    // that border a bare road cell (bits +x, -x, +z, -z: its ground runs on
    // up to the bare edge there).
    float roadX = 0.0f, roadZ = 0.0f, roadDX = 0.0f, roadDZ = 1.0f, roadHalf = 0.0f, roadSide = 1.0f;
    std::uint8_t roadSides = 0;
    std::uint8_t bareSides = 0; // its sides that border other bare ground (rock, water, a building)
    // (buildings builder) its sides that border a building's bare cell (bits
    // as roadSides): its ground runs on into that cell's square, the quarter
    // of it on this side (BuiltRunOn), up to the building's apron.
    std::uint8_t builtSides = 0;
    bool backing = false; // an ignition line's cell in its backing phase
};
// (buildings builder) A point (ox, oz) metres off a cell's centre, outside
// its own square on a side that borders a building's bare cell (builtSides),
// is the cell's to draw only in the quarter of that square on its side (the
// square split by its diagonals, so each part of it is drawn by one
// neighbour); inside its own square every point is. False also beyond a side
// that borders no built cell, and in the corners.
inline bool BuiltRunOn(std::uint8_t builtSides, float ox, float oz, float cellM)
{
    const float half = 0.5f * cellM;
    const bool outX = ox > half || ox<-half, outZ = oz> half || oz < -half;
    if (!outX && !outZ)
        return true;
    if (outX && outZ)
        return false;
    if (outX)
    {
        const std::uint8_t bit = ox > half ? 1u : 2u;
        return (builtSides & bit) != 0 && std::fabs(oz) <= cellM - std::fabs(ox);
    }
    const std::uint8_t bit = oz > half ? 4u : 8u;
    return (builtSides & bit) != 0 && std::fabs(ox) <= cellM - std::fabs(oz);
}

// A point (ox, oz) metres off the cell's centre lies on its road's bare
// surface or beyond it (it never burns from this cell).
bool BareAt(const BurningView& v, float ox, float oz);
// Seconds since the front reached the point (ox, oz) metres off a cell's
// centre (negative: not yet; -1e9 on or beyond its road's bare surface), from
// its anchor through the cell's own ellipse with the edge noise (EdgeWarp),
// so a burn's edge is the true front's, ragged by metres, in no cell square;
// a view without the anchor (exact false) reads its planar front. speedMps:
// the front's normal speed there now (m/s).
float FrontAgeAt(const BurningView& v, float ox, float oz, float cellM, float* speedMps = nullptr);
// The same without the road's bare surface (the field a cell's tongues read
// off a lattice: CellSurfaceTongues).
float FrontAgeField(const BurningView& v, float ox, float oz, float cellM);
// How far (m) the point (ox, oz) off the cell's centre lies behind the
// drawn front (UpdateBurnMap's char ramp): ageSec x speedMps as FrontAgeAt
// gives them, but on a seed's own field (exact) no more than the distance to
// the nearest edge of its burnt area: the seed's ellipse grown to now
// (gRun) round the seed's segment or disc, through the same edge noise, its
// support taken over kFrontDepthDirs directions. Near an ignition line a
// point the head passed seconds ago has the head's speed but lies a metre or
// two from the backing front: ageSec x speedMps put it tens of metres in and
// charred it black up to the backing flames (follow-up 13, shot 06). The
// burnt area's support along each direction is the view's own
// (FrontDepthSupportOf, once per view: the burn map reads tens of texels a
// view).
constexpr int kFrontDepthDirs = 16;
struct FrontDepthSupport
{
    bool on = false; // an exact view (a seed's own field)
    float s[kFrontDepthDirs] = {};
};
FrontDepthSupport FrontDepthSupportOf(const BurningView& v);
float FrontDepthAt(const BurningView& v, const FrontDepthSupport& sup, float ox, float oz, float ageSec,
                   float speedMps);
float FrontDepthAt(const BurningView& v, float ox, float oz, float ageSec, float speedMps);
// A view aged to newAge seconds (its planar and its exact front together: a
// burnt-out cell's record drawn on).
void AgeView(BurningView& v, float newAge);

// How hard (0..1) the point (ox, oz) metres off a burning cell's centre
// flames now: it lights when the front reaches it (FrontAgeAt), grows over up
// to 3 s and dies down over the last 40% of its own share of the cell's
// window. The whole cell's parts together span the cell's window, so the
// flames move with the front instead of lighting a cell at once.
float PartStrength(const BurningView& v, float ox, float oz, float cellM);

// The surface flame at the point (ox, oz) off a burning cell's centre, from
// the fire model: the front reaches the point at its own moment, the fuel
// there then flames for the fuel's residence time (Anderson 1969), so the
// flaming strip is R t_r deep, the head's tallest flames at its leading edge.
// heat (0..1) is how hard it burns (its share of the fuel still burning,
// the flame forming over kAttachSec); length (0..1) is the flame's length as a
// share of the cell's Byram length: the fuel's burning rate falls as it burns
// out (taken as linear over the residence time), and L follows I^0.46.
struct PartFlame
{
    float heat = 0.0f;
    float length = 0.0f;
};
constexpr float kAttachSec = 0.6f;
PartFlame SurfacePart(const BurningView& v, float ox, float oz, float cellM);

struct Stats
{
    int burning = 0; // flaming cells
    int front = 0;   // spreading front cells
    int burnt = 0;
    int known = 0;
    int tiles = 0;
    int ignitions = 0;
    int torching = 0;
    int spots = 0;
    int embers = 0;        // embers thrown (each lands as a spot fire or not)
    float spotMaxM = 0.0f; // the farthest spot fire from its tree (m)
    // Roads: cells that met a bare gap and drew for embers, those whose embers
    // crossed (a spot fire waits beyond), and the spot fires lit there.
    int roadDraws = 0, roadEmbers = 0, roadSpots = 0;
    // The front: anchors slid forward, offers from a giver's own piece of
    // front (its anchor did not count), and rays walked.
    int slides = 0, localOffers = 0;
    long long rays = 0;
    int steps = 0;
    int deferred = 0;
    double simTime = 0.0;
    // The burnt and burning cells' extent (their centres, world m; valid
    // once ignitions > 0).
    float extMinX = 1e9f, extMinZ = 1e9f, extMaxX = -1e9f, extMaxZ = -1e9f;
};

class FireGrid
{
  public:
    using FuelQuery = std::function<Fuel(float x, float z)>;
    using HeightQuery = std::function<float(float x, float z)>;

    FireGrid(const Params& params, FuelQuery fuel, HeightQuery height);
    ~FireGrid();
    FireGrid(const FireGrid&) = delete;
    FireGrid& operator=(const FireGrid&) = delete;

    // A point and an area start as young fires (their build-up ahead), an
    // area kMatureLineM or more across as a mature one.
    bool Ignite(float x, float z);
    int IgniteArea(float x, float z, float radiusM);
    // The cells along a line; kMatureLineM or longer, a mature fire.
    int IgniteLine(float x0, float z0, float x1, float z1);
    // Advances by dt seconds with the 10 m wind (m/s, world x east, z north).
    void Tick(float dt, float wind10X, float wind10Z);

    void SetMoisture(const Moisture& m);
    void SetWander(float deg) { _p.windWanderDeg = deg > 0.0f ? deg : 0.0f; }
    // The wander's turn (rad) at sim time t (the model's own clock: a step's
    // simTime, or DisplayTime between steps for the drawing).
    float WanderRad(double t) const { return WindWanderRad(t, _p.windWanderSeed, _p.windWanderDeg); }
    void SetBuildUpPerMin(float perMin) { _p.buildUpPerMin = perMin; }
    const Params& GetParams() const { return _p; }
    const Stats& GetStats() const { return _stats; }
    const SurfaceFire& Surface(int fm) const { return _surface[fm]; }

    void CollectBurning(std::vector<BurningView>& out) const;
    // The burnt-out cells on the burn's edge (a neighbour not caught), as
    // views (strength 0), for the char's edge.
    void CollectBurntEdges(std::vector<BurningView>& out) const;
    // The other burnt-out cells (their centres): char all over.
    void CollectBurntInterior(std::vector<std::pair<float, float>>& out) const;
    // Display seconds since the front reached the world point (x, z)
    // (FrontAgeAt from the cell holding it, or, if that is not caught, the
    // front of a burning neighbour coming into it); -1e9 when none has.
    float DrawnAgeAt(float x, float z) const;
    // The view of the cell under a point as CollectBurning gives it (a cell
    // not burning: its fuel and tree only, strength 0); false when unknown.
    bool ViewAt(float x, float z, BurningView& out, CellState* state = nullptr) const;
    CellState StateAt(float x, float z) const;
    // (the dance fix) Display seconds since the front reached the centre of
    // the cell under a point as the model has it (its centre's time, which
    // its burning out goes by; its neighbours' views may draw it sooner:
    // Cell::drawnArrival); -1e9 when it has not caught.
    float CentreAgeAt(float x, float z) const;
    // Flaming strength (0..1) and flame length (m) of the cell under a point.
    float StrengthAt(float x, float z, float* flameM = nullptr) const;
    void CollectBurnt(std::vector<std::pair<float, float>>& out) const;
    bool Bounds(float& minX, float& minZ, float& maxX, float& maxZ) const;
    bool Active() const { return !_front.empty() || _stats.burning > 0; }
    void Clear();

    float StrengthOf(const Cell& c) const;
    // Display seconds since the surface front reached a cell's tree.
    float TorchAgeOf(const Cell& c) const;
    // The clock the views run on: one step behind the simulation, moving
    // smoothly between steps (a cell that caught during the last step lights
    // at its own moment, never early, never in step-sized jumps).
    double DisplayTime() const { return _stats.simTime + static_cast<double>(_acc) - _p.stepSec; }

  private:
    static constexpr int kTile = 32;
    struct Tile
    {
        Cell cells[kTile * kTile];
    };
    struct Key
    {
        std::int32_t i, j;
    };

    // This step's best offer to an unburnt cell: when the front enters its
    // square and reaches its centre (sim s), its normal there and normal
    // speed at equilibrium, the build-up's share and the fire's age now, and
    // where it spreads from (a slide: a new anchor slideRun equilibrium
    // seconds along the ray from src, at (slideX, slideZ), made if it wins).
    struct Offer
    {
        Key key;
        float eta = 1e30f, etaCentre = 0.0f, etaLast = 0.0f;
        float rank = 1e30f; // eta less a hair for a wider source (a near tie keeps a front straight)
        float nx = 0.0f, nz = 0.0f, vn = 0.0f;
        float grown = 1.0f, ageNow = 0.0f;
        Anchor src;
        float qx = 0.0f, qz = 0.0f;             // the source's nearest point (off its centre)
        float hx = 1.0f, hz = 0.0f, ecc = 0.0f; // the cell's ellipse then
        bool slide = false;
        float slideRun = 0.0f, slideX = 0.0f, slideZ = 0.0f;
    };
    struct PendingSpot
    {
        float x = 0.0f, z = 0.0f, t = 0.0f;
    };

    Cell* CellAt(std::int32_t i, std::int32_t j, bool create);
    void FillView(const Cell& c, std::int32_t i, std::int32_t j, BurningView& v) const;
    // The view's front only (the planar and exact fields, the road).
    void FillFront(const Cell& c, std::int32_t i, std::int32_t j, BurningView& v) const;
    const Cell* CellAtConst(std::int32_t i, std::int32_t j) const;
    void Classify(Cell& c, std::int32_t i, std::int32_t j);
    // A cell joins the fire (false: not burnable, or the front is full).
    bool Admit(Cell& c);
    // Its torching, the lists and the stats, once its fields are set.
    void Light(std::int32_t i, std::int32_t j, Cell& c);
    // A point ignition (Ignite, a spot fire, an area's cells): the cell
    // catches now from the point (px, pz), its fire aged ageSec.
    bool CatchPoint(std::int32_t i, std::int32_t j, Cell& c, float px, float pz, float ageSec, float radiusM = 0.0f);
    // A seed cell's front, window and fire from its anchor (a point or an
    // area, or the line), with the current wind.
    void SeedFields(std::int32_t i, std::int32_t j, Cell& c);
    // Its crown torches, if its tree and its fire say so.
    void DecideTorch(std::int32_t i, std::int32_t j, Cell& c);
    // A cell catches from its best offer.
    bool CatchOffer(std::int32_t i, std::int32_t j, Cell& c, const Offer& o);
    // The giver's offers to an unburnt neighbour (Step): from its anchor
    // (when the ray counts) or from its own piece of front.
    void OfferTo(const Cell& g, std::int32_t gi, std::int32_t gj, Cell& n, std::int32_t ni, std::int32_t nj,
                 float wind10X, float wind10Z, bool contact);
    // The offer from a source to a cell (its centre at (x, z)), the time from
    // the ray (rayOk) or the cell's own ellipse alone; into the cell's slot.
    void Consider(Cell& n, std::int32_t ni, std::int32_t nj, const Anchor& src, float gCentre, float qx, float qz,
                  bool slide, float slideRun, float slideX, float slideZ, float crownRate);
    // The travel time (equilibrium s) along the straight ray from (x1, z1)
    // to (x0, z0) through each cell's own ellipse at the build-up share
    // grown (runNear: the part within nearM of (x0, z0)); false when it
    // meets a cell not known and burnable, or crosses a road.
    bool RayRun(float x0, float z0, float x1, float z1, float grown, float nearM, float& run, float& runNear);
    // The step between two cells' centres crosses a road's bare surface (the
    // cell whose road it is).
    const Cell* RoadBetween(const Cell& a, const Cell& b, float ax, float az, float bx, float bz) const;
    // A burning cell meets a road's bare gap: its one ember draw across it.
    void RoadEmbers(Cell& g, std::int32_t gi, std::int32_t gj, const Cell& road, float wind10X, float wind10Z);
    // Its flames' reach across a gap (m) and the gap along the wind (m).
    void GapReach(const Cell& g, const Cell& road, float wind10X, float wind10Z, float& gapEff, float& reach) const;
    // The giver's own piece of front (a point or line ignition: its anchor).
    // An inner cell (every neighbour caught or bare) whose square the drawn
    // front has crossed and whose residence is over (Step retires it).
    bool InteriorOut(const Cell& c, std::int32_t i, std::int32_t j, float now) const;
    Anchor LocalAnchor(const Cell& g, std::int32_t gi, std::int32_t gj);
    void Torches(float wind10X, float wind10Z);
    void Pending();
    // Seconds since the surface front reached a cell's tree, on the simulation's clock.
    float TorchAgeAt(const Cell& c, double t) const;
    // How hard (0..1) a torching cell's crown flames at that torch age (an
    // olive's briefer torch on the same shape).
    float TorchShape(const Cell& c, float torchAge) const;
    // Its crown flame's length (m) now: TorchRatio of the trees torching
    // round it (an olive's kOliveTorchRatio), less its foot's height, times
    // the tree's height and the envelope.
    float TorchFlameM(const Cell& c, std::int32_t i, std::int32_t j, float* maxM = nullptr) const;
    void Step(float wind10X, float wind10Z);
    void UpdateHead(Cell& c, float wind10X, float wind10Z);
    float Rand();
    float CentreX(std::int32_t i) const { return (static_cast<float>(i) + 0.5f) * _p.cellM; }
    float CentreZ(std::int32_t j) const { return (static_cast<float>(j) + 0.5f) * _p.cellM; }
    std::int32_t IndexOf(float v) const;
    static std::uint64_t TileKey(std::int32_t ti, std::int32_t tj)
    {
        return (static_cast<std::uint64_t>(static_cast<std::uint32_t>(ti)) << 32) | static_cast<std::uint32_t>(tj);
    }

    Params _p;
    FuelQuery _fuel;
    HeightQuery _height;
    SurfaceFire _surface[kFuelModels];
    std::unordered_map<std::uint64_t, Tile*> _tiles;
    std::vector<Key> _front;           // cells that can still spread
    std::vector<Key> _flaming;         // cells flaming now
    std::vector<Offer> _offers;        // unburnt cells offered this step, their best offers
    std::vector<PendingSpot> _pending; // road embers waiting to light beyond the far edge
    std::vector<Key> _seedFix;         // cells lit since the last step (they take its wind)
    std::uint32_t _anchorIds = 0;
    float _windX = 0.0f, _windZ = 0.0f; // this step's 10 m wind
    std::vector<Key> _caught;
    std::vector<Key> _torches; // torching trees, until their torch is over (they throw the embers)
    std::vector<std::uint8_t> _keep;
    std::size_t _cursor = 0;
    float _acc = 0.0f;
    std::uint32_t _windStamp = 1;
    float _stampWindX = 1e9f, _stampWindZ = 1e9f;
    std::uint32_t _rng;
    Stats _stats;
    bool _haveBounds = false;
    std::int32_t _minI = 0, _minJ = 0, _maxI = 0, _maxJ = 0;
};

} // namespace Poseidon::firespread
