// CounterAttack, phase 6A: air packages, strikes and alarm answers
// (guerrilla-mode/OCCUPIER-FORCES.md 3.1, 3.2, 6.5, 8.2, D3, D4, D9, D17, D20;
// the recipe and its trials in guerrilla-mode/AIR-SPIKE.md).
// Split out of CounterAttack.cpp, which sits near the FileSize limit.
//
// A helicopter insertion is an ordinary record (RETAKE, BASE, HQ_ASSAULT,
// REINFORCE, ALARM) whose package is FPTransportHeli.  Abstract, it flies
// the straight line from its origin (an occupier-held AIRFIELD, else the
// off-map edge, forcesOffMapEdge) to the target at 0.8 x maxSpeed.  When it
// must go live it is born AIRBORNE where the altitude-aware gate rates the
// point hidden (searched back from its progress point to the origin, then
// off-map), flies at real speed to an LZ 300..500 m from the target on the
// side away from the reported fighters, lands (in plain view if it must,
// D20), unloads, and its infantry becomes the record's live squad.  The
// airframe then flies home and OccupierAir removes it crew first where
// unseen (AirHome), refunding its price.  A landing more than 100 m off the
// LZ is a failed insertion: re-planned once (back to abstract flight, the
// airframe re-created at the next birth), then cancelled unseen.
//
// A strike (CAKStrike) is a record with no bodies: a jet or attack
// helicopter flies to strikePos; live (strikePos in the player's bubble) it
// is a real OccupierAir sortie for forcesSortieTime with the limited reveal,
// abstract it reaches the target on its clock.  Either way CAOpStruck goes
// out once, and the director applies the town-strike effects (7.3).

#include <Poseidon/Game/Guerrilla/CounterAttack.hpp>

#include <Poseidon/Game/Guerrilla/CampaignEnd.hpp> // IsMilitaryType
#include <Poseidon/Game/Guerrilla/ForcePackages.hpp>
#include <Poseidon/Game/Guerrilla/GarrisonCache.hpp>
#include <Poseidon/Game/Guerrilla/OccupierForces.hpp>
#include <Poseidon/Game/Guerrilla/RoadRoute.hpp> // SameLandmass: a base on another landmass is air-only
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>

#include <Poseidon/IO/Serialization/ParamArchive.hpp>

#include <Poseidon/World/World.hpp>
#include <Poseidon/World/Entities/Vehicles/Transport.hpp>
#include <Poseidon/AI/AI.hpp>
#include <Poseidon/AI/AICore.hpp>

#include <Poseidon/Foundation/Framework/DebugLog.hpp>
#include <Poseidon/Game/Guerrilla/DirectorLog.hpp> // GM_DIRECTOR_INFO
#include <Poseidon/Foundation/platform.hpp>

#include <math.h>
#include <string.h>

