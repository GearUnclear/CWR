// Script command surface for the Guerrilla headquarters (GuerrillaBase).
// Registered from its own INIT_MODULE at stage 3 so GGameState.Init()
// (GameStateExt, stage 2) has already run - same pattern as
// StashRegistryCommands.cpp.
//
//   gmHqCanEstablish [x,y,z]   -> bool  (a zone's area contains the position)
//   gmHqEstablish [x,y,z]      -> bool  (establish / move the HQ there)
//   gmHqEstablished            -> bool
//   gmHqZone                   -> zone name, "" while unestablished
//   gmHqPos / gmHqGaragePos / gmHqCachePos -> [x,y,z] (getPos order), [] none
//   gmHqBuilding / gmHqCache   -> object (objNull while unestablished / gone)
//   gmHqMoveCount              -> scalar
//   gmHqIndoors                -> bool
//   gmHqValue "<key>"          -> scalar tuning (hqMinPos | garageRadius |
//                                  garageInvulnerable), 0 unknown
//   gmGarageLock [veh, bool]   -> bool  (lock / unlock; beep-beep on lock)
//   gmGarageCount              -> scalar
//   gmGarageVehicle <i>        -> object
//   gmGarageHas <veh>          -> bool
//   gmGarageInRange [x,y,z]    -> bool

#include <Poseidon/Game/Guerrilla/GuerrillaBase.hpp>

#include <Poseidon/AI/EntityAI.hpp>
#include <Poseidon/World/Terrain/Landscape.hpp> // GLOB_LAND (height above ground)

#include <Poseidon/Game/Commands/GameStateExt.hpp>
#include <Poseidon/Game/Commands/GameStateExtCommon.hpp>
#include <Poseidon/Foundation/Common/FltOpts.hpp> // toInt
#include <Poseidon/Foundation/Modules/Modules.hpp>
#include <Poseidon/Foundation/platform.hpp>

using namespace Poseidon;
using namespace Poseidon::Guerrilla;

namespace Poseidon::Guerrilla
{
// Referenced from GuerrillaBase.cpp to keep this TU (only content besides
// this: file-static commands + module registration) in the link.
void EnsureGuerrillaBaseCommandsLinked() {}
} // namespace Poseidon::Guerrilla

// position in getPos order [easting, northing, height above ground] - the
// getPos/setPos convention (an absolute Y here would put a createVehicle at
// the garage spot 35 m in the air; TrafficCommands' MakePosValue idiom);
// [] for "none"
static GameValue PosValue(const GameState* state, Vector3Par pos, bool present)
{
    GameValue value = state->CreateGameValue(GameArray);
    GameArrayType& array = value;
    if (!present)
    {
        return value;
    }
    float height = pos.Y();
    if (GLandscape)
    {
        float dx, dz;
        height -= GLOB_LAND->SurfaceYAboveWater(pos.X(), pos.Z(), &dx, &dz);
    }
    array.Resize(3);
    array[0] = pos.X();
    array[1] = pos.Z();
    array[2] = height;
    return value;
}

// gmHqCanEstablish [x,y,z] -> bool
static GameValue GmHqCanEstablish(const GameState* state, GameValuePar oper1)
{
    Vector3 pos;
    if (!GetPos(state, pos, oper1))
    {
        return false;
    }
    return GuerrillaBase::Instance().CanEstablish(pos);
}

// gmHqEstablish [x,y,z] -> bool
static GameValue GmHqEstablish(const GameState* state, GameValuePar oper1)
{
    Vector3 pos;
    if (!GetPos(state, pos, oper1))
    {
        return false;
    }
    return GuerrillaBase::Instance().Establish(pos);
}

// gmHqEstablished -> bool
static GameValue GmHqEstablished(const GameState* /*state*/)
{
    return GuerrillaBase::Instance().IsEstablished();
}

// gmHqZone -> string
static GameValue GmHqZone(const GameState* /*state*/)
{
    return GameStringType(GuerrillaBase::Instance().ZoneName());
}

// gmHqPos -> [x,y,z] or []
static GameValue GmHqPos(const GameState* state)
{
    const GuerrillaBase& base = GuerrillaBase::Instance();
    return PosValue(state, base.HqPos(), base.IsEstablished());
}

// gmHqGaragePos -> [x,y,z] or []
static GameValue GmHqGaragePos(const GameState* state)
{
    const GuerrillaBase& base = GuerrillaBase::Instance();
    return PosValue(state, base.GaragePos(), base.IsEstablished());
}

// gmHqCachePos -> [x,y,z] or []
static GameValue GmHqCachePos(const GameState* state)
{
    const GuerrillaBase& base = GuerrillaBase::Instance();
    return PosValue(state, base.CachePos(), base.IsEstablished());
}

// gmHqBuilding -> object
static GameValue GmHqBuilding(const GameState* /*state*/)
{
    return GameValueExt((Object*)GuerrillaBase::Instance().Building());
}

