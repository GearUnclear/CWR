// Script command surface for the Guerrilla ZoneRegistry.  Registered from
// its own INIT_MODULE at stage 3 so GGameState.Init() (GameStateExt, stage 2)
// has already run.

#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>

#include <Poseidon/Game/Guerrilla/AlertMachine.hpp>

#include <Poseidon/Game/Commands/GameStateExt.hpp>
#include <Poseidon/Game/Commands/GameStateExtCommon.hpp>
#include <Poseidon/IO/ParamFileExt.hpp>           // Pars (gmClassExists probe)
#include <Poseidon/Foundation/Common/FltOpts.hpp> // toInt
#include <Poseidon/Foundation/Modules/Modules.hpp>
#include <Poseidon/Core/Global.hpp> // Glob.time (gmZoneStruck)
#include <Poseidon/World/World.hpp>                    // GWorld (gmPlayerPresence)
#include <Poseidon/World/Entities/Infantry/Person.hpp> // real player
#include <Poseidon/Foundation/platform.hpp>

#include <initializer_list>
#include <math.h> // floorf (WholeBodies)

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

// gmOccupierSide / gmResistanceSide -> the campaign's resolved faction sides
// (config/new-game selection, remembered by the save)
static GameValue GmOccupierSide(const GameState* /*state*/)
{
    return GameStringType(ZoneRegistry::Instance().OccupierSide());
}

static GameValue GmResistanceSide(const GameState* /*state*/)
{
    return GameStringType(ZoneRegistry::Instance().ResistanceSide());
}

// gmZone <index> -> 10-field tuple in GM_Z_* order; empty array out of range
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

    array.Resize(10);
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
    array[9] = z->capture;
    return value;
}

static GameValue GmZoneIndex(const GameState* /*state*/, GameValuePar oper1)
{
    GameStringType name = oper1;
    return (float)ZoneRegistry::Instance().FindZoneIndex(name);
}

// U3 (OCCUPIER-FORCES.md 5.5): a scripted garrison / garrisonMax is a count
// of whole soldiers, floored, never below 0
static float WholeBodies(float value)
{
    const float bodies = floorf(value);
    return bodies > 0 ? bodies : 0.0f;
}

