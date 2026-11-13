// Script command surface for occupier air (OccupierAir.hpp; the phase 6
// spike, guerrilla-mode/AIR-SPIKE.md).  Registered from its own INIT_MODULE
// at stage 3, the CounterAttackCommands.cpp pattern.  Test and driver
// levers: phase 6 proper launches air through CounterAttack's order flow.
//
//   gmAirInsert [heliClass, cargo, [bx, bz, altAgl], [tx, tz], side, ([lx, lz] | awayFrom [ax, az])]
//                         plan + spawn one helicopter insertion against the
//                         target: born airborne at [bx, bz] altAgl m above
//                         the ground (0 = the engine default pilot height),
//                         side "EAST" / "WEST" / "GUER".  The LZ is picked
//                         natively 300..500 m from the target (away from the
//                         6th element [ax, az] when it is given with a 7th
//                         element "away"), or taken verbatim from the 6th
//                         element [lx, lz] otherwise.  Returns the record id,
//                         -1 when refused (the reason is logged).
//   gmAirSortie [planeClass, [bx, bz, altAgl], [tx, tz], sortieTime, side]
//                         one jet / attack helicopter sortie; returns its id or -1
//   gmOccupierAir         one row per record (insertions first, then sorties):
//                         [0] state (AirInsertionState / AirSortieState
//                         number), [1] state time s, [2] hull alive (0/1),
//                         [3] crew alive in the hull, [4] cargo alive,
//                         [5] cargo seated, [6] landed offset m (-1 before
//                         touchdown), [7] [x, z, height above ground],
//                         [8] id, [9] "heli" | "jet", [10] state name,
//                         [11] LZ (heli) or target (jet) [x, z],
//                         [12] crew deleted at (record age s, -1 = not yet),
//                         [13] hull deleted at, [14] landings (heli) /
//                         reveals (jet), [15] speed m/s, [16] fail reason
//                         ("" = none), [17] record age s, [18] the hull
//                         (objNull once gone), [19] the cargo group (heli) /
//                         crew group (jet), [20] cargo born (heli) /
//                         lowest height above ground seen after 5 s (jet),
//                         [21] lifted off at age s (heli, -1) / touched the
//                         ground 0/1 (jet), [22] removal order (1 crew
//                         first, 2 hull first, 0 not yet), [23] the crew
//                         group (heli and jet; grpNull once deleted),
//                         [24] landing orders issued (heli; 0 for a jet)
//   gmAirExposure [x, altAgl, z]
//                         the altitude-aware exposure tier at that point for
//                         the current camera: 0 seen, 1 cone-only, 2 hidden
//   gmAirUnseen [x, altAgl, z]
//                         the altitude-aware despawn rule at that point (bool)
//   gmAirAbort id | [id, hullFirst]
//                         delete now, crew first then hull; hullFirst = true
//                         deletes the hull first (the engine-fact probe:
//                         that ejects the crew).  Returns the crew bodies the
//                         deletion left alive outside the airframe, -1 for an
//                         unknown or finished id.  gmAirAbort -1 clears every
//                         finished record.
//   gmAirReveal [id, object, ageSeconds, accuracy]
//                         limited reveal to a sortie (age < 60 s, accuracy < 1); bool
//   gmAirTune [key, value]
//                         one AirTuning value at run time ("landOrderDist",
//                         "landTolerance", "removeHold", "airAudible",
//                         "orbitRadius", "orbitLeg", "lzClear",
//                         "lzMaxGradient", "trace": s between per-record
//                         trace lines, 0 = off); bool

#include <Poseidon/Game/Guerrilla/OccupierAir.hpp>

#include <Poseidon/AI/AI.hpp>
#include <Poseidon/AI/EntityAI.hpp>
#include <Poseidon/World/Terrain/Landscape.hpp> // GLOB_LAND (heights above ground)
#include <Poseidon/World/Entities/Vehicles/Vehicle.hpp>
#include <Poseidon/Game/Commands/GameStateExt.hpp>
#include <Poseidon/Game/Commands/GameStateExtCommon.hpp>
#include <Poseidon/Foundation/Modules/Modules.hpp>
#include <Poseidon/Foundation/platform.hpp>

using namespace Poseidon;
using namespace Poseidon::Guerrilla;

namespace Poseidon::Guerrilla
{
void EnsureOccupierAirCommandsLinked() {}
} // namespace Poseidon::Guerrilla

