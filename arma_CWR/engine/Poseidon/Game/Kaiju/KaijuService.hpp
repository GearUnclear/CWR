#pragma once

// Internal state of the giant-creature support, shared by the service, the
// far draw pass and the script commands. Engine files include Kaiju.hpp, not
// this header.

#include <Poseidon/Game/Kaiju/Kaiju.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>
#include <Poseidon/Foundation/Types/LLinks.hpp>
#include <Poseidon/Graphics/Rendering/Lighting/Lights.hpp>
#include <Poseidon/World/Entities/Vehicles/Vehicle.hpp>
#include <Evaluator/express.hpp>
#include <memory>
#include <vector>

namespace Poseidon::Kaiju
{

enum FarMode
{
    FarOff,     // no far draw asked for
    FarNormal,  // inside the camera far plane: normal object pass, exempt from objectsZ, fog floor
    FarOwnPass, // reaches past the far plane: drawn by the far pass
    FarHidden   // beyond the far limit or off screen: not drawn
};

struct HitRecord
{
    OLink<Object> attacker;
    RString ammo;
    Vector3 modelPos;
    Vector3 worldPos;
    float delta = 0;
    float time = 0;
};

// A point light that rides on a unit. LightPointOnVehicle wants a flare shape
// and a config class; this one is only the light.
#pragma warning(disable : 4250)
class KaijuLight : public LightPoint, public AttachedOnVehicle
{
  public:
    KaijuLight(Object* vehicle, Vector3Par pos);
};
#pragma warning(default : 4250)

struct LightSlot
{
    RString name;
    Ref<KaijuLight> light;
    int pointIndex = -1; // animated memory point, or -1 for a fixed model offset
    Vector3 offset;
};

struct EmitterSlot
{
    RString name;
    GameValue drop; // a 19 element `drop` array, object last
    float rate = 0; // particles per second
    float carry = 0;
};

struct Unit
{
    OLink<Object> obj;
    Traits traits;

    // steer
    bool steer = false;
    bool steerToPos = false;
    Vector3 steerPos;
    float steerHeading = 0;
    Gait gait = GaitStop;
    float arriveRadius = 0;
    bool arrived = false;
    float steerRate = 1; // clip rate while steered, 1 = the clip's own speed

    // far draw, refreshed by FarBeginFrame
    FarMode farMode = FarOff;
    float farDist = 0;
    float farPlanarZ = 0;
    float farFloor = 0;
    int farFramesNormal = 0;
    int farFramesOwnPass = 0;
    int lastLevel = -1; // the LOD last chosen for drawing (normal pass or far pass)

    std::vector<HitRecord> hits;
    int hitsDropped = 0;

    int stepCount = 0;
    bool stepLeft = false;
    Vector3 stepPos;

    std::vector<LightSlot> lights;
    std::vector<EmitterSlot> emitters;
};

struct Tremor
{
    Vector3 pos;
    float strength = 0;
    float fullRadius = 0;
    float zeroRadius = 0;
    float start = 0;
    float duration = 0;
    float freq = 7.0f;
};

extern int GFarDebug; // KaijuFarDraw.cpp, development switch

std::vector<std::unique_ptr<Unit>>& Units();
Unit* FindUnit(const Object* obj);
Unit& EnsureUnit(Object* obj);
void RemoveUnit(const Object* obj);
// Recomputes the inline guard globals after any change.
void Recount();

bool SetTrait(Object* obj, const char* name, float value);
bool GetTrait(const Object* obj, const char* name, float& value);

void AddTremor(Vector3Par pos, float strength, float fullRadius, float zeroRadius, float duration, float freq);
// Total tremor gain felt at a position now, and the rate of the strongest.
float ShakeGainAt(Vector3Par pos, float* freq = nullptr);
int TremorCount();

bool SetLight(Object* obj, const char* slot, const char* point, Vector3Par offset, ColorVal color, float brightness);
int LightCount(const Object* obj);
bool SetEmitter(Object* obj, const char* slot, const GameValue& drop, float rate);

bool SetSteerPos(Object* obj, Vector3Par pos, Gait gait, float arriveRadius, float rate = 1.0f);
bool SetSteerHeading(Object* obj, float heading, Gait gait, float rate = 1.0f);
void StopSteer(Object* obj);

bool Knockback(Object* obj, Vector3Par deltaV);

// World position of a memory point of the object in its current pose.
bool PointWorld(const Object* obj, const char* point, Vector3& out);

bool Roar(Object* obj, const char* sound, float revealRadius, float fearRadius, float fearSeconds, int& revealed,
          int& fled);

} // namespace Poseidon::Kaiju