// gmHqCache -> object
static GameValue GmHqCache(const GameState* /*state*/)
{
    return GameValueExt((Object*)GuerrillaBase::Instance().Cache());
}

// gmHqMoveCount -> scalar
static GameValue GmHqMoveCount(const GameState* /*state*/)
{
    return (float)GuerrillaBase::Instance().MoveCount();
}

// gmHqIndoors -> bool
static GameValue GmHqIndoors(const GameState* /*state*/)
{
    return GuerrillaBase::Instance().IsIndoors();
}

// gmHqValue "<key>" -> scalar
static GameValue GmHqValue(const GameState* /*state*/, GameValuePar oper1)
{
    GameStringType key = oper1;
    const BaseTuning& t = GuerrillaBase::Instance().Tuning();
    if (stricmp(key, "hqMinPos") == 0)
    {
        return (float)t.hqMinPos;
    }
    if (stricmp(key, "garageRadius") == 0)
    {
        return t.garageRadius;
    }
    if (stricmp(key, "garageInvulnerable") == 0)
    {
        return t.garageInvulnerable ? 1.0f : 0.0f;
    }
    return 0.0f;
}

// gmGarageLock [veh, bool] -> bool
static GameValue GmGarageLock(const GameState* state, GameValuePar oper1)
{
    const GameArrayType& array = oper1;
    if (!CheckSize(state, array, 2))
    {
        return false;
    }
    if (!CheckType(state, array[0], GameObject) || !CheckType(state, array[1], GameBool))
    {
        return false;
    }
    Object* obj = GetObject(array[0]);
    EntityAI* veh = dyn_cast<EntityAI>(obj);
    if (!veh)
    {
        return false;
    }
    bool lock = array[1];
    return GuerrillaBase::Instance().GarageLock(veh, lock);
}

// gmGarageCount -> scalar
static GameValue GmGarageCount(const GameState* /*state*/)
{
    return (float)GuerrillaBase::Instance().GarageCount();
}

// gmGarageVehicle <i> -> object
static GameValue GmGarageVehicle(const GameState* /*state*/, GameValuePar oper1)
{
    int index = toInt((float)oper1);
    return GameValueExt((Object*)GuerrillaBase::Instance().GarageVehicle(index));
}

// gmGarageHas <veh> -> bool
static GameValue GmGarageHas(const GameState* /*state*/, GameValuePar oper1)
{
    Object* obj = GetObject(oper1);
    EntityAI* veh = dyn_cast<EntityAI>(obj);
    return GuerrillaBase::Instance().GarageHas(veh);
}

// gmGarageInRange [x,y,z] -> bool
static GameValue GmGarageInRange(const GameState* state, GameValuePar oper1)
{
    Vector3 pos;
    if (!GetPos(state, pos, oper1))
    {
        return false;
    }
    return GuerrillaBase::Instance().InGarageRange(pos);
}

INIT_MODULE(GuerrillaBase, 3)
{
    GGameState.NewFunction(GameFunction(GameBool, "gmHqCanEstablish", GmHqCanEstablish, GameArray));
    GGameState.NewFunction(GameFunction(GameBool, "gmHqEstablish", GmHqEstablish, GameArray));
    GGameState.NewNularOp(GameNular(GameBool, "gmHqEstablished", GmHqEstablished));
    GGameState.NewNularOp(GameNular(GameString, "gmHqZone", GmHqZone));
    GGameState.NewNularOp(GameNular(GameArray, "gmHqPos", GmHqPos));
    GGameState.NewNularOp(GameNular(GameArray, "gmHqGaragePos", GmHqGaragePos));
    GGameState.NewNularOp(GameNular(GameArray, "gmHqCachePos", GmHqCachePos));
    GGameState.NewNularOp(GameNular(GameObject, "gmHqBuilding", GmHqBuilding));
    GGameState.NewNularOp(GameNular(GameObject, "gmHqCache", GmHqCache));
    GGameState.NewNularOp(GameNular(GameScalar, "gmHqMoveCount", GmHqMoveCount));
    GGameState.NewNularOp(GameNular(GameBool, "gmHqIndoors", GmHqIndoors));
    GGameState.NewFunction(GameFunction(GameScalar, "gmHqValue", GmHqValue, GameString));
    GGameState.NewFunction(GameFunction(GameBool, "gmGarageLock", GmGarageLock, GameArray));
    GGameState.NewNularOp(GameNular(GameScalar, "gmGarageCount", GmGarageCount));
    GGameState.NewFunction(GameFunction(GameObject, "gmGarageVehicle", GmGarageVehicle, GameScalar));
    GGameState.NewFunction(GameFunction(GameBool, "gmGarageHas", GmGarageHas, GameObject));
    GGameState.NewFunction(GameFunction(GameBool, "gmGarageInRange", GmGarageInRange, GameArray));
}