namespace
{

float Num(const GameValue& v, float def = 0)
{
    return v.GetType() == GameScalar ? (float)v : def;
}

// [x, z(, third)] -> true; third defaults to 0
bool Pair(const GameValue& v, float& x, float& z, float* third = nullptr)
{
    if (v.GetType() != GameArray)
    {
        return false;
    }
    const GameArrayType& a = v;
    if (a.Size() < 2 || a[0].GetType() != GameScalar || a[1].GetType() != GameScalar)
    {
        return false;
    }
    x = a[0];
    z = a[1];
    if (third)
    {
        *third = a.Size() > 2 ? Num(a[2]) : 0.0f;
    }
    return true;
}

float GroundAt(float x, float z)
{
    return GLandscape ? GLOB_LAND->SurfaceYAboveWater(x, z) : 0.0f;
}

GameValue PairValue(const GameState* state, float x, float z)
{
    GameValue v = state->CreateGameValue(GameArray);
    GameArrayType& a = v;
    a.Resize(2);
    a[0] = x;
    a[1] = z;
    return v;
}

GameValue PosValue(const GameState* state, Vector3Par p, float heightAgl)
{
    GameValue v = state->CreateGameValue(GameArray);
    GameArrayType& a = v;
    a.Resize(3);
    a[0] = p.X();
    a[1] = p.Z();
    a[2] = heightAgl;
    return v;
}

} // namespace

static GameValue GmAirInsert(const GameState* /*state*/, GameValuePar oper1)
{
    const GameArrayType& args = oper1;
    float bx = 0, bz = 0, alt = 0, tx = 0, tz = 0;
    if (args.Size() < 5 || args[0].GetType() != GameString || !Pair(args[2], bx, bz, &alt) ||
        !Pair(args[3], tx, tz) || args[4].GetType() != GameString)
    {
        return GameValue(-1.0f);
    }
    OccupierAir& air = OccupierAir::Instance();
    AirInsertionPlan plan;
    plan.heliClass = (GameStringType)args[0];
    plan.cargo = (int)Num(args[1], 8);
    plan.birthPos = Vector3(bx, 0, bz);
    plan.birthAlt = alt;
    plan.target = Vector3(tx, GroundAt(tx, tz), tz);
    plan.side = (GameStringType)args[4];
    float lx = 0, lz = 0;
    const bool given = args.Size() > 5 && Pair(args[5], lx, lz);
    const bool away = given && args.Size() > 6 && args[6].GetType() == GameString &&
                      stricmp((GameStringType)args[6], "away") == 0;
    RString why;
    if (given && !away)
    {
        plan.lz = Vector3(lx, GroundAt(lx, lz), lz);
    }
    else
    {
        const Vector3 from(lx, 0, lz);
        if (!air.PickLz(plan.target, away ? &from : nullptr, 0, 0, plan.lz, why))
        {
            return GameValue(-1.0f);
        }
    }
    plan.lzRadius = air.Tuning().lzClear;
    int id = -1;
    if (!air.SpawnInsertion(plan, id, why))
    {
        return GameValue(-1.0f);
    }
    return GameValue((float)id);
}

static GameValue GmAirSortie(const GameState* /*state*/, GameValuePar oper1)
{
    const GameArrayType& args = oper1;
    float bx = 0, bz = 0, alt = 0, tx = 0, tz = 0;
    if (args.Size() < 5 || args[0].GetType() != GameString || !Pair(args[1], bx, bz, &alt) ||
        !Pair(args[2], tx, tz) || args[4].GetType() != GameString)
    {
        return GameValue(-1.0f);
    }
    int id = -1;
    RString why;
    if (!OccupierAir::Instance().SpawnSortie((GameStringType)args[0], Vector3(bx, 0, bz), alt,
                                            Vector3(tx, GroundAt(tx, tz), tz), Num(args[3], 300.0f),
                                            (GameStringType)args[4], id, why))
    {
        return GameValue(-1.0f);
    }
    return GameValue((float)id);
}

