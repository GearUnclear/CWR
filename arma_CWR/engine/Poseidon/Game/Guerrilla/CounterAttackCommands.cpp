// Script command surface for the occupier counter-attack service (issue #79,
// CounterAttack.hpp).  Registered from its own INIT_MODULE at stage 3, the
// JournalCommands.cpp / CampaignEndCommands.cpp pattern.
//
//   gmCounterAttacks            -> [[town, source, phase, clock, strength,
//                                     size, townsBefore, group, spawnPos,
//                                     spawnReason], ...]
//                                  one row per pending or running
//                                  counter-attack; phase is SCHEDULED /
//                                  TRAVEL / HOLDING / LIVE, clock the
//                                  seconds left (SCHEDULED, TRAVEL) or since
//                                  the town edge (LIVE; negative while the
//                                  squad is still on the road), group grpNull
//                                  unless LIVE; spawnPos where the live squad
//                                  was instantiated ([] before, and after a
//                                  reload), spawnReason "base" / "route" /
//                                  "garrison" ("" before); then the occupier
//                                  forces columns: [10] opId (0 legacy),
//                                  [11] kind, [12] origin, [13] package (the
//                                  CounterAttackOrders.hpp enums as numbers),
//                                  [14] live hulls (0 / 1), [15] eta s (the
//                                  TRAVEL clock, -1 otherwise), [16] hull
//                                  class ("" for foot), [17] funded (0 / 1),
//                                  [18] the live hull (objNull when none),
//                                  [19] infantry seated in it, [20] the
//                                  squad's leader (objNull when none), [21]
//                                  its living infantry (0 unless LIVE)
//   gmCounterAttackTune [key, value]
//                                  runtime override of one tuning value
//                                  (tests, debugging); keys: "enabled",
//                                  "firstTownChance", "chance",
//                                  "warLevelStep", "delayMin", "delayMax",
//                                  "size", "speed", "liveTimeout",
//                                  "birthMinToTarget", "birthGiveUp",
//                                  "sourceKeep", "maxLive", "townsTaken"
//                                  (the saved k counter), "forcesOpsMax",
//                                  "forcesLiveHullsMax".  A pending
//                                  SCHEDULED clock is clamped to the new
//                                  delayMax.  "speed" is a TEST LEVER with
//                                  occupierForces on: once set it wins over
//                                  the real speeds (OCCUPIER-FORCES.md 3.2)
//                                  for every leg computed afterwards.
//                                  Returns true when the key is known.
//   gmCounterAttackLaunchFunded [target, source, package, bodies, urgent,
//                                hullClass]
//                                  TEST LEVER: a director-style funded
//                                  operation straight into
//                                  CounterAttack::LaunchFunded.  target and
//                                  source are zone names or indices; package
//                                  "foot" / "truck" (or the ForcePackage
//                                  number); urgent (optional, default
//                                  false); hullClass (optional; default the
//                                  first class of the occupier's vehicles[]
//                                  ladder with transportSoldier >= 6).  The
//                                  hull price (truck 3 + 1 crew) is spent
//                                  from the occupier pool when it is active
//                                  (0 when it refuses).  Returns the opId
//                                  (900001 on), 0 when refused.

#include <Poseidon/Game/Guerrilla/CounterAttack.hpp>
#include <Poseidon/Game/Guerrilla/OccupierForces.hpp> // the test lever spends the hull price
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>
#include <Poseidon/IO/ParamFileExt.hpp> // Pars (hull facts for the test lever)

#include <Poseidon/AI/AI.hpp>                           // AIGroup (the group link)
#include <Poseidon/World/Entities/Vehicles/Vehicle.hpp> // Entity (the hull link)
#include <Poseidon/Game/Commands/GameStateExt.hpp>
#include <Poseidon/Game/Commands/GameStateExtCommon.hpp>
#include <Poseidon/Foundation/Modules/Modules.hpp>
#include <Poseidon/Foundation/platform.hpp>

using namespace Poseidon;
using namespace Poseidon::Guerrilla;

namespace Poseidon::Guerrilla
{
void EnsureCounterAttackCommandsLinked() {}
} // namespace Poseidon::Guerrilla

