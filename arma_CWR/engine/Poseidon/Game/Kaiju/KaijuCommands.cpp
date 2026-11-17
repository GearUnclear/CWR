// Script commands of the giant-creature support. The contract other features
// rely on is experiments/kaiju/features/engine/API.md; keep the two in step.
//
// Every command takes one array (or one value) and answers false, 0 or an
// empty array on a malformed argument instead of raising a script error, so
// a caller's fallback path stays quiet.

#include <Poseidon/Game/Kaiju/KaijuService.hpp>
#include <Poseidon/Game/Kaiju/KaijuMath.hpp>
#include <Poseidon/Game/Commands/GameStateExt.hpp>
#include <Poseidon/Game/Commands/GameStateExtCommon.hpp>
#include <Poseidon/World/Entities/Infantry/SoldierOld.hpp>
#include <Poseidon/World/Scene/Camera/Camera.hpp>
#include <Poseidon/World/Scene/Scene.hpp>
#include <Poseidon/World/Terrain/Landscape.hpp>
#include <Poseidon/Foundation/Modules/Modules.hpp>
#include <Poseidon/Foundation/platform.hpp>

using namespace Poseidon;
using namespace Poseidon::Kaiju;

namespace Poseidon::Kaiju
{
void EnsureKaijuCommandsLinked() {}
} // namespace Poseidon::Kaiju

