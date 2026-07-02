// Script command surface for the Guerrilla ZoneRegistry.  Registered from
// its own INIT_MODULE at stage 3 so GGameState.Init() (GameStateExt, stage 2)
// has already run.

#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>

#include <Poseidon/Game/Guerrilla/AlertMachine.hpp>

#include <Poseidon/Game/Commands/GameStateExt.hpp>
#include <Poseidon/Game/Commands/GameStateExtCommon.hpp>
#include <Poseidon/Foundation/Common/FltOpts.hpp> // toInt
#include <Poseidon/Foundation/Modules/Modules.hpp>
#include <Poseidon/Foundation/platform.hpp>

using namespace Poseidon;
using namespace Poseidon::Guerrilla;

namespace Poseidon::Guerrilla
{
// Referenced from ZoneRegistry.cpp to keep this TU (only content besides
// this: file-static commands + module registration) in the link.
void EnsureZoneRegistryCommandsLinked() {}
} // namespace Poseidon::Guerrilla

static GameValue GmZoneCount(const GameState* /*state*/)
{
    return (float)ZoneRegistry::Instance().NZones();
}

// gmZone <index> -> 9-field tuple in GM_Z_* order; empty array out of range
static GameValue GmZone(const GameState* state, GameValuePar oper1)
{
    GameValue value = state->CreateGameValue(GameArray);
    GameArrayType& array = value;

    int index = toInt((float)oper1);
    const ZoneRecord* z = ZoneRegistry::Instance().GetZone(index);
    if (!z)
    {
        return value;
    }

    array.Resize(9);
    array[0] = GameStringType(z->name);
    array[1] = GameStringType(z->type);
    array[2] = GameStringType(z->owner);
    array[3] = z->garrison;
    array[4] = z->support;
    array[5] = z->income;
    array[6] = z->heat;
    array[7] = GameStringType(z->marker);

    // position in getPos order [easting, northing, elevation]
    GameValue posValue = state->CreateGameValue(GameArray);
    GameArrayType& pos = posValue;
    pos.Resize(3);
    pos[0] = z->pos.X();
    pos[1] = z->pos.Z();
    pos[2] = z->pos.Y();
    array[8] = posValue;
    return value;
}

static GameValue GmZoneIndex(const GameState* /*state*/, GameValuePar oper1)
{
    GameStringType name = oper1;
    return (float)ZoneRegistry::Instance().FindZoneIndex(name);
}

// gmZoneSet [index, "field", value]; unknown field -> no-op
static GameValue GmZoneSet(const GameState* state, GameValuePar oper1)
{
    const GameArrayType& array = oper1;
    if (!CheckSize(state, array, 3))
    {
        return NOTHING;
    }
    if (!CheckType(state, array[0], GameScalar))
    {
        return NOTHING;
    }
    if (!CheckType(state, array[1], GameString))
    {
        return NOTHING;
    }

    ZoneRegistry& registry = ZoneRegistry::Instance();
    int index = toInt((float)array[0]);
    GameStringType field = array[1];
    ZoneRecord* z = registry.GetZoneMutable(index);
    if (!z)
    {
        return NOTHING;
    }

    if (stricmp(field, "owner") == 0)
    {
        z->owner = GameStringType(array[2]);
    }
    else if (stricmp(field, "garrison") == 0)
    {
        z->garrison = (float)array[2];
    }
    else if (stricmp(field, "support") == 0)
    {
        z->support = (float)array[2];
    }
    else if (stricmp(field, "income") == 0)
    {
        z->income = (float)array[2];
    }
    else if (stricmp(field, "liveOccupiers") == 0)
    {
        z->liveOccupiers = (float)array[2];
    }
    else if (stricmp(field, "heatRaise") == 0)
    {
        registry.HeatRaise(index, (float)array[2]);
    }
    else if (stricmp(field, "heatDecay") == 0)
    {
        registry.HeatDecay(index, (float)array[2]);
    }
    return NOTHING;
}

// gmZoneOnEvent ["captured"|"supportThreshold"|"revealed", handler]
// handler may be a STRING or a CODE value (GameDataCode's GetString returns
// the source, so the plain string coercion covers both)
static GameValue GmZoneOnEvent(const GameState* state, GameValuePar oper1)
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
    int type = ZoneRegistry::EventTypeFromName(eventName);
    if (type < 0)
    {
        return NOTHING;
    }
    GameStringType handler = array[1];
    ZoneRegistry::Instance().SetEventHandler((ZoneEventType)type, handler);
    return NOTHING;
}