// gmZoneSet [index, "field", value]; unknown field -> no-op.  garrison and
// garrisonMax are floored to whole bodies (at least 0); occCapture (the
// occupier's base-recapture meter) and standing (occupier standing) are
// clamped to 0..100.  An owner write resets both capture meters.
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
        // an ownership discontinuity ends any consolidation in progress -
        // without this a script-flipped zone keeps a stale meter forever
        // (frozen once ineligible) and hands it to a future re-occupier
        z->capture = 0;
        z->occCapture = 0;
        z->contestedLastTick = false;
    }
    else if (stricmp(field, "garrison") == 0)
    {
        z->garrison = WholeBodies((float)array[2]);
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
    else if (stricmp(field, "capture") == 0)
    {
        float capture = (float)array[2];
        if (capture < 0)
        {
            capture = 0;
        }
        if (capture > 100)
        {
            capture = 100;
        }
        z->capture = capture;
    }
    else if (stricmp(field, "garrisonMax") == 0)
    {
        z->garrisonMax = WholeBodies((float)array[2]);
    }
    else if (stricmp(field, "recoverClock") == 0)
    {
        z->recoverClock = (float)array[2];
    }
    else if (stricmp(field, "occCapture") == 0)
    {
        const float v = (float)array[2];
        z->occCapture = v < 0 ? 0.0f : (v > 100 ? 100.0f : v);
    }
    else if (stricmp(field, "standing") == 0)
    {
        const float v = (float)array[2];
        z->standing = v < 0 ? 0.0f : (v > 100 ? 100.0f : v);
    }
    else if (stricmp(field, "struckUntil") == 0)
    {
        // the town-strike stamp (mission time, OCCUPIER-FORCES.md 7.3): a test
        // lever for economy.sqs's half income
        const float v = (float)array[2];
        z->struckUntil = v < 0 ? 0.0f : v;
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

// gmZoneSim <index> -> the zone's off-screen contract state (issue #71):
// [inBubble, guer, occ, seenDay, seenMinute, contested, lastRemote,
//  garrisonMax, recoverClock, garrisonLive, garrisonExempt, flipDay,
//  flipMinute, pending] as of the last zone tick; empty array out of range.
// Slots 9..12 are the town-defender / retake fields (issue #79): the zone's
// own live garrison bodies wherever they stand, the start-town exemption, and
// the campaign day/minute of the last flip to the resistance (0 = never).
// guer (slot 1) is the resistance CAPTURE presence (the player per
// PlayerCapturesPresence).  Slot 13 is the S5 pending deposit
// (OCCUPIER-FORCES.md 3.3): whole bodies on their way into the reserve that
// neither defend nor materialize yet (live, not as of the tick).
// Occupier forces phase 4 (live values): slot 14 occCapture (the occupier's
// base-recapture meter on a resistance-held military zone, OCCUPIER-FORCES.md
// 7.2), 15 lostCount (times the resistance lost the zone back), 16 standing
// (occupier standing, 7.3), 17 struckUntil (the town-strike stamp in mission
// time, phase 6; 0 = never struck).
static GameValue GmZoneSim(const GameState* state, GameValuePar oper1)
{
    GameValue value = state->CreateGameValue(GameArray);
    GameArrayType& array = value;
    const ZoneRecord* z = ZoneRegistry::Instance().GetZone(toInt((float)oper1));
    if (!z)
    {
        return value;
    }
    array.Resize(18);
    array[0] = z->lastInBubble;
    array[1] = (float)z->lastGuer;
    array[2] = (float)z->lastOcc;
    array[3] = (float)z->seenDay;
    array[4] = (float)z->seenMinute;
    array[5] = z->contestedLastTick;
    array[6] = z->lastRemote;
    array[7] = z->garrisonMax;
    array[8] = z->recoverClock;
    array[9] = (float)z->lastGarrisonLive;
    array[10] = z->garrisonExempt;
    array[11] = (float)z->flipDay;
    array[12] = (float)z->flipMinute;
    array[13] = (float)z->pending;
    array[14] = z->occCapture;
    array[15] = (float)z->lostCount;
    array[16] = z->standing;
    array[17] = z->struckUntil;
    return value;
}

// gmZoneStruck <index> -> true while a CITY is inside its town-strike window
// (mission time < struckUntil, OCCUPIER-FORCES.md 7.3): economy.sqs pays it
// half income and no manpower.  False out of range.
static GameValue GmZoneStruck(const GameState* /*state*/, GameValuePar oper1)
{
    const float now = Glob.time - Poseidon::Foundation::Time(0);
    return ZoneRegistry::Instance().IsStruck(toInt((float)oper1), now);
}

// gmPlayerPresence -> the player-presence predicates (OCCUPIER-FORCES.md 8.1)
// sampled now: [counts, undercover, exposed, weaponInHands, warriorOutfit,
// liberatedTowns, holds, weaponSeen].  counts (slot 0) is CAPTURE
// (PlayerCapturesPresence, issue #86: not undercover, exposed or a weapon in
// hands); warriorOutfit (slot 4) is informational (it feeds HOLD only); holds
// (slot 6) is HOLD (PlayerHoldsPresence: a uniform always holds, D23; a
// disguised player only exposed, armed or with his weapon seen, D11);
// weaponSeen (slot 7) is UndercoverSystem::WeaponSeenCount() > 0.  The
// booleans read false without a live player.
static GameValue GmPlayerPresence(const GameState* state)
{
    GameValue value = state->CreateGameValue(GameArray);
    GameArrayType& array = value;
    array.Resize(8);
    PlayerPresenceInput in;
    Person* player = GWorld ? GWorld->GetRealPlayer() : nullptr;
    bool alive = player && !player->IsDammageDestroyed();
    if (alive)
    {
        ZoneRegistry::Instance().SamplePlayerPresence(player, in);
    }
    array[0] = alive && PlayerCapturesPresence(in);
    array[1] = in.undercover;
    array[2] = in.exposed;
    array[3] = in.weaponInHands;
    array[4] = in.warriorOutfit;
    array[5] = (float)ZoneRegistry::Instance().LiberatedTownCount();
    array[6] = alive && PlayerHoldsPresence(in);
    array[7] = in.weaponSeen;
    return value;
}

static GameValue ScalarArray(const GameState* state, std::initializer_list<float> values)
{
    GameValue value = state->CreateGameValue(GameArray);
    GameArrayType& array = value;
    array.Resize((int)values.size());
    int i = 0;
    for (float v : values)
    {
        array[i++] = v;
    }
    return value;
}

// gmSimDiag -> one live-entity / timing sample (issue #71 section 6):
// [zonesInBubble, zonesRemoteTicked, [liveOcc, liveRes, liveCiv, liveTotal],
//  [grpOcc, grpRes, grpCiv, grpMaxSide], garrisonSpawnedZones,
//  garrisonLiveBodies, [trafficCiv, parked, patrol, convoy],
//  [gmAvg, gmP95, gmMax], [frameAvg, frameP95, frameMax], budgetBreachBits]
// (ms timings over the FrameProfiler's 256-frame ring; breach bits 1 = live
// bodies, 2 = groups on one side)
static GameValue GmSimDiag(const GameState* state)
{
    SimDiag d;
    GatherSimDiag(d);
    GameValue value = state->CreateGameValue(GameArray);
    GameArrayType& array = value;
    array.Resize(10);
    array[0] = (float)d.zonesInBubble;
    array[1] = (float)d.zonesRemoteTicked;
    array[2] = ScalarArray(state, {(float)d.liveOcc, (float)d.liveRes, (float)d.liveCiv, (float)d.liveTotal});
    array[3] = ScalarArray(state, {(float)d.grpOcc, (float)d.grpRes, (float)d.grpCiv, (float)d.grpMaxSide});
    array[4] = (float)d.garrisonSpawnedZones;
    array[5] = (float)d.garrisonLiveBodies;
    array[6] = ScalarArray(
        state, {(float)d.trafficCiv, (float)d.trafficParked, (float)d.trafficPatrol, (float)d.trafficConvoy});
    array[7] = ScalarArray(state, {d.gmAvg, d.gmP95, d.gmMax});
    array[8] = ScalarArray(state, {d.frameAvg, d.frameP95, d.frameMax});
    array[9] = (float)SimBudgetBreaches(d, ZoneRegistry::Instance().Tuning());
    return value;
}

// gmSimDiagLog -> write one [gmsim] line now (plus budget WARNs); returns the
// breach bits.  The periodic line runs every simDiagInterval regardless.
static GameValue GmSimDiagLog(const GameState* /*state*/)
{
    return (float)ZoneRegistry::Instance().LogSimDiag();
}

// gmZoneOnEvent ["captured"|"supportThreshold"|"revealed"|"campaignLoaded"|
//               "captureStarted"|"contested"|"captureLost"|"townLost"|
//               "counterAttack"|"counterRepelled"|"baseLost"|"hqLost"|
//               "townStruck", handler]
// townStruck: an occupier air strike hit the town (OCCUPIER-FORCES.md 7.3;
// civilians.sqs starts its panic).
// baseLost: a resistance-held military zone went back to the occupier (its
// occCapture meter reached 100, OCCUPIER-FORCES.md 7.2).  hqLost: the HQ was
// destroyed (GuerrillaBase::Destroy through ZoneRegistry::QueueEvent); the
// handler runs only when the HQ stood in a zone (zoneIndex >= 0).
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

// gmFactionTierClass [sideOrClassName, warLevel]
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

// gmFactionCivTier [sideOrClassName, warLevel] -> the civilian-outfit rung
// (civTier[], issue #25); "" when the faction authors none - callers keep
// their warrior classes
static GameValue GmFactionCivTier(const GameState* state, GameValuePar oper1)
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
    return GameStringType(ZoneRegistry::Instance().FactionCivTierClass(side, (float)array[1]));
}

// gmFactionValue [sideOrClassName, key]
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

// gmFactionSquad [sideOrClassName, warLevel, count] -> array of classnames
// composed by the plan-15 role template (leader slot first; roles the tier
// does not field come back as riflemen).  Empty array when the faction has
// no tiers - callers fall back to their monoculture key (holdClass etc.).
static GameValue GmFactionSquad(const GameState* state, GameValuePar oper1)
{
    const GameArrayType& array = oper1;
    if (!CheckSize(state, array, 3))
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
    if (!CheckType(state, array[2], GameScalar))
    {
        return NOTHING;
    }
    GameStringType side = array[0];
    AutoArray<RString> classes;
    ZoneRegistry::Instance().FactionSquad(side, (float)array[1], toInt((float)array[2]), classes);

    GameValue value = state->CreateGameValue(GameArray);
    GameArrayType& out = value;
    out.Resize(classes.Size());
    for (int i = 0; i < classes.Size(); i++)
    {
        out[i] = GameStringType(classes[i]);
    }
    return value;
}

// gmFactionVehicle [sideOrClassName, warLevel]
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

// gmClassExists <classname> -> bool: the loaded data package carries the
// class under CfgVehicles (the same probe the plan-15 descriptor resolution
// runs; lets missions/tests ask the engine's question before a createUnit)
static GameValue GmClassExists(const GameState* /*state*/, GameValuePar oper1)
{
    GameStringType className = oper1;
    if (className.GetLength() == 0)
    {
        return false;
    }
    const ParamEntry* bank = Pars.FindEntry("CfgVehicles");
    return bank && bank->FindEntry(className) != nullptr;
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
    GGameState.NewNularOp(GameNular(GameString, "gmOccupierSide", GmOccupierSide));
    GGameState.NewNularOp(GameNular(GameString, "gmResistanceSide", GmResistanceSide));

    GGameState.NewFunction(GameFunction(GameArray, "gmZone", GmZone, GameScalar));
    GGameState.NewFunction(GameFunction(GameArray, "gmZoneSim", GmZoneSim, GameScalar));
    GGameState.NewFunction(GameFunction(GameBool, "gmZoneStruck", GmZoneStruck, GameScalar));
    GGameState.NewNularOp(GameNular(GameArray, "gmPlayerPresence", GmPlayerPresence));
    GGameState.NewNularOp(GameNular(GameArray, "gmSimDiag", GmSimDiag));
    GGameState.NewNularOp(GameNular(GameScalar, "gmSimDiagLog", GmSimDiagLog));
    GGameState.NewFunction(GameFunction(GameScalar, "gmZoneIndex", GmZoneIndex, GameString));
    GGameState.NewFunction(GameFunction(GameNothing, "gmZoneSet", GmZoneSet, GameArray));
    GGameState.NewFunction(GameFunction(GameNothing, "gmZoneOnEvent", GmZoneOnEvent, GameArray));
    GGameState.NewFunction(GameFunction(GameString, "gmFactionTierClass", GmFactionTierClass, GameArray));
    GGameState.NewFunction(GameFunction(GameString, "gmFactionCivTier", GmFactionCivTier, GameArray));
    GGameState.NewFunction(GameFunction(GameString, "gmFactionValue", GmFactionValue, GameArray));
    GGameState.NewFunction(GameFunction(GameString, "gmFactionVehicle", GmFactionVehicle, GameArray));
    GGameState.NewFunction(GameFunction(GameArray, "gmFactionSquad", GmFactionSquad, GameArray));
    GGameState.NewFunction(GameFunction(GameBool, "gmClassExists", GmClassExists, GameString));

    GGameState.NewFunction(GameFunction(GameScalar, "gmZoneAlert", GmZoneAlert, GameScalar));
    GGameState.NewFunction(GameFunction(GameArray, "gmZoneLastKnown", GmZoneLastKnown, GameScalar));
    GGameState.NewFunction(GameFunction(GameNothing, "gmAlertOnEvent", GmAlertOnEvent, GameArray));
    GGameState.NewFunction(GameFunction(GameNothing, "gmBreakUndercover", GmBreakUndercover, GameString));
}