namespace
{
// Bumped whenever a command or trait is added, so scripts can ask for a
// feature level: 1 = far draw, shake, traits, light, emitter, steer, hits,
// knockback, footfalls; 2 = the traits glow, farGlow, plantBlend, pace and
// bodySink and lodPin, kjSteer's rate, kjGround, a destroyed church collapses.
constexpr float kApiVersion = 2.0f;

bool IsNum(GameValuePar v)
{
    return v.GetType() == GameScalar;
}
bool IsStr(GameValuePar v)
{
    return v.GetType() == GameString;
}
bool IsObj(GameValuePar v)
{
    return v.GetType() == GameObject;
}
bool IsArr(GameValuePar v)
{
    return v.GetType() == GameArray;
}

GameValue PosValue(const GameState* state, Vector3Par pos)
{
    GameValue value = state->CreateGameValue(GameArray);
    GameArrayType& array = value;
    array.Resize(3);
    array[0] = pos.X();
    array[1] = pos.Z();
    array[2] = pos.Y();
    return value;
}

Gait GaitFromName(const char* name)
{
    if (!stricmp(name, "walk"))
    {
        return GaitWalk;
    }
    if (!stricmp(name, "slow"))
    {
        return GaitSlow;
    }
    if (!stricmp(name, "fast"))
    {
        return GaitFast;
    }
    return GaitStop;
}

GameValue Version(const GameState*)
{
    return kApiVersion;
}

// kjSet [unit, "trait", value]
GameValue Set(const GameState*, GameValuePar oper)
{
    const GameArrayType& a = oper;
    if (a.Size() != 3 || !IsObj(a[0]) || !IsStr(a[1]) || !(IsNum(a[2]) || a[2].GetType() == GameBool))
    {
        return false;
    }
    float value = IsNum(a[2]) ? (float)a[2] : ((bool)a[2] ? 1.0f : 0.0f);
    return SetTrait(GetObject(a[0]), GameStringType(a[1]), value);
}

// kjGet [unit, "trait"]
GameValue Get(const GameState*, GameValuePar oper)
{
    const GameArrayType& a = oper;
    float value = 0;
    if (a.Size() != 2 || !IsObj(a[0]) || !IsStr(a[1]) || !GetTrait(GetObject(a[0]), GameStringType(a[1]), value))
    {
        return -1.0f;
    }
    return value;
}

// kjClear unit
GameValue Clear(const GameState*, GameValuePar oper)
{
    RemoveUnit(GetObject(oper));
    return true;
}

// kjShake [positionOrObject, strength, fullRadius, zeroRadius, duration, rate]
GameValue Shake(const GameState* state, GameValuePar oper)
{
    const GameArrayType& a = oper;
    if (a.Size() < 5 || a.Size() > 6)
    {
        return false;
    }
    for (int i = 1; i < a.Size(); i++)
    {
        if (!IsNum(a[i]))
        {
            return false;
        }
    }
    Vector3 pos;
    if (IsObj(a[0]))
    {
        Object* obj = GetObject(a[0]);
        if (!obj)
        {
            return false;
        }
        pos = obj->Position();
    }
    else if (!IsArr(a[0]) || !GetPos(state, pos, a[0]))
    {
        return false;
    }
    AddTremor(pos, a[1], a[2], a[3], a[4], a.Size() == 6 ? (float)a[5] : 7.0f);
    return true;
}

// kjShakeGain: the tremor gain the camera feels right now
GameValue ShakeGain(const GameState*)
{
    if (!GScene || !GScene->GetCamera())
    {
        return 0.0f;
    }
    return ShakeGainAt(GScene->GetCamera()->Position());
}

// kjLight [unit, "slot", "memoryPoint" or [x, y, z] model offset, [r, g, b], brightness]
GameValue LightCmd(const GameState*, GameValuePar oper)
{
    const GameArrayType& a = oper;
    if (a.Size() != 5 || !IsObj(a[0]) || !IsStr(a[1]) || !IsArr(a[3]) || !IsNum(a[4]))
    {
        return false;
    }
    const GameArrayType& c = a[3];
    if (c.Size() != 3 || !IsNum(c[0]) || !IsNum(c[1]) || !IsNum(c[2]))
    {
        return false;
    }
    RString point;
    Vector3 offset = VZero;
    if (IsStr(a[2]))
    {
        point = GameStringType(a[2]);
    }
    else if (IsArr(a[2]))
    {
        // model space, written the way scripts write positions: [right, forward, up]
        const GameArrayType& o = a[2];
        if (o.Size() != 3 || !IsNum(o[0]) || !IsNum(o[1]) || !IsNum(o[2]))
        {
            return false;
        }
        offset = Vector3(o[0], o[2], o[1]);
    }
    else
    {
        return false;
    }
    return SetLight(GetObject(a[0]), GameStringType(a[1]), point, offset, Color(c[0], c[1], c[2]), a[4]);
}

// kjEmitter [unit, "slot", dropArray, particlesPerSecond]
GameValue Emitter(const GameState*, GameValuePar oper)
{
    const GameArrayType& a = oper;
    if (a.Size() != 4 || !IsObj(a[0]) || !IsStr(a[1]) || !IsArr(a[2]) || !IsNum(a[3]))
    {
        return false;
    }
    const GameArrayType& drop = a[2];
    float rate = a[3];
    if (rate > 0 && drop.Size() != 18 && drop.Size() != 19)
    {
        return false;
    }
    return SetEmitter(GetObject(a[0]), GameStringType(a[1]), a[2], rate);
}

// kjSteer [unit, position or heading in degrees, "walk" | "slow" | "fast" | "stop", arriveRadius, rate]
// rate (optional, default 1): the rate the gait's clip plays at, 1 = its own speed
GameValue Steer(const GameState* state, GameValuePar oper)
{
    const GameArrayType& a = oper;
    if (a.Size() < 3 || a.Size() > 5 || !IsObj(a[0]) || !IsStr(a[2]))
    {
        return false;
    }
    float rate = a.Size() == 5 && IsNum(a[4]) ? (float)a[4] : 1.0f;
    if (rate <= 0)
    {
        rate = 1.0f;
    }
    Object* obj = GetObject(a[0]);
    if (!dyn_cast<Man>(obj))
    {
        return false;
    }
    Gait gait = GaitFromName(GameStringType(a[2]));
    if (IsNum(a[1]))
    {
        return SetSteerHeading(obj, (float)a[1] * (H_PI / 180.0f), gait, rate);
    }
    Vector3 pos;
    if (!GetPos(state, pos, a[1]))
    {
        return false;
    }
    float arrive = a.Size() >= 4 && IsNum(a[3]) ? (float)a[3] : 0.0f;
    if (arrive <= 0)
    {
        arrive = floatMax(obj->GetRadius() * 0.25f, 3.0f);
    }
    return SetSteerPos(obj, pos, gait, arrive, rate);
}

// kjSteerStop unit
GameValue SteerStop(const GameState*, GameValuePar oper)
{
    StopSteer(GetObject(oper));
    return true;
}

// kjSteerState unit -> [active, arrived, metres left]
GameValue SteerState(const GameState* state, GameValuePar oper)
{
    GameValue value = state->CreateGameValue(GameArray);
    GameArrayType& array = value;
    array.Resize(3);
    Object* obj = GetObject(oper);
    Unit* u = FindUnit(obj);
    bool active = u && u->steer;
    array[0] = active;
    array[1] = active && u->arrived;
    float left = -1;
    if (active && u->steerToPos)
    {
        float dx = u->steerPos.X() - obj->Position().X();
        float dz = u->steerPos.Z() - obj->Position().Z();
        left = sqrt(dx * dx + dz * dz);
    }
    array[2] = left;
    return value;
}

// kjHits unit -> [[attacker, "ammo", model position, damage, time, world position], ...], oldest first;
// empties the queue. Positions are [x, y, z]: model space is [right, forward, up], world z is above sea level.
GameValue Hits(const GameState* state, GameValuePar oper)
{
    GameValue value = state->CreateGameValue(GameArray);
    GameArrayType& array = value;
    Unit* u = FindUnit(GetObject(oper));
    if (!u)
    {
        return value;
    }
    for (const HitRecord& r : u->hits)
    {
        GameValue rec = state->CreateGameValue(GameArray);
        GameArrayType& ra = rec;
        ra.Resize(6);
        ra[0] = CreateGameObject(r.attacker.GetLink());
        ra[1] = r.ammo;
        ra[2] = PosValue(state, r.modelPos);
        ra[3] = r.delta;
        ra[4] = r.time;
        ra[5] = PosValue(state, r.worldPos);
        array.Add(rec);
    }
    u->hits.clear();
    return value;
}

// kjImpulse [object, [x, y, z] velocity change in m/s]
GameValue Impulse(const GameState*, GameValuePar oper)
{
    const GameArrayType& a = oper;
    Vector3 dv;
    if (a.Size() != 2 || !IsObj(a[0]) || !IsArr(a[1]) || !GetVector(dv, a[1]))
    {
        return false;
    }
    return Knockback(GetObject(a[0]), dv);
}

// kjDamage [victim, owner, position or object or 0, hit, range]
// Explosion-style damage through the normal damage path (armour, hit zones,
// events, kill credit to the owner) with no shot and no effects. A position
// of 0 means the victim's own centre.
GameValue Damage(const GameState* state, GameValuePar oper)
{
    const GameArrayType& a = oper;
    if (a.Size() != 5 || !IsObj(a[0]) || !IsNum(a[3]) || !IsNum(a[4]))
    {
        return false;
    }
    Object* victim = GetObject(a[0]);
    if (!victim || !victim->GetShape())
    {
        return false;
    }
    EntityAI* owner = IsObj(a[1]) ? dyn_cast<EntityAI>(GetObject(a[1])) : nullptr;
    Vector3 pos = victim->COMPosition();
    if (IsObj(a[2]))
    {
        Object* at = GetObject(a[2]);
        if (at)
        {
            pos = at->Position();
        }
    }
    else if (IsArr(a[2]) && !GetPos(state, pos, a[2]))
    {
        return false;
    }
    victim->IndirectDammage(nullptr, owner, pos, a[3], floatMax((float)a[4], 0.1f));
    return true;
}

// kjRoar [unit, "CfgSounds class" or "", revealRadius, fearRadius, fearSeconds] -> [groups revealed to, groups fleeing]
GameValue RoarCmd(const GameState* state, GameValuePar oper)
{
    GameValue value = state->CreateGameValue(GameArray);
    GameArrayType& array = value;
    array.Resize(2);
    array[0] = 0.0f;
    array[1] = 0.0f;
    const GameArrayType& a = oper;
    if (a.Size() < 4 || a.Size() > 5 || !IsObj(a[0]) || !IsStr(a[1]) || !IsNum(a[2]) || !IsNum(a[3]) ||
        (a.Size() == 5 && !IsNum(a[4])))
    {
        return value;
    }
    int revealed = 0, fled = 0;
    Roar(GetObject(a[0]), GameStringType(a[1]), a[2], a[3], a.Size() == 5 ? (float)a[4] : 45.0f, revealed, fled);
    array[0] = (float)revealed;
    array[1] = (float)fled;
    return value;
}

// kjPoint [object, "memoryPoint"] or kjPoint [object, [right, forward, up]]
// -> [x, y, z above sea level] in the current pose, or [] when there is no such point
GameValue Point(const GameState* state, GameValuePar oper)
{
    GameValue none = state->CreateGameValue(GameArray);
    const GameArrayType& a = oper;
    if (a.Size() != 2 || !IsObj(a[0]))
    {
        return none;
    }
    Object* obj = GetObject(a[0]);
    if (!obj)
    {
        return none;
    }
    Vector3 world;
    if (IsStr(a[1]))
    {
        if (!PointWorld(obj, GameStringType(a[1]), world))
        {
            return none;
        }
    }
    else if (IsArr(a[1]))
    {
        const GameArrayType& o = a[1];
        if (o.Size() != 3 || !IsNum(o[0]) || !IsNum(o[1]) || !IsNum(o[2]))
        {
            return none;
        }
        world = obj->PositionModelToWorld(Vector3(o[0], o[2], o[1]));
    }
    else
    {
        return none;
    }
    return PosValue(state, world);
}

// kjGround [x, y] -> [height of the ground above sea level, metres of water over it]
// The ground is the terrain itself, also under the sea (a script can only read
// the surface there: an object set on the sea floats). Version 2.
GameValue Ground(const GameState* state, GameValuePar oper)
{
    GameValue value = state->CreateGameValue(GameArray);
    GameArrayType& array = value;
    const GameArrayType& a = oper;
    if (a.Size() < 2 || !IsNum(a[0]) || !IsNum(a[1]) || !GLandscape)
    {
        return value;
    }
    const float ground = GLandscape->SurfaceY((float)a[0], (float)a[1]);
    const float water = GLandscape->GetSeaLevel() - ground;
    array.Resize(2);
    array[0] = ground;
    array[1] = water > 0 ? water : 0.0f;
    return value;
}

// kjSteps unit -> [count, last was left foot, position of the last footfall]
GameValue Steps(const GameState* state, GameValuePar oper)
{
    GameValue value = state->CreateGameValue(GameArray);
    GameArrayType& array = value;
    array.Resize(3);
    Unit* u = FindUnit(GetObject(oper));
    array[0] = u ? (float)u->stepCount : 0.0f;
    array[1] = u ? u->stepLeft : false;
    array[2] = PosValue(state, u ? u->stepPos : VZero);
    return value;
}

// kjFarState unit -> [mode, distance, fog floor, frames in normal pass, frames in far pass, LOD last chosen]
// mode: 0 off or past the limit, 1 normal pass, 2 far pass, 3 off screen
GameValue FarState(const GameState* state, GameValuePar oper)
{
    GameValue value = state->CreateGameValue(GameArray);
    GameArrayType& array = value;
    array.Resize(6);
    Unit* u = FindUnit(GetObject(oper));
    array[5] = u ? (float)u->lastLevel : -1.0f;
    array[0] = u ? (float)u->farMode : 0.0f;
    array[1] = u ? u->farDist : 0.0f;
    array[2] = u ? u->farFloor : 0.0f;
    array[3] = u ? (float)u->farFramesNormal : 0.0f;
    array[4] = u ? (float)u->farFramesOwnPass : 0.0f;
    return value;
}

// kjFarDebug n: development switch of the far pass, see KaijuFarDraw.cpp
GameValue FarDebug(const GameState*, GameValuePar oper)
{
    GFarDebug = toInt((float)oper);
    return true;
}

// kjLightCount unit
GameValue LightCountCmd(const GameState*, GameValuePar oper)
{
    return (float)LightCount(GetObject(oper));
}
} // namespace