namespace Poseidon
{
namespace Guerrilla
{

namespace
{

float AirDist2D(Vector3Par a, Vector3Par b)
{
    const float dx = a.X() - b.X();
    const float dz = a.Z() - b.Z();
    return sqrtf(dx * dx + dz * dz);
}

bool IsZeroPos(Vector3Par p)
{
    return p.X() == 0 && p.Z() == 0;
}

// metres between birth candidates on the flight line
constexpr float BirthStep = 250.0f;
// beyond the map edge: the first off-map candidate, and the step outward
constexpr float OffMapMargin = 1500.0f;
constexpr float OffMapStep = 1000.0f;
constexpr int OffMapTries = 5;
// a landing farther than this from the LZ is a failed insertion (3.2)
constexpr float LandTolerance = 100.0f;
// the limited reveal: a fresh sighting, re-issued while the sortie is over its target
constexpr float RevealEvery = 10.0f;
constexpr float RevealMaxAge = 60.0f;
// insertion states mirrored from OccupierAir.hpp (AirInsertionState, AirFailReason)
constexpr int AirStateDeparting = 3;
constexpr int AirFailHullLost = 3;

} // namespace

// ---------------------------------------------------------------------------
// pure helpers
// ---------------------------------------------------------------------------

Vector3 CounterAttack::OffMapEdgePoint(Vector3Par target, float mapSize, int edge, float margin)
{
    if (mapSize <= 0)
    {
        return VZero;
    }
    const float x = target.X();
    const float z = target.Z();
    int e = edge;
    if (e < 0 || e >= NCounterAttackMapEdges)
    {
        // the edge nearest the target
        const float dn = mapSize - z;
        const float ds = z;
        const float de = mapSize - x;
        const float dw = x;
        e = CAEdgeNorth;
        float best = dn;
        if (ds < best)
        {
            best = ds;
            e = CAEdgeSouth;
        }
        if (de < best)
        {
            best = de;
            e = CAEdgeEast;
        }
        if (dw < best)
        {
            e = CAEdgeWest;
        }
    }
    switch (e)
    {
        case CAEdgeNorth:
            return Vector3(x, 0, mapSize + margin);
        case CAEdgeSouth:
            return Vector3(x, 0, -margin);
        case CAEdgeEast:
            return Vector3(mapSize + margin, 0, z);
        default:
            return Vector3(-margin, 0, z);
    }
}

float CounterAttack::AirBirthAlt(int package)
{
    return package == FPJet ? 250.0f : 100.0f;
}

bool CounterAttack::ReportedNear(const ZoneRegistry& registry, int zone, Vector3& centre, float* age)
{
    const ZoneRecord* z = registry.GetZone(zone);
    if (!z)
    {
        return false;
    }
    const OccupierForces& of = OccupierForces::Instance();
    for (int i = 0; i < of.SightingRows(); i++)
    {
        if (stricmp(of.SightingZone(i), z->name) != 0)
        {
            continue;
        }
        const AutoArray<Vector3>* pts = of.SightingPositions(i);
        if (!pts || pts->Size() == 0)
        {
            continue;
        }
        Vector3 sum = VZero;
        for (int k = 0; k < pts->Size(); k++)
        {
            sum += (*pts)[k];
        }
        centre = sum * (1.0f / (float)pts->Size());
        if (age)
        {
            *age = of.SightingAge(i);
        }
        return true;
    }
    return false;
}

// ---------------------------------------------------------------------------
// queries and validation
// ---------------------------------------------------------------------------

int CounterAttack::FindAlarm() const
{
    for (int i = 0; i < _records.Size(); i++)
    {
        if (_records[i].kind == CAKAlarm)
        {
            return i;
        }
    }
    return -1;
}

int CounterAttack::FindStrikeOn(const CounterAttackOrder& order) const
{
    for (int i = 0; i < _records.Size(); i++)
    {
        const Record& r = _records[i];
        if (r.kind != CAKStrike)
        {
            continue;
        }
        if (order.strikeZone >= 0 && r.strikeZone == order.strikeZone)
        {
            return i;
        }
        if (order.strikeZone < 0 && !IsZeroPos(order.strikePos) && AirDist2D(r.strikePos, order.strikePos) < 500.0f)
        {
            return i;
        }
    }
    return -1;
}

const char* CounterAttack::StrikeRefusal(const ZoneRegistry& registry, const CounterAttackOrder& order)
{
    if (order.package != FPJet && order.package != FPAttackHeli)
    {
        return "a strike flies a jet or an attack helicopter";
    }
    if (order.hullClass.GetLength() == 0)
    {
        return "a strike names no airframe";
    }
    const ZoneRecord* z = order.strikeZone >= 0 ? registry.GetZone(order.strikeZone) : nullptr;
    if (order.strikeZone >= 0 && !z)
    {
        return "the strike zone is unknown";
    }
    if (!z && IsZeroPos(order.strikePos))
    {
        return "a strike needs a reported position or a town";
    }
    return nullptr;
}

void CounterAttack::EmitAt(const ZoneRegistry& registry, const Record& r, int type, int zone, Vector3Par pos,
                           int bodies, int units, const char* note)
{
    if (!Listener())
    {
        return;
    }
    CounterAttackOpEvent ev;
    ev.opId = r.opId;
    ev.type = type;
    ev.kind = r.kind;
    ev.origin = r.origin;
    ev.target = registry.FindZoneIndex(r.target);
    ev.source = r.source.GetLength() > 0 ? registry.FindZoneIndex(r.source) : -1;
    ev.bodies = bodies;
    ev.units = units;
    ev.note = note ? note : "";
    ev.zone = zone;
    ev.pos = pos;
    Dispatch(ev);
}

// ---------------------------------------------------------------------------
// origin and birth
// ---------------------------------------------------------------------------

Vector3 CounterAttack::AirOriginPos(const ZoneRegistry& registry, const CounterAttackView& view, const Record& r,
                                    Vector3Par dest) const
{
    const ZoneRecord* field = r.airOrigin >= 0 ? registry.GetZone(r.airOrigin) : nullptr;
    if (field && stricmp(field->owner, registry.OccupierSide()) == 0)
    {
        return field->pos;
    }
    const float mapSize = view.mapSize > 0 ? view.mapSize : _mapSize;
    const Vector3 edge = OffMapEdgePoint(dest, mapSize, _tuning.offMapEdge, OffMapMargin);
    if (!IsZeroPos(edge))
    {
        return edge;
    }
    // no map size known (unit tests without one): the paying base stands in
    const ZoneRecord* src = registry.GetZone(registry.FindZoneIndex(r.source));
    return src ? src->pos : dest;
}

bool CounterAttack::PickAirBirth(const ZoneRegistry& registry, const CounterAttackView& view, const Record& r,
                                 const CounterAttackOps& ops, Vector3Par dest, float progress, Vector3& birth) const
{
    const Vector3 origin = !IsZeroPos(r.airFrom) ? r.airFrom : AirOriginPos(registry, view, r, dest);
    const float alt = AirBirthAlt(r.package);
    auto passes = [&](Vector3Par c) {
        if (AirDist2D(c, dest) < _tuning.birthMinToTarget)
        {
            return false;
        }
        const Vector3 asl(c.X(), ops.GroundY(c.X(), c.Z()) + alt, c.Z());
        return ops.AirExposure(asl) == CAExposeHidden;
    };
    // back from the progress point to the origin, on the straight line
    const float len = AirDist2D(origin, dest);
    const float p = progress < 0 ? 0 : (progress > 1 ? 1 : progress);
    Vector3 dir = dest - origin;
    dir[1] = 0;
    for (float d = p * len; d >= 0; d -= BirthStep)
    {
        const Vector3 c = len > 1e-3f ? origin + dir * (d / len) : origin;
        if (passes(c))
        {
            birth = c;
            return true;
        }
    }
    if (passes(origin))
    {
        birth = origin;
        return true;
    }
    // then beyond the map edge, further out each try (off-map is sea and it
    // renders: it is admissible only where the gate passes at altitude)
    const float mapSize = view.mapSize > 0 ? view.mapSize : _mapSize;
    const Vector3 edge = OffMapEdgePoint(dest, mapSize, _tuning.offMapEdge, OffMapMargin);
    if (IsZeroPos(edge))
    {
        return false;
    }
    Vector3 out = edge - dest;
    out[1] = 0;
    const float outLen = out.Size();
    for (int k = 0; k < OffMapTries; k++)
    {
        const Vector3 c = outLen > 1e-3f ? edge + out * ((float)k * OffMapStep / outLen) : edge;
        if (passes(c))
        {
            birth = c;
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// helicopter insertion
// ---------------------------------------------------------------------------

int CounterAttack::AirGoLive(ZoneRegistry& registry, const CounterAttackView& view, int target, Record& r,
                             CounterAttackOps* ops, float fraction)
{
    (void)target;
    const Vector3 dest = DestOf(registry, r);
    if (!ops || IsZeroPos(dest))
    {
        return BirthWait;
    }
    // the infantry goes on the ground rules when the air cannot carry it:
    // no LZ, or the airframe could not be created (the price back)
    auto toFoot = [&](const char* why) {
        LOG_WARN(Core, "CounterAttack: [op {}] {} for the insertion on {}; the force goes on foot", r.opId, why,
                 (const char*)r.target);
        if (r.airReplanned)
        {
            OccupierForces::Instance().Refund(r.priceUnits, "air re-plan: no insertion");
            Emit(registry, r, CAOpHullRefund, 0, 0, "insertion impossible: on foot");
        }
        else
        {
            Emit(registry, r, CAOpHullRefund, 0, r.priceUnits, "insertion impossible: on foot");
        }
        r.package = FPFoot;
        r.hullClass = RString();
        r.crew = 0;
        r.priceUnits = 0;
        r.hullDone = true;
        r.airReplanned = false;
        r.airOrigin = -1;
        r.route.Clear();
        r.abstractSpeed = r.abstractSpeed > 0 ? FootSpeed : 0;
        r.travel = 0;
        r.retry = 0;
    };
    Vector3 birth = VZero;
    if (!PickAirBirth(registry, view, r, *ops, dest, fraction, birth))
    {
        if (!r.hiddenLogged)
        {
            LOG_WARN(Core,
                     "CounterAttack: [op {}] the insertion on {} has no unseen birth point at altitude (airfield or"
                     " off-map); retrying every {:.0f}s",
                     r.opId, (const char*)r.target, RetryInterval);
            r.hiddenLogged = true;
        }
        return BirthNoPoint;
    }
    Vector3 away = VZero;
    const bool reported = ReportedNear(registry, registry.FindZoneIndex(r.target), away);
    Vector3 lz = VZero;
    if (!ops->AirPickLz(dest, reported ? &away : nullptr, lz))
    {
        toFoot("no LZ");
        return BirthWait;
    }
    AIGroup* cargo = nullptr;
    Entity* hull = nullptr;
    AIGroup* crewGroup = nullptr;
    const float alt = AirBirthAlt(r.package);
    const int id = ops->AirInsert(r.hullClass, r.strength, birth, alt, lz, dest, cargo, hull, crewGroup);
    if (id <= 0 || !cargo)
    {
        toFoot("the airframe could not be created");
        return BirthWait;
    }
    CounterAttackAirStatus st;
    const int born = ops->AirStatus(id, st) && st.cargoBorn > 0 ? st.cargoBorn : r.strength;
    if (born < r.strength)
    {
        // partial-spawn failures go to the pool (5.5)
        OccupierForces::Instance().Refund(r.strength - born, "insertion seats");
        r.strength = born;
    }
    r.phase = CALive;
    r.group = cargo;
    r.hull = hull;
    r.crewGroup = crewGroup;
    r.airId = id;
    r.airLz = lz;
    r.airLanded = false;
    r.dismounted = false;
    r.leg = LegUnknown;
    r.route.Clear();
    r.resend = ResendInterval;
    r.spawnReason = CASpawnRoute;
    r.spawnPos = birth;
    r.deferTime = 0;
    r.hiddenLogged = false;
    r.deferLogged = false;
    // the live clock: the flight and the walk from the LZ before the live
    // timeout starts counting (the town edge resets it)
    const float speed = r.abstractSpeed > 0 ? r.abstractSpeed / ForcePackages::AirCruise : 50.0f;
    r.clock = -(AirDist2D(birth, lz) / (speed > 1 ? speed : 1.0f) + 600.0f);
    GM_DIRECTOR_INFO(Core,
             "CounterAttack: [op {}] {} with {} soldier(s) born airborne at [{:.0f}, {:.0f}] ({:.0f} m up) for {}; LZ"
             " [{:.0f}, {:.0f}] {:.0f} m out{}",
             r.opId, (const char*)r.hullClass, r.strength, birth.X(), birth.Z(), alt, (const char*)r.target, lz.X(),
             lz.Z(), AirDist2D(lz, dest), reported ? ", away from the reported fighters" : "");
    Emit(registry, r, CAOpBorn, r.strength, 0, "airborne");
    EmitAt(registry, r, CAOpAirborne, r.airOrigin, birth, r.strength, 0, r.airOrigin >= 0 ? "airfield" : "off-map");
    return BirthDone;
}

void CounterAttack::SendAirHome(Record& r)
{
    if (r.airId > 0)
    {
        AirHome h;
        h.opId = r.opId;
        h.kind = r.kind;
        h.origin = r.origin;
        h.target = r.target;
        h.source = r.source;
        h.units = r.HullPrice();
        h.crew = r.crew;
        h.airId = r.airId;
        h.direct = r.airReplanned;
        _airHomes.Add(h);
    }
    r.hullDone = true;
    r.hull = nullptr;
    r.crewGroup = nullptr;
    r.airId = 0;
}

bool CounterAttack::AdvanceAirLive(ZoneRegistry& registry, const CounterAttackView& view, CounterAttackOps* ops,
                                   Record& r)
{
    CounterAttackAirStatus st;
    if (!ops || !ops->AirStatus(r.airId, st))
    {
        // the airframe's record is gone (an old save, or dropped): the
        // infantry stands where it is
        LOG_WARN(Core, "CounterAttack: [op {}] insertion #{} unknown; its infantry goes on as a live squad", r.opId,
                 r.airId);
        r.dismounted = true;
        SendAirHome(r);
        return false;
    }
    const int target = registry.FindZoneIndex(r.target);
    if (st.landed && !r.airLanded)
    {
        r.airLanded = true;
        GM_DIRECTOR_INFO(Core, "CounterAttack: [op {}] {} down {:.0f} m from its LZ at {}", r.opId, (const char*)r.hullClass,
                 st.landedOffset, (const char*)r.target);
        EmitAt(registry, r, CAOpLanded, target, st.landedAt, r.strength, 0,
               st.landedOffset > LandTolerance ? "off the LZ" : "on the LZ");
    }
    const bool landedOk = st.landed && st.landedOffset >= 0 && st.landedOffset <= LandTolerance && st.failReason == 0;
    if (landedOk && st.state >= AirStateDeparting)
    {
        // unloaded: the infantry is the record's live squad and assaults
        // from the LZ; the airframe flies home (AirHome)
        Vector3 leader = VZero;
        const int alive = ops->Alive(r.group, leader);
        GM_DIRECTOR_INFO(Core, "CounterAttack: [op {}] {} soldier(s) out at the LZ for {}; the airframe departs", r.opId,
                 alive, (const char*)r.target);
        r.dismounted = true;
        r.leg = LegUnknown;
        r.route.Clear();
        r.resend = 0;
        SendAirHome(r);
        return false;
    }
    if (!st.gone && !(st.hullLost))
    {
        return false; // flying, landing or unloading; or a failed airframe not yet removed
    }
    Vector3 leader = VZero;
    const int out = ops->Alive(r.group, leader); // cargo still aboard was removed with the airframe
    if (st.hullLost)
    {
        // shot down or wrecked: the airframe and its crew are lost; whoever
        // got out alive fights on
        GM_DIRECTOR_INFO(Core, "CounterAttack: [op {}] {} lost with its crew; {} of {} soldier(s) survive", r.opId,
                 (const char*)r.hullClass, out, r.strength);
        OccupierForces::Instance().NoteLoss(r.crew, "air crew lost");
        SettleCrew(registry, r, r.crew, "air crew lost");
        if (r.airReplanned)
        {
            OccupierForces::Instance().NoteLoss(r.HullPrice(), "airframe lost");
            Emit(registry, r, CAOpHullLost, 0, 0, "airframe lost");
        }
        else
        {
            Emit(registry, r, CAOpHullLost, 0, r.HullPrice(), "airframe lost");
        }
        r.hullDone = true;
        r.hull = nullptr;
        r.crewGroup = nullptr;
        r.airId = 0;
        if (out < r.strength)
        {
            OccupierForces::Instance().NoteLoss(r.strength - out, "insertion lost");
        }
        if (out <= 0)
        {
            AddLeftover(r.source, nullptr, r.group, 0);
            Emit(registry, r, CAOpRepelled, 0, r.strength, "the insertion was shot down");
            return true;
        }
        r.strength = out;
        r.dismounted = true;
        r.leg = LegUnknown;
        r.route.Clear();
        return false;
    }
    if (out > 0)
    {
        // part of the cargo got out before the airframe left (an unload
        // timeout): the men on the ground are the squad, the airframe is home
        GM_DIRECTOR_INFO(Core, "CounterAttack: [op {}] {} of {} soldier(s) got out before the airframe left", r.opId, out,
                 r.strength);
        if (out < r.strength)
        {
            OccupierForces::Instance().Refund(r.strength - out, "insertion: flown home aboard");
            r.strength = out;
        }
        r.dismounted = true;
        r.leg = LegUnknown;
        r.route.Clear();
        r.airId = 0;
        r.hull = nullptr;
        r.crewGroup = nullptr;
        r.hullDone = true;
        CrewHome(registry, view, r.source, r.crew);
        SettleCrew(registry, r, r.crew, "air crew home");
        if (r.airReplanned)
        {
            OccupierForces::Instance().Refund(r.HullPrice(), "air re-plan hull home");
            Emit(registry, r, CAOpHullRefund, r.crew, 0, "airframe home");
        }
        else
        {
            Emit(registry, r, CAOpHullRefund, r.crew, r.HullPrice(), "airframe home");
        }
        return false;
    }
    // a failed insertion (off the LZ, a timeout, no pilot): the airframe was
    // removed unseen with its cargo still aboard.  Re-planned once (back to
    // abstract flight, the airframe re-created at the next birth, no refund),
    // then cancelled unseen (3.2)
    r.group = nullptr;
    r.hull = nullptr;
    r.crewGroup = nullptr;
    r.airId = 0;
    r.airLanded = false;
    if (r.airReplans < 1)
    {
        r.airReplans++;
        r.phase = CATravel;
        r.route.Clear();
        r.travel = r.travel > 0 ? r.travel : LegTime(registry, r, ops);
        r.clock = r.travel;
        r.retry = 0;
        GM_DIRECTOR_INFO(Core, "CounterAttack: [op {}] failed insertion on {} ({}); re-planned once", r.opId,
                 (const char*)r.target, st.failReason);
        return false;
    }
    GM_DIRECTOR_INFO(Core, "CounterAttack: [op {}] the insertion on {} failed twice; cancelled unseen", r.opId,
             (const char*)r.target);
    CancelUnseen(registry, view, r);
    return true;
}

void CounterAttack::StepAirHomes(ZoneRegistry& registry, const CounterAttackView& view, CounterAttackOps* ops)
{
    for (int i = _airHomes.Size() - 1; i >= 0; i--)
    {
        const AirHome h = _airHomes[i];
        CounterAttackAirStatus st;
        const bool known = ops && ops->AirStatus(h.airId, st);
        if (known && !st.gone)
        {
            continue;
        }
        if (!ops)
        {
            continue; // no world (unit tests without air): it waits
        }
        Record r;
        r.opId = h.opId;
        r.kind = h.kind;
        r.origin = h.origin;
        r.target = h.target;
        r.source = h.source;
        r.airReplanned = h.direct; // SettleCrew: a direct airframe is on no row
        if (known && st.hullLost)
        {
            OccupierForces::Instance().NoteLoss(h.crew, "air crew lost");
            SettleCrew(registry, r, h.crew, "air crew lost on the way home");
            if (h.direct)
            {
                OccupierForces::Instance().NoteLoss(h.units, "airframe lost");
            }
            Emit(registry, r, CAOpHullLost, 0, h.direct ? 0 : h.units, "airframe lost on the way home");
        }
        else
        {
            CrewHome(registry, view, h.source, h.crew);
            SettleCrew(registry, r, h.crew, "air crew home");
            if (h.direct)
            {
                OccupierForces::Instance().Refund(h.units, "air re-plan hull home");
            }
            GM_DIRECTOR_INFO(Core, "CounterAttack: [op {}] the airframe of the insertion on {} is home", h.opId,
                     (const char*)h.target);
            Emit(registry, r, CAOpHullRefund, h.crew, h.direct ? 0 : h.units, "airframe home");
        }
        _airHomes.Delete(i);
    }
}

// ---------------------------------------------------------------------------
// the air re-plan of the birth chain (3.1 step 4)
// ---------------------------------------------------------------------------

bool CounterAttack::GroundSourceShares(const ZoneRegistry& registry, const Record& r) const
{
    const Vector3 dest = DestOf(registry, r);
    if (IsZeroPos(dest))
    {
        return true;
    }
    for (int i = 0; i < registry.NZones(); i++)
    {
        const ZoneRecord* z = registry.GetZone(i);
        if (!z || !CampaignEnd::IsMilitaryType(z->type) || stricmp(z->owner, registry.OccupierSide()) != 0 ||
            stricmp(z->name, r.source) == 0 || stricmp(z->name, r.target) == 0)
        {
            continue;
        }
        if (r.tried.GetLength() > 0 && strstr((const char*)r.tried, (const char*)z->name) != nullptr)
        {
            continue;
        }
        if (RoadRoute::SameLandmass(z->pos, dest))
        {
            return true;
        }
    }
    return false;
}

bool CounterAttack::AirReplan(ZoneRegistry& registry, const CounterAttackView& view, Record& r,
                              const ForcePackageCatalogue& catalogue)
{
    if (!r.funded || r.IsAir() || r.kind == CAKStrike || r.phase != CATravel || r.strength < 1 ||
        !catalogue.Offered(FPTransportHeli) || ForcePackages::SeatCap(FPTransportHeli, catalogue) < r.strength)
    {
        return false;
    }
    const Vector3 dest = DestOf(registry, r);
    if (IsZeroPos(dest))
    {
        return false;
    }
    // the nearest occupier-held airfield, else off-map (dearer, D3)
    int field = -1;
    float best = 0;
    for (int i = 0; i < registry.NZones(); i++)
    {
        const ZoneRecord* z = registry.GetZone(i);
        if (!z || stricmp(z->type, "AIRFIELD") != 0 || stricmp(z->owner, registry.OccupierSide()) != 0)
        {
            continue;
        }
        const float d = AirDist2D(z->pos, dest);
        if (field < 0 || d < best)
        {
            field = i;
            best = d;
        }
    }
    const int crew = ForcePackages::CrewOf(FPTransportHeli, catalogue);
    const int price = ForcePackages::HullPrice(FPTransportHeli, field < 0) + crew;
    // the operation's ledger row holds the airframe like a ground hull, so
    // its refund or loss settles through the row (gmOccupierOps and the row
    // agree); with no row to take it the price is settled straight to the
    // pool (airReplanned)
    bool booked = false;
    if (!OccupierForces::Instance().SpendOnOp(r.opId, price, FPTransportHeli, "air re-plan", booked))
    {
        return false;
    }
    // the ground hull (if any) goes back through the ledger as usual
    const int owed = r.OwedUnits();
    if (owed > 0)
    {
        Emit(registry, r, CAOpHullRefund, 0, owed, "air re-plan: ground hull not used");
    }
    r.package = FPTransportHeli;
    r.hullClass = catalogue.hulls[FPTransportHeli].className;
    r.crew = crew;
    r.priceUnits = price;
    r.hullDone = false;
    r.escortClass = RString();
    r.escortCrew = 0;
    r.escortPriceUnits = 0;
    r.escortDone = true;
    r.airReplanned = !booked;
    r.airOrigin = field;
    r.route.Clear();
    r.deferTime = 0;
    r.hiddenLogged = false;
    r.edgeHold = false;
    r.retry = 0;
    if (r.abstractSpeed > 0 || _tuning.forcesOn)
    {
        r.abstractSpeed = ForcePackages::SpeedOf(FPTransportHeli, catalogue);
    }
    const Vector3 origin = AirOriginPos(registry, view, r, dest);
    r.airFrom = origin;
    const float speed = r.abstractSpeed > 0 ? r.abstractSpeed : 50.0f;
    r.travel = AirDist2D(origin, dest) / speed;
    if (r.travel < MinTravel)
    {
        r.travel = MinTravel;
    }
    r.clock = r.travel;
    GM_DIRECTOR_INFO(Core,
             "CounterAttack: [op {}] no road to {} passes the gate; re-planned as a {} insertion from {} ({} units,"
             " {:.0f}s in the air{})",
             r.opId, (const char*)r.target, (const char*)r.hullClass,
             field >= 0 ? (const char*)registry.GetZone(field)->name : "off-map", price, r.travel,
             booked ? ", on the ledger row" : ", settled to the pool directly");
    return true;
}

// ---------------------------------------------------------------------------
// strikes (D17)
// ---------------------------------------------------------------------------

bool CounterAttack::LaunchStrike(ZoneRegistry& registry, const CounterAttackView& view, const CounterAttackOrder& order)
{
    const ZoneRecord* z = order.strikeZone >= 0 ? registry.GetZone(order.strikeZone) : nullptr;
    const ZoneRecord* field = order.airOrigin >= 0 ? registry.GetZone(order.airOrigin) : nullptr;
    Record r;
    r.target = z ? z->name : RString();
    r.source = field ? field->name : RString();
    r.townsBefore = _townsTaken;
    r.seed = MakeSeed(z ? (const char*)z->name : "STRIKE", 0, 0, _townsTaken) ^ (0x57A1CE00u + (unsigned)order.opId);
    r.opId = order.opId;
    r.kind = CAKStrike;
    r.origin = order.origin;
    r.package = order.package;
    r.hullClass = order.hullClass;
    r.crew = order.crew > 0 ? order.crew : 1;
    r.priceUnits = order.priceUnits > 0 ? order.priceUnits : 0;
    r.urgent = order.urgent;
    r.funded = order.funded;
    r.abstractSpeed = order.abstractSpeed > 0 ? order.abstractSpeed : 0;
    r.lethality = order.lethality > 0 ? order.lethality : 1.0f;
    r.airOrigin = field ? order.airOrigin : -1;
    r.strikeZone = z ? order.strikeZone : -1;
    r.strikePos = !IsZeroPos(order.strikePos) ? order.strikePos : (z ? z->pos : VZero);
    r.destPos = r.strikePos;
    r.strength = 0;
    r.size = 0;
    r.phase = CATravel;
    const Vector3 from = AirOriginPos(registry, view, r, r.strikePos);
    r.airFrom = from;
    const float speed = r.abstractSpeed > 0 ? r.abstractSpeed : 50.0f;
    r.travel = AirDist2D(from, r.strikePos) / speed;
    r.clock = r.travel;
    _records.Add(r);
    const Record& added = _records[_records.Size() - 1];
    GM_DIRECTOR_INFO(Core, "CounterAttack: [op {}] strike by {} on {} at [{:.0f}, {:.0f}] from {}; {:.0f}s in the air",
             added.opId, (const char*)added.hullClass, z ? (const char*)z->name : "a reported position",
             added.strikePos.X(), added.strikePos.Z(), field ? (const char*)field->name : "off-map", added.clock);
    Emit(registry, added, CAOpLaunched, 0, added.priceUnits, "strike");
    return true;
}

bool CounterAttack::AdvanceStrike(ZoneRegistry& registry, const CounterAttackView& view, float elapsed,
                                  CounterAttackOps* ops, Record& r)
{
    auto struck = [&](const char* how) {
        if (!r.struck)
        {
            r.struck = true;
            GM_DIRECTOR_INFO(Core, "CounterAttack: [op {}] the strike by {} reaches {} ({})", r.opId,
                     (const char*)r.hullClass, r.target.GetLength() > 0 ? (const char*)r.target : "the reported position",
                     how);
            EmitAt(registry, r, CAOpStruck, r.strikeZone, r.strikePos, 0, 0, how);
        }
    };
    if (r.phase == CALive)
    {
        CounterAttackAirStatus st;
        const bool known = ops && ops->AirStatus(r.airId, st);
        if (known && st.loiter)
        {
            struck("live");
        }
        if (known && st.loiter && !st.gone)
        {
            // the limited reveal (3.2, D9): fresh sightings only, never an exact fix
            r.revealClock -= elapsed;
            if (r.revealClock <= 0)
            {
                r.revealClock = RevealEvery;
                Vector3 at = VZero;
                float age = 0;
                const int zone = r.strikeZone >= 0 ? r.strikeZone : OccupierForces::ZoneOfPosition(registry, r.strikePos);
                if (ReportedNear(registry, zone, at, &age) && age < RevealMaxAge)
                {
                    ops->AirRevealNear(r.airId, at, age);
                }
            }
        }
        if (known && !st.gone)
        {
            return false;
        }
        r.hullDone = true;
        r.hull = nullptr;
        r.crewGroup = nullptr;
        r.airId = 0;
        if (known && st.hullLost)
        {
            GM_DIRECTOR_INFO(Core, "CounterAttack: [op {}] the strike airframe {} was lost", r.opId, (const char*)r.hullClass);
            OccupierForces::Instance().NoteLoss(r.crew, "air crew lost");
            SettleCrew(registry, r, r.crew, "strike crew lost");
            Emit(registry, r, CAOpHullLost, 0, r.HullPrice(), "strike airframe lost");
            return true;
        }
        CrewHome(registry, view, r.source, r.crew);
        SettleCrew(registry, r, r.crew, "strike crew home");
        Emit(registry, r, CAOpHullRefund, r.crew, r.HullPrice(), "strike airframe home");
        return true;
    }
    if (r.retry > 0)
    {
        r.retry -= elapsed;
    }
    // abstract flight.  The strike goes live when its target is in the
    // player's bubble and the budgets allow an airframe (6.5: a strike is a
    // group and a hull), born unseen at altitude on its line
    if (ops && view.playerValid && InBubbleAt(registry, view, r.strikePos) && r.retry <= 0)
    {
        const ZoneTuning& zt = registry.Tuning();
        const bool overGroups = zt.budgetSideGroups > 0 && view.occGroups + 1 > zt.budgetSideGroups;
        const bool overHulls = LiveHulls() + 1 > _tuning.forcesLiveHullsMax;
        Vector3 birth = VZero;
        const float progress = r.travel > 0 ? 1.0f - r.clock / r.travel : 1.0f;
        if (!overGroups && !overHulls && PickAirBirth(registry, view, r, *ops, r.strikePos, progress, birth))
        {
            Entity* hull = nullptr;
            AIGroup* crewGroup = nullptr;
            const float alt = AirBirthAlt(r.package);
            const int id = ops->AirSortie(r.hullClass, birth, alt, r.strikePos, _tuning.sortieTime, hull, crewGroup);
            if (id > 0)
            {
                r.phase = CALive;
                r.airId = id;
                r.hull = hull;
                r.crewGroup = crewGroup;
                r.spawnPos = birth;
                r.revealClock = 0;
                GM_DIRECTOR_INFO(Core, "CounterAttack: [op {}] strike {} born airborne at [{:.0f}, {:.0f}] ({:.0f} m up)",
                         r.opId, (const char*)r.hullClass, birth.X(), birth.Z(), alt);
                Emit(registry, r, CAOpBorn, 0, 0, "airborne");
                EmitAt(registry, r, CAOpAirborne, r.airOrigin, birth, 0, 0,
                       r.airOrigin >= 0 ? "airfield" : "off-map");
                return false;
            }
            LOG_WARN(Core, "CounterAttack: [op {}] strike airframe {} could not be created; it stays abstract",
                     r.opId, (const char*)r.hullClass);
        }
        r.retry = RetryInterval;
    }
    if (r.clock > 0)
    {
        r.clock -= elapsed;
        if (r.clock > 0)
        {
            return false;
        }
    }
    // abstract: no bodies are killed; the cost is the support and standing
    // loss the director applies on the event (7.3)
    struck("abstract");
    r.hullDone = true;
    CrewHome(registry, view, r.source, r.crew);
    SettleCrew(registry, r, r.crew, "strike crew home");
    Emit(registry, r, CAOpHullRefund, r.crew, r.HullPrice(), "abstract: home");
    return true;
}

// ---------------------------------------------------------------------------
// persistence
// ---------------------------------------------------------------------------

LSError CounterAttack::AirHome::Serialize(ParamArchive& ar)
{
    PARAM_CHECK(ar.Serialize("opId", opId, 1, 0))
    PARAM_CHECK(ar.Serialize("kind", kind, 1, (int)CAKRetake))
    PARAM_CHECK(ar.Serialize("origin", origin, 1, (int)CAOLegacy))
    PARAM_CHECK(ar.Serialize("target", target, 1, RString()))
    PARAM_CHECK(ar.Serialize("source", source, 1, RString()))
    PARAM_CHECK(ar.Serialize("units", units, 1, 0))
    PARAM_CHECK(ar.Serialize("crew", crew, 1, 0))
    PARAM_CHECK(ar.Serialize("airId", airId, 1, 0))
    PARAM_CHECK(ar.Serialize("direct", direct, 1, false))
    return LSOK;
}

} // namespace Guerrilla
} // namespace Poseidon