// gmFactionTierClass [sideString, warLevel]
static GameValue GmFactionTierClass(const GameState* state, GameValuePar oper1)
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
    if (!CheckType(state, array[1], GameScalar))
    {
        return NOTHING;
    }
    GameStringType side = array[0];
    return GameStringType(ZoneRegistry::Instance().FactionTierClass(side, (float)array[1]));
}

// gmFactionValue [sideString, key]
static GameValue GmFactionValue(const GameState* state, GameValuePar oper1)
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
    if (!CheckType(state, array[1], GameString))
    {
        return NOTHING;
    }
    GameStringType side = array[0];
    GameStringType key = array[1];
    return GameStringType(ZoneRegistry::Instance().FactionValue(side, key));
}

// gmFactionVehicle [sideString, warLevel]
static GameValue GmFactionVehicle(const GameState* state, GameValuePar oper1)
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
    if (!CheckType(state, array[1], GameScalar))
    {
        return NOTHING;
    }
    GameStringType side = array[0];
    return GameStringType(ZoneRegistry::Instance().FactionVehicle(side, (float)array[1]));
}

// gmZoneAlert <zoneIndex> -> 0 GREEN | 1 YELLOW | 2 RED
// (GREEN when the registry is inactive or the index is out of range)
static GameValue GmZoneAlert(const GameState* /*state*/, GameValuePar oper1)
{
    int index = toInt((float)oper1);
    return (float)AlertMachine::Instance().GetZoneState(index);
}

// gmZoneLastKnown <zoneIndex> -> last-known player position in getPos order
// [x, z, h]; empty array when the zone never had a qualifying contact
static GameValue GmZoneLastKnown(const GameState* state, GameValuePar oper1)
{
    GameValue value = state->CreateGameValue(GameArray);
    GameArrayType& array = value;

    Vector3 pos;
    if (!AlertMachine::Instance().GetLastKnown(toInt((float)oper1), pos))
    {
        return value;
    }
    array.Resize(3);
    array[0] = pos.X();
    array[1] = pos.Z();
    array[2] = pos.Y();
    return value;
}

// gmAlertOnEvent ["alertChanged"|"undercoverBroken", handler]
// handler may be a STRING or a CODE value (same coercion as gmZoneOnEvent)
static GameValue GmAlertOnEvent(const GameState* state, GameValuePar oper1)
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
    int type = AlertMachine::EventTypeFromName(eventName);
    if (type < 0)
    {
        return NOTHING;
    }
    GameStringType handler = array[1];
    AlertMachine::Instance().SetEventHandler((AlertEventType)type, handler);
    return NOTHING;
}

// gmBreakUndercover <reasonString> - script-side trigger for the fired-EH
// half of the undercover break (the vehicle half is polled natively);
// consumed by the next alert tick, only effective while gmUndercover is true
static GameValue GmBreakUndercover(const GameState* /*state*/, GameValuePar oper1)
{
    GameStringType reason = oper1;
    AlertMachine::Instance().RequestBreak(reason);
    return NOTHING;
}

INIT_MODULE(GuerrillaZoneRegistry, 3)
{
    GGameState.NewNularOp(GameNular(GameScalar, "gmZoneCount", GmZoneCount));

    GGameState.NewFunction(GameFunction(GameArray, "gmZone", GmZone, GameScalar));
    GGameState.NewFunction(GameFunction(GameScalar, "gmZoneIndex", GmZoneIndex, GameString));
    GGameState.NewFunction(GameFunction(GameNothing, "gmZoneSet", GmZoneSet, GameArray));
    GGameState.NewFunction(GameFunction(GameNothing, "gmZoneOnEvent", GmZoneOnEvent, GameArray));
    GGameState.NewFunction(GameFunction(GameString, "gmFactionTierClass", GmFactionTierClass, GameArray));
    GGameState.NewFunction(GameFunction(GameString, "gmFactionValue", GmFactionValue, GameArray));
    GGameState.NewFunction(GameFunction(GameString, "gmFactionVehicle", GmFactionVehicle, GameArray));

    GGameState.NewFunction(GameFunction(GameScalar, "gmZoneAlert", GmZoneAlert, GameScalar));
    GGameState.NewFunction(GameFunction(GameArray, "gmZoneLastKnown", GmZoneLastKnown, GameScalar));
    GGameState.NewFunction(GameFunction(GameNothing, "gmAlertOnEvent", GmAlertOnEvent, GameArray));
    GGameState.NewFunction(GameFunction(GameNothing, "gmBreakUndercover", GmBreakUndercover, GameString));
}
