// Script command surface for the Guerrilla Traffic service plus the road
// network queries the evaluator never had (gmRoad* / nearestRoads, issue
// #33).  Registered from its own INIT_MODULE at stage 3 so GGameState.Init()
// (GameStateExt, stage 2) has already run - same pattern as
// GarrisonCacheCommands.cpp.

#include <Poseidon/Game/Guerrilla/Traffic.hpp>

#include <Poseidon/Game/Commands/GameStateExt.hpp>
#include <Poseidon/Game/Commands/GameStateExtCommon.hpp>
#include <Poseidon/World/World.hpp>
#include <Poseidon/World/Terrain/Landscape.hpp> // InvLandGrid / InRange
#include <Poseidon/World/Terrain/Roads.hpp>     // GRoadNet
#include <Poseidon/World/Entities/Vehicles/Transport.hpp>
#include <Poseidon/Foundation/Common/FltOpts.hpp> // toInt
#include <Poseidon/Foundation/Modules/Modules.hpp>
#include <Poseidon/Foundation/platform.hpp>

using namespace Poseidon;
using namespace Poseidon::Guerrilla;

namespace Poseidon::Guerrilla
{
// Referenced from Traffic.cpp to keep this TU (only content besides this:
// file-static commands + module registration) in the link.
void EnsureTrafficCommandsLinked() {}
} // namespace Poseidon::Guerrilla

// position in getPos order [easting, northing, height above ground] - the
// getPos/setPos convention, so a road point feeds straight into setPos
static GameValue MakePosValue(const GameState* state, Vector3Par pos)
{
    GameValue value = state->CreateGameValue(GameArray);
    GameArrayType& array = value;
    array.Resize(3);
    float height = pos.Y();
    if (GLandscape)
    {
        float dx, dz;
        height -= GLOB_LAND->SurfaceYAboveWater(pos.X(), pos.Z(), &dx, &dz);
    }
    array[0] = pos.X();
    array[1] = pos.Z();
    array[2] = height;
    return value;
}

// ---------------------------------------------------------------------------
// gmTraffic*
// ---------------------------------------------------------------------------

// gmTrafficCount "civ"|"patrol"|"convoy"|"all" -> scalar
static GameValue GmTrafficCount(const GameState* /*state*/, GameValuePar oper1)
{
    GameStringType kind = oper1;
    return (float)Traffic::Instance().Count(Traffic::KindFromName(kind));
}

// gmTrafficVehicles -> array of live traffic OBJECTs (dead links skipped)
static GameValue GmTrafficVehicles(const GameState* state)
{
    GameValue value = state->CreateGameValue(GameArray);
    GameArrayType& array = value;
    Traffic& traffic = Traffic::Instance();
    for (int i = 0; i < traffic.NEntries(); i++)
    {
        Transport* veh = traffic.EntryVehicle(i);
        if (veh)
        {
            array.Add(GameValueExt(veh));
        }
    }
    return value;
}

// gmTrafficInfo <veh> -> [kind, originIdx, destIdx, state]; [] when untracked
static GameValue GmTrafficInfo(const GameState* state, GameValuePar oper1)
{
    GameValue value = state->CreateGameValue(GameArray);
    GameArrayType& array = value;
    Transport* veh = dyn_cast<Transport>(GetObject(oper1));
    TrafficKind kind;
    int origin, dest;
    TrafficState st;
    if (!veh || !Traffic::Instance().FindEntry(veh, kind, origin, dest, st))
    {
        return value;
    }
    array.Resize(4);
    array[0] = GameStringType(Traffic::KindName(kind));
    array[1] = (float)origin;
    array[2] = (float)dest;
    array[3] = (float)st;
    return value;
}

// gmTrafficOnEvent ["spawned"|"despawned"|"commandeered"|"arrived"|"driverKilled", handler]
// handler may be a STRING or a CODE value (GameDataCode's GetString returns
// the source, so the plain string coercion covers both).  driverKilled is
// the killed-EH EXPRESSION attached to every civilian driver at spawn.
static GameValue GmTrafficOnEvent(const GameState* state, GameValuePar oper1)
{
    const GameArrayType& array = oper1;
    if (!CheckSize(state, array, 2))
    {
        return NOTHING;
    }
    if (!CheckType(state, array[0], GameString))
    {
        return NOTHING;
    }
    GameStringType eventName = array[0];
    int type = Traffic::EventTypeFromName(eventName);
    if (type < 0)
    {
        return NOTHING;
    }
    GameStringType handler = array[1];
    Traffic::Instance().SetEventHandler((TrafficEventType)type, handler);
    return NOTHING;
}

// gmTrafficRelease <veh> -> bool: registry half of a commandeer (the entry
// leaves the live table; no commands are issued to the crew)
static GameValue GmTrafficRelease(const GameState* /*state*/, GameValuePar oper1)
{
    Transport* veh = dyn_cast<Transport>(GetObject(oper1));
    return Traffic::Instance().Release(veh);
}

// gmTrafficForceSpawn [kind, zoneIdx] -> OBJECT (objNull on failure); test
// aid: bypasses the chance roll and the caps, not the road placement
static GameValue GmTrafficForceSpawn(const GameState* state, GameValuePar oper1)
{
    const GameArrayType& array = oper1;
    if (!CheckSize(state, array, 2))
    {
        return OBJECT_NULL;
    }
    if (!CheckType(state, array[0], GameString))
    {
        return OBJECT_NULL;
    }
    if (!CheckType(state, array[1], GameScalar))
    {
        return OBJECT_NULL;
    }
    GameStringType kindName = array[0];
    int kind = Traffic::KindFromName(kindName);
    int zone = toInt((float)array[1]);
    Transport* veh = Traffic::Instance().ForceSpawn(kind, zone);
    if (!veh)
    {
        return OBJECT_NULL;
    }
    return GameValueExt(veh);
}

