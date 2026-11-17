#pragma once

#include <Poseidon/Graphics/Rendering/Effects/Smokes.hpp>

namespace Poseidon
{

// The script command `drop ParticleArray` (Smokes.cpp, ParticleDrop).
//
// ParticleArray, as OFP 1.99 (ColdWarAssault.exe) reads it:
//    0 ShapeName                 String
//    1 AnimationName             String
//    2 Type                      String, "Billboard" or "SpaceObject" (exact case)
//    3 TimerPeriod               Number
//    4 LifeTime                  Number
//    5 Position                  Array or String, see below
//    6 MoveVelocity              Array, exactly [x, y, z]
//    7 RotationVelocity          Number
//    8 Weight                    Number
//    9 Volume                    Number
//   10 Rubbing                   Number
//   11 Size                      Array of Numbers (keyframes over the lifetime)
//   12 Color                     Array of [r, g, b, a] Arrays (keyframes)
//   13 AnimationPhase            Array of Numbers (keyframes)
//   14 RandomDirectionPeriod     Number
//   15 RandomDirectionIntensity  Number
//   16 OnTimer                   String (script, "" for none)
//   17 BeforeDestroy             String (script, "" for none)
//   18 Object                    Object, or anything else for none
//
// 1.99 takes exactly 19 elements; the Remaster also takes 18 (no Object).
//
// Position with an Object: model space, an Array of exactly [x, y, z] or a
// memory point name (String); the velocity turns with the object too. A null
// Object drops nothing and is not an error.
//
// Position without an Object (18 elements, or element 18 any non-Object value, such as "" or a String like "_unit"
// that only names a variable): an Array [x, y] or [x, y, z] read the way the position commands read one (::GetPos): z
// is the height above the land or sea surface, 0 when left out. In 1.99 this branch calls ::GetPos (0x4cdbf8 in
// ColdWarAssault.exe, a two-to-three element check then SurfaceYAboveWater). The Remaster 3.05 source read it with
// GetVector3, exactly three elements taken as a raw world height, which rejected the two-element form with "Generic
// error in expression" and buried three-element drops at sea level on high ground. It is also the inverse of the [x, y,
// height above surface] array a particle's OnTimer script receives.

using ParticleDropSurfaceY = float (*)(float x, float z);

enum class ParticleDropParse
{
    Ready,    // `out` holds a particle to create
    Rejected, // an argument error, reported through the state when there is one
    NoTarget, // the Object is null: drop nothing, no error
};

struct ParticleDropRequest
{
    EParticleType type = PT_Billboard;
    Vector3 position;
    Vector3 moveVelocity;
    AutoArray<float> size;
    AutoArray<Color> color;
    AutoArray<float> animationPhase;
};

bool IsParticleDropArgumentCountSupported(int count);

// Position without an Object: [x, y] or [x, y, z above the surface] to a world
// position; surfaceY gives the land or sea surface height at world (x, z).
bool GetParticleDropWorldPosition(Vector3& ret, GameValuePar oper, ParticleDropSurfaceY surfaceY);

// Validates a ParticleArray and resolves its position, velocity and keyframes.
// state may be null (no error is reported then).
ParticleDropParse ParseParticleDrop(const GameState* state, const GameArrayType& args, ParticleDropRequest& out,
                                    ParticleDropSurfaceY surfaceY);

} // namespace Poseidon