INIT_MODULE(KaijuCommands, 3)
{
    GGameState.NewNularOp(GameNular(GameScalar, "kjVersion", Version));
    GGameState.NewNularOp(GameNular(GameScalar, "kjShakeGain", ShakeGain));
    GGameState.NewFunction(GameFunction(GameBool, "kjSet", Set, GameArray));
    GGameState.NewFunction(GameFunction(GameScalar, "kjGet", Get, GameArray));
    GGameState.NewFunction(GameFunction(GameBool, "kjClear", Clear, GameObject));
    GGameState.NewFunction(GameFunction(GameBool, "kjShake", Shake, GameArray));
    GGameState.NewFunction(GameFunction(GameBool, "kjLight", LightCmd, GameArray));
    GGameState.NewFunction(GameFunction(GameScalar, "kjLightCount", LightCountCmd, GameObject));
    GGameState.NewFunction(GameFunction(GameBool, "kjEmitter", Emitter, GameArray));
    GGameState.NewFunction(GameFunction(GameBool, "kjSteer", Steer, GameArray));
    GGameState.NewFunction(GameFunction(GameBool, "kjSteerStop", SteerStop, GameObject));
    GGameState.NewFunction(GameFunction(GameArray, "kjSteerState", SteerState, GameObject));
    GGameState.NewFunction(GameFunction(GameArray, "kjHits", Hits, GameObject));
    GGameState.NewFunction(GameFunction(GameBool, "kjImpulse", Impulse, GameArray));
    GGameState.NewFunction(GameFunction(GameBool, "kjDamage", Damage, GameArray));
    GGameState.NewFunction(GameFunction(GameArray, "kjRoar", RoarCmd, GameArray));
    GGameState.NewFunction(GameFunction(GameArray, "kjPoint", Point, GameArray));
    GGameState.NewFunction(GameFunction(GameArray, "kjSteps", Steps, GameObject));
    GGameState.NewFunction(GameFunction(GameArray, "kjGround", Ground, GameArray));
    GGameState.NewFunction(GameFunction(GameArray, "kjFarState", FarState, GameObject));
    GGameState.NewFunction(GameFunction(GameBool, "kjFarDebug", FarDebug, GameScalar));
}