static GameValue GmOccupierAir(const GameState* state)
{
    const OccupierAir& air = OccupierAir::Instance();
    GameValue value = state->CreateGameValue(GameArray);
    GameArrayType& rows = value;
    for (int i = 0; i < air.InsertionCount(); i++)
    {
        const AirInsertion* r = air.GetInsertion(i);
        Entity* hull = r->hull;
        AIGroup* crew = r->crew;
        AIGroup* cargo = r->cargo;
        int crewAlive = 0, cargoAlive = 0, cargoSeated = 0;
        for (int u = 0; crew && hull && u < MAX_UNITS_PER_GROUP; u++)
        {
            AIUnit* unit = crew->UnitWithID(u + 1);
            crewAlive += unit && unit->GetLifeState() == AIUnit::LSAlive && unit->GetVehicleIn() == hull ? 1 : 0;
        }
        for (int u = 0; cargo && u < MAX_UNITS_PER_GROUP; u++)
        {
            AIUnit* unit = cargo->UnitWithID(u + 1);
            if (unit && unit->GetLifeState() == AIUnit::LSAlive)
            {
                cargoAlive++;
                cargoSeated += hull && unit->GetVehicleIn() == hull ? 1 : 0;
            }
        }
        const Vector3 pos = hull ? hull->Position() : r->core.landedAt;
        GameValue rowValue = state->CreateGameValue(GameArray);
        GameArrayType& row = rowValue;
        row.Resize(25);
        row[0] = (float)r->core.state;
        row[1] = r->core.stateTime;
        row[2] = hull && !hull->IsDammageDestroyed() ? 1.0f : 0.0f;
        row[3] = (float)crewAlive;
        row[4] = (float)cargoAlive;
        row[5] = (float)cargoSeated;
        row[6] = r->core.landedOffset;
        row[7] = PosValue(state, pos, hull ? pos.Y() - GroundAt(pos.X(), pos.Z()) : 0.0f);
        row[8] = (float)r->id;
        row[9] = GameStringType("heli");
        row[10] = GameStringType(OccupierAir::StateName(r->core.state));
        row[11] = PairValue(state, r->core.lz.X(), r->core.lz.Z());
        row[12] = r->crewGoneAt;
        row[13] = r->hullGoneAt;
        row[14] = (float)r->core.landings;
        row[15] = hull ? hull->Speed().Size() : 0.0f;
        row[16] = GameStringType(OccupierAir::FailName(r->core.failReason));
        row[17] = r->core.age;
        row[18] = GameValueExt((Object*)hull);
        row[19] = GameValueExt(cargo);
        row[20] = (float)r->cargoBorn;
        row[21] = r->core.liftedOffAt;
        row[22] = (float)r->removeSeq;
        row[23] = GameValueExt(crew);
        row[24] = (float)r->core.landOrders;
        rows.Add(rowValue);
    }
    for (int i = 0; i < air.SortieCount(); i++)
    {
        const AirSortie* s = air.GetSortie(i);
        Entity* hull = s->hull;
        AIGroup* crew = s->crew;
        int crewAlive = 0;
        for (int u = 0; crew && hull && u < MAX_UNITS_PER_GROUP; u++)
        {
            AIUnit* unit = crew->UnitWithID(u + 1);
            crewAlive += unit && unit->GetLifeState() == AIUnit::LSAlive && unit->GetVehicleIn() == hull ? 1 : 0;
        }
        const Vector3 pos = hull ? hull->Position() : s->core.target;
        GameValue rowValue = state->CreateGameValue(GameArray);
        GameArrayType& row = rowValue;
        row.Resize(25);
        row[0] = (float)s->core.state;
        row[1] = s->core.stateTime;
        row[2] = hull && !hull->IsDammageDestroyed() ? 1.0f : 0.0f;
        row[3] = (float)crewAlive;
        row[4] = 0.0f;
        row[5] = 0.0f;
        row[6] = -1.0f;
        row[7] = PosValue(state, pos, hull ? pos.Y() - GroundAt(pos.X(), pos.Z()) : 0.0f);
        row[8] = (float)s->id;
        row[9] = GameStringType("jet");
        row[10] = GameStringType(OccupierAir::SortieStateName(s->core.state));
        row[11] = PairValue(state, s->core.target.X(), s->core.target.Z());
        row[12] = s->crewGoneAt;
        row[13] = s->hullGoneAt;
        row[14] = (float)s->reveals;
        row[15] = hull ? hull->Speed().Size() : 0.0f;
        row[16] = GameStringType(OccupierAir::FailName(s->core.failReason));
        row[17] = s->core.age;
        row[18] = GameValueExt((Object*)hull);
        row[19] = GameValueExt(crew);
        row[20] = s->core.minHeightSeen < 1e8f ? s->core.minHeightSeen : -1.0f;
        row[21] = s->core.touchedGround ? 1.0f : 0.0f;
        row[22] = s->crewGoneAt >= 0 ? (s->hullGoneAt >= s->crewGoneAt ? 1.0f : 2.0f) : 0.0f;
        row[23] = GameValueExt(crew);
        row[24] = 0.0f;
        rows.Add(rowValue);
    }
    return value;
}