static GameValue GmCounterAttacks(const GameState* state)
{
    const CounterAttack& ca = CounterAttack::Instance();
    GameValue value = state->CreateGameValue(GameArray);
    GameArrayType& rows = value;
    for (int i = 0; i < ca.Count(); i++)
    {
        const CounterAttack::Record* r = ca.Get(i);
        GameValue rowValue = state->CreateGameValue(GameArray);
        GameArrayType& row = rowValue;
        row.Resize(22);
        row[0] = GameStringType(r->target);
        row[1] = GameStringType(r->source);
        row[2] = GameStringType(CounterAttack::PhaseName(r->phase));
        row[3] = r->clock;
        row[4] = (float)r->strength;
        row[5] = (float)r->size;
        row[6] = (float)r->townsBefore;
        row[7] = GameValueExt(r->phase == CALive ? r->group.GetLink() : (AIGroup*)nullptr);
        GameValue posValue = state->CreateGameValue(GameArray);
        if (r->spawnReason != CASpawnNone)
        {
            // script order [easting, northing, height above ground]
            GameArrayType& pos = posValue;
            pos.Resize(3);
            pos[0] = r->spawnPos.X();
            pos[1] = r->spawnPos.Z();
            pos[2] = 0.0f;
        }
        row[8] = posValue;
        row[9] = GameStringType(CounterAttack::SpawnReasonName(r->spawnReason));
        row[10] = (float)r->opId;
        row[11] = (float)r->kind;
        row[12] = (float)r->origin;
        row[13] = (float)r->package;
        row[14] = (float)CounterAttack::LiveHullOf(*r);
        row[15] = r->phase == CATravel ? r->clock : -1.0f;
        row[16] = GameStringType(r->hullClass);
        row[17] = r->funded ? 1.0f : 0.0f;
        row[18] = GameValueExt((Object*)r->hull.GetLink());
        row[19] = (float)CounterAttack::MountedOf(*r);
        row[20] = GameValueExt((Object*)(r->phase == CALive ? CounterAttack::LeaderOf(*r) : nullptr));
        row[21] = (float)CounterAttack::AliveOf(*r);
        rows.Add(rowValue);
    }
    return value;
}

static GameValue GmCounterAttackTune(const GameState* /*state*/, GameValuePar oper1)
{
    const GameArrayType& args = oper1;
    if (args.Size() < 2 || args[0].GetType() != GameString)
    {
        return GameValue(false);
    }
    const RString key = (GameStringType)args[0];
    const float v = args[1].GetType() == GameBool ? ((GameBoolType)args[1] ? 1.0f : 0.0f) : (float)args[1];
    CounterAttack& ca = CounterAttack::Instance();
    CounterAttackTuning& t = ca.TuningMutable();
    if (stricmp(key, "enabled") == 0)
    {
        t.enabled = v != 0;
    }
    else if (stricmp(key, "firstTownChance") == 0)
    {
        t.firstTownChance = v < 0 ? 0 : (v > 1 ? 1 : v);
    }
    else if (stricmp(key, "chance") == 0)
    {
        t.chance = v < 0 ? 0 : (v > 1 ? 1 : v);
    }
    else if (stricmp(key, "warLevelStep") == 0)
    {
        t.warLevelStep = v < 0 ? 0 : v;
    }
    else if (stricmp(key, "maxLive") == 0)
    {
        t.maxLive = v < 0 ? 0 : (int)v;
    }
    else if (stricmp(key, "townsTaken") == 0)
    {
        ca.SetTownsTaken((int)v);
    }
    else if (stricmp(key, "delayMin") == 0)
    {
        t.delayMin = v < 0 ? 0 : v;
        if (t.delayMax < t.delayMin)
        {
            t.delayMax = t.delayMin;
        }
    }
    else if (stricmp(key, "delayMax") == 0)
    {
        t.delayMax = v < t.delayMin ? t.delayMin : v;
        ca.ClampScheduled(t.delayMax);
    }
    else if (stricmp(key, "size") == 0)
    {
        t.size = v < 0 ? 0 : v;
    }
    else if (stricmp(key, "speed") == 0)
    {
        t.speed = v < 0.5f ? 0.5f : v;
        t.speedLever = true; // the test lever wins over the real speeds (3.2)
    }
    else if (stricmp(key, "forcesOpsMax") == 0)
    {
        t.forcesOpsMax = v < 0 ? 0 : (int)v;
    }
    else if (stricmp(key, "forcesLiveHullsMax") == 0)
    {
        t.forcesLiveHullsMax = v < 0 ? 0 : (int)v;
    }
    else if (stricmp(key, "birthMinToTarget") == 0)
    {
        t.birthMinToTarget = v < 0 ? 0 : v;
    }
    else if (stricmp(key, "birthGiveUp") == 0)
    {
        t.birthGiveUp = v < 1 ? 1 : v;
    }
    else if (stricmp(key, "liveTimeout") == 0)
    {
        t.liveTimeout = v < 0 ? 0 : v;
    }
    else if (stricmp(key, "sourceKeep") == 0)
    {
        t.sourceKeep = v < 0 ? 0 : (v > 1 ? 1 : v);
    }
    else
    {
        return GameValue(false);
    }
    return GameValue(true);
}

