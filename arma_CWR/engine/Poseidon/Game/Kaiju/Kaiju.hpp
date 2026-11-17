#pragma once

// Giant-creature support ("kaiju"): the small engine seams a 40 m Man-class
// creature needs. Everything here is off until a script command or a type's
// `class Kaiju` config block switches it on for one unit, so stock content
// takes the fast path (one integer test) and behaves exactly as before.
//
// This header is included from hot engine files, so it carries only plain
// data and inline guards. The service lives in KaijuService.cpp, the far
// draw pass in KaijuFarDraw.cpp, the script commands in KaijuCommands.cpp.
// Script-facing contract: experiments/kaiju/features/engine/API.md.

#include <Poseidon/Foundation/Math/Math3D.hpp>

namespace Poseidon
{
class Object;
class EntityAI;
class Man;
class Scene;
class ParamEntry;

namespace Kaiju
{

// Per-unit switches. A zero or negative value means "stock behaviour".
struct Traits
{
    float farDraw = 0;          // draw out to this distance (m), past objectsZ and the view distance
    float farContrast = 0.5f;   // silhouette strength at the fog end (0..1)
    float wadeDepth = 0;        // water depth (m) the unit stands in without drowning
    bool obstacleBlind = false; // planner: houses, trees and dense cells cost like open ground
    bool noSpeedDamage = false; // no crash damage for moving faster than 10 m/s on the ground
    float damageCeiling = -1;   // total and zone damage never pass this (0..1); < 0 = no ceiling
    bool hitReport = false;     // every hit is queued for kjHits, without the 0.05 floor
    float stepShake = 0;        // tremor strength started by each footfall
    float gridRadius = 0;       // widen object-grid queries to this radius (m)
    bool nearMip = false;       // texture mip levels from the nearest point of the bounding sphere, not the origin
    float glow = -1;            // scale of the shining (fullbright) faces: 0 dark, 1 full, to 2 white hot; < 0 = stock
    float farGlow = 0;          // night: lowest fog visibility of a far-drawn unit (0..1), so a lit furnace carries
    bool plantBlend = false;    // blending from a stand into a walk keeps the standing foot planted
    float pace = 1;             // rate of every move that travels (0.2..2), 1 = stock
    float bodySink = 0;         // metres the dead body is set into the ground (a fall clip that leaves it hanging)
    bool lodPin = false;        // the drawn LOD follows distance alone, not the frame load
};

enum Gait
{
    GaitStop,
    GaitWalk,
    GaitSlow,
    GaitFast
};

struct SteerOrder
{
    float headChange = 0; // radians, + = turn right
    float speed = 0;      // > 0.1 = move forward
    Gait gait = GaitStop;
};

// Number of units carrying any trait, any far draw, any steer order, and
// the widest grid radius asked for. Plain globals so the guards inline.
extern int GTraitCount;
extern int GFarCount;
extern int GSteerCount;
extern float GGridRadius;

const Traits* FindTraits(const Object* obj);
inline const Traits* TraitsOf(const Object* obj)
{
    return GTraitCount ? FindTraits(obj) : nullptr;
}

// Reads the optional `class Kaiju` block of a unit's type (Man constructor).
void ApplyTypeConfig(Object* obj, const ParamEntry& typeCfg);

// ---- far draw (KaijuFarDraw.cpp)
// Classifies every far-drawn unit for this frame; call once per frame before
// objects are submitted for drawing.
void FarBeginFrame(Scene& scene);
// True when the far pass draws the object this frame, so the normal object
// pass must not.
bool FarOwnsDraw(const Object* obj);
// True when the object is exempt from the objectsZ cut in the normal pass.
bool FarExemptFromObjectsZ(const Object* obj);
// Fog visibility floor for the object in the normal pass (0 = none).
float FarFogFloorFor(const Object* obj);
// Reserves the far depth slice for the frame (before the sky).
void FarBeginScene(Scene& scene);
// Draws the far objects; call after the horizon, before the object passes.
void FarDrawPass(Scene& scene);
// Gives the full depth range back (after the 3D scene).
void FarEndScene();
// True when the unit must keep animating although it is past objectsZ.
bool FarKeepsSimulating(const Object* obj, float dist2);

// Notes the LOD chosen for a unit this frame (read back by kjFarState).
void NoteDrawLevel(const Object* obj, int level);

// ---- camera shake
// Adds the current tremor offset to the camera transform.
void ApplyCameraShake(Matrix4& camTransform);

// ---- steer
bool GetSteer(const Man* man, SteerOrder& out);
// Clip rate a steered unit asked for (kjSteer's optional rate; 1 = the clip's
// own speed). False when the unit is not steered.
bool GetSteerRate(const Man* man, float& rate);

// ---- glow
// Emissive scale (0..1) and white-hot share (0..1) of the unit's shining faces.
// False when the unit has no glow trait (stock material).
bool GetGlow(const Object* obj, float& scale, float& heat);
// The material indices a shining face can carry: MSShining (200, from the
// face flag or the point flag alone) and 220, which is what the model loader
// makes of a face flagged shining whose points are flagged shining too
// (ShapeSetup.cpp, "shining is offset +20"). The stock material table has no
// entry for 220 and lights it like any face; a unit with the glow trait
// treats both as shining.
inline bool IsShiningMaterial(int index)
{
    return index == 200 || index == 220;
}

// ---- damage, footfalls
// Called at the end of Object::LocalDammage, before HitBy. May lower the
// total damage back under the unit's ceiling. `delta` is the total damage the
// hit added before any clamp.
void OnLocalDammage(Object* obj, EntityAI* owner, const char* ammo, Vector3Par modelPos, float delta);
void OnFootfall(Man* man, bool left);

// ---- service lifetime
void Simulate(float deltaT);
void InitMission();
// Drops every record (and with it every light and emitter bound to a unit)
// while the world still exists. Called from World::~World: the static record
// list would otherwise be destroyed by the C runtime at exit, after the world
// its lights detach from is gone.
void Shutdown();

} // namespace Kaiju
} // namespace Poseidon
