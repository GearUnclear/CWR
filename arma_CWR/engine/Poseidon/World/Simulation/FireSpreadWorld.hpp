#pragma once

// Spreading wildland fire in the world (FIRE-SPREAD explore lane). The
// physics is the pure core in FireSpread.hpp; this is its engine side: fuel
// from the terrain and map objects, the weather's wind and rain, the flames,
// smoke and lights drawn from capped budgets, burn damage, soldiers running
// from the flames, smoke that blocks AI sight, and the script commands.
//
// Everything here is behind POSEIDON_XP_FIRESPREAD (unset or "0" = off). Off,
// no hook does anything and no script command is registered, so the game is
// the same as without this file.

#include <Poseidon/Foundation/Math/Math3D.hpp>
#include <Poseidon/World/Simulation/FireSpread.hpp>
#include <Poseidon/World/Simulation/FireObjects.hpp>
#include <Poseidon/World/Simulation/FireSparks.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <utility>
#include <vector>

namespace Poseidon
{
class IAnimator;
}

namespace Poseidon::firespread
{

// POSEIDON_XP_FIRESPREAD set and not "0" (read once).
bool Enabled();

// Once per simulation step (World::Simulate, with the other global systems).
void SimulateWorld(float deltaT);

// AI sight through the fire's smoke: the transmittance (0..1) along a line
// of sight. 1 when the fire is off or the line misses the smoke.
float SmokeSight(Vector3Par from, Vector3Par to);

// Ignition sources: an explosion on the ground (shells, grenades, fuel) and
// a burning wreck's embers (called each step while it burns).
void NotifyExplosion(Vector3Par pos, float indirectHit, float indirectHitRange);
void NotifyBurning(Vector3Par pos, float deltaT, float strength);
// (buildings builder) A fireball (a vehicle's: the wrecks builder calls it)
// touching buildings: each within radiusM of pos takes a pass (a shed or a
// barn lights, a house through its windows and eaves at a chance); a big
// one's (radiusM over kFireballBigM) heat reaches out to 2 radii.
constexpr float kFireballBigM = 15.0f;
void NotifyBuildingHeat(Vector3Par pos, float radiusM);

// The plume's puffs (Smokes.hpp PlumePuff; FirePlume.hpp): the fire's 10 m
// wind (world m/s, x east and z north), and the light the flames below throw
// on a puff at `p` (lux, at the flames' colour, linear rgb; false and black
// where none reaches).
void PlumeWind(float& x, float& z);
bool PlumeFireLight(Vector3Par p, float rgb[3]);
// (smoketrim) Whether the renderer keeps the world pass's set-up when it goes
// back to the 3D pass from the world's own screen-space draws in the same
// frame (EngineGL33::BeginPass): only while the fire lane is on, and unless
// the test aid fireSpreadSet "keeppass" 0 asks for the full set-up each time.
bool KeepWorldPassSetup();
// The fire's heat release now (MW, eased over 8 s; 0 with no fire): the line
// plume's law (FirePlume.hpp MakePlumeLaw(PlumeHeatMW(), wind10, 0, true)).
float PlumeHeatMW();

// The plume's shadow on the land (the renderer's sun: EngineGL33_Lighting.cpp
// UploadLitPlume): up to kPlumeShadowBlobs soft blobs along the plume, from
// its puffs grouped by how far they have come: each its middle (world x, z)
// and height (world y), its spread along the wind and across it (a Gaussian's
// sigma, m), the wind's direction (unit, x and z) and the vertical optical
// depth at its middle. The sun under the plume is exp(-tau / sin(sun)) of
// the sum (consult eighth 1: about 60% less near the source, 40% at 1 km and
// 25% at 2 km). False and count 0: no plume.
constexpr int kPlumeShadowBlobs = 8;
struct PlumeShadowBlob
{
    float x = 0.0f, z = 0.0f, y = 0.0f;
    float alongM = 1.0f, acrossM = 1.0f;
    float dirX = 0.0f, dirZ = 1.0f;
    float tau = 0.0f;
};
struct PlumeShadowView
{
    PlumeShadowBlob blobs[kPlumeShadowBlobs];
    int count = 0;
};
bool GetPlumeShadow(PlumeShadowView& out);

// The scorched ground for the renderer: a size x size single-channel map
// (row-major, row 0 at the south edge z0, column 0 at the west edge x0, each
// texel texelM metres square) over a world window round the fire. 0 untouched,
// up to 255 burnt black; a flaming cell is part way. version changes whenever
// the texels or the window change, so the renderer re-uploads only then.
// groundY is the land's height (world y, m) over the same window on a coarser
// groundSize x groundSize grid (same orientation, each sample at its square's
// centre), so a shader can tell how far above the ground a wall or a trunk
// fragment is; groundVersion changes only when the window moves.
struct BurnMapView
{
    const unsigned char* texels = nullptr;
    int size = 0;
    float x0 = 0.0f, z0 = 0.0f, texelM = 4.0f;
    unsigned version = 0;
    const float* groundY = nullptr;
    int groundSize = 0;
    unsigned groundVersion = 0;
    // The plants' own fire (PlantTable: plantSlots x kPlantSlotFloats floats,
    // kPlantSlotsPerRow slots a row of the atlas; null without one), changed
    // whenever plantVersion does, and the fire's clock (s) its times run on.
    const float* plants = nullptr;
    int plantSlots = 0;
    unsigned plantVersion = 0;
    double clock = 0.0;
};
// False (and out untouched) when the fire is off or nothing has burnt yet.
bool GetBurnMap(BurnMapView& out);

// The flames for the renderer: one flame tongue per entry, its foot on the
// ground at (x, y, z), its tip height metres up and leaned (leanX, leanZ)
// metres downwind. The renderer draws it as a ribbon along its axis (which
// stands up from the fuel and bends over), turned about that axis to face
// the eye; width is the ribbon's width (m), its flipbook frame's (the tongue
// in its middle, nothing at its sides: FlameFrameWidth). A burning front is
// many of them side by side. heat (0..1) is how hard it burns now (it rises
// as its point lights and falls as it dies down, so a tongue never pops);
// seed (0..1) is fixed for the tongue's whole life, so it keeps its own
// pattern from frame to frame. style is the kind of flame it shows (one
// flipbook each, tools/firespread/gen_flame_flipbook.py STYLE_ORDER), fixed
// for its life too. rise is how fast its visible features climb (m/s) and
// lengthRef the tongue's full length (m), both fixed for its life, so the
// renderer plays its flipbook at the speed real fire moves without ever
// jumping. time is the fire's clock in seconds (it stops while the game is
// paused). The list is rebuilt every simulation step (every frame).
enum class FlameStyle : unsigned char
{
    Front = 0,   // a grass head's tongue: a bright foot, a slim pointed body
    Tongues = 1, // a slimmer, livelier tongue that sheds pockets more often
    Creep = 2,   // a low flank or backing flame: stubby, a wide foot
    Bushy = 3,   // a shrub or crown tongue: fat, often forked, sooty tips
    Bed = 4,     // the burning bed (BedHeat): a ribbon lying on the ground, drawn procedurally, no flipbook
};
constexpr int kFlameStyles = 4; // the flipbook's styles (Front to Bushy)
constexpr int kFlameKinds = 5;  // and the bed
// Where each style's flame tip sits in its flipbook frame, as a share of the
// frame's height from the fuel bed: the highest row with flame half the time
// (gen_flame_flipbook.py prints it: "tip"). The renderer draws a tongue's
// frame its length over this tall, so its tip is at its length, as the fire
// model says.
constexpr float kFlameTipShare[kFlameStyles] = {0.551f, 0.566f, 0.523f, 0.543f};
inline float FlameTipShare(int style)
{
    return kFlameTipShare[style < 0 ? 0 : (style >= kFlameStyles ? kFlameStyles - 1 : style)];
}
// A flipbook frame's width over its height (gen_flame_flipbook.py FRAME_W /
// FRAME_H): a tongue L long is drawn on a ribbon this times its frame's
// height wide, so its shape is the generator's (a base 0.18 to 0.30 L wide,
// the wildfire consult's 0.15 to 0.3).
constexpr float kFlameFrameAspect = 0.5f;
inline float FlameFrameWidth(int style, float lengthM)
{
    return kFlameFrameAspect * lengthM / FlameTipShare(style);
}
// The flames' tempo (the wildfire consult, 2026-11-17, fourth round). A
// flame's visible features (its bulges, tips and pockets, the large eddies:
// about half the centreline gas speed of McCaffrey 1979) climb at
// kFeatureRiseK sqrt(L) m/s averaged over a flame L m long, so it licks (one
// stretch and break-off of its tip, a bulge travelling about 0.65 L) at about
// kLickK / sqrt(L) Hz: a 1 m grass flame about 2.3 times a second, a 20 m
// crown flame once in 2 s. Its tempo is the eye's cue of its size (Froude
// scaling: a big flame built from parts with a small flame's timing reads as
// a small fire, or a film run fast). A wind quickens it a little, by
// (midflame / kLickWindRefMps)^0.2 within 1 and kLickWindMax.
constexpr float kFeatureRiseK = 1.5f, kLickK = 2.3f;
constexpr float kLickWindRefMps = 2.0f, kLickWindMax = 1.3f;
inline float FeatureRiseMps(float lengthM)
{
    return kFeatureRiseK * std::sqrt(std::max(lengthM, 0.05f));
}
inline float LickHz(float lengthM)
{
    return kLickK / std::sqrt(std::max(lengthM, 0.05f));
}
inline float LickWindFactor(float midflameMps)
{
    return std::clamp(std::pow(std::max(midflameMps, 0.0f) / kLickWindRefMps, 0.2f), 1.0f, kLickWindMax);
}
// A torching tree's column puffs as a whole at the lower of a pool fire's
// 1.5 / sqrt(D) Hz (Cetegen and Ahmed 1993; D its crown's width, within
// kPuffBaseMinM..MaxM) and its flame's own lick rate: about 0.6 Hz for a
// 12 m pine.
constexpr float kPuffK = 1.5f, kPuffBaseMinM = 1.0f, kPuffBaseMaxM = 8.0f;
inline float ColumnPuffHz(float crownM, float flameM)
{
    return std::min(kPuffK / std::sqrt(std::clamp(crownM, kPuffBaseMinM, kPuffBaseMaxM)), LickHz(flameM));
}
// A plant (a bush, a reed clump or an olive standing in a cell) that flares
// as a body when the front reaches it, as a small torching tree (the wildfire
// consult, 2026-11-17, fifth round, and its species answer for Abel's
// shapes): its burn class.
enum class PlantClass : std::uint8_t
{
    Thicket, // A: a flammable evergreen thicket (maquis: krovi): flame 2 h
    Leafy,   // B: a summer-green leafy bush (ker listnac): scorches; a weak flare at the head only
    Reed,    // R: a tall reed or cane clump (ker rakosi): a fast, tall flash
    Olive,   // a cultivated olive: a partial flare where the surface fire reaches 1000 kW/m
};
// Bushes change their cell's fuel only when they cover much of it (the
// consult's cover rule; a plant covers pi r^2 of its 64 m2 cell): over
// kCoverClump of the cell a thicket clump makes it grass with open shrub
// (model 2), over kCoverStand a stand makes it tall grass for reeds (3),
// green brush for leafy bushes (5), dormant brush for thickets (6), or
// chaparral for thickets over kThicketTallM (4). Below that the ground's own
// fuel carries the fire and each bush flares as a plant.
constexpr float kCoverClump = 1.0f / 3.0f, kCoverStand = 2.0f / 3.0f, kThicketTallM = 1.8f;
// The fuel model the cover rule gives a cell (model: the ground's own) for
// its bushes' cover by class (m2, pi r^2 each) over its area (m2) and its
// tallest thicket (m).
inline int CoverModel(int model, float coverThicket, float coverLeafy, float coverReed, float cellArea, float thicketH)
{
    const float cover = (coverThicket + coverLeafy + coverReed) / std::max(cellArea, 1.0f);
    if (cover > kCoverStand)
    {
        if (coverReed >= coverThicket && coverReed >= coverLeafy)
            return 3;
        if (coverLeafy > coverThicket)
            return 5;
        return thicketH > kThicketTallM ? 4 : 6;
    }
    if (cover > kCoverClump && coverThicket >= coverReed && coverThicket >= coverLeafy)
        return 2;
    return model;
}
// A plant's flare (Plant, Flare: the wildfire consult, 2026-11-17, fifth
// round and its species answer). It begins as the front reaches the plant
// if the surface fire there is strong enough for its class (a thicket or a
// reed clump under surface flames of kFlareMinFlameM, a leafy bush only
// where they pass kLeafyMinShare of its height: at the head; an olive from
// kOliveMinKwM), climbs to its peak (a sudden "whoomp" two thirds of the
// way: kWhoompShare of it before), holds, dies down and, for a thicket,
// leaves short stem flames (kFlareStemShare of its fill tongues) a while.
// Its flame, FlareShapeOf by class (h the plant's height): a thicket 2 h,
// peak at 3 s + 2 s a metre, full 10 s + 10 s a metre, down over as long,
// stem flames 30 s + 30 s a metre (at most kFlareStemMaxSec); a leafy bush
// 1.15 h on its windward lower half, 3 s up, 9 s full, 4 s down; a reed
// clump 1.5 h, 3.5 s up, 6 s a metre full, 10 s down, thinner and more
// tongues, clearer and less sooty; an olive 1.35 h in its crown, 12 s up,
// 25 s full, 20 s down. One leaning flame body of main tongues (from the
// upper body, 2 h x 0.7 to 1.1 for a thicket) and half as many again fill
// tongues (the whole body, 0.5 to 1 h), their feet over the plant's width
// (kFlareLeeShare of them in its lee half), leaning in kFlareWindShare of
// the 10 m wind (45 to 50 degrees at 11 m/s for a 2 to 3 m bush), puffing
// as a whole (ColumnPuffHz of its width and flame: about 1 Hz) by up to
// kFlarePuffStretch, each tongue at its own length's tempo. A wide thicket
// or an olive takes the 3 m layout (7 main tongues). Its smoke and light go
// into the fire's (FlareSmoke, FlareHeat), and its peak throws sparks.
constexpr float kFlareStemMaxSec = 150.0f, kFlareLeeShare = 0.6f;

struct FlareShape
{
    float flameM = 0.0f;                      // its full flame's length (m)
    float peakSec = 0.0f, fullSec = 0.0f;     // up to its peak, then full
    float downSec = 0.0f, stemSec = 0.0f;     // dying down, then stem flames
    int mainN = 0, fillN = 0;                 // its main and fill tongues
    float mainFoot0 = 0.5f, mainFoot1 = 0.8f; // the main tongues' feet (shares of its height)
    float fillFoot0 = 0.2f, fillFoot1 = 0.7f; // the fill tongues' feet
    float spread = 0.5f;                      // the feet within this share of its width of its centre
    float leeShare = kFlareLeeShare;          // the share of the feet in its lee half
    float widthK = 1.0f, bushyShare = 0.7f;   // its tongues' width, and the share drawn bushy
    float absorb = 5.0f, woody = 1.0f;        // visible absorption (1/m), and its smoke's woodiness
};
inline FlareShape FlareShapeOf(PlantClass cls, float plantH)
{
    const float h = std::max(plantH, 0.3f);
    auto layout = [](float hl) { return std::clamp(static_cast<int>(std::lround(1.0f + 2.0f * hl)), 2, 8); };
    FlareShape s;
    switch (cls)
    {
        case PlantClass::Leafy:
            s.flameM = 1.15f * h;
            s.peakSec = 3.0f;
            s.fullSec = 9.0f;
            s.downSec = 4.0f;
            s.mainN = s.fillN = 3;
            s.mainFoot0 = 0.25f;
            s.mainFoot1 = 0.5f;
            s.fillFoot0 = 0.1f;
            s.fillFoot1 = 0.45f;
            s.spread = 0.4f;
            s.leeShare = 0.0f; // its windward side
            s.bushyShare = 0.5f;
            s.absorb = 4.0f;
            s.woody = 0.6f;
            break;
        case PlantClass::Reed:
            s.flameM = 1.5f * h;
            s.peakSec = 3.5f;
            s.fullSec = 6.0f * h;
            s.downSec = 10.0f;
            s.mainN = static_cast<int>(std::lround(1.5f * static_cast<float>(layout(h))));
            s.mainFoot0 = 0.3f;
            s.mainFoot1 = 0.7f;
            s.fillFoot0 = 0.1f;
            s.fillFoot1 = 0.6f;
            s.leeShare = 0.5f;
            s.widthK = 0.7f;
            s.bushyShare = 0.0f;
            s.absorb = 3.5f;
            s.woody = 0.0f;
            break;
        case PlantClass::Olive:
            s.flameM = 1.35f * h;
            s.peakSec = 12.0f;
            s.fullSec = 25.0f;
            s.downSec = 20.0f;
            s.mainN = layout(3.0f);
            s.mainFoot0 = 0.55f;
            s.mainFoot1 = 0.85f;
            s.fillFoot0 = 0.35f;
            s.fillFoot1 = 0.8f;
            s.bushyShare = 0.6f;
            break;
        default: // a thicket
            s.flameM = 2.0f * h;
            s.peakSec = 3.0f + 2.0f * h;
            s.fullSec = s.downSec = 10.0f + 10.0f * h;
            s.stemSec = std::min(kFlareStemMaxSec, 30.0f + 30.0f * h);
            s.mainN = layout(std::max(h, 3.0f));
            break;
    }
    if (s.fillN == 0)
        s.fillN = static_cast<int>(std::lround(1.5f * static_cast<float>(s.mainN)));
    return s;
}
// Lingering (the wildfire consult, 2026-11-17, sixth round): what keeps
// burning behind the front. The fire model's residence time (Anderson's
// 384 / sigma), which sets its intensity, spread, flames, torching and
// spotting, is left alone; the longer burning is drawn on its own clock.
// Each point of the ground flames from when the front reaches it for the
// drawn residence time tr (DrawResidenceSec: the model's own for grass; in
// shrub, hardwood litter and conifer litter the heavier fuel's slower burning,
// kShrubBandSec, kLitterBandSec, kTimberBandSec, chosen so the share of the
// ground still flaming meets the consult's timber table: about 80% at 10 s,
// 40 to 60% at 30 s, 10 to 15% at 60 s), its flame easing from full to
// kBandEndShare of it, then a tail (LingerFlame): falling as
// exp(-(t - tr) / tr) (a third of the band's end after one more tr, a
// seventh after two), its cover breaking up into separate flames (half the
// points alight at 1.5 tr, a fifth at 2.5 tr, none by kTailEnd tr).
constexpr float kShrubBandSec = 30.0f, kLitterBandSec = 18.0f, kTimberBandSec = 20.0f;
constexpr float kBandEndShare = 0.65f, kTailEnd = 3.5f, kLingerAttachSec = 0.6f;
// The char's edge ramp in the burn map (UpdateBurnMap): s metres behind the
// drawn front (negative: ahead of it), 0 at and ahead of the front, rising to
// 1 widthM behind it. Real fire chars only behind its flames: a ramp centred
// on the front left a thin dark rim on the grass in front of a backing fire.
// (The map is read bilinearly from 2 m texels, so a texel just behind the
// front still reaches a little way ahead: at most 1 / (2 widthM) of the ramp
// at the front, and the ground shader's cover starts above that.)
inline float BurnEdgeRamp(float s, float widthM)
{
    return std::clamp(s / std::max(widthM, 1e-3f), 0.0f, 1.0f);
}

// ---- The burn map's burnt interior (the lattice fix) ----
// The texels whose centres lie in [lo, hi) along one axis of a burn map whose
// first texel starts at x0, texelM a texel: i0 to i1 - 1 (none when no centre
// lies in it). Neighbouring cells' spans meet with no gap and no overlap
// whatever the texel, so every texel goes to the cell its centre lies in.
inline void BurnTexelSpan(float lo, float hi, float x0, float texelM, int& i0, int& i1)
{
    i0 = static_cast<int>(std::ceil((lo - x0) / texelM - 0.5f));
    i1 = static_cast<int>(std::ceil((hi - x0) / texelM - 0.5f));
}

// The burnt-out cells with only burnt ground round them (FireGrid::
// CollectBurntInterior: their centres, cellM squares) charred black (255) in a
// size x size burn map of texelM texels from (x0, z0): every texel whose
// centre lies in one's square (BurnTexelSpan). UpdateBurnMap painted
// lround(cellM / texelM) texels from each square's first: one under the 8 m
// cells of a burn past about 2 km (its map's texels 6 m), so every fourth row
// and column of the black stayed unburnt, a lattice of grass lines 24 m apart.
inline void PaintBurntInterior(const std::vector<std::pair<float, float>>& centres, float cellM, float x0, float z0,
                               float texelM, int size, unsigned char* texels)
{
    const float hc = 0.5f * cellM;
    for (const auto& p : centres)
    {
        int i0 = 0, i1 = 0, j0 = 0, j1 = 0;
        BurnTexelSpan(p.first - hc, p.first + hc, x0, texelM, i0, i1);
        BurnTexelSpan(p.second - hc, p.second + hc, z0, texelM, j0, j1);
        for (int j = std::max(j0, 0); j < std::min(j1, size); ++j)
            for (int i = std::max(i0, 0); i < std::min(i1, size); ++i)
                texels[static_cast<std::size_t>(j) * static_cast<std::size_t>(size) + static_cast<std::size_t>(i)] =
                    255;
    }
}

// ---- The burn map never unburns (the dance fix) ----
// A burn map's window: its first texel's corner (world m), its texel (m) and
// its texels a side (0: no map).
struct BurnWindow
{
    float x0 = 0.0f, z0 = 0.0f, texelM = 0.0f;
    int size = 0;
};
// A burn map just painted (now, its texels) kept at least as burnt as the one
// before it (before, prev): each texel at least what the map before read at
// its centre, bilinearly as the renderer reads it (outside that map nothing).
// The same window is texel for texel; a moved one (the fire neared its edge)
// on the same texels is the old map shifted; a coarser one (the fire
// outgrew it) reads between the old texels. Char only ever grows: the edge
// ramp's metres behind the front, its width (two texels) and a coarser
// texel's sampling may each read a little less than the last map at a texel,
// which drew the ground there a shade less burnt for a frame.
inline void LatchBurnMap(const unsigned char* prev, const BurnWindow& before, const BurnWindow& now,
                         unsigned char* texels)
{
    if (!prev || before.size <= 0 || before.texelM <= 0.0f || now.size <= 0)
        return;
    const std::size_t n = static_cast<std::size_t>(now.size);
    if (before.size == now.size && before.texelM == now.texelM && before.x0 == now.x0 && before.z0 == now.z0)
    {
        for (std::size_t k = 0; k < n * n; ++k)
            texels[k] = std::max(texels[k], prev[k]);
        return;
    }
    const int m = before.size;
    auto at = [&](int i, int j) -> float
    {
        return i < 0 || j < 0 || i >= m || j >= m
                   ? 0.0f
                   : static_cast<float>(
                         prev[static_cast<std::size_t>(j) * static_cast<std::size_t>(m) + static_cast<std::size_t>(i)]);
    };
    for (int j = 0; j < now.size; ++j)
    {
        // the texel's centre in the old map's texels, from its first texel's centre
        const float v = (now.z0 + (static_cast<float>(j) + 0.5f) * now.texelM - before.z0) / before.texelM - 0.5f;
        const int v0 = static_cast<int>(std::floor(v));
        const float fv = v - static_cast<float>(v0);
        if (v0 < -1 || v0 >= m)
            continue;
        for (int i = 0; i < now.size; ++i)
        {
            const float u = (now.x0 + (static_cast<float>(i) + 0.5f) * now.texelM - before.x0) / before.texelM - 0.5f;
            const int u0 = static_cast<int>(std::floor(u));
            if (u0 < -1 || u0 >= m)
                continue;
            const float fu = u - static_cast<float>(u0);
            const float b = (at(u0, v0) * (1.0f - fu) + at(u0 + 1, v0) * fu) * (1.0f - fv) +
                            (at(u0, v0 + 1) * (1.0f - fu) + at(u0 + 1, v0 + 1) * fu) * fv;
            unsigned char& t = texels[static_cast<std::size_t>(j) * n + static_cast<std::size_t>(i)];
            t = std::max(t, static_cast<unsigned char>(std::clamp(std::lround(b), 0L, 255L)));
        }
    }
}

inline float DrawResidenceSec(int model, float residenceSec)
{
    const float tr = std::max(residenceSec, 0.5f);
    if (model >= 4 && model <= 7)
        return std::max(tr, kShrubBandSec);
    if (model == 8 || model == 9)
        return std::max(tr, kLitterBandSec);
    if (model >= 10)
        return std::max(tr, kTimberBandSec);
    return tr;
}
// A point's flame a seconds after the front reached it (tr its drawn
// residence): its length and heat as shares of the cell's, and the share of
// the points still alight (a point burns while its own draw is under it).
struct LingerPoint
{
    float length = 0.0f, heat = 0.0f, cover = 0.0f;
};
inline LingerPoint LingerFlame(float a, float tr)
{
    LingerPoint p;
    tr = std::max(tr, 0.5f);
    if (a <= 0.0f || a >= kTailEnd * tr)
        return p;
    const float x = a / tr;
    if (x < 1.0f)
    {
        const float in = std::min(1.0f, a / std::min(kLingerAttachSec, 0.3f * tr));
        const float formed = in * in * (3.0f - 2.0f * in);
        p.length = formed * (1.0f - (1.0f - kBandEndShare) * x);
        p.heat = formed * (1.0f - 0.5f * x);
        p.cover = 1.0f;
        return p;
    }
    const float decay = std::exp(1.0f - x);
    p.length = kBandEndShare * decay;
    p.heat = 0.5f * decay;
    p.cover = x < 1.5f   ? 1.0f - (x - 1.0f)
              : x < 2.5f ? 0.5f - 0.3f * (x - 1.5f)
                         : 0.2f * (kTailEnd - x) / (kTailEnd - 2.5f);
    return p;
}
// Residual flamelets behind the band (tussock bases, thatch clumps, matted
// patches): kFlameletsPerCell in a model 1 grass cell (4 to 12 by the cell;
// FlameletScale by fuel), each lit at the band's trailing edge, alight for a
// lognormal time (FlameletLifeSec: median kFlameletMedianSec, log sigma
// kFlameletSigma, so 66% alight at 30 s, 28% at 60 s, 6% at 2 min; at most
// kFlameletMaxSec), 0.15 to 0.5 m tall at birth and shrinking to 0.1 m, then
// a glow point for one to three times its life.
constexpr float kFlameletsPerCell = 8.0f, kFlameletMedianSec = 40.0f, kFlameletSigma = 0.7f;
constexpr float kFlameletMaxSec = 360.0f;
inline float FlameletLifeSec(float u1, float u2)
{
    const float z = std::sqrt(-2.0f * std::log(std::clamp(u1, 1e-6f, 1.0f))) * std::cos(6.2831853f * u2);
    return std::min(kFlameletMaxSec, kFlameletMedianSec * std::exp(kFlameletSigma * z));
}
// By the cell's fuel, relative to model 1 grass: grass with shrub litter
// twice, tall grass one and a half times; shrub and timber cells have their
// own debris instead (FireWorld::Draw); bare ground none.
inline float FlameletScale(int model)
{
    switch (model)
    {
        case 1:
            return 1.0f;
        case 2:
            return 2.0f;
        case 3:
            return 1.5f;
        default:
            return 0.0f;
    }
}
// Flame feet are pinned (the wildfire consult, 2026-11-17, seventh round): a
// flame's base is where its fuel gives off gas, and the fuel does not move.
// A tongue's foot never moves for its life; what swings sideways is its
// upper body, the tip up to the share of the tongue's own length (now) below
// either side (the renderer bends the axis by height^1.4, so the foot stays):
// in the band kSwayBand typically, up to kSwayBandMax, over SwayPeriodSec
// (1.26 sqrt(L) s, 1 to 2.5 s); in the tail kSwayTail to kSwayTailMax over
// 0.5 to 1 s; a residual flamelet (a tuft) flaps about its foot, kSwayTuft
// to kSwayTuftMax over 0.2 to 0.4 s. Peaks change place along the band by
// dying and being born, not by travelling: each band tongue sinks by up to
// kPeakSwing now and then (every 2 to 5 s, cross-fading over 0.3 to 0.75 s).
constexpr float kSwayBand = 0.1f, kSwayBandMax = 0.15f, kSwayTail = 0.15f, kSwayTailMax = 0.2f;
constexpr float kSwayTuft = 0.2f, kSwayTuftMax = 0.3f, kPeakSwing = 0.35f;
inline float SwayPeriodSec(float lengthM)
{
    return std::clamp(1.26f * std::sqrt(std::max(lengthM, 0.0f)), 1.0f, 2.5f);
}
// A residual flamelet's lean (a tuft): the tilt law (FlameTiltRad's) in the
// wind at its own mid height over burnt ground (a log profile, roughness
// kTuftZ0), at most kTuftTiltMax; a lull (0..1) stands it up toward
// kTuftTiltLull, a gust (0..1) presses it toward kTuftTiltGust. It is wide
// for its length (kTuftWiden times a tongue's frame: a body 0.4 to 0.7 of
// its length), flaps its length by kTuftFlutter at about kTuftFlapHz, its
// flipbook's features barely climbing (kTuftRise: it flickers in place, no
// flow along it), and a gust gutters it to (1 - kTuftGutter) of its length,
// one in four at any time. A tail tongue shorter than kTuftFromM takes the
// tuft's width (fully at half of it) and the tuft's lean cap.
constexpr float kTuftZ0 = 0.02f, kTuftTiltMax = 1.134f, kTuftTiltLull = 0.698f, kTuftTiltGust = 1.222f;
constexpr float kTuftWiden = 2.5f, kTuftFlutter = 0.3f, kTuftFlapHz = 4.5f, kTuftRise = 0.1f, kTuftGutter = 0.6f;
constexpr float kTuftFromM = 0.6f;
inline float TuftTiltRad(float wind10, float lengthM, float lull, float gust)
{
    const float L = std::max(lengthM, 0.05f);
    const float z = std::max(0.5f * L, 0.05f);
    const float u = std::max(wind10, 0.0f) * std::log(z / kTuftZ0) / std::log(10.0f / kTuftZ0);
    const float base = std::min(std::atan(kFlameTiltK * u / std::sqrt(9.81f * L)), kTuftTiltMax);
    const float calm = base - std::max(base - kTuftTiltLull, 0.0f) * std::clamp(lull, 0.0f, 1.0f);
    return calm + (std::max(kTuftTiltGust, calm) - calm) * std::clamp(gust, 0.0f, 1.0f);
}
inline float TuftShare(float lengthM)
{
    const float k = std::clamp((kTuftFromM - lengthM) / (0.5f * kTuftFromM), 0.0f, 1.0f);
    return k * k * (3.0f - 2.0f * k);
}
// One tongue of a burning cell's surface flames (CellSurfaceTongues): its key,
// its foot off the cell's centre (m: fixed to the cell for its life), the
// time since the front reached it (s), its full length (m) and its flame now
// (LingerFlame's shares), and its level of detail's keep (0..1).
struct SurfaceTongue
{
    std::uint32_t key = 0;
    float ox = 0.0f, oz = 0.0f, age = 0.0f, lmax = 0.0f, keep = 0.0f;
    LingerPoint flame;
};
// The grid's row and tongue spacing (m), the far tongues' widening, and the
// light each kept tongue carries for those its level of detail drops (with
// its keep: SurfaceTongue::keep x gain over the cell's tongues is about their
// number at every level, so a far cell keeps its light; FlameSheet::gain).
struct SurfaceGrid
{
    float a = 1.0f, b = 1.0f, widen = 1.0f, gain = 1.0f;
};
// ---- Flames at range (fix 08; the realism audit's C13) ----
// The level of detail is worked in the reference view's pixels (the
// director's 86 degree view, kLodRefLeft its half width's tangent): a zoomed
// camera (a smaller Camera::Left()) sees a tongue bigger, so the distance its
// level of detail is worked at is FlameLodDistance (a 6 times zoom at 1.3 km
// thins as at about 220 m, not as at 1.3 km).
constexpr float kLodRefLeft = 0.933f;
inline float FlameLodDistance(float distM, float cameraLeft)
{
    return distM * (cameraLeft > 1e-3f ? cameraLeft : 1e-3f) / kLodRefLeft;
}
// The share of a row's tongues the level of detail keeps on average
// (FireSpreadWorld.cpp LodKeep: the odd indices fade out over the first
// level, those not a multiple of 4 over the second, and so on): 2^-lod at
// whole levels, linear between.
inline float FlameLodMeanKeep(float lod)
{
    if (lod <= 0.0f)
        return 1.0f;
    const int n = static_cast<int>(lod);
    const float lo = 1.0f / static_cast<float>(1 << (n < 30 ? n : 30));
    return lo - 0.5f * lo * (lod - static_cast<float>(n));
}
// How far flames are drawn (m). By day to kFlameDrawDayM (past it only the
// smoke shows: the consult's eighth round, 6), the debris left burning
// behind the front to kLingerDayM (by day a flamelet is gone beyond a few
// hundred metres). At night every flame is far brighter than the exposure
// and visible to the horizon, so both reach the camera's far clip (at most
// kFlameDrawNightMaxM), eased in over the dusk by the scene's night effect
// (0..1). The last kFlameRangeFadeShare of either range fades out
// (FlameRangeFade), so nothing pops at its edge.
constexpr float kFlameDrawDayM = 3000.0f, kLingerDayM = 200.0f, kFlameDrawNightMaxM = 8000.0f;
constexpr float kFlameNightFrom = 0.1f, kFlameNightTo = 0.6f;
constexpr float kFlameRangeFadeShare = 0.15f;
inline float FlameNightShare(float night)
{
    const float k = std::clamp((night - kFlameNightFrom) / (kFlameNightTo - kFlameNightFrom), 0.0f, 1.0f);
    return k * k * (3.0f - 2.0f * k);
}
inline float FlameDrawRangeM(float night, float clipFarM)
{
    const float n = FlameNightShare(night);
    return std::min(kFlameDrawDayM + (kFlameDrawNightMaxM - kFlameDrawDayM) * n, std::max(clipFarM, 500.0f));
}
inline float FlameLingerRangeM(float night, float clipFarM)
{
    const float n = FlameNightShare(night);
    return std::min(kLingerDayM + (kFlameDrawNightMaxM - kLingerDayM) * n, std::max(clipFarM, 500.0f));
}
inline float FlameRangeFade(float distM, float rangeM)
{
    const float from = (1.0f - kFlameRangeFadeShare) * rangeM;
    const float k = std::clamp((distM - from) / std::max(rangeM - from, 1.0f), 0.0f, 1.0f);
    return 1.0f - k * k * (3.0f - 2.0f * k);
}
// The tongues one cell's surface flames stand as now: v the cell (live, or
// its record with its age now), trDraw its drawn residence, cellKey its
// tongues' key base, gridX/Z the direction its rows face when it lit all at
// once (the wind when it was first seen: fixed, so a turning wind moves no
// foot), dist its distance from the eye and lodBias the budget's. A grid of
// points fixed to the cell and turned to its front (kTongueSpacing), each
// alight from when the front reaches it through its band and tail, a share
// of the tail's points dropped as it breaks up (the rest never move).
// Nothing here reads the clock: the feet are where the fuel is.
SurfaceGrid CellSurfaceTongues(const BurningView& v, float trDraw, float cellM, std::uint32_t cellKey, float gridX,
                               float gridZ, float dist, float lodBias, std::vector<SurfaceTongue>& out);
// The burning bed (the realism audit's fix 7; the wildfire consult,
// 2026-11-16, second round 4 and third round): under a band of tongues the
// fuel bed burns as one continuous zone, 3 to 6 times as bright as the flame
// above it, the tongues' feet rooted in it. Drawn as bed streaks
// (FlameStyle::Bed: straight ribbons lying on the ground, the renderer's own
// procedural flame): kBedShare of the band's tongues (all) each lay one from
// its foot downwind, as long as its flame leans (L sin(tilt), within
// kBedLenMinM..MaxM), kBedWidthShare of its full length wide (at least
// kBedCoverK of the tongue spacing and kBedWidthMinM, so neighbours overlap),
// its axis lifted kBedLift of its width, its emissivity from the flaming
// zone's depth seen side on (rate x residence, at least its own width, at
// most kBedPathMaxM) in the fuel's visible absorption: the continuous zone is
// dense. From above they read as a band lying
// on the ground streaked along the wind, from the side as a low strip, from
// downwind as a row of low blobs under the tongues. Its light: kBedK at the
// band's leading edge (the renderer's), kBedBackCoolK cooler at its back, its
// heat across the band BedHeat (x the share of the drawn residence since the
// front reached it): full over its front kBedFrontShare, kBedFrontDrop less
// by kBedBackFrom, then fading out over the back third.
constexpr float kBedShare = 1.0f, kBedWidthShare = 0.45f, kBedCoverK = 1.6f, kBedLift = 0.3f;
constexpr float kBedLenMinM = 0.2f, kBedLenMaxM = 3.5f, kBedWidthMinM = 0.1f, kBedPathMaxM = 1.5f;
constexpr float kBedFrontShare = 0.4f, kBedBackFrom = 0.667f, kBedFrontDrop = 0.25f, kBedBackCoolK = 120.0f;
inline float BedHeat(float x)
{
    if (x <= 0.0f || x >= 1.0f)
        return 0.0f;
    const float front =
        1.0f - kBedFrontDrop * std::clamp((x - kBedFrontShare) / (kBedBackFrom - kBedFrontShare), 0.0f, 1.0f);
    const float k = std::clamp((x - kBedBackFrom) / (1.0f - kBedBackFrom), 0.0f, 1.0f);
    return front * (1.0f - k * k * (3.0f - 2.0f * k));
}
// The bed's share of its cell's light (CellFlameLight): the luminance it
// adds over its height beyond what the flame body's average (kFlameLightNits,
// calibrated on whole flames with their bright base) already counts: a
// head's light rises about a quarter, a slow edge's low flames far more.
constexpr float kBedLightGain = 0.35f;
// The fire's edges (the realism audit's fix 7; the consult's 6d: a flank's
// flaming strip 0.5 to 1 m deep, a backing fire's 0.1 to 0.3 m, an unbroken
// line wherever the grass is). A cell's band of tongues is the strip the
// front's flaming zone makes, rate x residence deep; on a slow front (a
// backing fire at 0.013 m/s, a flank in a light wind) that is less than the
// row spacing its tongues stand in, so its rows lit one at a time left the
// edge dark between them (the pinned build's dead back edge and broken
// flanks). The band lasts at least long enough to be kBandMinRows rows deep
// (BandResidenceSec: the drawn residence lengthened to kBandMinRows rows over
// the front's speed through the cell, at least kBandMinRateMps, at most
// kBandMaxSec), so every edge stands as a continuous low line of its own
// flames at its own rate (fix 4's front: the flank's and the back's normal
// speed and intensity), its burning bed under it. A head's strip, metres
// deep, is unchanged.
constexpr float kBandMinRows = 1.6f, kBandMinRateMps = 0.005f, kBandMaxSec = 120.0f;
float BandResidenceSec(float trDraw, float rateMps, float flameMaxM);
struct FlameSheet
{
    float x = 0.0f, y = 0.0f, z = 0.0f;
    float width = 0.0f, height = 0.0f;
    float leanX = 0.0f, leanZ = 0.0f;
    float heat = 0.0f;
    float seed = 0.0f;
    float rise = 0.0f;
    float lengthRef = 1.0f;
    // The luminous zone's emissivity at its densest (0..1) seen side on, from
    // the tongue's optical path across it (the renderer raises it toward end
    // on, where the sight line runs through more flame); by Kirchhoff it
    // hides that share of what is behind it.
    float emissivity = 0.6f;
    // Its axis' bend (a power, 0: the renderer's own, a flame standing up from
    // its fuel and bending over; 1: straight) and how far up its foot fades in
    // (m; 0: the renderer's).
    float bend = 0.0f;
    float baseFade = 0.0f;
    // Its light's temperature offset (K): a torching crown's upper tongues
    // are its cooler tips.
    float tempOffsetK = 0.0f;
    // Its level of detail's gain: the light (and the optical depth) of the
    // tongues it stands for over its own, its fade in or out included (1: it
    // stands for itself alone). Far off the level of detail drops tongues and
    // the kept ones carry their light, so a far front is as bright as a near
    // one (the realism audit's C13).
    float gain = 1.0f;
    FlameStyle style = FlameStyle::Front;
    // (fix 6) A big tongue drawn as a volume, not a ribbon: its visible
    // absorption coefficient (1/m; 0: a ribbon, as every small tongue). The
    // renderer integrates the flipbook along each sight line through a body
    // round the tongue's straight axis (foot to tip), so it never shows a flat
    // card from any side and never squashes end on. volumeR scales the body's
    // reach round its axis (1: the frame's), volumeSoot (0..1) how sooty its
    // tips are (dark red into black-brown smoke: shrub and crown flames; 0:
    // clean, as grass and reeds burn).
    float volumeK = 0.0f;
    float volumeR = 1.0f;
    float volumeSoot = 0.0f;
    // Its full length (m), its growth and its puff's stretch aside (0: its
    // length now). The volumes are drawn far to near by where their middles
    // stand at this length, so the order holds still while they puff: two
    // bodies that overlap never swap places in step with the puff (each swap
    // showed as a patch of the column changing at once). Not sent to the
    // renderer.
    float volumeLen = 0.0f;
    // Its puff (fix 6 follow-up). A volume keeps a steady length and width:
    // the column's puff does not stretch it (a solid body scaled 22% either
    // way at the puff's rate reads as the fire inflating and deflating). The
    // puff is a bulge that climbs the body with the gas: the renderer widens
    // the body by volumeBulge of its width where the bulge is and necks it as
    // much half a wave on (0: no bulge), the wave volumeWaveM long (m, along
    // the tongue: the bulge's speed over the puff's rate), at the phase
    // volumePuff (turns) at its foot now. The foot itself never moves or
    // widens.
    float volumePuff = 0.0f;
    float volumeWaveM = 0.0f;
    float volumeBulge = 0.0f;
};
// (fix 6) A tongue this long or longer (its full length, m), standing on a
// plant (a flare) or in a torching crown, is drawn as a volume.
constexpr float kVolumeFromM = 1.5f;
// (fix 6 follow-up) The bulge of a volume's puff as the renderer takes it in
// one number (FlameSheet::volumePuff, volumeWaveM, volumeBulge): the whole
// part holds the wave's length in 0.1 m (1 to 2047) plus 2048 times the
// bulge in steps of kVolumeBulgeStep (0 to 15), the fraction the phase at the
// foot (turns). 0: no bulge.
constexpr float kVolumeBulgeStep = 0.02f;
inline float VolumePuffCode(float puffTurns, float waveM, float bulge)
{
    const float amp = std::floor(std::clamp(bulge / kVolumeBulgeStep, 0.0f, 15.0f) + 0.5f);
    if (amp < 1.0f || waveM <= 0.0f)
        return 0.0f;
    const float wave = std::floor(std::clamp(waveM * 10.0f, 1.0f, 2047.0f) + 0.5f);
    const float turns = puffTurns - std::floor(puffTurns);
    return amp * 2048.0f + wave + std::min(turns, 0.998f);
}

// (fix 6) A torching tree's flame column (the wildfire consult, second round,
// section 3; the realism audit's fix 6), at its full size. Below the tree's
// top the flame wraps the crown, crown-wide (its widest kTorchWideAt of the
// way up the crown); above it the column necks to kTorchNeckShare of the
// crown's width kTorchNeckRise of the crown's width above the top, then
// tapers to a point at its tip. The tip stands TorchRatio tree heights above
// the ground (Albini's flame height is vertical: 1.5 h for one tree), so its
// length along the lean is longer. The column leans from mid-crown
// (kTorchAxisAt of the crown up) at the tilt law (FlameTiltRad) in the wind at
// mid-crown (a log profile from the 10 m wind, roughness kTorchWindZ0) and the
// tree's full flame at its tip ratio (a group's taller flame leans less; the
// caller eases the ratio, so it never swings as the torch grows or its
// neighbours join it), at most kTorchTiltMax: a big flame leans no further
// than about 50 degrees. That tilt is the chord from mid-crown to the tip; a
// bent-over plume, the axis rises steeply over the crown and bends over
// higher up, its downwind offset growing as the height's share to the
// kTorchBendPow (TorchColumnOffset).
constexpr float kTorchTiltMax = 0.8727f; // 50 degrees
constexpr float kTorchWindZ0 = 0.1f;
constexpr float kTorchNeckShare = 0.6f, kTorchNeckRise = 0.5f, kTorchWideAt = 0.3f, kTorchAxisAt = 0.5f;
constexpr float kTorchBendPow = 1.5f;
struct TorchColumn
{
    float crownBaseM = 0.0f; // the crown's foot above the ground (m)
    float crownR = 0.0f;     // the crown's radius at its foot (m)
    float wideM = 0.0f;      // the flame's widest height above the ground (m), crownR wide ...
    float neckM = 0.0f;      // ... necking to neckR at this height ...
    float neckR = 0.0f;
    float tipM = 0.0f;    // ... and tapering to its tip, this high above the ground (m)
    float axisM = 0.0f;   // its axis leans from this height above the ground (m)
    float tilt = 0.0f;    // its lean from vertical (rad)
    float windMps = 0.0f; // the wind at mid-crown (m/s)
};
// treeH the tree's height (m), crownW its crown's width at its foot (m),
// tipRatio the flame's tip in tree heights (TorchRatio), wind10 the 10 m wind
// (m/s). Pure (no clock).
TorchColumn TorchColumnOf(float treeH, float crownW, float tipRatio, float wind10);
// The column's radius (m) at height zM above the ground: the crown's width
// below its widest, necking above the top, tapering to 0 at the tip.
float TorchColumnRadius(const TorchColumn& c, float zM);
// How far downwind of the tree (m) the column's axis stands at height zM
// above the ground (0 up to mid-crown, (tipM - axisM) tan(tilt) at the tip).
float TorchColumnOffset(const TorchColumn& c, float zM);
struct FlameView
{
    const FlameSheet* sheets = nullptr;
    int count = 0;
    float time = 0.0f;
};
// False (and out untouched) when the fire is off or nothing is flaming.
bool GetFlames(FlameView& out);
// The renderer's count of its last frame's flames by how it drew them (fix
// 08: ribbons from the near flipbook, from the far book, splats), for the
// stats line (far=).
void NoteFlameDraw(int nearRibbons, int farRibbons, int splats);

// ---- The fire's light (the wildfire consult, 2026-11-17, eighth round, 4;
// the showcase audit's fix 5) ----
// Every flame lights its surroundings by its luminous intensity (cd): the
// luminance of its body times the area it shows as drawn. A grass head's
// solid body is about kFlameLightNits (soot at 1450 to 1500 K, emissivity near
// 1; the consult), so 100 m of a 2.6 m head (1.5 m tall at its 55 degree
// lean) is about 750000 cd: 1100 lux on a wall 10 m off, 70 at 100 m, 0.75
// at 1 km (a line source; CombatLights.hpp kWildfireRigPerLux puts it on the
// rig's scale). The fire's lights (FireWorld::UpdateLights) and the ground's
// halo (GetFlameLightCells) take their strength from the flames as drawn
// (the band, its tail and the flamelets of each cell, CellFlameLight; a
// flaring plant, FlareLightCd; a torching crown, TorchLightCd), so the light
// grows and dies with them: the ignition line's cells, lit all at once with
// a model window of minutes, go dark with their drawn band and tail.
constexpr float kFlameLightNits = 5000.0f;  // a grass or litter flame's body at full heat (cd/m2)
constexpr float kFlareLightNits = 6000.0f;  // a flaring bush's
constexpr float kTorchLightNits = 10000.0f; // a torching crown's (the consult: 15 m x 3 to 4 m at about 10000)
constexpr float kFlameLightDim = 0.3f;      // a dying flame's (heat 0) share of that (the renderer's kFlipDim)
constexpr float kTorchLightArea = 0.5f;     // a torch column's area over its length times its crown's width
constexpr float kFlameletLightCd = 45.0f;   // one residual flamelet (a 0.3 m tuft: the consult, about 45 cd)
// The ash's own glow behind a cell's band (0..1, the shaders' embers): it
// rises as the band leaves a point and fades over kEmberGlowSec (grass: the
// consult's red sparkle, 10 to 20 s), kEmberGlowWoodySec in shrubs and litter.
constexpr float kEmberGlowSec = 12.0f, kEmberGlowWoodySec = 60.0f;
// One cell's surface flames now (live, or its record with its age now; trDraw
// its drawn residence, midflameMps its midflame wind): their luminous
// intensity (cd) seen side on, the mean height they stand (m) and the ash's
// glow. The cell is taken in kFlameLightSlices slices across its front, each
// flaming as LingerFlame says at the drawn front's age there (the renderer's
// points do: FrontAgeField, CellLightSliceAges; `sliceAge` the caller's, or
// nullptr to read them now), its rows of tongues stacking to a wall as opaque
// as their emissivity allows.
constexpr int kFlameLightSlices = 4;
struct CellLight
{
    float cd = 0.0f, heightM = 0.0f, ember = 0.0f;
};
// The drawn front's age (s since it reached the point, FrontAgeField) at the
// slices' points: kFlameLightSlices points through the cell's centre along
// its front (frontX/Z; north when it lit all at once), at the slices' middles.
void CellLightSliceAges(const BurningView& v, float cellM, float out[kFlameLightSlices]);
CellLight CellFlameLight(const BurningView& v, float trDraw, float cellM, float midflameMps,
                         const float* sliceAge = nullptr);
// A torching crown's light (cd): kTorchLightNits over its column (the flame
// now, torchM, of its peak torchMaxM; the crown kTorchCrownShare of the tree's
// height across), dimmer while it grows and dies.
float TorchLightCd(float torchM, float torchMaxM, float treeH);
// A flaring plant's (cd): its flame now (flameM, tilted tiltRad) over its
// width.
float FlareLightCd(float flameM, float widthM, float tiltRad);
// A light's flicker: smooth value noise in [-1, 1] at `rate` values a second;
// and the fast flicker of the flames' light (zero mean, about 0.4 standard
// deviation, its power at 1.5 to 3 Hz: two sines of their own frequencies
// and phases by the seed, and a little noise so it never repeats).
float LightNoise(float t, float rate, std::uint32_t seed);
float LightFlicker(float t, std::uint32_t seed);

// The flicker of the fire's light where it lands (the consult's eighth
// table). Every light and the halo breathe with the whole fire's gusts
// (kFireSlowAmp over several seconds) and their own block's (kBlockSlowAmp);
// a block light flickers kBlockFastAmp at 1.5 to 3 Hz, so 100 m and more
// from the head (the whole front adds up, four to eight blocks) about 5%
// fast and 15 to 20% slow; the ground halo beside the flames (10 m from a
// grass head: the nearest 10 to 20 m of front, two or three cells)
// kHaloFastAmp per cell, so 15 to 25% fast; a torching crown kTorchSwing at
// its column's puff (0.5 to 0.7 Hz, in step with the drawn column's crown);
// a flaring bush kFlareSwing at its own puff; the plume's fill with the
// lights it is lit by (their power-weighted flicker) and kFillSlowAmp more.
constexpr float kFireSlowAmp = 0.2f, kBlockSlowAmp = 0.12f, kBlockFastAmp = 0.15f, kHaloFastAmp = 0.45f;
constexpr float kTorchSwing = 0.3f, kFlareSwing = 0.22f, kFillSlowAmp = 0.05f;
constexpr float kLightFastRate = 4.5f, kLightSlowRate = 0.35f, kFireSlowRate = 0.25f; // noise values a second

// The lights: one persistent light per 32 m block of burning ground
// (kLightBlockM), and one for each torching crown and flaring plant, at most
// kFireLightsMax at once (the frame's list takes twelve combat lights), the
// strongest at the camera (their candela over the squared distance, all
// within kLightNearM alike) picked with hysteresis (LightPick): a light keeps
// its place unless another beats it by kLightSwapRatio for kLightSwapSec. A
// source without a light of its own lends its power to the nearest block
// light (so no light leaves the scene when a source loses its place). Each
// light's power follows its sources over kLightAttackSec rising and
// kLightReleaseSec falling (never a step), its place and shape over
// kLightMoveSec; a light that loses its place fades out the same way.
constexpr int kFireLightsMax = 8;
constexpr float kLightBlockM = 32.0f, kLightNearM = 40.0f, kLightReachM = 3000.0f;
constexpr float kLightSwapRatio = 1.5f, kLightSwapSec = 1.0f;
constexpr float kLightAttackSec = 1.2f, kLightReleaseSec = 1.8f, kLightMoveSec = 1.5f;
// A light's window ends where it gives kLightRangeLux, at most kLightRangeMaxM.
constexpr float kLightRangeLux = 0.002f, kLightRangeMaxM = 3000.0f;
// The lit smoke as a lamp (the consult: 1 to 5 lux on level ground 100 m
// out, more than the flames give it there directly): one broad light over
// the fire, kFillHeightM above its ground and kFillDownwindM downwind (the
// plume's foot leans), its core kFillCoreMinM or half the fire's spread more,
// its power kFillShare of all the flames'.
constexpr float kFillShare = 0.08f, kFillHeightM = 60.0f, kFillDownwindM = 40.0f, kFillCoreMinM = 80.0f;
// The follow of a light's power (or of anything else that must not step):
// toward `target` over `riseSec` rising and `fallSec` falling.
inline float LightFollow(float cur, float target, float dt, float riseSec, float fallSec)
{
    const float tau = target > cur ? riseSec : fallSec;
    return cur + (target - cur) * (1.0f - std::exp(-std::max(dt, 0.0f) / std::max(tau, 1e-3f)));
}
// The pick (LightPick::Update): this frame's candidates (any order) and how
// much each lights what the camera sees.
struct LightCandidate
{
    std::uint64_t id = 0;
    float score = 0.0f;
};
struct LightPick
{
    std::uint64_t ids[kFireLightsMax] = {}; // 0: a free place
    std::uint64_t challenger = 0;
    float challengeSec = 0.0f;
    // A light whose source went out leaves; a free place takes the strongest
    // candidate without one at once; with none free the strongest of the
    // rest takes the weakest's place once it has beaten it by kLightSwapRatio
    // for kLightSwapSec (dt seconds since the last update).
    void Update(const LightCandidate* c, int n, float dt);
    bool Has(std::uint64_t id) const;
};

// The ground's halo (the shaders' burnFireLight, EngineGL33_Lighting.cpp
// BuildFireHeat): each cell's flames light the ground round them with
// HaloLux (lux at r metres of plan from the cell's centre, for its cd), the
// near field of a low flame wall at a grazing angle that the block lights
// leave out (their cosine is taken from at least a block's spread off): with
// them, level ground 10 m from a grass head takes about 60 to 90 lux, 30 m
// off about 8 (the consult: 35 to 90 at 10 m). It ends smoothly by
// kHaloReachM. The cells (GetFlameLightCells): every cell whose flames or
// glow show, its light already flickered (kHaloFastAmp), its glow (0..1).
constexpr float kHaloK = 5.5f, kHaloA2 = 11.8f, kHaloReachM = 28.0f;
inline float HaloLux(float cd, float r)
{
    if (r >= kHaloReachM || cd <= 0.0f)
        return 0.0f;
    const float x = r * r / (kHaloReachM * kHaloReachM);
    const float w = (1.0f - x) * (1.0f - x);
    const float q = r * r + kHaloA2;
    return kHaloK * cd / (q * q) * w;
}
struct FlameLightCell
{
    float x = 0.0f, z = 0.0f; // the cell's centre (world m)
    float cd = 0.0f;          // its flames' light now (cd, flickered)
    float ember = 0.0f;       // its ash's glow (0..1)
};
struct FlameLightView
{
    const FlameLightCell* cells = nullptr;
    int count = 0;
    float cellM = 8.0f;
};
// False (and out untouched) when the fire is off or nothing lights.
bool GetFlameLightCells(FlameLightView& out);

// The sparks for the renderer (FireSparks.hpp: one SparkPoint each, world
// positions and velocities), rebuilt every simulation step, and their glare:
// at night a spark past glareFromM (fully past glareToM) is drawn at least
// glareM across at its own luminance (the wildfire consult, eighth round:
// within 20 m a spark is its own 2 to 5 cm, beyond that at night 8 to 18 cm
// passes as the eye's glare), 0 by day. False (and out untouched) when the
// fire is off or none is in the air.
struct SparkView
{
    const SparkPoint* points = nullptr;
    int count = 0;
    float glareM = 0.0f, glareFromM = 20.0f, glareToM = 40.0f;
};
bool GetSparks(SparkView& out);

// The renderer's char tells a tree, a bush or a forest block (its trunk and
// branches char whole once burnt out, its leaves burn away) from a house or
// a wall (soot from the ground up). Shape::Draw brackets a shape's sections
// with Begin/EndVegetationDraw, which mark whether the drawing object's shape
// is one (its map type); the backend reads DrawingVegetation per draw. While
// the fire is off nothing is marked and DrawingVegetation is false.
bool DrawingVegetation();
// Returns the mark before it, for EndVegetationDraw.
bool BeginVegetationDraw(IAnimator* source);
void EndVegetationDraw(bool previous);
// (followups) A burning wreck's char and hot shell: Begin/EndVegetationDraw
// also mark whether the drawing object is a vehicle the fire burns, or a
// proxy of one (its camouflage, its load: Begin/EndWreckProxies round
// Object::DrawProxies; never its crew or weapons): its char (0 untouched .. 1
// burnt out) and its shell's glow (0..1; FireObjects.hpp WreckStateAt). The
// backend hands them to the object main (EngineGL33_LightingShadersSurface.cpp
// burnWreck) in its char class, with the glow's light on the scene's scale
// (WreckGlowNits): kWreckGlowNits at full glow, falling as the glow to the
// power 0.75 as it cools (about 16 cd/m2 just after a car's flames, 6 at 2
// h, 2 at its end), so it stays a dull red for hours, in fewer patches
// (EngineGL33_LightingShadersSurface.cpp burnWreck). Set by eye with the
// test aid fireSpreadSet "wreckglow" (the followups' sc_fu_glowcal): the
// night's fire gain falls with flames in view and rises to its cap without,
// so beside a burning grass line 12 times the glow barely showed and 75
// times read a dull orange-red, while in the dark 30 times it (with
// panel-wide patches) burnt a panel out flat orange, and 600 times its
// square salmon-white. A luminance taken as the
// embers' is (pi L lux on a white surface's 1.6, times the fire's light's
// rig scale and the night's gain, CombatLights.hpp kWildfireRigPerLux and
// its tuning), so by day it is lost in the sun. WreckBurnClass packs
// them after 3: its char in 128 steps, its glow in 64, the light's log10
// from kWreckGlowLogMin over kWreckGlowLogSpan in 255 (0 none);
// WreckBurnUnpack is the shader's reading. False while the fire is off or
// nothing burns.
constexpr float kWreckGlowNits = 30.0f;
inline float WreckGlowNits(float glow) { return kWreckGlowNits * std::pow(std::clamp(glow, 0.0f, 1.0f), 0.75f); }
constexpr float kWreckGlowLogMin = -7.0f, kWreckGlowLogSpan = 8.0f;
bool DrawingWreck(float& charK, float& glow);
// The test aid fireSpreadSet "wreckglow": the glow's light this many times
// its own (1 unless a test set it).
float WreckGlowTestScale();
bool BeginWreckProxies(const Object* parent);
void EndWreckProxies(bool pushed);
inline float WreckBurnClass(float charK, float glow, float lightRig)
{
    const int c = static_cast<int>(std::lround(std::clamp(charK, 0.0f, 1.0f) * 127.0f));
    const int g = static_cast<int>(std::lround(std::clamp(glow, 0.0f, 1.0f) * 63.0f));
    int e = 0;
    if (lightRig > 0.0f)
        e = std::clamp(
            static_cast<int>(std::lround((std::log10(lightRig) - kWreckGlowLogMin) / kWreckGlowLogSpan * 254.0f)) + 1,
            1, 255);
    return 3.0f + static_cast<float>(c * 16384 + g * 256 + e) / 2097152.0f;
}
inline void WreckBurnUnpack(float cls, float& charK, float& glow, float& lightRig)
{
    const float f = std::floor((cls - 3.0f) * 2097152.0f + 0.5f);
    const float c = std::floor(f / 16384.0f), rest = f - c * 16384.0f;
    const float g = std::floor(rest / 256.0f), e = rest - g * 256.0f;
    charK = c / 127.0f;
    glow = g / 63.0f;
    lightRig = e > 0.5f ? std::pow(10.0f, kWreckGlowLogMin + (e - 1.0f) / 254.0f * kWreckGlowLogSpan) : 0.0f;
}
// The tree, bush or forest block's crown has burnt away whole: the burn at
// its foot as the renderer's char reads it (the map there and kReach round
// it at kRing), past the leaves' last threshold, so the shadow pass drops its
// leaves as the lit shader does. False while the fire is off.
bool TreeLeavesGone(const Object* obj);

// ---- Each plant's own fire (the realism audit's fix 3: causes C9 and C11) ----
// Every tree, bush, reed clump and olive the fire comes near has a fire of
// its own, so its leaves, bark and flare follow what happened to it and never
// its 8 m cell's burn value. It is reached (its contact) when the drawn
// flames first touch it: the drawn front (DrawnReachSec, the tongues' own
// timing) at its crown's edge on the side the fire comes from (a bush, a reed
// clump or an olive), or under its crown (kPlantTreeReachShare of its
// radius, a tree). From then on, what the surface fire that reached it was
// (its intensity and flame length, the front's direction) and whether it
// flared or torched set its look (PlantLookOf, the wildfire consult's fifth
// and eighth answers):
//   - crown scorch: leaves brown up to CrownScorchM above the ground (3 to 5 m
//     at a 2000 kW/m head, 0.5 to 1 m on a 100 kW/m flank, the flames' height
//     at the back), the top rising to it over kPlantScorchRiseSec, a soft edge
//     higher on the lee side; a stand pine's crown (its base 4 to 6 m up) is
//     barely touched, a broadleaf browns on its lowest 3 to 5 m;
//   - bark char (BarkCharM): up the trunk to 0.5 to 1 m on the windward side
//     and 2 to 4 m on the lee side at the head, 0.3 to 0.6 m on a flank,
//     0.2 to 0.4 m and nearly even at the back, with a ragged slanted top;
//   - burning away, only for a plant that flares or torches: its leaves go
//     clump by clump in a ragged sweep from its windward base to its lee top
//     through the flare's rise and full flame (or the torch's), each going
//     brown, then black and glowing, then gone (a burning thicket's or a
//     torching crown's scorched leaves char black within the first third of
//     the sweep, a dark lattice in the flames); the clumps it keeps char
//     toward keptChar (a thicket's and a reed clump's as thin strands, its
//     stems, never round blobs). A thicket ends a black skeleton at
//     kThicketKeep of its bulk, a flared olive a blackened frame with brown leaves (kOliveGone of
//     them burnt), a reed clump black stubble kReedStubbleM high, a leafy bush
//     loses its windward lower half's leaves by half; only a torched pine
//     loses all its needles. Its trunk and branches char whole (frameTop)
//     as the flames climb it.
// Nothing changes in a frame: every edge is a smooth noise contour in the
// world, never a texel of a map.
enum class PlantKind : std::uint8_t
{
    Thicket,   // a maquis thicket (krovi): flares fully
    Leafy,     // a summer-green leafy bush (ker listnac): scorches; a weak flare at the head
    Reed,      // a reed or cane clump (ker rakosi): a fast, tall flash
    Olive,     // a cultivated olive: flares partly from kOliveMinKwM
    Pine,      // a conifer (pine, spruce, mountain pine): torches as the fire model says
    Broadleaf, // every other tree: scorches, never flares
};
constexpr float kPlantTreeReachShare = 0.5f, kPlantTreeReachMaxM = 1.5f;
constexpr float kPlantScorchRiseSec = 20.0f;
constexpr float kThicketKeep = 0.25f, kOliveGone = 0.6f, kReedStubbleM = 0.4f, kReedGone = 0.93f;
constexpr float kLeafySweep = 0.5f, kLeafyGone = 0.5f;
// The drawn flames' timing (CellSurfaceTongues): the seconds after the cell
// v caught that its drawn front reaches the point (px, pz). Since fix 4 the
// drawn front is the arrival field FrontAgeAt reads (the front on the model's
// ellipse, ragged by its edge noise; never on a road's bare surface); a view
// without it (exact, grid and planar all false) reads its planar front,
// clamped into the cell (0 for a cell lit all at once).
inline float DrawnReachSec(const BurningView& v, float px, float pz, float cellM)
{
    return v.age - FrontAgeAt(v, px - v.x, pz - v.z, cellM);
}
// The height (m above the ground) a surface fire of intensity kwM (kW/m)
// browns the leaves of a crown it does not burn (Van Wagner 1973 bent over by
// the wind, as the consult worked it for this wind: 3 to 5 m at the head, 0.5
// to 1 m on a flank), never under kScorchMinM.
constexpr float kScorchHeadM = 4.0f, kScorchHeadKwM = 2000.0f, kScorchPow = 0.55f;
constexpr float kScorchMinM = 0.15f, kScorchMaxM = 6.0f;
inline float CrownScorchM(float kwM)
{
    return std::clamp(kScorchHeadM * std::pow(std::max(kwM, 0.0f) / kScorchHeadKwM, kScorchPow), kScorchMinM,
                      kScorchMaxM);
}
// Bark char from a surface flame flameM long (m) passing a stem (the consult's
// eighth answer, after Gutsell and Johnson 1996): on its lee side (the side
// the fire runs on to) about 1.26 L - 0.27 (3 m at a 2.6 m head, 0.55 m on a
// 0.65 m flank), on its windward side a share of that falling from nearly
// even at the back to a quarter at the head; a thin stem (a bush's) chars
// more evenly.
constexpr float kBarkLeeK = 1.256f, kBarkLeeOffsetM = 0.27f, kBarkMinM = 0.2f, kBarkMaxM = 4.0f;
inline void BarkCharM(float flameM, bool thinStem, float& windM, float& leeM)
{
    leeM = std::clamp(kBarkLeeK * flameM - kBarkLeeOffsetM, kBarkMinM, kBarkMaxM);
    const float k = std::clamp((flameM - 0.5f) / 2.0f, 0.0f, 1.0f);
    const float even = k * k * (3.0f - 2.0f * k);
    windM = leeM * (thinStem ? 0.9f - 0.3f * even : 0.85f - 0.6f * even);
}
// The share (0..1) of the world a plant's leaf-clump noise (the shader's
// puddleNoise, smooth value noise) holds under each tenth: the threshold
// under which that share of it lies, so a plant keeps exactly its share.
constexpr float kPlantNoiseQuantile[11] = {0.0f,    0.2096f, 0.2993f, 0.3721f, 0.4375f, 0.5f,
                                           0.5626f, 0.6281f, 0.7008f, 0.7905f, 1.0f};
inline float PlantNoiseThreshold(float share)
{
    const float s = std::clamp(share, 0.0f, 1.0f) * 10.0f;
    const int i = std::min(static_cast<int>(s), 9);
    return kPlantNoiseQuantile[i] + (kPlantNoiseQuantile[i + 1] - kPlantNoiseQuantile[i]) * (s - static_cast<float>(i));
}
// A skeleton of stems: the kept share of a plant's leaf fragments as a band
// round the noise's median (|n - 0.5| under this half width) instead of its
// highest clumps, so what stays is thin winding strands, not round blobs.
inline float PlantStrandHalfWidth(float keepShare)
{
    return PlantNoiseThreshold(0.5f + 0.5f * std::clamp(keepShare, 0.0f, 1.0f)) - 0.5f;
}
// A plant's look in its own fire, all times in seconds after its contact
// (what the plant table carries to the shader for it).
struct PlantLook
{
    float scorchTopM = 0.0f, scorchRiseSec = kPlantScorchRiseSec; // brown leaves up to here, rising over this
    float scorchChar = 0.0f; // while it flares or torches, its scorched leaves char toward black by this (0..1)
    bool strands = false;    // what it keeps is thin strands (stems), not clumps of leaves
    float barkWindM = 0.0f, barkLeeM = 0.0f; // bark char up the stem
    float burn0Sec = 0.0f, burn1Sec = 0.0f;  // the burning sweep (none unless burn1Sec > burn0Sec) ...
    float sweepMax = 1.0f;                   // ... reaching this share of the plant (1 all of it)
    float goneShare = 0.0f;                  // the share of its leaf clumps the sweep burns away ...
    float keptChar = 0.0f;                   // ... and how black the rest end (0 brown, 1 charcoal)
    float frameTopM = 0.0f;                  // its trunk and branches char whole up to here with the sweep
    float keepBelowM = 0.0f;                 // no clump under this height goes (a reed clump's stubble)
    float clumpM = 0.5f;                     // the size of its leaf clumps (m)
    float glow = 0.0f;                       // the sweep's burning edge glows this much (0..1)
};
// kind, its height h (m); the surface fire that reached it (kwM, flameM);
// whether it flared (its FlareShape) or torched (torchSec after its contact
// its crown began: TorchEnvelope's delay).
PlantLook PlantLookOf(PlantKind kind, float h, float kwM, float flameM, const FlareShape* flare, bool torched,
                      float torchSec);
// The plant table the renderer reads (the burn atlas's last rows): an open
// addressing hash on the plant's foot, kPlantSlotFloats floats a slot (the
// layout below), slots a power of two, kPlantSlotsPerRow a row, never more
// than kPlantLoadDiv-th taken, each slot within kPlantProbes of its home. A slot's key
// is its foot (x, z); it sits in the 1 m bucket of its foot and, when its
// foot is within kPlantKeyNearM of a bucket's edge, in that neighbour's too,
// so the shader, which rebuilds the foot from the drawn object's origin to a
// millimetre or so, finds it whatever side of the edge it lands. Empty slots
// hold kPlantEmpty in x. Times (contact) run on the fire's clock wrapped at
// kPlantClockWrap, as the shader's litBurnHeat.z: kPlantNotYet before the
// plant is reached, kPlantLongAgo once its look is final.
constexpr int kPlantSlotFloats = 32, kPlantSlotsPerRow = 16;
constexpr float kPlantKeyNearM = 0.06f;
constexpr float kPlantEmpty = -1.0e9f, kPlantNotYet = 1.0e9f, kPlantLongAgo = -1.0f;
constexpr double kPlantClockWrap = 628.31853;
constexpr int kPlantProbes = 16, kPlantLoadDiv = 3;
enum PlantSlotField : int
{
    kPlantX = 0,
    kPlantZ,
    kPlantY,
    kPlantH,
    kPlantContact,
    kPlantDirX,
    kPlantDirZ,
    kPlantScorchTop,
    kPlantScorchRise,
    kPlantBarkWind,
    kPlantBarkLee,
    kPlantBurn0,
    kPlantBurn1,
    kPlantSweepMax,
    kPlantGoneThreshold, // PlantNoiseThreshold of the share that goes
    kPlantKeptChar,
    kPlantFrameTop,
    kPlantKeepBelow,
    kPlantClump,
    kPlantGlow,
    kPlantScorchChar,
    kPlantStrands, // 1: the kept share is a band of the noise (thin strands); kPlantGoneThreshold its half width
    kPlantFields
};
static_assert(kPlantFields <= kPlantSlotFloats, "a plant's fields fit its slot");
// A bucket's home slot before the mask: the two coordinates mixed, then
// MurmurHash3's finaliser, so neighbouring buckets land far apart (linear
// probing would pile them up). The shader's burnPlantAt is the same.
inline std::uint32_t PlantBucketHash(int bx, int bz)
{
    std::uint32_t h = static_cast<std::uint32_t>(bx) * 73856093U ^ static_cast<std::uint32_t>(bz) * 19349663U;
    h ^= h >> 16;
    h *= 0x85ebca6bU;
    h ^= h >> 13;
    h *= 0xc2b2ae35U;
    h ^= h >> 16;
    return h;
}
struct PlantTable
{
    std::vector<float> data;
    int slots = 0, used = 0;
    // A table of slots (a power of two), all empty.
    void Reset(int slotCount);
    // The slot its foot (x, z) takes in each bucket it belongs to (out, up to
    // four; -1 past them), or false when the table is too full, kept `reserve`
    // slots short of its load (room held back for buildings). A refusal leaves
    // the table as it was (no slot half taken).
    bool Insert(float x, float z, int out[4], int reserve = 0);
    // The slot whose foot is within kPlantKeyNearM of (x, z) (as the shader
    // probes), or -1.
    int Find(float x, float z) const;
    float* Slot(int s) { return data.data() + static_cast<std::size_t>(s) * kPlantSlotFloats; }
    const float* Slot(int s) const { return data.data() + static_cast<std::size_t>(s) * kPlantSlotFloats; }
};
// (end-state agent, 2026-11-18) A burnt building's look lasts as long as the
// building does: once the table was full, a refused plant cleared every
// building's slots and the building's own write found no room, so a gutted
// house was drawn whole again (Cancon, the wrecks-after shooter, 46 min). A
// foot in the table and the slots it holds (a plant's or a building's).
struct PlantFootRef
{
    float x = 0.0f, z = 0.0f;
    int* slots = nullptr; // its four slots (-1: none)
};
// Room a table holds back for the buildings that have no slots yet (four each,
// and at least kPlantBuildingReserveMin).
constexpr int kPlantBuildingReserveMin = 128;
inline int PlantBuildingReserve(int buildingsWithoutSlots)
{
    return std::max(kPlantBuildingReserveMin, 4 * buildingsWithoutSlots);
}
// A new foot `add` into the plant table t (a plant's, or a building's with
// isBuilding): it goes in while the table has room (a plant's kept `reserve`
// short); else the table doubles, up to maxSlots, and takes every building's
// foot first (those holding slots), then every plant's, then the new one. A
// plant that fits nowhere is refused and t and every slot are left exactly as
// they were: a plant never costs a building its slots. A building that fits
// nowhere takes the table at its largest from the plants: buildings first,
// then as many plants as fit (the rest lose their slots, `dropped`). grew:
// every slot moved, the caller writes every look again. False only when the
// buildings alone cannot fit.
bool PlantTableAdd(PlantTable& t, const PlantFootRef& add, bool isBuilding, int reserve, int maxSlots,
                   const std::vector<PlantFootRef>& buildings, const std::vector<PlantFootRef>& plants, bool& grew,
                   int& dropped);

// ---- Buildings (buildings builder; the wildfire consult, 2026-11-18, tenth answer) ----
// A building's own footprint (its shape's box) and an apron kBuildingApronM
// round it carry no fuel (a cell whose centre lies in them is bare); the grass
// beside it burns up to the apron, the cells next to a bare one running their
// ground on into it (BurningView::builtSides). Each building is an object of
// its own: its kind, a chance to catch each time a fire passes it (the
// moment the drawn front reaches its apron), a delay, then its own timeline.
// What burns is drawn by the shared object fire (the wrecks builder's).
enum class BuildingKind : std::uint8_t
{
    StoneHouse,  // stone or brick walls, timber floors, a tiled timber roof (house and building map types, a chapel)
    Shed,        // a wooden or tin shed, a hut, a coop
    Barn,        // a wooden barn with hay
    Church,      // stone walls and tower, a tall dry timber roof
    FuelStation, // pumps and a kiosk (the tanks underground never explode)
    Masonry,     // a large masonry block (a hospital): as a stone house, half the chance, slower
    Never,       // a bunker, a fortress, concrete: never burns
    Fence,       // a wooden fence (no apron: the grass burns to its foot, and its foot lights it)
};
constexpr int kBuildingKinds = 8;
constexpr float kBuildingApronM = 2.0f;
constexpr float kBuildingMinHeightM = 2.4f;
// Its kind from its map type (FireSpreadWorld.cpp passes the shape's) and its
// shape's name (lower case; the islands' are Czech: kulna and bouda a shed,
// stodola a barn, seno hay, benzina a fuel station, panelak a concrete block,
// dumruina a ruin, garaz an army garage).
// mapType: 0 a house or building, 1 a church, 2 a chapel, 3 a bunker or
// fortress, 4 a fuel station, 5 a hospital, 6 a fence (a wooden one, by its
// name: Newplot, Statek_plot, ohrada, plot_provizorni, plot_zboreny; a wire,
// metal or brick one never burns).
BuildingKind BuildingKindOf(int mapType, const char* lowerName);
// A shed or barn whose walls are masonry (Abel's statek_kulna and stodola_*:
// plastered stone with a timber roof) keeps its walls when its fire is out
// (a roofless shell, as a house); a timber one (bouda, drevnik and the rest)
// burns away to its lowest metre.
bool BuildingWallsStand(const char* lowerName);
// The two fire days (the tenth answer's build-to table): the build's ember
// ignition chance (pig, from the fine fuel's moisture as NotifyBurning has it)
// and the 10 m wind. FireDayIndex is 0 on the typical summer afternoon (pig
// 0.63, 4.5 m/s), 1 on the worst fire day (0.96, 12.5 m/s), half from each;
// past them (a damp day below 0, a storm above 1) within -2 and 1.5. A chance
// or a count between the days is DayBlend: geometric, typical x (worst /
// typical)^s.
constexpr float kDayPigTypical = 0.63f, kDayPigWorst = 0.96f, kDayWindTypical = 4.5f, kDayWindWorst = 12.5f;
inline float EmberIgnitionChance(float m1h)
{
    return std::pow(std::clamp((0.20f - m1h) / 0.17f, 0.0f, 1.0f), 1.5f);
}
inline float FireDayIndex(float pig, float wind10)
{
    const float sm = (pig - kDayPigTypical) / (kDayPigWorst - kDayPigTypical);
    const float sw = (wind10 - kDayWindTypical) / (kDayWindWorst - kDayWindTypical);
    return std::clamp(0.5f * (sm + sw), -2.0f, 1.5f);
}
inline float DayBlend(float typical, float worst, float s)
{
    return typical * std::pow(std::max(worst, 1e-6f) / std::max(typical, 1e-6f), s);
}
// The chance a building catches when a grass fire runs past it (the tenth
// answer's judgement, the middle of each range): a stone house on clear ground
// 2% typical, 10% worst; with bushes, a hedge or a pine against it (vegAgainst)
// 7.5% and 30%; a wooden shed 15% and 65%; a barn with hay 15% and 75%; a
// church and a fuel station as a stone house; a masonry block half of it;
// never past kBuildingChanceMax, none under kBuildingPigMin.
constexpr float kBuildingChanceMax = 0.95f, kBuildingPigMin = 0.05f;
float BuildingPassChance(BuildingKind kind, bool vegAgainst, float pig, float wind10);
// (followups) Each draw a building makes (its catch on a pass, its delay,
// which room first, its windows, its embers) is a hash of its own seed (its
// position), the draw's number and the run's seed, so a run repeats exactly
// with one run seed. The run seed is new each game (each new world and each
// fireSpreadClear): drawn from the clock, never from the game's own random
// stream (GRandGen), so nothing else in the game moves; in a deterministic
// world (POSEIDON_DETERMINISTIC_WORLD) a fixed sequence by the game's count
// of them, so a capture repeats; fireSpreadSet "bseed" fixes it for repeated
// trials. BuildingRunSeedFor is that seed for the generation-th roll of a
// game (entropy: the clock's), BuildingDrawFor the draw (0..1).
std::uint32_t BuildingRunSeedFor(std::uint32_t generation, bool deterministic, std::uint64_t entropy);
float BuildingDrawFor(std::uint32_t buildingSeed, int k, std::uint32_t runSeed);
// Embers landing on it: each a pass's chance (it is how a pass lights it).
// A burning neighbour's heat (gapM between their footprints, m) on the day s
// (FireDayIndex): wood (a shed, a barn) lights by kHeatNearM and not past
// kHeatWoodM; a house's eaves, shutters and windows kHeatEaves{Typical,Worst}
// within kHeatNearM (DayBlend), tapering to none at kHeatWoodM. The tenth
// answer has houses in a row 5 to 8 m apart passing fire along and its
// village losing one stone house in ten to one in three on the worst day;
// with Abel's terraces (gaps near 0 m, four to eight neighbours within 12 m)
// a house that lit each neighbour at 0.7 burnt 58% of the largest village on
// the worst day and a fifth on the typical (toolsillage_mc.py, 2000 trials,
// every house passed), so the outcome sets these (35% and 6% at most).
constexpr float kHeatWoodM = 12.0f, kHeatNearM = 6.0f, kHeatEavesTypical = 0.12f, kHeatEavesWorst = 0.3f;
float BuildingHeatChance(BuildingKind kind, float gapM, float dayIndex);
// Its delay (s) from the front passing to the first flame inside, from a
// uniform u: a stone house, a church or a masonry block 5 to 30 min (median
// 12; mostly embers lodged under tiles, in eaves, through a window), a shed or
// a barn 0.5 to 4 min (flame contact), a fuel station 1 to 5 min.
float BuildingDelaySec(BuildingKind kind, float u);
// Its timeline (s from the first flame inside), drawn once from uniforms u
// (kBuildingTimelineDraws of them, 0..1), the tenth answer's table and its
// timeline of a stone house:
//   stone house, room first: grey smoke from the eaves or a window,
//     thickening and darkening, to flashSec (10 to 20 min); a window's flames
//     1 to 3 m out (windowM), the rest joining by spreadSec (20 to 40 min),
//     smoke black; the roof falls in at collapseSec (30 to 60 min): a burst of
//     sparks and embers, flames peakM (3 to 8 m) above the wall tops, peakMW
//     (20 to 40 MW); the upper floor falls at floorSec (10 to 30 min later),
//     flames drop inside the shell; burnt down at downSec (1.5 to 3 h); the
//     rubble glows glowSec (6 to 12 h) and smokes thinly smokeSec (1 to 2
//     days) after that;
//   stone house, roof first (embers under the tiles, or a neighbour's heat on
//     the eaves): smoke from the tiles, the roof alight at flashSec (10 to 15
//     min, roofM 2 to 5 m through it), in at collapseSec (25 to 45 min), the
//     rooms below after (spreadSec);
//   masonry: a stone house, its times kMasonrySlow longer, its flames lower;
//   church: the roof alight at flashSec (15 to 30 min), at its peak (peakM 5 to
//     12 m above the ridge) from 30 to 60 min, in at collapseSec (1 to 2 h),
//     the walls and tower standing; down at 2 to 4 h;
//   shed: fully alight at flashSec (2 to 4 min, peakM 3 to 5 m), down at
//     collapseSec (8 to 15 min), a burning heap to downSec (30 to 45 min),
//     embers glowSec (2 to 4 h);
//   barn: fully alight 5 to 10 min (10 to 15 m), collapsed 15 to 30 min,
//     flaming to 1 to 2 h, the hay smouldering white smokeSec (days);
//   fuel station: pumps and kiosk alight within minutes (3 to 6 m, heavy black
//     smoke), the pumps' pools out by 10 to 30 min (spreadSec), the kiosk down
//     at 30 to 60 min;
//   wooden fence: alight along itself within 1 to 3 min (0.8 to 1.5 m), down
//     (fallen) at 10 to 30 min, its posts glowing half an hour.
constexpr int kBuildingTimelineDraws = 10;
constexpr float kMasonrySlow = 1.5f;
struct BuildingTimeline
{
    BuildingKind kind = BuildingKind::StoneHouse;
    bool roofFirst = false;
    float flashSec = 0.0f, spreadSec = 0.0f, collapseSec = 0.0f, floorSec = 0.0f, downSec = 0.0f;
    float glowSec = 0.0f, smokeSec = 0.0f;
    float peakM = 0.0f, windowM = 0.0f, roofM = 0.0f, peakMW = 0.0f;
};
BuildingTimeline BuildingTimelineOf(BuildingKind kind, bool roofFirst, const float* u);
// What it shows t seconds after its first flame inside: the share of its
// windows alight (each lights at its own share, windowShare, through
// flashSec..spreadSec) and a lit window's flame (m); flames above its wall
// tops (topM: through the roof, then out of the roofless shell), from its
// whole body (bodyM: a shed, a barn, a kiosk and its pumps) and low inside
// its shell (innerM); its heat release (MW), its smoke (0..1 of its peak's)
// and that smoke's black share; its rubble's glow (0..1); whether its roof (a
// shed's or barn's whole body) has fallen in, and whether anything still
// flames.
struct BuildingPhase
{
    float windows = 0.0f, windowM = 0.0f;
    float topM = 0.0f, bodyM = 0.0f, innerM = 0.0f;
    float heatMW = 0.0f, smoke = 0.0f, black = 0.0f, glow = 0.0f;
    bool collapsed = false, flaming = false, over = false;
};
BuildingPhase BuildingPhaseAt(const BuildingTimeline& tl, float t);
// A window's own moment within flashSec..spreadSec (0 the first: windowShare
// 0 lights at flashSec) and its flame t seconds after the first flame inside
// (its flame grows over kWindowGrowSec, dims when the floor falls).
constexpr float kWindowGrowSec = 45.0f;
float BuildingWindowFlameM(const BuildingTimeline& tl, float windowShare, float t);
// Embers at the roof's collapse (a shed's or barn's): typical 1 to 3 (median
// 15 m, to 80 m), worst 3 to 10 (median 40 m, to 300 m), DayBlend by s; a
// shed half a house's.
constexpr float kCollapseEmbersTypical = 2.0f, kCollapseEmbersWorst = 6.5f;
constexpr float kCollapseEmberMedianTypicalM = 15.0f, kCollapseEmberMedianWorstM = 40.0f;
constexpr float kCollapseEmberMaxTypicalM = 80.0f, kCollapseEmberMaxWorstM = 300.0f;
// A burning building lights cured grass by its heat this far from its walls
// (a house 12 to 14 m; a shed half), at its peak.
constexpr float kHeatGrassHouseM = 13.0f, kHeatGrassShedM = 5.0f;
// At most this many buildings burn (flame, smoke) at once; one that catches
// past it waits for a place.
constexpr int kBuildingFiresMax = 32;

// ---- Wrecks (fire-objects: the wrecks builder) ----
// With the fire on, a destroyed vehicle burns on as the fire's own object
// fires (FireObjects.hpp: its class's timeline from the wildfire consult's
// tenth answer, its flames through the fire's flame pass, its smoke black and
// upright through the plume, its light through the fire's lights, its heat
// and embers lighting the ground round it), and its fuel fireball reaches the
// fire model whatever its size. Smokes.cpp's SmokeSourceVehicle calls these
// as the wreck explodes; while the fire is off they do nothing, so the stock
// wreck is as it was.
// The fuel in the wreck's fireball (kg): the stock's stockKg, or with the
// fire on, by its class (FireObjects.hpp WreckFireballKg: a car's 5 to 10 kg,
// a tanker's 500 to 1000).
float WreckFireballFuelKg(const Object* vehicle, float stockKg);
// The most fuel the wreck's fireball may take (kg): the stock's cap, a
// tanker's (FireObjects.hpp kTankerBallMaxKg) with the fire on.
float WreckFireballMaxKg(const Object* vehicle);
// The wreck's fire begins: its fireball (radius m; 0: none) lights the ground
// it reaches now. True when the fire burns it on (the stock's own smoke and
// ember light then stop: Smokes.cpp).
bool NotifyWreck(const Object* vehicle, float fireballRadiusM);
// The wreck's own blast (its FuelExplosion through ExplosionDammage) is its
// fire's: NotifyExplosion leaves it alone while this is on, and the blast
// digs no crater and throws no earth (InWreckBlast: a fuel fire and a cook-off
// are not a shell in the ground).
void SetWreckBlast(bool on);
bool InWreckBlast();

// Registers fireSpreadIgnite, fireSpreadSet, fireSpreadStats and
// fireSpreadClear (only when Enabled()).
void RegisterScriptCommands();

} // namespace Poseidon::firespread