// ---------------------------------------------------------------------------
// gmRoad* / nearestRoads (RoadNet bindings)
// ---------------------------------------------------------------------------

// gmRoadNearest <pos> -> pos on the nearest road within 20 m, [] when none
static GameValue GmRoadNearest(const GameState* state, GameValuePar oper1)
{
    GameValue empty = state->CreateGameValue(GameArray);
    Vector3 pos;
    if (!GetPos(state, pos, oper1) || !GRoadNet)
    {
        return empty;
    }
    Vector3 road = GRoadNet->GetNearestRoadPoint(pos);
    // GetNearestRoadPoint hands the input back unchanged when no road is in
    // reach; distinguish that from a point that happens to sit on a road
    if (road.Distance2(pos) < 1e-6f && !GRoadNet->IsOnRoad(pos, 1.0f))
    {
        return empty;
    }
    return MakePosValue(state, road);
}

// gmRoadPath [from, to] -> array of positions along the road net (the
// engine's short-hop A*; empty when no path is found within its budget)
static GameValue GmRoadPath(const GameState* state, GameValuePar oper1)
{
    GameValue value = state->CreateGameValue(GameArray);
    GameArrayType& array = value;
    const GameArrayType& args = oper1;
    if (!CheckSize(state, args, 2) || !GRoadNet)
    {
        return value;
    }
    Vector3 from, to;
    if (!GetPos(state, from, args[0]) || !GetPos(state, to, args[1]))
    {
        return value;
    }
    RoadPathArray path;
    if (!GRoadNet->SearchPath(from, to, path))
    {
        return value;
    }
    for (int i = 0; i < path.Size(); i++)
    {
        array.Add(MakePosValue(state, path[i]));
    }
    return value;
}

// gmRoadsNear [pos, radius] -> array of road-segment centre positions within
// radius (unlocked segments only); nearestRoads is its binary alias:
// <pos> nearestRoads <radius>
static GameValue RoadsNear(const GameState* state, Vector3Par pos, float radius)
{
    GameValue value = state->CreateGameValue(GameArray);
    GameArrayType& array = value;
    if (!GRoadNet || !GLandscape || radius <= 0)
    {
        return value;
    }
    const float r2 = radius * radius;
    int cells = toIntCeil(radius * InvLandGrid);
    int cx = toIntFloor(pos.X() * InvLandGrid);
    int cz = toIntFloor(pos.Z() * InvLandGrid);
    for (int zz = cz - cells; zz <= cz + cells; zz++)
    {
        for (int xx = cx - cells; xx <= cx + cells; xx++)
        {
            if (!InRange(zz, xx))
            {
                continue;
            }
            const RoadList& list = GRoadNet->GetRoadList(xx, zz);
            for (int i = 0; i < list.Size(); i++)
            {
                const RoadLink* link = list[i];
                if (!link || link->IsLocked())
                {
                    continue;
                }
                Vector3 c = link->GetCenter();
                float dx = c.X() - pos.X();
                float dz = c.Z() - pos.Z();
                if (dx * dx + dz * dz > r2)
                {
                    continue;
                }
                array.Add(MakePosValue(state, c));
            }
        }
    }
    return value;
}

static GameValue GmRoadsNear(const GameState* state, GameValuePar oper1)
{
    const GameArrayType& args = oper1;
    if (!CheckSize(state, args, 2))
    {
        return state->CreateGameValue(GameArray);
    }
    Vector3 pos;
    if (!GetPos(state, pos, args[0]) || !CheckType(state, args[1], GameScalar))
    {
        return state->CreateGameValue(GameArray);
    }
    return RoadsNear(state, pos, (float)args[1]);
}

static GameValue NearestRoads(const GameState* state, GameValuePar oper1, GameValuePar oper2)
{
    Vector3 pos;
    if (!GetPos(state, pos, oper1))
    {
        return state->CreateGameValue(GameArray);
    }
    return RoadsNear(state, pos, (float)oper2);
}

INIT_MODULE(GuerrillaTraffic, 3)
{
    GGameState.NewFunction(GameFunction(GameScalar, "gmTrafficCount", GmTrafficCount, GameString));
    GGameState.NewNularOp(GameNular(GameArray, "gmTrafficVehicles", GmTrafficVehicles));
    GGameState.NewFunction(GameFunction(GameArray, "gmTrafficInfo", GmTrafficInfo, GameObject));
    GGameState.NewFunction(GameFunction(GameNothing, "gmTrafficOnEvent", GmTrafficOnEvent, GameArray));
    GGameState.NewFunction(GameFunction(GameBool, "gmTrafficRelease", GmTrafficRelease, GameObject));
    GGameState.NewFunction(GameFunction(GameObject, "gmTrafficForceSpawn", GmTrafficForceSpawn, GameArray));

    GGameState.NewFunction(GameFunction(GameArray, "gmRoadNearest", GmRoadNearest, GameObjectOrArray));
    GGameState.NewFunction(GameFunction(GameArray, "gmRoadPath", GmRoadPath, GameArray));
    GGameState.NewFunction(GameFunction(GameArray, "gmRoadsNear", GmRoadsNear, GameArray));
    GGameState.NewOperator(
        GameOperator(GameArray, "nearestRoads", function, NearestRoads, GameObjectOrArray, GameScalar));
}