static bool PointAt(GameValuePar oper1, Vector3& pos)
{
    const GameArrayType& a = oper1;
    if (a.Size() < 3 || a[0].GetType() != GameScalar || a[1].GetType() != GameScalar ||
        a[2].GetType() != GameScalar)
    {
        return false;
    }
    const float x = a[0];
    const float alt = a[1];
    const float z = a[2];
    pos = Vector3(x, GroundAt(x, z) + alt, z);
    return true;
}

static GameValue GmAirExposure(const GameState* /*state*/, GameValuePar oper1)
{
    Vector3 pos;
    if (!PointAt(oper1, pos))
    {
        return GameValue(0.0f);
    }
    return GameValue((float)OccupierAir::ExposureAt(OccupierAir::EngineViewer(), pos));
}

static GameValue GmAirUnseen(const GameState* /*state*/, GameValuePar oper1)
{
    Vector3 pos;
    if (!PointAt(oper1, pos))
    {
        return GameValue(false);
    }
    return GameValue(OccupierAir::UnseenAt(OccupierAir::EngineViewer(), pos));
}

static GameValue GmAirAbort(const GameState* /*state*/, GameValuePar oper1)
{
    int id = -1;
    bool hullFirst = false;
    if (oper1.GetType() == GameScalar)
    {
        id = (int)(float)oper1;
    }
    else if (oper1.GetType() == GameArray)
    {
        const GameArrayType& a = oper1;
        id = a.Size() > 0 ? (int)Num(a[0], -1) : -1;
        hullFirst = a.Size() > 1 && a[1].GetType() == GameBool && (bool)(GameBoolType)a[1];
    }
    if (id < 0)
    {
        OccupierAir::Instance().Clear();
        return GameValue(0.0f);
    }
    return GameValue((float)OccupierAir::Instance().Abort(id, hullFirst));
}

static GameValue GmAirReveal(const GameState* /*state*/, GameValuePar oper1)
{
    const GameArrayType& a = oper1;
    if (a.Size() < 4 || a[1].GetType() != GameObject)
    {
        return GameValue(false);
    }
    Object* obj = GetObject(a[1]);
    Entity* target = dyn_cast<Entity>(obj);
    return GameValue(OccupierAir::Instance().Reveal((int)Num(a[0], -1), target, Num(a[2], 1e9f), Num(a[3], 1.0f)));
}

static GameValue GmAirTune(const GameState* /*state*/, GameValuePar oper1)
{
    const GameArrayType& a = oper1;
    if (a.Size() < 2 || a[0].GetType() != GameString || a[1].GetType() != GameScalar)
    {
        return GameValue(false);
    }
    const RString key = (GameStringType)a[0];
    const float v = a[1];
    AirTuning& t = OccupierAir::Instance().TuningMutable();
    if (stricmp(key, "landOrderDist") == 0)
    {
        t.landOrderDist = v;
    }
    else if (stricmp(key, "landTolerance") == 0)
    {
        t.landTolerance = v;
    }
    else if (stricmp(key, "removeHold") == 0)
    {
        t.removeHold = v;
    }
    else if (stricmp(key, "airAudible") == 0)
    {
        t.airAudible = v;
    }
    else if (stricmp(key, "orbitRadius") == 0)
    {
        t.orbitRadius = v;
    }
    else if (stricmp(key, "orbitLeg") == 0)
    {
        t.orbitLeg = v;
    }
    else if (stricmp(key, "lzClear") == 0)
    {
        t.lzClear = v;
    }
    else if (stricmp(key, "lzMaxGradient") == 0)
    {
        t.lzMaxGradient = v;
    }
    else if (stricmp(key, "trace") == 0)
    {
        t.trace = v < 0 ? 0 : v;
    }
    else
    {
        return GameValue(false);
    }
    return GameValue(true);
}

INIT_MODULE(GuerrillaOccupierAir, 3)
{
    GGameState.NewFunction(GameFunction(GameScalar, "gmAirInsert", GmAirInsert, GameArray));
    GGameState.NewFunction(GameFunction(GameScalar, "gmAirSortie", GmAirSortie, GameArray));
    GGameState.NewNularOp(GameNular(GameArray, "gmOccupierAir", GmOccupierAir));
    GGameState.NewFunction(GameFunction(GameScalar, "gmAirExposure", GmAirExposure, GameArray));
    GGameState.NewFunction(GameFunction(GameBool, "gmAirUnseen", GmAirUnseen, GameArray));
    GGameState.NewFunction(GameFunction(GameScalar, "gmAirAbort", GmAirAbort, GameScalar | GameArray));
    GGameState.NewFunction(GameFunction(GameBool, "gmAirReveal", GmAirReveal, GameArray));
    GGameState.NewFunction(GameFunction(GameBool, "gmAirTune", GmAirTune, GameArray));
}