// a zone argument: a name or an index
static int ZoneArg(const ZoneRegistry& registry, GameValuePar v)
{
    if (v.GetType() == GameString)
    {
        return registry.FindZoneIndex((GameStringType)v);
    }
    if (v.GetType() == GameScalar)
    {
        return (int)(float)v;
    }
    return -1;
}

static GameValue GmCounterAttackLaunchFunded(const GameState* /*state*/, GameValuePar oper1)
{
    static int nextOpId = 900001; // test operations, clear of the director's ids
    const GameArrayType& args = oper1;
    if (args.Size() < 4)
    {
        return GameValue(0.0f);
    }
    ZoneRegistry& registry = ZoneRegistry::Instance();
    CounterAttack& ca = CounterAttack::Instance();
    CounterAttackOrder order;
    order.target = ZoneArg(registry, args[0]);
    order.source = ZoneArg(registry, args[1]);
    if (args[2].GetType() == GameString)
    {
        const RString p = (GameStringType)args[2];
        order.package = stricmp(p, "truck") == 0 ? FPTruck : FPFoot;
    }
    else
    {
        order.package = (int)(float)args[2];
    }
    order.bodies = args[3].GetType() == GameScalar ? (int)(float)args[3] : 0;
    order.urgent = args.Size() > 4 && args[4].GetType() == GameBool && (bool)(GameBoolType)args[4];
    order.kind = CAKRetake;
    order.origin = CAODirector;
    order.funded = true;
    order.abstractSpeed = CounterAttack::FootSpeed;
    if (order.package != FPFoot)
    {
        RString hull =
            args.Size() > 5 && args[5].GetType() == GameString ? RString((GameStringType)args[5]) : RString();
        const ParamEntry* vehicles = Pars.FindEntry("CfgVehicles");
        if (hull.GetLength() == 0)
        {
            const FactionRecord* f = registry.FindFactionForSide(registry.OccupierSide());
            for (int i = 0; f && i < f->vehicles.Size() && hull.GetLength() == 0; i++)
            {
                const ParamEntry* c = vehicles ? vehicles->FindEntry(f->vehicles[i]) : nullptr;
                if (c && c->IsClass() && c->ReadValue("transportSoldier", 0.0f) >= 6.0f)
                {
                    hull = f->vehicles[i];
                }
            }
        }
        const ParamEntry* c = vehicles && hull.GetLength() > 0 ? vehicles->FindEntry(hull) : nullptr;
        if (!c || !c->IsClass())
        {
            return GameValue(0.0f);
        }
        order.hullClass = hull;
        order.crew = 1;
        order.abstractSpeed = CounterAttack::RealSpeed(order.package, c->ReadValue("maxSpeed", 0.0f));
        const int cargo = (int)c->ReadValue("transportSoldier", 0.0f);
        if (cargo > 0 && order.bodies > cargo)
        {
            order.bodies = cargo;
        }
        // the hull and its driver, paid by the pool (6.5: truck 3, crew 1)
        order.priceUnits = OccupierForces::Instance().Spend(3 + order.crew, "test lever") ? 3 + order.crew : 0;
    }
    order.opId = nextOpId;
    CounterAttackView view;
    ca.BuildView(view);
    AutoArray<ZoneEventRecord> fired;
    if (!ca.LaunchFunded(registry, view, order, ca.EngineOps(), fired))
    {
        OccupierForces::Instance().Refund(order.priceUnits, "test lever refused");
        return GameValue(0.0f);
    }
    registry.FireEvents(fired);
    nextOpId++;
    return GameValue((float)order.opId);
}

INIT_MODULE(GuerrillaCounterAttack, 3)
{
    GGameState.NewNularOp(GameNular(GameArray, "gmCounterAttacks", GmCounterAttacks));
    GGameState.NewFunction(GameFunction(GameBool, "gmCounterAttackTune", GmCounterAttackTune, GameArray));
    GGameState.NewFunction(
        GameFunction(GameScalar, "gmCounterAttackLaunchFunded", GmCounterAttackLaunchFunded, GameArray));
}
