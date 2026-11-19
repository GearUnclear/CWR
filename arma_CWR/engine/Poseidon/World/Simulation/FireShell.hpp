#pragma once

// ---- Burnt building shells (the end-state agent, 2026-11-18) ----
// A house the fire guts (its roof fallen in: FireSpreadWorld.cpp
// UpdateBuildings) is marked on the object itself (Object::IsGutted), set on
// every machine through the replicated damage message (Object::GutByFire, the
// ammo name kShellAmmo) and kept in a savegame, so a remote client and a load
// show the same shell and the stock crumple (MakeDestroyed: the house sunk
// and skewed, "shelled" in the wildfire consult's eleventh answer) never runs
// on it. A gutted object draws and collides with its shape's shell: a copy of
// its own LODShape made once per shape (ShellShapeOf) whose collision, fire
// and view geometry stop at the wall tops (the roof's components gone, a
// component that spans the eaves cut there), so shots and sight pass over the
// walls and into the shell, and whose visual levels keep every face of the
// house as it was (the burnt look, burnBuilding, decides what of them shows)
// plus its rubble: the roof's own faces fallen in, broken up into a heap
// 0.3 to 1 m deep inside the walls (the eleventh answer: mostly broken red
// tile, ash, a few charred beams, never filling the shell).
// All of it only while the fire lane is on (POSEIDON_XP_FIRESPREAD).

#include <cstdint>
#include <vector>

namespace Poseidon
{
class Object;
class LODShape;
class LODShapeWithShadow;
} // namespace Poseidon

namespace Poseidon::firespread
{

// The ammo name of the fire's gutting damage (no config ammo has a leading '#').
constexpr const char* kShellAmmo = "#udfireshell";

// How the shell of a shape is cut: a gutted masonry building keeps its walls
// to their tops (Walls; a Church the same, with no chimney: the geometry
// agent's rule takes anything small on its roof for a timber turret); a
// timber shed or barn burnt away keeps its lowest kShellStubM (Stub).
enum class ShellMode : std::uint8_t
{
    Walls,
    Church,
    Stub,
};
constexpr float kShellStubM = 0.5f;

// ---- Pure pieces (unit tested) ----

// A face of a visual level as the wall-top estimate reads it: its lowest and
// mean height (model y), its normal's up share and its area (m2).
struct ShellFaceStat
{
    float minY = 0.0f, meanY = 0.0f, ny = 0.0f, area = 0.0f;
};
// The wall tops (model y) of a house: the eave line under its roof. Its roof
// faces are those turned up (ny > kShellRoofNy) in its upper part (mean above
// baseY + kShellRoofFrom of its height); the eave is their lowest edge, the
// area-weighted kShellEaveQuantile of their lowest points (a porch roof or a
// dormer's cheek low on it does not pull it down). With no roof face, the
// buildings builder's share of its height.
constexpr float kShellRoofNy = 0.3f, kShellRoofFrom = 0.45f, kShellEaveQuantile = 0.1f;
float ShellWallTopY(const std::vector<ShellFaceStat>& faces, float baseY, float topY);

// The rubble's depth (m) over its floor at (x, z) in the model's frame: a
// heap of depth d (a one-storey house 0.35 m, two storeys 0.6), higher along
// the walls and under the ridge line, low in patches, within 0.15 and 1.1 m
// and never within 0.3 m of the wall tops. hx, hz: the walls' half extents
// round (cx, cz); ridgeAlongX: the ridge runs along x; seed: the shape's.
struct ShellRubbleFrame
{
    float cx = 0.0f, cz = 0.0f, hx = 1.0f, hz = 1.0f;
    float floorY = 0.0f, wallTopY = 3.0f;
    bool ridgeAlongX = true;
    std::uint32_t seed = 0;
};
float ShellRubbleDepth(const ShellRubbleFrame& f, float x, float z);
// A roof point (x, z) brought inside the walls: the roof's own extent (its
// half extents rhx, rhz round rcx, rcz) mapped onto the walls' less an inset.
constexpr float kShellRubbleInsetM = 0.3f;
void ShellRubbleInside(const ShellRubbleFrame& f, float rcx, float rcz, float rhx, float rhz, float& x, float& z);

// A geometry level's point above the cut (model y) brought down onto it
// (kShellSquash of its height above stays, so no face of a convex component
// that spans the cut goes flat); a component whose lowest point is at or
// above the cut less kShellComponentGapM goes.
constexpr float kShellSquash = 0.02f, kShellComponentGapM = 0.05f;
inline float ShellSquashY(float y, float cutY)
{
    return y > cutY ? cutY + (y - cutY) * kShellSquash : y;
}
inline bool ShellComponentGoes(float componentMinY, float cutY)
{
    return componentMinY >= cutY - kShellComponentGapM;
}

// ---- The engine side (FireShell.cpp) ----

// The wall tops (model y) of a shape, from its first visual level (cached).
float ShellWallTopOf(const LODShape* shape);
// The shell of a house shape for a mode and a stage, made once and kept: a
// copy of the shape (CopyClassFrom), on which the geometry agent's rule
// (FireBuildingShell.hpp AddBurntShellVisual) adds the chimneys' piers and
// the walls' inner sides and, at stage 1, hides what the fire took; then its
// collision, fire and view levels cut at the wall tops and its rubble merged
// into its visual levels. Stage 0 is worn while the roof falls in (the
// shader's ragged cut takes the roof), stage 1 from 0.9 of the fall on and
// whenever a gutted house is met with no fire of its own (a load, a remote
// machine). nullptr: none.
// `seed` (stage 1: a building's own, ShellSeedOf) draws its broken wall tops
// and gables (FireBuildingShell.hpp PlanBurntTops): a shell for each gutted
// building, so no two burnt houses match.
LODShapeWithShadow* ShellShapeOf(LODShapeWithShadow* shape, ShellMode mode, int stage, std::uint32_t seed = 0);
// obj (marked gutted) wears its shell's `stage`; a no-op while the fire is off.
void ShellWear(Object* obj, int stage);
// obj (just marked gutted) takes its shell at the stage the fire here says
// (ShellStageNow), and the fire is told (NotifyGutted).
void ShellGut(Object* obj);
// obj (repaired) takes its own shape again.
void ShellUngut(Object* obj);
// Whether a shape is a shell, its stage (-1: not a shell), and whether its
// visual levels hide the roof (the burnt look then cuts nothing).
bool IsShellShape(const LODShape* shape);
int ShellStageOf(const LODShape* shape);
bool ShellHidesRoof(const LODShape* shape);
// A shell's own house shape (the shape itself when it is no shell).
const LODShape* ShellSourceOf(const LODShape* shape);
// Every shell made, dropped (another island).
void ShellClear();

// FireSpreadWorld.cpp: a building gutted on this machine (its own fire, a
// remote one's message, a load, a clear) gets its burnt look for good; a
// repaired one loses it.
void NotifyGutted(Object* obj);
void NotifyUngutted(Object* obj);
// The mode of the shell a building of this shape takes (by its kind).
ShellMode ShellModeOf(const LODShape* shape);
// The stage obj's shell should be worn at now: 0 while this machine's own
// fire has its roof falling in, else 1.
int ShellStageNow(const Object* obj);
// The seed of obj's broken tops: its place and the run's seed (fireSpreadSet
// "bseed", as the buildings' catch draws): the same building breaks the same
// way in a run, another run another way. (A remote machine or a load with
// another run seed draws other breaks: the look only, its collision is cut at
// the wall tops whatever they are.)
std::uint32_t ShellSeedOf(const Object* obj);

} // namespace Poseidon::firespread
