#include <Poseidon/Game/Guerrilla/CounterAttack.hpp>

#include <Poseidon/Game/Guerrilla/AlertMachine.hpp> // zone alert states (alarm answers, 8.2)
#include <Poseidon/Game/Guerrilla/CampaignEnd.hpp> // IsMilitaryType
#include <Poseidon/Game/Guerrilla/OccupierAir.hpp> // phase 6A: insertions and sorties
#include <Poseidon/Game/Guerrilla/GarrisonCache.hpp>
#include <Poseidon/Game/Guerrilla/GuerrillaBase.hpp>  // HqZone: the HQ's town is never a target (N4a)
#include <Poseidon/Game/Guerrilla/OccupierForces.hpp> // the pool seams (5.5), pass-through while inactive
#include <Poseidon/Game/Guerrilla/RoadRoute.hpp>      // the road route search (BuildRoute)
#include <Poseidon/Game/Guerrilla/Traffic.hpp>        // the shared perception gate (issue #53)
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>

#include <Poseidon/IO/ParamFileExt.hpp> // Pars / ExtParsMission
#include <Poseidon/IO/Serialization/ParamArchive.hpp>

#include <Poseidon/Core/Application.hpp>         // GApp (ENGINE_CONFIG)
#include <Poseidon/Core/Config/EngineConfig.hpp> // ENGINE_CONFIG.objectsZ / horizontZ (perception gate)

#include <Poseidon/World/World.hpp>
#include <Poseidon/World/Scene/Scene.hpp>                  // GScene (sun darkness: the hull gate, M8)
#include <Poseidon/Graphics/Rendering/Lighting/Lights.hpp> // LightSun::NightEffect
#include <Poseidon/World/Terrain/Landscape.hpp> // surface Y, dry-land test
#include <Poseidon/World/Terrain/Roads.hpp>     // GRoadNet road snap
#include <Poseidon/World/Entities/Infantry/Person.hpp>
#include <Poseidon/World/Entities/Vehicles/Transport.hpp> // ground hulls (3.2)
#include <Poseidon/AI/AI.hpp>
#include <Poseidon/AI/AICore.hpp>
#include <Poseidon/AI/VehicleAI.hpp> // Rank
#include <Poseidon/Network/Network.hpp>

#include <Random/randomGen.hpp>

#include <Poseidon/Foundation/Common/FltOpts.hpp> // toIntFloor / toIntCeil (road survey)
#include <Poseidon/Foundation/Containers/BoolArray.hpp>
#include <Poseidon/Foundation/Framework/DebugLog.hpp>
#include <Poseidon/Foundation/platform.hpp>

#include <math.h>
#include <string.h>

#include <algorithm>
#include <functional>
#include <queue>
#include <unordered_map>
#include <utility>
#include <vector>

// Shared command internals (global namespace, the GarrisonCache idiom):
// createUnit / deleteVehicle without the script-value parsing, and the body of
// the `join` command (GameStateExtGrp.cpp).
void CreateUnit(AIGroup* group, RString type, Vector3Par position, RString init, float skill, Rank rank);
void DeleteVehicle(Entity* veh);
void ProcessJoinGroups(OLinkArray<AIUnit>& units, AIGroup* group);
// the moveIn* body (GameStateExtUi.cpp): seat a soldier, no script parsing
bool NativeMoveIn(Poseidon::Person* soldier, Poseidon::Transport* veh, GetInPosition position);

namespace Poseidon::Guerrilla
{

// Defined in CounterAttackCommands.cpp (keeps the command TU in the link).
void EnsureCounterAttackCommandsLinked();

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
CounterAttack& CounterAttack::Instance()
{
    EnsureCounterAttackCommandsLinked();
    static CounterAttack instance;
    return instance;
}
#pragma clang diagnostic pop

static float Dist2D(float ax, float az, float bx, float bz)
{
    const float dx = ax - bx;
    const float dz = az - bz;
    return sqrtf(dx * dx + dz * dz);
}

static int MinInt(int a, int b)
{
    return a < b ? a : b;
}

static int MaxInt(int a, int b)
{
    return a > b ? a : b;
}

static float Dist2D(Vector3Par a, Vector3Par b)
{
    return Dist2D(a.X(), a.Z(), b.X(), b.Z());
}

static bool IsZeroPos(Vector3Par p)
{
    return p.X() == 0 && p.Z() == 0;
}

static float Clamp01(float v)
{
    return v < 0 ? 0 : (v > 1 ? 1 : v);
}

// the index of the first route point at or past fraction of its length
static int RouteIndexAt(const AutoArray<Vector3>& route, float fraction)
{
    const int n = route.Size();
    if (n == 0)
    {
        return 0;
    }
    const float want = Clamp01(fraction) * CounterAttack::RouteLength(route) - 0.01f;
    float arc = 0;
    for (int i = 1; i < n; i++)
    {
        if (arc >= want)
        {
            return i - 1;
        }
        arc += Dist2D(route[i - 1], route[i]);
    }
    return n - 1;
}

// ---------------------------------------------------------------------------
// lifecycle
// ---------------------------------------------------------------------------

void CounterAttack::Clear()
{
    _tuning = CounterAttackTuning();
    _records.Clear();
    _homeHulls.Clear();
    _leftovers.Clear();
    _accum = 0;
    _townsTaken = 0;
}

void CounterAttack::InitMission()
{
    Clear();
    LoadFromConfig();
}

void CounterAttack::LoadFromConfig()
{
    const ParamEntry* zones = ExtParsMission.FindEntry("CfgGuerrillaZones");
    if (!zones)
    {
        zones = Pars.FindEntry("CfgGuerrillaZones");
    }
    LoadFromParams(zones);
}

// read a key and repair it into [lo, hi] (logged), the town-garrison idiom
static float ReadRanged(const ParamEntry& cfg, const char* key, float def, float lo, float hi)
{
    const float v = cfg.ReadValue(key, def);
    if (v < lo || v > hi)
    {
        const float fixed = v < lo ? lo : hi;
        LOG_WARN(Core, "CounterAttack: {} = {} repaired to {} (range {}..{})", key, (double)v, (double)fixed,
                 (double)lo, (double)hi);
        return fixed;
    }
    return v;
}

void CounterAttack::LoadFromParams(const ParamEntry* zonesCfg)
{
    _tuning = CounterAttackTuning();
    if (!zonesCfg)
    {
        return;
    }
    const ParamEntry& c = *zonesCfg;
    CounterAttackTuning& t = _tuning;
    t.enabled = c.ReadValue("counterAttack", 1.0f) != 0.0f;
    t.size = ReadRanged(c, "counterAttackSize", t.size, 0.0f, 4.0f);
    t.firstTownChance = ReadRanged(c, "counterAttackFirstTownChance", t.firstTownChance, 0.0f, 1.0f);
    t.chance = ReadRanged(c, "counterAttackChance", t.chance, 0.0f, 1.0f);
    t.warLevelStep = ReadRanged(c, "counterAttackWarLevelStep", t.warLevelStep, 0.0f, 2.0f);
    t.delayMin = ReadRanged(c, "counterAttackDelayMin", t.delayMin, 0.0f, 86400.0f);
    t.delayMax = ReadRanged(c, "counterAttackDelayMax", t.delayMax, 0.0f, 86400.0f);
    if (t.delayMax < t.delayMin)
    {
        LOG_WARN(Core, "CounterAttack: counterAttackDelayMax {} below counterAttackDelayMin {}, raised to it",
                 (double)t.delayMax, (double)t.delayMin);
        t.delayMax = t.delayMin;
    }
    t.sourceKeep = ReadRanged(c, "counterAttackSourceKeep", t.sourceKeep, 0.0f, 1.0f);
    t.speed = ReadRanged(c, "counterAttackSpeed", t.speed, 0.5f, 100.0f);
    t.roadFactor = ReadRanged(c, "counterAttackRoadFactor", t.roadFactor, 1.0f, 5.0f);
    t.defenderWeight = ReadRanged(c, "counterAttackDefenderWeight", t.defenderWeight, 0.0f, 100.0f);
    t.repelLoss = ReadRanged(c, "counterAttackRepelLoss", t.repelLoss, 0.0f, 1.0f);
    t.liveTimeout = ReadRanged(c, "counterAttackLiveTimeout", t.liveTimeout, 60.0f, 86400.0f);
    t.maxLive = (int)ReadRanged(c, "counterAttackMaxLive", (float)t.maxLive, 0.0f, 8.0f);
    t.birthMinToTarget = ReadRanged(c, "counterAttackBirthMinToTarget", t.birthMinToTarget, 0.0f, 2000.0f);
    t.birthGiveUp = ReadRanged(c, "counterAttackBirthGiveUp", t.birthGiveUp, 30.0f, 86400.0f);
    // occupier forces (OCCUPIER-FORCES.md 16): the layer key is read here
    // too, so the movement rules know the key without the pool service
    t.forcesOn = c.ReadValue("occupierForces", OccupierForcesTuning::KeyDefault) != 0.0f;
    // forcesOpsMax is OccupierForces' key (one reader, one writer: its
    // tuning and gmForcesTune "opsMax"); OpsCap reads it from there
    t.forcesLiveHullsMax = (int)ReadRanged(c, "forcesLiveHullsMax", (float)t.forcesLiveHullsMax, 0.0f, 20.0f);
    // phase 6A air (16): the strike time over target and the parent
    // nation's map edge (N / S / E / W; absent or anything else = nearest)
    t.sortieTime = ReadRanged(c, "forcesSortieTime", t.sortieTime, 60.0f, 1800.0f);
    t.offMapEdge = -1;
    if (const ParamEntry* edge = c.FindEntry("forcesOffMapEdge"))
    {
        const RString e = (RStringB)*edge;
        const char ch = e.GetLength() > 0 ? ((const char*)e)[0] : 0;
        t.offMapEdge = ch == 'N' || ch == 'n'   ? CAEdgeNorth
                       : ch == 'S' || ch == 's' ? CAEdgeSouth
                       : ch == 'E' || ch == 'e' ? CAEdgeEast
                       : ch == 'W' || ch == 'w' ? CAEdgeWest
                                                : -1;
    }
}

// ---------------------------------------------------------------------------
// pure rules
// ---------------------------------------------------------------------------

float CounterAttack::DispatchChance(const CounterAttackTuning& t, int townsBefore)
{
    if (!t.enabled)
    {
        return 0;
    }
    // the first town ever taken is a probe the occupier may ignore; from the
    // second on it answers every one
    return townsBefore > 0 ? t.chance : t.firstTownChance;
}

int CounterAttack::ForceSize(const CounterAttackTuning& t, int groupSize, float warLevel)
{
    // the war level only ever scales the force up
    float scale = 1.0f + t.warLevelStep * (warLevel - 1.0f);
    if (scale < 1.0f)
    {
        scale = 1.0f;
    }
    const float bodies = t.size * (float)(groupSize > 0 ? groupSize : 1) * scale;
    const int n = (int)floorf(bodies + 0.5f);
    if (n > MaxForce)
    {
        return MaxForce;
    }
    return n < 1 ? 1 : n;
}

CounterAttackDecision CounterAttack::Decide(const CounterAttackTuning& t, const char* /*town*/, int townsBefore,
                                            const CounterAttackContext& ctx)
{
    CounterAttackDecision d;
    d.chance = DispatchChance(t, townsBefore);
    d.roll = Roll01(ctx.seed, 0);
    d.go = d.roll < d.chance;
    d.size = d.go ? ForceSize(t, ctx.groupSize, ctx.warLevel) : 0;
    return d;
}

float CounterAttack::HoldOdds(const CounterAttackTuning& t, int defenders, int attackers)
{
    if (attackers <= 0)
    {
        return 1.0f;
    }
    if (defenders <= 0)
    {
        return 0.0f;
    }
    const float d = (float)defenders * t.defenderWeight;
    return d / ((float)attackers + d);
}

float CounterAttack::TravelTime(const CounterAttackTuning& t, float straightDistance)
{
    // the whole road leg from the base to the town: a live squad appears on
    // that road (unseen) and walks the rest, so nothing is cut off the end
    const float leg = straightDistance > 0 ? straightDistance : 0;
    const float seconds = leg * t.roadFactor / (t.speed > 0 ? t.speed : 1.0f);
    return seconds < MinTravel ? MinTravel : seconds;
}

float CounterAttack::RealSpeed(int package, float maxSpeedKmh)
{
    if (package == FPFoot || maxSpeedKmh <= 0)
    {
        return FootSpeed;
    }
    const bool air = package == FPTransportHeli || package == FPAttackHeli || package == FPJet;
    return (air ? AirCruise : GroundCruise) * maxSpeedKmh / 3.6f;
}

float CounterAttack::TravelTimeFor(const CounterAttackTuning& t, const Record& r, float straightDistance,
                                   float roadLength)
{
    // the test lever, and the legacy pacing with the key off (3.2: the real
    // speeds apply only with occupierForces on)
    if (t.speedLever || (!t.forcesOn && r.abstractSpeed <= 0))
    {
        return TravelTime(t, straightDistance);
    }
    const float speed = r.abstractSpeed > 0 ? r.abstractSpeed : FootSpeed;
    // a real road route already is the road: no roadFactor on top of it;
    // with only the straight distance known, the road factor stands in
    const float leg = roadLength > 0 ? roadLength : (straightDistance > 0 ? straightDistance : 0) * t.roadFactor;
    const float seconds = leg / speed;
    return seconds < MinTravel ? MinTravel : seconds;
}

float CounterAttack::HoldOddsLethal(const CounterAttackTuning& t, int defenders, float attackLethal)
{
    if (attackLethal <= 0)
    {
        return 1.0f;
    }
    if (defenders <= 0)
    {
        return 0.0f;
    }
    const float d = (float)defenders * t.defenderWeight;
    return d / (attackLethal + d);
}

float CounterAttack::RouteLength(const AutoArray<Vector3>& route)
{
    float len = 0;
    for (int i = 1; i < route.Size(); i++)
    {
        len += Dist2D(route[i - 1], route[i]);
    }
    return len;
}

Vector3 CounterAttack::RoutePoint(const AutoArray<Vector3>& route, float fraction)
{
    const int n = route.Size();
    if (n == 0)
    {
        return VZero;
    }
    if (fraction <= 0 || n == 1)
    {
        return route[0];
    }
    const float want = Clamp01(fraction) * RouteLength(route);
    float walked = 0;
    for (int i = 1; i < n; i++)
    {
        const float seg = Dist2D(route[i - 1], route[i]);
        if (seg > 0 && walked + seg >= want)
        {
            const float k = (want - walked) / seg;
            return route[i - 1] + (route[i] - route[i - 1]) * k;
        }
        walked += seg;
    }
    return route[n - 1];
}

void CounterAttack::ResampleRoute(const AutoArray<Vector3>& in, float step, AutoArray<Vector3>& out)
{
    out.Clear();
    if (in.Size() == 0)
    {
        return;
    }
    out.Add(in[0]);
    for (int i = 1; i < in.Size(); i++)
    {
        const Vector3 a = in[i - 1];
        const Vector3 b = in[i];
        const float seg = Dist2D(a, b);
        if (seg < 0.01f)
        {
            continue; // a repeated vertex
        }
        const int parts = step > 0 ? (int)ceilf(seg / step) : 1;
        for (int k = 1; k < parts; k++)
        {
            out.Add(a + (b - a) * ((float)k / (float)parts));
        }
        out.Add(b);
    }
}

CounterAttackSpawnPick CounterAttack::PickSpawnPoint(const AutoArray<Vector3>& route, float progress, bool launch,
                                                     float minToTarget, const CounterAttackGate& gate)
{
    CounterAttackSpawnPick pick; // CASpawnDeferred
    const int n = route.Size();
    if (n == 0)
    {
        return pick;
    }
    const Vector3 town = route[n - 1];
    const float len = RouteLength(route);
    const float min2 = minToTarget * minToTarget;
    // the candidates in search order, with their arc from the base
    AutoArray<Vector3> pts;
    AutoArray<float> arcs;
    AutoArray<float> arcOf;
    arcOf.Resize(n);
    float arc = 0;
    for (int i = 0; i < n; i++)
    {
        if (i > 0)
        {
            arc += Dist2D(route[i - 1], route[i]);
        }
        arcOf[i] = arc;
    }
    if (launch)
    {
        // no progress yet: from the base toward the town
        for (int i = 0; i < n; i++)
        {
            pts.Add(route[i]);
            arcs.Add(arcOf[i]);
        }
    }
    else
    {
        // a force the player walked up to: its progress point, then back
        // toward the base - never ahead of where it has got to, and never
        // further back than it must
        const float at = Clamp01(progress) * len;
        const Vector3 here = RoutePoint(route, Clamp01(progress));
        const float hx = here.X() - town.X();
        const float hz = here.Z() - town.Z();
        if (hx * hx + hz * hz < min2)
        {
            // already at the town edge: it stays there, abstract
            pick.reason = CASpawnHold;
            pick.pos = here;
            pick.fraction = Clamp01(progress);
            return pick;
        }
        pts.Add(here);
        arcs.Add(at);
        for (int i = n - 1; i >= 0; i--)
        {
            if (arcOf[i] < at - 0.01f)
            {
                pts.Add(route[i]);
                arcs.Add(arcOf[i]);
            }
        }
    }
    // the first hidden candidate wins; a point hidden only by the cone counts
    // only near the base, and only when no hidden point exists (Traffic's
    // tiers, issue #53 T2)
    int coneOnly = -1;
    for (int k = 0; k < pts.Size(); k++)
    {
        const float dx = pts[k].X() - town.X();
        const float dz = pts[k].Z() - town.Z();
        if (dx * dx + dz * dz < min2)
        {
            continue; // never born on top of the town
        }
        const int exposure = gate.Exposure(pts[k]);
        if (exposure == CAExposeHidden)
        {
            pick.reason = arcs[k] < BaseReach ? CASpawnBase : CASpawnRoute;
            pick.pos = pts[k];
            pick.fraction = len > 0 ? Clamp01(arcs[k] / len) : 0.0f;
            return pick;
        }
        if (exposure == CAExposeConeOnly && coneOnly < 0 && arcs[k] <= ConeOnlyBaseRadius)
        {
            coneOnly = k;
        }
    }
    if (coneOnly >= 0)
    {
        pick.reason = arcs[coneOnly] < BaseReach ? CASpawnBase : CASpawnRoute;
        pick.pos = pts[coneOnly];
        pick.fraction = len > 0 ? Clamp01(arcs[coneOnly] / len) : 0.0f;
    }
    return pick;
}

float CounterAttack::RouteFractionNear(const AutoArray<Vector3>& route, Vector3Par pos)
{
    const int n = route.Size();
    const float len = RouteLength(route);
    if (n < 2 || len <= 0)
    {
        return 0;
    }
    // the nearest point on any segment, by arc
    float best = -1;
    float bestArc = 0;
    float arc = 0;
    for (int i = 1; i < n; i++)
    {
        const Vector3 a = route[i - 1];
        const Vector3 b = route[i];
        const float seg = Dist2D(a, b);
        float k = 0;
        if (seg > 0)
        {
            k = ((pos.X() - a.X()) * (b.X() - a.X()) + (pos.Z() - a.Z()) * (b.Z() - a.Z())) / (seg * seg);
            k = Clamp01(k);
        }
        const Vector3 q = a + (b - a) * k;
        const float d = Dist2D(q, pos);
        if (best < 0 || d < best)
        {
            best = d;
            bestArc = arc + seg * k;
        }
        arc += seg;
    }
    return Clamp01(bestArc / len);
}

void CounterAttack::ReturnToRoad(Record& r, float fraction, int alive, float wholeTravel)
{
    if (r.travel <= 0)
    {
        r.travel = wholeTravel > MinTravel ? wholeTravel : MinTravel;
    }
    // the arrival clock picks up where the march got to: progress = fraction
    r.phase = CATravel;
    r.strength = alive;
    r.clock = r.travel * (1.0f - Clamp01(fraction));
    r.group = nullptr;
    r.leg = LegUnknown;
    r.wp = 0;
    r.retry = 0;
    r.deferTime = 0;
    r.hiddenLogged = false;
    r.deferLogged = false;
    r.wholeRoad = false;
    r.unseenFor = 0;
    r.arrivalLive = false;
    r.dismounted = false;
    r.stopping = -1;
    r.escortWait = false;
}

bool CounterAttack::RoadIsReasonable(float roadLength, float straightDistance)
{
    return roadLength <= straightDistance * MaxRoadDetour + RoadDetourSlack;
}

float CounterAttack::Progress(const Record& r)
{
    if (r.travel <= 0)
    {
        return 0;
    }
    return Clamp01((r.travel - r.clock) / r.travel);
}

float CounterAttack::MarchAllowance(const AutoArray<Vector3>& route, float fraction)
{
    return RouteLength(route) * (1.0f - Clamp01(fraction)) / MarchSpeed + MarchSlack;
}

const char* CounterAttack::SpawnReasonName(int reason)
{
    switch (reason)
    {
        case CASpawnBase:
            return "base";
        case CASpawnRoute:
            return "route";
        case CASpawnGarrison:
            return "garrison";
        case CASpawnDeferred:
            return "deferred";
        default:
            return "";
    }
}

int CounterAttack::Spare(const CounterAttackTuning& t, float stored, int live, float garrisonMax, bool useLive)
{
    const int storedBodies = (int)floorf(stored > 0 ? stored : 0);
    const int liveBodies = live > 0 ? live : 0;
    const int keep = (int)floorf((garrisonMax > 0 ? garrisonMax : 0) * t.sourceKeep);
    const int available = storedBodies + liveBodies - keep;
    const int pool = useLive ? liveBodies : storedBodies;
    const int spare = MinInt(available, pool);
    return spare > 0 ? spare : 0;
}

// splitmix32-style finalizer over seed and salt: deterministic, platform
// independent, no engine RNG state touched
float CounterAttack::Roll01(unsigned int seed, int salt)
{
    unsigned int x = seed ^ (0x9E3779B9u * (unsigned int)(salt + 1));
    x ^= x >> 16;
    x *= 0x7FEB352Du;
    x ^= x >> 15;
    x *= 0x846CA68Bu;
    x ^= x >> 16;
    return (float)(x >> 8) / 16777216.0f;
}

unsigned int CounterAttack::MakeSeed(const char* town, int flipDay, int flipMinute, int townsBefore)
{
    // FNV-1a over the town name, then the flip stamp and k (towns taken before)
    unsigned int h = 2166136261u;
    for (const char* p = town ? town : ""; *p; p++)
    {
        h ^= (unsigned char)*p;
        h *= 16777619u;
    }
    const unsigned int stamp = (unsigned int)(flipDay * 1440 + flipMinute);
    h ^= stamp * 0x85EBCA6Bu;
    h *= 16777619u;
    h ^= (unsigned int)townsBefore * 0xC2B2AE35u;
    h *= 16777619u;
    return h;
}

const char* CounterAttack::PhaseName(int phase)
{
    switch (phase)
    {
        case CAScheduled:
            return "SCHEDULED";
        case CATravel:
            return "TRAVEL";
        case CAHolding:
            return "HOLDING";
        case CALive:
            return "LIVE";
        default:
            return "";
    }
}

// ---------------------------------------------------------------------------
// the service
// ---------------------------------------------------------------------------

int CounterAttack::FindByTarget(const char* town) const
{
    for (int i = 0; i < _records.Size(); i++)
    {
        // an HQ_ASSAULT is about the HQ, not the zone it stands in: it never
        // occupies the zone's one-record slot (FindHqAssault finds it)
        if (_records[i].kind != CAKHqAssault && _records[i].kind != CAKStrike && stricmp(_records[i].target, town) == 0)
        {
            return i;
        }
    }
    return -1;
}

int CounterAttack::FindHqAssault() const
{
    for (int i = 0; i < _records.Size(); i++)
    {
        if (_records[i].kind == CAKHqAssault)
        {
            return i;
        }
    }
    return -1;
}

int CounterAttack::WaveRecords() const
{
    int n = 0;
    for (int i = 0; i < _records.Size(); i++)
    {
        n += _records[i].origin == CAOWave ? 1 : 0;
    }
    return n;
}

Vector3 CounterAttack::DestOf(const ZoneRegistry& registry, const Record& r)
{
    if (r.destPos.X() != 0 || r.destPos.Z() != 0)
    {
        return r.destPos;
    }
    const ZoneRecord* z = registry.GetZone(registry.FindZoneIndex(r.target));
    return z ? z->pos : VZero;
}

int CounterAttack::HqAssaultPresence(Vector3Par hqPos, float radius, CounterAttackOps* ops) const
{
    int bodies = 0;
    const float rSq = radius * radius;
    for (int i = 0; i < _records.Size(); i++)
    {
        const Record& r = _records[i];
        if (r.kind != CAKHqAssault)
        {
            continue; // D14: passers-by, columns and every other kind never count
        }
        if (r.phase == CALive)
        {
            AIGroup* grp = r.group;
            bodies += ops && grp ? ops->AliveWithin(grp, hqPos, radius) : 0;
        }
        else if (r.phase == CAHolding && !r.suspended)
        {
            const float dx = r.destPos.X() - hqPos.X();
            const float dz = r.destPos.Z() - hqPos.Z();
            bodies += dx * dx + dz * dz <= rSq ? r.strength : 0;
        }
    }
    return bodies;
}

int CounterAttack::FindByOpId(int opId) const
{
    for (int i = 0; i < _records.Size(); i++)
    {
        if (_records[i].opId == opId)
        {
            return i;
        }
    }
    return -1;
}

int CounterAttack::LiveHullOf(const Record& r)
{
    return r.hull.GetLink() != nullptr ? 1 : 0;
}

int CounterAttack::LiveEscortOf(const Record& r)
{
    return r.escort.GetLink() != nullptr ? 1 : 0;
}

int CounterAttack::BornOps(bool includeWave) const
{
    int n = 0;
    for (int i = 0; i < _records.Size(); i++)
    {
        const Record& r = _records[i];
        if (!includeWave && r.origin == CAOWave)
        {
            continue;
        }
        n += r.funded && (r.phase == CALive || LiveHullOf(r) + LiveEscortOf(r) > 0) ? 1 : 0;
    }
    return n;
}

int CounterAttack::OpsCap() const
{
    // the one forcesOpsMax: OccupierForces' while it is active; the local
    // default when this service runs on its own (the key off, unit tests)
    const OccupierForces& of = OccupierForces::Instance();
    const int opsMax = of.Active() ? of.Tuning().opsMax : _tuning.forcesOpsMax;
    return opsMax + WaveRecords();
}

int CounterAttack::LiveHulls() const
{
    int n = 0;
    for (int i = 0; i < _records.Size(); i++)
    {
        n += LiveHullOf(_records[i]) + LiveEscortOf(_records[i]);
    }
    for (int i = 0; i < _homeHulls.Size(); i++)
    {
        n += _homeHulls[i].hull.GetLink() != nullptr ? 1 : 0;
    }
    return n + _airHomes.Size(); // phase 6A: airframes flying home
}

int CounterAttack::SettlePresence(const ZoneRegistry& registry, int zoneIndex) const
{
    const ZoneRecord* z = registry.GetZone(zoneIndex);
    if (!z || !_tuning.forcesOn)
    {
        return 0;
    }
    int bodies = 0;
    for (int i = 0; i < _records.Size(); i++)
    {
        // a holding force (suspended in the player's bubble or not) is the
        // town's (or base's) new garrison when it reverts (5.4); its bodies
        // are not lost.  An HQ_ASSAULT holds the HQ, not the zone: it never
        // settles there.
        if (_records[i].phase == CAHolding && _records[i].kind != CAKHqAssault &&
            stricmp(_records[i].target, z->name) == 0)
        {
            bodies += _records[i].strength;
        }
    }
    return bodies;
}

// ---------------------------------------------------------------------------
// the director's ear
// ---------------------------------------------------------------------------

namespace
{
// constant-initialized (no global constructor): the listener and the queue
// guard; the queue itself lives in a function-local static
CounterAttackListener* GListener = nullptr;
bool GDispatching = false;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
AutoArray<CounterAttackOpEvent>& EventQueue()
{
    static AutoArray<CounterAttackOpEvent> queue;
    return queue;
}
#pragma clang diagnostic pop

// the events queued while a listener call ran, in order
void DrainEvents()
{
    if (GDispatching || !GListener)
    {
        return;
    }
    GDispatching = true;
    for (int i = 0; i < EventQueue().Size(); i++)
    {
        const CounterAttackOpEvent next = EventQueue()[i];
        if (GListener)
        {
            GListener->OnCounterAttackOp(next);
        }
    }
    EventQueue().Clear();
    GDispatching = false;
}
} // namespace

void CounterAttack::SetListener(CounterAttackListener* listener)
{
    GListener = listener;
}

CounterAttackListener* CounterAttack::Listener()
{
    return GListener;
}

void CounterAttack::Dispatch(const CounterAttackOpEvent& ev)
{
    if (!GListener)
    {
        return;
    }
    if (GDispatching)
    {
        // never re-enter the listener: an event raised from inside a
        // listener call waits for the outer call to return
        EventQueue().Add(ev);
        return;
    }
    GDispatching = true;
    GListener->OnCounterAttackOp(ev);
    GDispatching = false;
    DrainEvents();
}

void CounterAttack::Emit(const ZoneRegistry& registry, const Record& r, int type, int bodies, int units,
                         const char* note)
{
    if (!GListener)
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
    Dispatch(ev);
}

int CounterAttack::AbstractPresence(const ZoneRegistry& registry, int zoneIndex) const
{
    const ZoneRecord* z = registry.GetZone(zoneIndex);
    if (!z)
    {
        return 0;
    }
    int bodies = 0;
    for (int i = 0; i < _records.Size(); i++)
    {
        // a suspended force (its town in the player's bubble) does nothing
        // to the town until the player leaves.  Every kind counts: a BASE
        // force holding a base works its occCapture, and an HQ_ASSAULT
        // holding at the HQ is occupier presence in the zone the HQ stands
        // in (OCCUPIER-FORCES.md 7.2, 8.1)
        if (_records[i].phase == CAHolding && !_records[i].suspended && stricmp(_records[i].target, z->name) == 0)
        {
            bodies += _records[i].strength;
        }
    }
    return bodies;
}

void CounterAttack::ClampScheduled(float maxDelay)
{
    for (int i = 0; i < _records.Size(); i++)
    {
        if (_records[i].phase == CAScheduled && _records[i].clock > maxDelay)
        {
            _records[i].clock = maxDelay;
        }
    }
}

int CounterAttack::LiveSquads() const
{
    int n = 0;
    for (int i = 0; i < _records.Size(); i++)
    {
        n += _records[i].phase == CALive ? 1 : 0;
    }
    return n;
}

int CounterAttack::HeldTowns(const ZoneRegistry& registry)
{
    int n = 0;
    for (int i = 0; i < registry.NZones(); i++)
    {
        const ZoneRecord* z = registry.GetZone(i);
        if (z && !z->garrisonExempt && stricmp(z->type, "CITY") == 0 &&
            stricmp(z->owner, registry.ResistanceSide()) == 0)
        {
            n++;
        }
    }
    return n;
}

bool CounterAttack::OnTownFlipped(const ZoneRegistry& registry, int zoneIndex, int groupSize, float warLevel,
                                  int hqZone)
{
    if (!_tuning.enabled || CampaignEnd::Instance().Won())
    {
        return false; // off, or the campaign is won (7.4: nothing launches)
    }
    const ZoneRecord* z = registry.GetZone(zoneIndex);
    if (!z || stricmp(z->type, "CITY") != 0 || stricmp(z->owner, registry.ResistanceSide()) != 0)
    {
        return false;
    }
    if (z->garrisonExempt)
    {
        // the start-town headquarters: never a target (it has no garrison
        // to come back to, and the HQ is the campaign's anchor)
        return false;
    }
    // k: the towns taken before this one, the most of the saved count and the
    // towns held now (this town already counts as held at the event)
    int held = HeldTowns(registry) - 1;
    if (held < 0)
    {
        held = 0;
    }
    const int before = _townsTaken > held ? _townsTaken : held;
    _townsTaken = before + 1;
    // the AWAKE hook (OCCUPIER-FORCES.md 6.2, 14 item 4): the director hears
    // the flip first; when it takes it, the flip is counted but no legacy
    // answer is scheduled.  Not re-entrant: a flip raised from inside a
    // listener call (never expected) gets the legacy answer.
    if (GListener && !GDispatching)
    {
        GDispatching = true;
        const bool taken = GListener->OnTownFlipped(zoneIndex, before);
        GDispatching = false;
        DrainEvents();
        if (taken)
        {
            LOG_INFO(Core, "CounterAttack: {} rose; the occupier director takes it ({} town(s) taken before)",
                     (const char*)z->name, before);
            return false;
        }
    }
    if (zoneIndex == hqZone && hqZone >= 0)
    {
        // the town the player's HQ stands in (N4a): never the object of a
        // flip answer before the director exists.  The flip still counts as
        // a town taken (only the target is skipped).
        LOG_INFO(Core, "CounterAttack: {} rose; it holds the player's headquarters, no answer", (const char*)z->name);
        return false;
    }
    if (FindByTarget(z->name) >= 0)
    {
        return false; // one record per town
    }
    Record r;
    r.target = z->name;
    r.townsBefore = before;
    r.seed = MakeSeed(z->name, z->flipDay, z->flipMinute, before);
    CounterAttackContext ctx;
    ctx.groupSize = groupSize;
    ctx.warLevel = warLevel;
    ctx.seed = r.seed;
    const CounterAttackDecision d = Decide(_tuning, z->name, before, ctx);
    if (!d.go)
    {
        LOG_INFO(Core, "CounterAttack: {} rose; the occupier does not answer (chance {:.2f}, {} town(s) taken before)",
                 (const char*)z->name, d.chance, before);
        return false;
    }
    r.size = d.size;
    r.phase = CAScheduled;
    r.clock = _tuning.delayMin + (_tuning.delayMax - _tuning.delayMin) * Roll01(r.seed, 1);
    _records.Add(r);
    LOG_INFO(
        Core,
        "CounterAttack: {} rose; a force of {} will move on it in {:.0f}s (chance {:.2f}, {} town(s) taken before, "
        "war level {:.0f})",
        (const char*)z->name, r.size, r.clock, d.chance, before, warLevel);
    return true;
}

bool CounterAttack::LaunchFrom(ZoneRegistry& registry, const CounterAttackView& view, int target, int source, int size,
                               CounterAttackOps* ops, AutoArray<ZoneEventRecord>& fired)
{
    const ZoneRecord* town = registry.GetZone(target);
    const ZoneRecord* src = registry.GetZone(source);
    if (!_tuning.enabled || view.won || !town || !src || size < 1 || stricmp(town->type, "CITY") != 0 ||
        stricmp(town->owner, registry.ResistanceSide()) != 0 || town->garrisonExempt || target == view.hqZone ||
        stricmp(src->owner, registry.OccupierSide()) != 0 || FindByTarget(town->name) >= 0)
    {
        return false;
    }
    // a legacy order (CAKRetake, CAOLegacy, foot, opId 0, unfunded)
    Record r;
    r.target = town->name;
    r.townsBefore = _townsTaken;
    r.seed = MakeSeed(town->name, town->flipDay, town->flipMinute, _townsTaken) ^ 0x5A17C0DEu;
    r.size = size;
    return LaunchCore(registry, view, target, source, size, r, ops, fired);
}

bool CounterAttack::LaunchFunded(ZoneRegistry& registry, const CounterAttackView& view, const CounterAttackOrder& order,
                                 CounterAttackOps* ops, AutoArray<ZoneEventRecord>& fired)
{
    const ZoneRecord* town = registry.GetZone(order.target);
    const ZoneRecord* src = registry.GetZone(order.source);
    const bool hq = order.kind == CAKHqAssault;
    const Vector3 hqDest = order.destPos.X() != 0 || order.destPos.Z() != 0 ? order.destPos : view.hqPos;
    // wave operations come on top of the director's slots (7.1)
    const int opsCap = OpsCap() + (order.origin == CAOWave ? 1 : 0);
    const char* why = nullptr;
    if (view.won || view.lost)
    {
        why = view.won ? "the campaign is won" : "the campaign is lost";
    }
    else if (order.kind == CAKRetake)
    {
        if (!town || stricmp(town->type, "CITY") != 0 || stricmp(town->owner, registry.ResistanceSide()) != 0 ||
            town->garrisonExempt)
        {
            why = "the target is not a held town";
        }
        else if (order.target == view.hqZone)
        {
            why = "the target holds the player's headquarters";
        }
    }
    else if (order.kind == CAKBase)
    {
        if (!town || !CampaignEnd::IsMilitaryType(town->type) || stricmp(town->owner, registry.ResistanceSide()) != 0)
        {
            why = "the target is not a held base";
        }
    }
    else if (hq)
    {
        if (!view.hqEstablished || (hqDest.X() == 0 && hqDest.Z() == 0))
        {
            why = "there is no HQ to assault";
        }
        else if (FindHqAssault() >= 0)
        {
            why = "an HQ assault is already under way";
        }
        else if (order.target >= 0 && !town)
        {
            why = "the HQ zone is unknown";
        }
    }
    else if (order.kind == CAKReinforce)
    {
        if (!town || town->garrisonExempt || !GarrisonCache::OccupierHolds(registry, *town) ||
            order.target == order.source)
        {
            why = "the target is not an occupier-held post to top up";
        }
    }
    else if (order.kind == CAKAlarm)
    {
        // 8.2: the answered zone is occupier-held and is never the source;
        // one alarm answer at a time
        const ZoneRecord* alarm = registry.GetZone(order.alarmZone >= 0 ? order.alarmZone : order.target);
        if (!alarm || !GarrisonCache::OccupierHolds(registry, *alarm))
        {
            why = "the alarm zone is not occupier-held";
        }
        else if ((order.alarmZone >= 0 ? order.alarmZone : order.target) == order.source)
        {
            why = "the zone under attack never answers its own alarm";
        }
        else if (FindAlarm() >= 0)
        {
            why = "an alarm answer is already under way";
        }
        else
        {
            town = alarm;
        }
    }
    else if (order.kind == CAKStrike)
    {
        why = StrikeRefusal(registry, order);
    }
    else
    {
        why = "unknown operation kind";
    }
    if (!why && order.IsAirPackage() && order.airOrigin >= 0)
    {
        const ZoneRecord* field = registry.GetZone(order.airOrigin);
        if (!field || stricmp(field->type, "AIRFIELD") != 0 || stricmp(field->owner, registry.OccupierSide()) != 0)
        {
            why = "the air origin is not an occupier-held airfield";
        }
    }
    if (!why && order.IsAirPackage() && order.hullClass.GetLength() == 0)
    {
        why = "an air package names no airframe";
    }
    if (!why && order.kind == CAKStrike)
    {
        if (BornOps() >= opsCap)
        {
            why = "forcesOpsMax born operations are in flight";
        }
        else if (FindStrikeOn(order) >= 0)
        {
            why = "a strike on that target is already under way";
        }
    }
    if (!why && order.kind == CAKStrike)
    {
        return LaunchStrike(registry, view, order);
    }
    if (why)
    {
        LOG_INFO(Core, "CounterAttack: [op {}] refused: {}", order.opId, why);
        return false;
    }
    if (!hq && FindByTarget(town->name) >= 0)
    {
        why = "the target already has an operation";
    }
    else if (!why && (!src || stricmp(src->owner, registry.OccupierSide()) != 0))
    {
        why = "the source is not occupier-held";
    }
    else if (!why && order.bodies < 1)
    {
        why = "no bodies ordered";
    }
    else if (!why && BornOps() >= opsCap)
    {
        why = "forcesOpsMax born operations are in flight";
    }
    if (why)
    {
        LOG_INFO(Core, "CounterAttack: [op {}] refused: {}", order.opId, why);
        return false;
    }
    Record r;
    r.target = town ? town->name : RString();
    r.townsBefore = _townsTaken;
    r.seed = MakeSeed(town ? (const char*)town->name : "HQ", town ? town->flipDay : 0, town ? town->flipMinute : 0,
                      _townsTaken) ^
             (0x0F0F1E5Du + (unsigned)order.opId);
    r.destPos = hq ? hqDest : order.destPos;
    r.size = order.bodies;
    r.opId = order.opId;
    r.kind = order.kind;
    r.origin = order.origin;
    r.package = order.package;
    r.priceUnits = order.priceUnits > 0 ? order.priceUnits : 0;
    r.hullClass = order.package != FPFoot ? order.hullClass : RString();
    r.crew = r.hullClass.GetLength() > 0 ? (order.crew > 0 ? order.crew : 1) : 0;
    // a column's escort (6.5, phase 5): its share comes off the carrier's
    // price, so each hull refunds its own
    r.escortClass = r.hullClass.GetLength() > 0 ? order.escortClass : RString();
    r.escortCrew = r.escortClass.GetLength() > 0 ? (order.escortCrew > 0 ? order.escortCrew : 1) : 0;
    r.escortPriceUnits =
        r.escortClass.GetLength() > 0 ? MinInt(order.escortPriceUnits > 0 ? order.escortPriceUnits : 0, r.priceUnits) : 0;
    r.priceUnits -= r.escortPriceUnits;
    r.urgent = order.urgent;
    r.funded = order.funded;
    r.abstractSpeed = order.abstractSpeed > 0 ? order.abstractSpeed : 0;
    r.lethality = order.lethality > 0 ? order.lethality : 1.0f;
    // phase 6A: the air origin, the alarm's zone and reported position
    r.airOrigin = r.IsAir() ? order.airOrigin : -1;
    if (r.IsAir())
    {
        // the straight flight line starts at the airfield or the off-map edge
        const ZoneRecord* t0 = registry.GetZone(order.kind == CAKAlarm && order.alarmZone >= 0 ? order.alarmZone
                                                                                               : order.target);
        const Vector3 dest0 = !IsZeroPos(r.destPos) ? r.destPos : (t0 ? t0->pos : VZero);
        r.airFrom = AirOriginPos(registry, view, r, dest0);
    }
    const int targetIdx =
        order.kind == CAKAlarm ? (order.alarmZone >= 0 ? order.alarmZone : order.target) : order.target;
    if (order.kind == CAKAlarm)
    {
        r.alarmZone = targetIdx;
        r.alarmPos = order.alarmPos;
        // 8.2: it travels to the reported position and resolves there
        if (order.alarmPos.X() != 0 || order.alarmPos.Z() != 0)
        {
            r.destPos = order.alarmPos;
        }
    }
    return LaunchCore(registry, view, targetIdx, order.source, order.bodies, r, ops, fired);
}

bool CounterAttack::LaunchCore(ZoneRegistry& registry, const CounterAttackView& view, int target, int source, int size,
                               Record& r, CounterAttackOps* ops, AutoArray<ZoneEventRecord>& fired)
{
    const ZoneRecord* src = registry.GetZone(source);
    if (!src)
    {
        return false;
    }
    const int liveBodies = source < view.garrisonLive.Size() ? view.garrisonLive[source] : 0;
    // a foot force with its town in the bubble marches the live garrison
    // out; a hull package always leaves from the stored reserve (its hull is
    // born unseen on the road, never in a watched motor pool)
    const int opsCap = OpsCap() + (r.origin == CAOWave ? 1 : 0);
    // phase 5 (6.5): a hull package whose stored reserve cannot pay its
    // bodies goes on foot when the source can march them out live
    // (ForcePackages::MustDowngrade); the pool price goes back once the
    // record exists
    const bool liveFoot = ops != nullptr && liveBodies > 0 && InBubbleAt(registry, view, DestOf(registry, r)) &&
                          RoomLive(registry, view) && (!r.funded || BornOps() < opsCap);
    const int stored = Spare(_tuning, src->garrison, liveBodies, src->garrisonMax, false);
    const int marched = liveFoot ? Spare(_tuning, src->garrison, liveBodies, src->garrisonMax, true) : 0;
    int hullRefund = 0;
    if (r.HasHullPackage() && ForcePackages::MustDowngrade(r.package, size, stored, liveFoot ? marched : stored))
    {
        LOG_INFO(Core, "CounterAttack: [op {}] {} can spare {} stored of the {} bodies for the {}: on foot", r.opId,
                 (const char*)src->name, stored, size, (const char*)r.hullClass);
        hullRefund = r.priceUnits + r.escortPriceUnits; // the whole column: carrier and escort
        r.package = FPFoot;
        r.hullClass = RString();
        r.crew = 0;
        r.priceUnits = 0;
        r.escortClass = RString();
        r.escortCrew = 0;
        r.escortPriceUnits = 0;
        r.abstractSpeed = r.abstractSpeed > 0 ? FootSpeed : 0;
    }
    // M1: a funded order is granted from the STORED reserve, exactly what
    // the director quoted (FundedSpare); its live garrison marches out only
    // when the stored reserve cannot pay the order whole and the march can
    // (a downgraded hull package, a test lever).  A legacy order marches
    // the live garrison whenever its town is in the bubble, as before.
    const bool live = !r.HasHullPackage() && liveFoot && (!r.funded || (stored < size && marched >= size));
    const int spare = live ? marched : stored;
    const int n = MinInt(size, spare);
    if (n < 1 || (r.funded && n < size))
    {
        if (r.opId != 0)
        {
            LOG_INFO(Core, "CounterAttack: [op {}] refused: {} can spare {} of the {} ordered ({} stored, {} live)",
                     r.opId, (const char*)src->name, n, size, src->garrison, liveBodies);
        }
        return false;
    }
    if (!Launch(registry, target, source, n, live, ops, r, fired))
    {
        return false;
    }
    r.wholeRoad = r.phase == CATravel && InBubbleAt(registry, view, DestOf(registry, r));
    _records.Add(r);
    const Record& added = _records[_records.Size() - 1];
    Emit(registry, added, CAOpLaunched, added.strength, added.priceUnits, live ? "live" : "abstract");
    if (hullRefund > 0)
    {
        Emit(registry, added, CAOpHullRefund, 0, hullRefund, "no stored spare: on foot");
    }
    if (added.phase == CALive)
    {
        Emit(registry, added, CAOpBorn, added.strength, 0, SpawnReasonName(added.spawnReason));
    }
    return true;
}

bool CounterAttack::InBubbleAt(const ZoneRegistry& registry, const CounterAttackView& view, Vector3Par pos)
{
    if (!view.playerValid || (pos.X() == 0 && pos.Z() == 0))
    {
        return false;
    }
    return Dist2D(pos.X(), pos.Z(), view.playerX, view.playerZ) <= registry.Tuning().cacheRadius;
}

bool CounterAttack::InBubble(const ZoneRegistry& registry, const CounterAttackView& view, int zone) const
{
    const ZoneRecord* z = registry.GetZone(zone);
    if (!z || !view.playerValid)
    {
        return false;
    }
    return Dist2D(z->pos.X(), z->pos.Z(), view.playerX, view.playerZ) <= registry.Tuning().cacheRadius;
}

int CounterAttack::PickSource(const ZoneRegistry& registry, const CounterAttackView& view, int target, bool allowLive,
                              int& spare, bool& live) const
{
    const ZoneRecord* town = registry.GetZone(target);
    spare = 0;
    live = false;
    if (!town)
    {
        return -1;
    }
    int best = -1;
    float bestDist = 0;
    for (int i = 0; i < registry.NZones(); i++)
    {
        const ZoneRecord* z = registry.GetZone(i);
        if (!z || !CampaignEnd::IsMilitaryType(z->type) || stricmp(z->owner, registry.OccupierSide()) != 0)
        {
            continue;
        }
        // a ground force walks or drives: a base on another landmass (Malden's
        // Southeast Compound, an island of its own) never sources one
        if (!RoadRoute::SameLandmass(z->pos, town->pos))
        {
            continue;
        }
        const int liveBodies = i < view.garrisonLive.Size() ? view.garrisonLive[i] : 0;
        const bool useLive = allowLive && liveBodies > 0;
        const int s = Spare(_tuning, z->garrison, liveBodies, z->garrisonMax, useLive);
        if (s < 1)
        {
            continue;
        }
        const float d = Dist2D(z->pos.X(), z->pos.Z(), town->pos.X(), town->pos.Z());
        if (best < 0 || d < bestDist)
        {
            best = i;
            bestDist = d;
            spare = s;
            live = useLive;
        }
    }
    return best;
}

const AutoArray<Vector3>& CounterAttack::RouteOf(const ZoneRegistry& registry, Record& r, CounterAttackOps* ops)
{
    if (r.route.Size() > 0)
    {
        return r.route;
    }
    const Vector3 dest = DestOf(registry, r);
    const Vector3 airStart = !IsZeroPos(r.airLz) ? r.airLz : (!r.hullDone ? r.airFrom : VZero);
    if (r.IsAir() && (dest.X() != 0 || dest.Z() != 0) && !IsZeroPos(airStart))
    {
        // phase 6A: air flies the straight line from its origin (3.2, set
        // at launch: airFrom); an inserted squad walks the straight line
        // from its LZ
        AutoArray<Vector3> line;
        line.Add(airStart);
        line.Add(dest);
        r.routeIsRoad = false;
        ResampleRoute(line, RouteStep, r.route);
        return r.route;
    }
    const ZoneRecord* src = registry.GetZone(registry.FindZoneIndex(r.source));
    if (!src || (dest.X() == 0 && dest.Z() == 0))
    {
        return r.route;
    }
    AutoArray<Vector3> raw;
    if (ops)
    {
        ops->BuildRoute(src->pos, dest, raw);
    }
    const float straight = Dist2D(src->pos, dest);
    if (raw.Size() >= 2 && !RoadIsReasonable(RouteLength(raw), straight))
    {
        // men on foot do not walk a road that winds this far round: the
        // straight line instead
        LOG_INFO(Core, "CounterAttack: the road from {} to {} is {:.0f} m for {:.0f} m straight; cross-country",
                 (const char*)r.source, (const char*)r.target, RouteLength(raw), straight);
        raw.Clear();
    }
    r.routeIsRoad = raw.Size() >= 2;
    if (raw.Size() < 2)
    {
        // no road net (or no connection): the straight line, base to town
        raw.Clear();
        raw.Add(src->pos);
        raw.Add(dest);
    }
    ResampleRoute(raw, RouteStep, r.route);
    return r.route;
}

bool CounterAttack::MustGoLive(const ZoneRegistry& registry, const CounterAttackView& view, int target, Record& r,
                               CounterAttackOps* ops) const
{
    if (!view.playerValid)
    {
        return false;
    }
    (void)target;
    if (InBubbleAt(registry, view, DestOf(registry, r)))
    {
        return true;
    }
    // the player stands on the road ahead of (or at) the force
    const Vector3 at = RoutePoint(RouteOf(registry, r, ops), Progress(r));
    return Dist2D(at.X(), at.Z(), view.playerX, view.playerZ) <= registry.Tuning().cacheRadius;
}

// the town edge (zoneArea from dest) on the line from dest to pos: where an
// armed hull supports (6.5, phase 5); pos itself when already inside
static Vector3 EdgePoint(float edgeR, Vector3Par pos, Vector3Par dest)
{
    Vector3 back = pos - dest;
    back[1] = 0;
    const float d = back.Size();
    return d > edgeR && d > 1e-3f ? dest + back * (edgeR / d) : Vector3(pos);
}

void CounterAttack::March(const ZoneRegistry& registry, int target, Record& r, CounterAttackOps* ops, Vector3Par leader,
                          float elapsed)
{
    (void)target;
    const Vector3 dest = DestOf(registry, r);
    if ((dest.X() == 0 && dest.Z() == 0) || !ops)
    {
        return;
    }
    const AutoArray<Vector3>& route = RouteOf(registry, r, ops);
    const int n = route.Size();
    // a mounted force rides to counterAttackBirthMinToTarget and dismounts
    // there (3.2); men on foot deploy at the zone edge plus TownEdgeMargin
    const bool mounted = LiveHullOf(r) > 0 && !r.dismounted;
    const float edge = mounted ? _tuning.birthMinToTarget : registry.Tuning().zoneArea + TownEdgeMargin;
    if (n == 0 || Dist2D(leader, dest) <= edge)
    {
        // the town edge: deploy and move in; the live timeout starts now
        r.leg = LegTown;
        if (r.clock < 0)
        {
            r.clock = 0;
        }
        r.resend = ResendInterval;
        if (mounted)
        {
            // the hull stops first: a cargo seat ejects its man with the
            // hull's speed, and at road speed that kills him
            if (r.stopping < 0)
            {
                r.stopping = 0;
                ops->DriveOn(r.crewGroup, leader);
                if (LiveEscortOf(r) > 0)
                {
                    // the escort does not overtake into the town: it closes
                    // up to the town edge and supports from there (6.5)
                    ops->Support(r.escortCrewGroup, EdgePoint(registry.Tuning().zoneArea, leader, dest));
                }
                r.leg = LegRoad;
                return;
            }
            r.stopping += elapsed;
            if (ops->HullSpeed(r.hull) > DismountSpeed && r.stopping < StopMax)
            {
                r.leg = LegRoad;
                return;
            }
            // stopped: the infantry gets out and assaults; a truck holds
            // back where it stands, an armed hull drives up to the town
            // edge and supports (ForcePackages::EdgeRole, phase 5)
            ops->Dismount(r.group, r.hull);
            r.dismounted = true;
            r.stopping = -1;
            if (ForcePackages::SupportsAtEdge(r.package))
            {
                ops->Support(r.crewGroup, EdgePoint(registry.Tuning().zoneArea, leader, dest));
            }
        }
        if (LiveEscortOf(r) > 0)
        {
            // a column's escort always supports (6.5)
            ops->Support(r.escortCrewGroup, EdgePoint(registry.Tuning().zoneArea, leader, dest));
        }
        ops->Assault(r.group, dest);
        LOG_INFO(Core, "CounterAttack: the force on {} reached the town edge ({:.0f} m out){}; it moves in",
                 r.kind == CAKHqAssault ? "the HQ" : (const char*)r.target, Dist2D(leader, dest),
                 mounted ? " and dismounts" : "");
        Emit(registry, r, CAOpArrived, r.strength, 0, mounted ? "live, dismounted" : "live");
        return;
    }
    // a column keeps together (6.5): the carrier waits where it is while its
    // escort (a slower tank, say) is more than EscortSpread behind, and
    // drives on once it has closed to EscortRejoin
    if (mounted && LiveEscortOf(r) > 0)
    {
        Vector3 escortAt = VZero;
        if (ops->Alive(r.escortCrewGroup, escortAt) > 0)
        {
            const float spread = Dist2D(escortAt, leader);
            if (!r.escortWait && spread > EscortSpread)
            {
                r.escortWait = true;
                ops->DriveOn(r.crewGroup, leader);
                ops->DriveOn(r.escortCrewGroup, leader);
            }
            else if (r.escortWait && spread <= EscortRejoin)
            {
                r.escortWait = false;
                r.resend = 0; // the leg is re-sent below
            }
            if (r.escortWait)
            {
                return;
            }
        }
        else
        {
            r.escortWait = false;
        }
    }
    // a hull covers ground fast: longer legs, so it does not stop at every
    // 200 m point
    const int stride = mounted ? 3 * WaypointStride : WaypointStride;
    // the route point nearest the leader, searched around the current leg
    // (the whole road after a reload)
    const bool known = r.leg == LegRoad;
    const int lo = known ? MaxInt(0, r.wp - 2 * stride) : 0;
    const int hi = known ? MinInt(n - 1, r.wp + 2 * stride) : n - 1;
    int at = lo;
    float atD = -1;
    for (int i = lo; i <= hi; i++)
    {
        const float d = Dist2D(leader, route[i]);
        if (atD < 0 || d < atD)
        {
            atD = d;
            at = i;
        }
    }
    bool issue = false;
    if (!known)
    {
        r.leg = LegRoad;
        r.wp = MinInt(at + stride, n - 1);
        issue = true;
    }
    else if (r.wp >= 0 && r.wp < n && (at >= r.wp - 1 || Dist2D(leader, route[r.wp]) < WaypointReach))
    {
        // this leg is done: the next one ends WaypointStride points on
        const int next = MinInt(at + stride, n - 1);
        if (next != r.wp)
        {
            r.wp = next;
            issue = true;
        }
    }
    r.resend -= elapsed;
    if (r.resend <= 0)
    {
        issue = true; // a stalled leg is re-sent
    }
    if (issue && r.wp >= 0 && r.wp < n)
    {
        r.resend = ResendInterval;
        if (mounted)
        {
            ops->DriveOn(r.crewGroup, route[r.wp]); // the Traffic idiom: the driver, never an arcade waypoint
        }
        else
        {
            ops->MoveOn(r.group, route[r.wp]);
        }
        if (LiveEscortOf(r) > 0)
        {
            ops->DriveOn(r.escortCrewGroup, route[r.wp]); // the escort drives the same legs (6.5)
        }
    }
}

void CounterAttack::ReturnToSource(ZoneRegistry& registry, const CounterAttackView& view, const Record& r, int bodies)
{
    if (bodies <= 0)
    {
        return;
    }
    const int src = registry.FindZoneIndex(r.source);
    ZoneRecord* s = registry.GetZoneMutable(src);
    if (!s || stricmp(s->owner, registry.OccupierSide()) != 0)
    {
        // no base to go home to: the pool (5.5; inactive, they are gone as before)
        OccupierForces::Instance().Refund(bodies, "no base");
        LOG_INFO(Core, "CounterAttack: {} survivor(s) of the force on {} have no base to return to ('{}' is gone)",
                 bodies, (const char*)r.target, (const char*)r.source);
        Emit(registry, r, CAOpReturned, 0, bodies, "no base");
        return;
    }
    const int live = src < view.garrisonLive.Size() ? view.garrisonLive[src] : 0;
    // whole bodies (U3): the room is floored, the pending deposit counts
    // against it (S5 bodies are on their way into this reserve)
    const float room = s->garrisonMax - (float)live - floorf(s->garrison) - (float)s->pending;
    const int back = MinInt(bodies, room > 0 ? (int)floorf(room) : 0);
    // inside the player's deposit bubble the survivors are a pending deposit
    // (S5): they join the reserve unseen or once he leaves
    const bool pending =
        GarrisonCache::AddReserve(registry, src, back, src, view.playerValid, view.playerX, view.playerZ);
    // the surplus over the base's room: the pool (5.5)
    if (bodies > back)
    {
        OccupierForces::Instance().Refund(bodies - back, "surplus");
    }
    LOG_INFO(Core, "CounterAttack: {} of {} survivor(s) back at {}{} (reserve {}, pending {}, of {})", back, bodies,
             (const char*)s->name, pending ? " as a pending deposit" : "", s->garrison, s->pending, s->garrisonMax);
    Emit(registry, r, CAOpReturned, back, bodies - back, pending ? "home, pending" : "home");
}

static void AddEvent(AutoArray<ZoneEventRecord>& fired, ZoneEventType type, int zone)
{
    ZoneEventRecord ev;
    ev.type = type;
    ev.zoneIndex = zone;
    fired.Add(ev);
}

bool CounterAttack::Fight(ZoneRegistry& registry, const CounterAttackView& view, int target, Record& r,
                          AutoArray<ZoneEventRecord>& fired)
{
    // the legacy abstract fight (key off; OCCUPIER-FORCES.md 6.5): the odds
    // weigh the attackers' lethality (A_lethal = bodies x lethality), the
    // losses count bodies (A_bodies).  A won fight on a base reverts it
    // through the base recapture's seam (never reached with the key on).
    const ZoneRecord* town = registry.GetZone(target);
    const int defenders = town ? town->lastGuer : 0;
    const float lethal = (float)r.strength * (r.lethality > 0 ? r.lethality : 1.0f);
    const float hold = HoldOddsLethal(_tuning, defenders, lethal);
    const float roll = Roll01(r.seed, r.salt++);
    if (roll < hold)
    {
        int lost = (int)ceilf((float)r.strength * _tuning.repelLoss);
        if (lost > r.strength)
        {
            lost = r.strength;
        }
        LOG_INFO(Core, "CounterAttack: {} fighter(s) in {} beat off {} attacker(s) (odds {:.2f}, roll {:.2f}); {} lost",
                 defenders, (const char*)r.target, r.strength, hold, roll, lost);
        OccupierForces::Instance().NoteLoss(lost, "repelled");
        ReturnToSource(registry, view, r, r.strength - lost);
        AddEvent(fired, ZECounterRepelled, target);
        Emit(registry, r, CAOpRepelled, r.strength - lost, lost, "abstract fight");
        return true;
    }
    LOG_INFO(Core, "CounterAttack: {} attacker(s) retake {} from {} fighter(s) (odds {:.2f}, roll {:.2f})", r.strength,
             (const char*)r.target, defenders, hold, roll);
    if (town && stricmp(town->type, "CITY") != 0)
    {
        registry.RevertBase(target, fired, r.strength);
    }
    else
    {
        registry.RevertTown(target, fired);
    }
    Emit(registry, r, CAOpSettled, r.strength, 0, "abstract fight");
    return true;
}

namespace
{
// the living crewmen of a hull's group, seated or not (L4): at least the
// seated count HullState saw, at most the crew the record pays for
int LivingCrew(CounterAttackOps* ops, AIGroup* group, int seated, int cap)
{
    Vector3 at = VZero;
    const int any = ops && group ? ops->Alive(group, at) : 0;
    const int living = any > seated ? any : seated;
    return living < cap ? living : cap;
}

// M8: the gate rates points for a ground hull (its headlights at night)
// while this scope lives, for men on foot again after it
struct HullGateScope
{
    CounterAttackOps* ops;
    HullGateScope(CounterAttackOps* o, bool on) : ops(o)
    {
        if (ops)
        {
            ops->SetHullGate(on);
        }
    }
    ~HullGateScope()
    {
        if (ops)
        {
            ops->SetHullGate(false);
        }
    }
    HullGateScope(const HullGateScope&) = delete;
    HullGateScope& operator=(const HullGateScope&) = delete;
};
} // namespace

int CounterAttack::GoLive(ZoneRegistry& registry, const CounterAttackView& view, int target, Record& r,
                          CounterAttackOps* ops, float fraction, AutoArray<ZoneEventRecord>& fired)
{
    const Vector3 dest = DestOf(registry, r);
    if (!ops || (dest.X() == 0 && dest.Z() == 0))
    {
        return BirthWait;
    }
    const ZoneTuning& zt = registry.Tuning();
    // a hull package is born mounted (its hull re-created after a return to
    // abstract travel), except at the town edge, where the infantry is
    // already on foot (6.5 arrival births) or once the hull was lost
    const bool mounted = r.HasHullPackage() && !r.hullDone && !r.arrivalLive;
    // a column's escort is born with it (6.5, phase 5), even after the
    // carrier was lost
    const bool escorted = r.HasEscort() && !r.escortDone && !r.arrivalLive;
    const int groups = (mounted ? 2 : 1) + (escorted ? 1 : 0);
    const int bodies = r.strength + (mounted ? r.crew : 0) + (escorted ? r.escortCrew : 0);
    const bool overLive = LiveSquads() >= _tuning.maxLive;
    const bool overBodies = zt.budgetLiveBodies > 0 && view.liveBodies + bodies > zt.budgetLiveBodies;
    const bool overGroups = zt.budgetSideGroups > 0 && view.occGroups + groups > zt.budgetSideGroups;
    // forcesOpsMax counts born operations; an unborn one waits for a slot
    // (OpsCap: wave operations raise it by their own count)
    const bool overOps = r.funded && BornOps() >= OpsCap();
    const int newHulls = (mounted ? 1 : 0) + (escorted ? 1 : 0);
    const bool overHulls = newHulls > 0 && LiveHulls() + newHulls > _tuning.forcesLiveHullsMax;
    if (overLive || overBodies || overGroups || overOps || overHulls)
    {
        if (!r.deferLogged)
        {
            LOG_WARN(
                Core,
                "CounterAttack: force on {} deferred (live squads {}/{}, live AI {}+{}/{}, occupier groups {}+{}/{},"
                " born ops {}/{}, live hulls {}/{}); retrying every {:.0f}s",
                (const char*)r.target, LiveSquads(), _tuning.maxLive, view.liveBodies, bodies, zt.budgetLiveBodies,
                view.occGroups, groups, zt.budgetSideGroups, BornOps(), OpsCap(), LiveHulls(),
                _tuning.forcesLiveHullsMax, RetryInterval);
            r.deferLogged = true;
        }
        return BirthWait;
    }
    if (mounted && r.IsAir())
    {
        // phase 6A: a helicopter insertion, born airborne (3.1, 3.2)
        return AirGoLive(registry, view, target, r, ops, fraction);
    }
    // the user's rule: a force is only ever born where the player cannot
    // perceive it, on its road between the base and the town, and walks
    // the rest
    const AutoArray<Vector3>& route = RouteOf(registry, r, ops);
    // a mounted birth (carrier or escort) is gated as a vehicle (M8)
    const HullGateScope hullGate(ops, mounted || escorted);
    const CounterAttackSpawnPick pick = PickSpawnPoint(route, fraction, r.wholeRoad, _tuning.birthMinToTarget, *ops);
    if (pick.reason == CASpawnHold)
    {
        return BirthHold;
    }
    if (pick.reason == CASpawnDeferred)
    {
        if (!r.hiddenLogged)
        {
            LOG_WARN(Core,
                     "CounterAttack: force on {} from {} deferred: no point of its road ({}) is out of the player's"
                     " perception; retrying every {:.0f}s, its clock stopped, {} after {:.0f}s",
                     (const char*)r.target, (const char*)r.source,
                     r.wholeRoad ? "from the base" : "behind its progress point", RetryInterval,
                     r.funded || _tuning.forcesOn ? "the next base back or cancelled unseen" : "turning back",
                     _tuning.birthGiveUp);
            r.hiddenLogged = true;
        }
        return BirthNoPoint;
    }
    const int n = route.Size();
    const int pickIndex = RouteIndexAt(route, pick.fraction);
    const int legEnd = MinInt(pickIndex + WaypointStride, n - 1);
    AIGroup* grp = nullptr;
    Entity* hull = nullptr;
    AIGroup* crewGroup = nullptr;
    if (mounted)
    {
        // facing the next route point (3.2: road-snapped, along the road)
        const Vector3 ahead = route[MinInt(pickIndex + 1, n - 1)];
        Vector3 dir = ahead - pick.pos;
        dir[1] = 0;
        grp = ops->SpawnMounted(r.hullClass, r.crew, r.strength, pick.pos, dir, hull, crewGroup);
        if (!grp)
        {
            // the hull (or its driver) could not be created: the infantry
            // goes on foot, the whole price back to the director
            LOG_WARN(Core, "CounterAttack: [op {}] hull '{}' could not be created; the force on {} goes on foot",
                     r.opId, (const char*)r.hullClass, (const char*)r.target);
            Emit(registry, r, CAOpHullRefund, 0, r.priceUnits, "hull not created: on foot");
            r.hullDone = true;
            r.abstractSpeed = r.abstractSpeed > 0 ? FootSpeed : 0;
            r.travel = 0; // L5: a later return to the road re-times the leg at foot pace
            grp = ops->SpawnSquad(r.strength, pick.pos, route[legEnd]);
        }
    }
    else
    {
        grp = ops->SpawnSquad(r.strength, pick.pos, route[legEnd]);
    }
    if (!grp)
    {
        return BirthWait;
    }
    if (hull)
    {
        // crew the hull has no seat for stay home: back to the pool
        int crewAlive = 0;
        ops->HullState(hull, crewGroup, crewAlive);
        if (crewAlive < r.crew)
        {
            // L3: the price drops with the crew, so HullPrice (the hull's
            // share) stays what it was
            OccupierForces::Instance().Refund(r.crew - crewAlive, "partial spawn");
            r.priceUnits -= r.crew - crewAlive;
            r.crew = crewAlive;
        }
    }
    // a partial spawn (classes that refused to materialize): the missing
    // bodies go back to the pool (5.5)
    Vector3 leader = VZero;
    const int spawned = ops->Alive(grp, leader);
    if (spawned > 0 && spawned < r.strength)
    {
        LOG_WARN(Core, "CounterAttack: force on {} materialized {} of {}; the rest to the pool", (const char*)r.target,
                 spawned, r.strength);
        OccupierForces::Instance().Refund(r.strength - spawned, "partial spawn");
        r.strength = spawned;
    }
    // the column's escort (6.5, phase 5): on the road EscortGap behind the
    // birth point, under the same unseen rule (hidden, or no worse than the
    // birth point itself), its own crew group, driven like the carrier
    Entity* escortHull = nullptr;
    AIGroup* escortGroup = nullptr;
    if (escorted)
    {
        const Vector3 ahead = route[MinInt(pickIndex + 1, n - 1)];
        Vector3 heading = ahead - pick.pos;
        heading[1] = 0;
        heading = heading.SquareSize() > 1e-4f ? heading.Normalized() : VForward;
        const Vector3 at = pick.pos - heading * EscortGap;
        const int exposure = ops->Exposure(at);
        if (exposure == CAExposeHidden || exposure == ops->Exposure(pick.pos))
        {
            escortHull = ops->SpawnEscort(r.escortClass, r.escortCrew, at, heading, escortGroup);
        }
        if (!escortHull)
        {
            // not created, or its spot is watched: the column goes without
            // it, its whole price back to the director
            LOG_WARN(Core, "CounterAttack: [op {}] escort '{}' could not be born; the force on {} goes without it",
                     r.opId, (const char*)r.escortClass, (const char*)r.target);
            Emit(registry, r, CAOpHullRefund, 0, r.escortPriceUnits, "escort not created");
            r.escortDone = true;
        }
        else
        {
            // crew the escort has no seat for stay home: back to the pool
            int escortAlive = 0;
            ops->HullState(escortHull, escortGroup, escortAlive);
            if (escortAlive < r.escortCrew)
            {
                OccupierForces::Instance().Refund(r.escortCrew - escortAlive, "partial spawn");
                r.escortPriceUnits -= r.escortCrew - escortAlive;
                r.escortCrew = escortAlive;
            }
        }
    }
    const bool afterDeferral = r.deferLogged || r.hiddenLogged;
    r.group = grp;
    r.hull = hull;
    r.crewGroup = crewGroup;
    r.escort = escortHull;
    r.escortCrewGroup = escortGroup;
    r.dismounted = false;
    r.phase = CALive;
    r.clock = -MarchAllowance(route, pick.fraction);
    r.resend = ResendInterval;
    r.retry = 0;
    r.deferTime = 0;
    r.leg = LegRoad;
    r.wp = legEnd;
    r.spawnReason = pick.reason;
    r.spawnPos = pick.pos;
    r.deferLogged = false;
    r.hiddenLogged = false;
    const float toPlayer = view.playerValid ? Dist2D(pick.pos.X(), pick.pos.Z(), view.playerX, view.playerZ) : -1.0f;
    LOG_INFO(Core,
             "CounterAttack: {} soldier(s) from {} appear at [{:.0f}, {:.0f}], {:.0f} m from the player (reason: {}{}, "
             "{:.0f}% along {:.0f} m of road){}; they march on {}",
             r.strength, (const char*)r.source, pick.pos.X(), pick.pos.Z(), toPlayer,
             pick.reason == CASpawnBase ? "base" : "route point", afterDeferral ? ", after a deferral" : "",
             pick.fraction * 100.0f, RouteLength(route), hull ? " in a hull" : "", (const char*)r.target);
    AddEvent(fired, ZECounterAttack, target);
    Emit(registry, r, CAOpBorn, r.strength, 0, SpawnReasonName(pick.reason));
    return BirthDone;
}

bool CounterAttack::RoomLive(const ZoneRegistry& registry, const CounterAttackView& view) const
{
    const ZoneTuning& zt = registry.Tuning();
    return LiveSquads() < _tuning.maxLive && (zt.budgetSideGroups <= 0 || view.occGroups + 1 <= zt.budgetSideGroups);
}

float CounterAttack::LegTime(const ZoneRegistry& registry, Record& r, CounterAttackOps* ops) const
{
    const ZoneRecord* src = registry.GetZone(registry.FindZoneIndex(r.source));
    const Vector3 dest = DestOf(registry, r);
    if (!src || (dest.X() == 0 && dest.Z() == 0))
    {
        return MinTravel;
    }
    const float straight = Dist2D(src->pos.X(), src->pos.Z(), dest.X(), dest.Z());
    float road = 0;
    if (!_tuning.speedLever && (_tuning.forcesOn || r.abstractSpeed > 0))
    {
        // real speeds run over the real road when the road net gives one
        const AutoArray<Vector3>& route = RouteOf(registry, r, ops);
        road = r.routeIsRoad ? RouteLength(route) : 0.0f;
    }
    return TravelTimeFor(_tuning, r, straight, road);
}

bool CounterAttack::Launch(ZoneRegistry& registry, int target, int src, int n, bool live, CounterAttackOps* ops,
                           Record& r, AutoArray<ZoneEventRecord>& fired)
{
    const ZoneRecord* town = registry.GetZone(target);
    ZoneRecord* s = registry.GetZoneMutable(src);
    if ((!town && r.kind != CAKHqAssault) || !s || n < 1) // an HQ assault may target no zone
    {
        return false;
    }
    r.source = s->name;
    if (live)
    {
        // the base is in the bubble: its own soldiers (already there, in the
        // player's world) march out and take the road to the town
        const AutoArray<Vector3>& route = RouteOf(registry, r, ops);
        const int legEnd = MinInt(WaypointStride, route.Size() - 1);
        int taken = 0;
        AIGroup* grp = ops && legEnd >= 0 ? ops->DetachFromGarrison(src, n, route[legEnd], taken) : nullptr;
        if (!grp || taken <= 0)
        {
            return false;
        }
        r.strength = taken;
        r.group = grp;
        r.phase = CALive;
        r.clock = -MarchAllowance(route, 0);
        r.resend = ResendInterval;
        r.leg = LegRoad;
        r.wp = legEnd;
        r.spawnReason = CASpawnGarrison;
        r.spawnPos = s->pos;
        LOG_INFO(Core, "CounterAttack: {} soldier(s) of the {} garrison march on {} ({:.0f} m of road)", taken,
                 (const char*)s->name, (const char*)r.target, RouteLength(route));
        AddEvent(fired, ZECounterAttack, target);
        return true;
    }
    s->garrison -= (float)n;
    if (s->garrison < 0)
    {
        s->garrison = 0;
    }
    r.strength = n;
    r.phase = CATravel;
    r.clock = LegTime(registry, r, ops);
    r.travel = r.clock;
    r.retry = 0;
    LOG_INFO(Core, "CounterAttack: {} of the {} reserve leave for {}; on the road {:.0f}s (reserve now {})", n,
             (const char*)s->name, (const char*)r.target, r.clock, s->garrison);
    return true;
}

bool CounterAttack::Advance(ZoneRegistry& registry, const CounterAttackView& view, float elapsed, CounterAttackOps* ops,
                            Record& r, AutoArray<ZoneEventRecord>& fired)
{
    if (r.kind == CAKStrike)
    {
        return AdvanceStrike(registry, view, elapsed, ops, r);
    }
    const int target = registry.FindZoneIndex(r.target);
    ZoneRecord* town = registry.GetZoneMutable(target);
    // an HQ_ASSAULT marches on HqPos (9.1): its target zone may be -1 (an HQ
    // outside every zone), and it is valid while that HQ stands where it
    // was sent
    const bool hq = r.kind == CAKHqAssault;
    if (!town && !hq)
    {
        if (r.phase == CALive && ops && r.group)
        {
            ops->Remove(r.group);
        }
        Emit(registry, r, CAOpAborted, r.strength, 0, "target gone");
        return true;
    }
    const Vector3 dest = DestOf(registry, r);
    bool held = false;
    bool valid = false;
    // an alarm answer (8.2) goes to an occupier-held zone like a top-up
    // column and ends in its garrison (the surplus to the pool)
    const bool alarm = r.kind == CAKAlarm;
    const bool reinforce = r.kind == CAKReinforce || alarm;
    if (hq)
    {
        if (!view.hqEstablished && !r.hqDone)
        {
            // the HQ fell (destroyed: only Destroy un-establishes it): the
            // assault succeeded.  An abstract force settles now, its bodies
            // to the pool (there is no garrison to join); a live one leaves
            // the world once nobody sees it (below)
            if (r.phase != CALive)
            {
                LOG_INFO(Core, "CounterAttack: [op {}] the HQ fell; the {} soldier(s) of the assault go back to the pool",
                         r.opId, r.strength);
                OccupierForces::Instance().Refund(r.strength, "HQ assault done");
                Emit(registry, r, CAOpSettled, r.strength, r.strength, "the HQ fell");
                return true;
            }
            r.hqDone = true;
            LOG_INFO(Core, "CounterAttack: [op {}] the HQ fell; the assault squad leaves once nobody sees it", r.opId);
        }
        const bool moved =
            view.hqEstablished && Dist2D(view.hqPos.X(), view.hqPos.Z(), dest.X(), dest.Z()) > registry.Tuning().zoneArea;
        held = view.hqEstablished && !moved;
        valid = held;
        if (moved && r.phase == CALive && !r.hqDone && r.clock < _tuning.liveTimeout)
        {
            // the HQ it was sent at moved away: the squad withdraws (the
            // live timeout path: home once out of the player's sight)
            LOG_INFO(Core, "CounterAttack: [op {}] the HQ moved; the assault squad withdraws", r.opId);
            r.clock = _tuning.liveTimeout;
        }
    }
    else
    {
        held = stricmp(town->owner, registry.ResistanceSide()) == 0;
        // a top-up column (5.4) goes to a town the occupier holds; everything
        // else goes to a town the resistance holds
        valid = reinforce ? GarrisonCache::OccupierHolds(registry, *town) : held;
    }
    // defenders present at the target (6.5): the town's or base's HOLD
    // presence, or the HQ's (view.hqDefenders)
    const bool defendersHere =
        hq ? view.hqDefenders > 0 : (town->lastGuer > 0 || (_tuning.forcesOn && town->lastGuerHold > 0));

    switch (r.phase)
    {
        case CAScheduled:
        {
            if (!held || hq)
            {
                LOG_INFO(Core, "CounterAttack: {} fell before the force left; cancelled", (const char*)r.target);
                Emit(registry, r, CAOpAborted, 0, 0, "the town fell before the force left");
                return true;
            }
            r.clock -= elapsed;
            if (r.clock > 0)
            {
                return false;
            }
            if (target == view.hqZone)
            {
                // the player set up his headquarters in the town meanwhile
                // (N4a): the HQ's town is never a target
                LOG_INFO(Core, "CounterAttack: {} now holds the player's headquarters; the force is cancelled",
                         (const char*)r.target);
                Emit(registry, r, CAOpAborted, 0, 0, "the town holds the player's headquarters");
                return true;
            }
            const bool inBubble = InBubble(registry, view, target);
            int spare = 0;
            bool live = false;
            // marching a live garrison out adds a group, not bodies: it still
            // respects the live-squad and group budgets, else the force is
            // drawn from a stored reserve and waits on the road instead
            const int src =
                PickSource(registry, view, target, inBubble && ops != nullptr && RoomLive(registry, view), spare, live);
            if (src < 0)
            {
                LOG_INFO(Core, "CounterAttack: no occupier base can spare a man for {}; no counter-attack",
                         (const char*)r.target);
                Emit(registry, r, CAOpAborted, 0, 0, "no base can spare a man");
                return true;
            }
            if (!Launch(registry, target, src, MinInt(r.size, spare), live, ops, r, fired))
            {
                r.clock = RetryInterval;
            }
            else
            {
                Emit(registry, r, CAOpLaunched, r.strength, 0, live ? "live" : "abstract");
                if (r.phase == CALive)
                {
                    Emit(registry, r, CAOpBorn, r.strength, 0, SpawnReasonName(r.spawnReason));
                }
            }
            // launched with the town already in the bubble: born anywhere on
            // its road (from the base), not only behind its progress point
            r.wholeRoad = r.phase == CATravel && inBubble;
            return false;
        }
        case CATravel:
        {
            if (!valid)
            {
                if (reinforce || hq)
                {
                    LOG_INFO(Core, "CounterAttack: {} is no longer a valid target; the force goes home",
                             hq ? "the HQ" : (const char*)r.target);
                    ReturnToSource(registry, view, r, r.strength);
                    Emit(registry, r, CAOpAborted, r.strength, 0,
                         hq ? "the HQ moved" : "the top-up target was lost");
                    return true;
                }
                if (_tuning.forcesOn)
                {
                    return RevertedUnder(registry, view, target, r);
                }
                LOG_INFO(Core, "CounterAttack: {} fell while the force was on the road; it turns back",
                         (const char*)r.target);
                ReturnToSource(registry, view, r, r.strength);
                Emit(registry, r, CAOpAborted, r.strength, 0, "the town fell while the force was on the road");
                return true;
            }
            if (MustGoLive(registry, view, target, r, ops))
            {
                // the force would come into the player's world: it is born
                // on its road where the player cannot see it, or waits; its
                // arrival clock does not run meanwhile and it does nothing to
                // the town
                // the birth chain (3.1, D6) runs for funded records and, with
                // the key on, for legacy (director DORMANT) records too: no
                // turn-back as normal behaviour.  Key off: the legacy turn-back
                const bool chain = r.funded || _tuning.forcesOn;
                // M2: a force held at the town edge (BirthHold: the player
                // in its town) counts toward the give-up too; then it is born
                // at the town edge out of his sight, or cancelled unseen
                if (r.hiddenLogged || (chain && r.edgeHold))
                {
                    r.deferTime += elapsed;
                    if (r.deferTime >= _tuning.birthGiveUp && chain && r.edgeHold)
                    {
                        r.deferTime = 0;
                        const int born = ArriveLive(registry, view, target, r, ops, fired);
                        if (born == BirthDone || born == BirthWait)
                        {
                            return false; // live, or over budget: it waits at the edge (6.5)
                        }
                        CancelUnseen(registry, view, r);
                        return true;
                    }
                    if (r.deferTime >= _tuning.birthGiveUp)
                    {
                        if (chain)
                        {
                            // the birth chain (3.1, D6): the next base back,
                            // then cancel unseen; never a visible turn-back
                            if (Resource(registry, view, target, r, ops))
                            {
                                return false;
                            }
                            // 3.1 step 4 (phase 6A): the air re-plan
                            if (AirReplan(registry, view, r, OccupierForces::Instance().Catalogue()))
                            {
                                return false;
                            }
                            CancelUnseen(registry, view, r);
                            return true;
                        }
                        LOG_INFO(Core,
                                 "CounterAttack: the force on {} found no unseen road for {:.0f}s; it turns back to {}",
                                 (const char*)r.target, r.deferTime, (const char*)r.source);
                        ReturnToSource(registry, view, r, r.strength);
                        Emit(registry, r, CAOpAborted, r.strength, 0, "no unseen road: turned back");
                        return true;
                    }
                }
                if (r.retry > 0)
                {
                    r.retry -= elapsed;
                    return false;
                }
                const int birth = GoLive(registry, view, target, r, ops, Progress(r), fired);
                if (birth == BirthHold && !r.edgeHold && chain)
                {
                    LOG_INFO(Core,
                             "CounterAttack: [op {}] the force on {} holds at the town edge (the player is in the"
                             " town); born there unseen or cancelled after {:.0f}s",
                             r.opId, (const char*)r.target, _tuning.birthGiveUp);
                }
                r.edgeHold = birth == BirthHold;
                if (birth == BirthNoPoint && chain && Resource(registry, view, target, r, ops))
                {
                    // 3.1 step 3 at once: the next base back whose road the
                    // gate passes; born from there on the next step.  Only
                    // when no base passes does the force wait (BirthGiveUp)
                    return false;
                }
                if (birth == BirthNoPoint && chain && !GroundSourceShares(registry, r) &&
                    AirReplan(registry, view, r, OccupierForces::Instance().Catalogue()))
                {
                    // 3.1 step 4 at once (phase 6A): no other base shares
                    // the target's landmass, so waiting cannot help a ground
                    // force; the helicopter goes now
                    return false;
                }
                if (birth != BirthDone)
                {
                    // BirthHold: already at the town edge, it stays there,
                    // abstract and idle, until the player leaves or it can
                    // be born
                    r.retry = RetryInterval;
                }
                return false;
            }
            r.deferTime = 0;
            r.hiddenLogged = false;
            r.edgeHold = false;
            const bool onRoad = r.clock > 0;
            if (onRoad)
            {
                r.retry = 0;
                r.clock -= elapsed;
                if (r.clock > 0)
                {
                    return false;
                }
            }
            r.clock = 0;
            if (onRoad)
            {
                // off-screen arrival
                AddEvent(fired, ZECounterAttack, target);
                Emit(registry, r, CAOpArrived, r.strength, 0, "abstract");
            }
            if (alarm && !r.answered)
            {
                r.answered = true;
                EmitAt(registry, r, CAOpAlarmAnswered, target, IsZeroPos(r.alarmPos) ? town->pos : r.alarmPos,
                       r.strength, 0, "abstract");
            }
            if (reinforce)
            {
                // the top-up column joins the town's garrison, up to its room
                const int live = target < view.garrisonLive.Size() ? view.garrisonLive[target] : 0;
                const float room = town->garrisonMax - (float)live - floorf(town->garrison) - (float)town->pending;
                const int in = MinInt(r.strength, room > 0 ? (int)floorf(room) : 0);
                GarrisonCache::AddReserve(registry, target, in, registry.FindZoneIndex(r.source), view.playerValid,
                                          view.playerX, view.playerZ);
                LOG_INFO(Core, "CounterAttack: {} of {} soldier(s) top up {} (reserve {}, pending {})", in, r.strength,
                         (const char*)r.target, town->garrison, town->pending);
                if (alarm)
                {
                    if (r.strength > in)
                    {
                        OccupierForces::Instance().Refund(r.strength - in, "alarm answer surplus"); // 8.2
                    }
                }
                else
                {
                    ReturnToSource(registry, view, r, r.strength - in);
                }
                Emit(registry, r, CAOpSettled, in, 0, "topped up");
                return true;
            }
            if (defendersHere)
            {
                if (!_tuning.forcesOn && !hq)
                {
                    return Fight(registry, view, target, r, fired);
                }
                // 6.5: a real fight against present defenders, born at the
                // town edge under S; over budget it waits there doing nothing
                if (r.retry > 0)
                {
                    r.retry -= elapsed;
                    return false;
                }
                ArriveLive(registry, view, target, r, ops, fired);
                return false;
            }
            r.phase = CAHolding;
            r.retry = 0;
            if (hq)
            {
                LOG_INFO(Core, "CounterAttack: [op {}] {} attacker(s) stand at the undefended HQ", r.opId, r.strength);
            }
            else
            {
                LOG_INFO(Core, "CounterAttack: {} attacker(s) walk into empty {} (support {}, occCapture {})",
                         r.strength, (const char*)r.target, town->support, town->occCapture);
            }
            return false;
        }
        case CAHolding:
        {
            if (!valid && hq)
            {
                LOG_INFO(Core, "CounterAttack: [op {}] the HQ moved; the holding force goes home", r.opId);
                ReturnToSource(registry, view, r, r.strength);
                Emit(registry, r, CAOpAborted, r.strength, 0, "the HQ moved");
                return true;
            }
            if (!valid)
            {
                // the drain reverted the town (or someone else did): with the
                // key on the holding bodies are its new garrison (settled by
                // RevertTown through ZoneTickInputs::settleOcc)
                LOG_INFO(Core, "CounterAttack: {} retaken by the holding force", (const char*)r.target);
                Emit(registry, r, CAOpSettled, r.strength, 0, _tuning.forcesOn ? "holding force settled" : "held");
                return true;
            }
            // the town in the player's bubble: the force is never born at the
            // town centre; it is suspended (no drain, no odds roll) until he
            // leaves
            r.suspended = InBubbleAt(registry, view, dest);
            if (r.suspended)
            {
                // M2: with the key on (or a funded record) the suspension
                // counts toward the give-up: then the force is born at the
                // town edge out of the player's sight and walks in, or it is
                // cancelled unseen; never immune for good
                if (_tuning.forcesOn || r.funded)
                {
                    r.deferTime += elapsed;
                    if (r.deferTime >= _tuning.birthGiveUp)
                    {
                        r.deferTime = 0;
                        r.phase = CATravel;
                        r.clock = 0;
                        r.retry = 0;
                        r.suspended = false;
                        const int born = ArriveLive(registry, view, target, r, ops, fired);
                        if (born == BirthDone || born == BirthWait)
                        {
                            return false;
                        }
                        CancelUnseen(registry, view, r);
                        return true;
                    }
                }
                return false;
            }
            r.deferTime = 0;
            if (!_tuning.forcesOn && !hq)
            {
                if (town->lastGuer > 0)
                {
                    return Fight(registry, view, target, r, fired);
                }
                return false;
            }
            if (hq ? view.hqDefenders > 0 : (town->lastGuer > 0 || town->lastGuerHold > 0))
            {
                // 6.5: defenders came back; the holding force fights them
                // live, born at the town edge (it waits there over budget)
                r.phase = CATravel;
                r.clock = 0;
                r.retry = 0;
                ArriveLive(registry, view, target, r, ops, fired);
            }
            return false;
        }
        case CALive:
        {
            if (r.airId > 0 && !r.dismounted)
            {
                // phase 6A: an insertion in the air, landing or unloading
                return AdvanceAirLive(registry, view, ops, r);
            }
            Vector3 leader = VZero;
            const int alive = ops && r.group ? ops->Alive(r.group, leader) : 0;
            if (LiveHullOf(r) > 0 && ops)
            {
                CheckHull(registry, r, ops);
            }
            if (LiveEscortOf(r) > 0 && ops)
            {
                CheckEscort(registry, r, ops);
            }
            if (hq && r.hqDone)
            {
                // the HQ fell under the squad: it leaves the world where
                // nobody sees it (never a visible vanish), its bodies to the
                // pool (no garrison to join)
                if (alive == 0)
                {
                    AddLeftover(r.source, nullptr, r.group, 0); // L7: its dead group, unseen
                    Emit(registry, r, CAOpSettled, 0, 0, "the HQ fell; nobody left standing");
                    return true;
                }
                const float edge = registry.Tuning().cacheRadius + GarrisonCache::DespawnHysteresis;
                const bool away =
                    !view.playerValid || Dist2D(leader.X(), leader.Z(), view.playerX, view.playerZ) > edge;
                const bool hullUnseen =
                    (LiveHullOf(r) == 0 || (ops && ops->HullUnseen(r.hull, r.crewGroup))) &&
                    (LiveEscortOf(r) == 0 || (ops && ops->HullUnseen(r.escort, r.escortCrewGroup)));
                if (ops && away && ops->BodiesUnseen(r.group) && hullUnseen)
                {
                    ops->Remove(r.group);
                    OccupierForces::Instance().Refund(alive, "HQ assault done");
                    LOG_INFO(Core, "CounterAttack: [op {}] the HQ assault squad ({}) leaves unseen; back to the pool",
                             r.opId, alive);
                    Emit(registry, r, CAOpSettled, alive, alive, "the HQ fell");
                    return true;
                }
                return false;
            }
            if (reinforce && valid && r.withdrawing)
            {
                r.withdrawing = false; // the town is the occupier's again: the top-up goes on
                r.leg = LegUnknown;
            }
            if (!valid && !hq)
            {
                if (reinforce)
                {
                    // M5: the top-up squad's town was lost.  It walks home and
                    // leaves the world only where nobody sees it (the squad
                    // rule), never in the player's view
                    if (alive == 0)
                    {
                        OccupierForces::Instance().NoteLoss(r.strength, "top-up squad lost");
                        AddLeftover(r.source, nullptr, r.group, 0); // L7: its dead group, unseen
                        Emit(registry, r, CAOpAborted, 0, r.strength, "the top-up target was lost; the squad is gone");
                        return true;
                    }
                    const ZoneRecord* home = registry.GetZone(registry.FindZoneIndex(r.source));
                    if (!r.withdrawing)
                    {
                        r.withdrawing = true;
                        r.resend = 0;
                        LOG_INFO(Core, "CounterAttack: {} is no longer the occupier's; the top-up squad goes home",
                                 (const char*)r.target);
                    }
                    r.resend -= elapsed;
                    if (r.resend <= 0 && ops && home)
                    {
                        r.resend = ResendInterval;
                        if (LiveHullOf(r) > 0 && !r.dismounted)
                        {
                            ops->DriveOn(r.crewGroup, home->pos);
                        }
                        else
                        {
                            ops->MoveOn(r.group, home->pos);
                        }
                    }
                    const float edge = registry.Tuning().cacheRadius + GarrisonCache::DespawnHysteresis;
                    const bool away =
                        !view.playerValid || Dist2D(leader.X(), leader.Z(), view.playerX, view.playerZ) > edge;
                    if (ops && away && ops->BodiesUnseen(r.group))
                    {
                        ops->Remove(r.group);
                        if (alive < r.strength)
                        {
                            OccupierForces::Instance().NoteLoss(r.strength - alive, "top-up squad"); // L4
                        }
                        LOG_INFO(Core, "CounterAttack: the top-up squad for {} ({}) leaves unseen for {}",
                                 (const char*)r.target, alive, (const char*)r.source);
                        ReturnToSource(registry, view, r, alive);
                        Emit(registry, r, CAOpAborted, alive, 0, "the top-up target was lost");
                        return true;
                    }
                    return false;
                }
                if (alive > 0 && ops)
                {
                    AdoptLive(registry, target, r, ops, alive);
                }
                else
                {
                    OccupierForces::Instance().NoteLoss(r.strength, "dead before the revert");
                    AddLeftover(r.source, nullptr, r.group, 0); // L7: its dead group, unseen
                    Emit(registry, r, CAOpAborted, 0, r.strength, "the town reverted; the squad is gone");
                }
                return true;
            }
            if (alive == 0)
            {
                LOG_INFO(Core, "CounterAttack: the force on {} is gone; the town held", (const char*)r.target);
                AddEvent(fired, ZECounterRepelled, target);
                OccupierForces::Instance().NoteLoss(r.strength, "repelled");
                AddLeftover(r.source, nullptr, r.group, 0); // L7: its dead group leaves unseen
                Emit(registry, r, CAOpRepelled, 0, r.strength, "wiped out");
                return true;
            }
            if (alarm && r.leg == LegTown && !r.answered)
            {
                r.answered = true;
                EmitAt(registry, r, CAOpAlarmAnswered, target, IsZeroPos(r.alarmPos) ? town->pos : r.alarmPos, alive,
                       0, "live");
            }
            if (alarm && r.leg == LegTown)
            {
                // 8.2: it searches the last-known position and ends when the
                // zone is below RED and it found no contact
                const bool red = target < view.alertState.Size() && view.alertState[target] >= ASRed;
                const bool contact = town->lastGuer > 0;
                r.alarmQuiet = !red && !contact ? r.alarmQuiet + elapsed : 0.0f;
                if (r.alarmQuiet < AlarmQuietHold && r.clock < _tuning.liveTimeout)
                {
                    return false;
                }
            }
            if (reinforce && r.leg == LegTown)
            {
                // the top-up squad reached the town: part of its garrison now
                if (alive < r.strength)
                {
                    OccupierForces::Instance().NoteLoss(r.strength - alive, "top-up squad"); // L4
                }
                if (LiveHullOf(r) > 0 && !r.dismounted)
                {
                    ops->Dismount(r.group, r.hull);
                    r.dismounted = true;
                }
                ops->Adopt(target, r.group);
                LOG_INFO(Core, "CounterAttack: {} soldier(s) join the {} garrison", alive, (const char*)r.target);
                Emit(registry, r, CAOpSettled, alive, 0, "topped up");
                return true;
            }
            if (r.leg == LegRoad && !r.arrivalLive)
            {
                // a marching squad nobody can perceive, away from the
                // player's bubble, returns to the abstract road (Traffic's
                // despawn rule): its slot and budget are freed, its
                // survivors and its progress carried over.  A squad born to
                // fight present defenders never does (it would be re-born at
                // once).
                const float edge = registry.Tuning().cacheRadius + GarrisonCache::DespawnHysteresis;
                const bool away = Dist2D(leader.X(), leader.Z(), view.playerX, view.playerZ) > edge &&
                                  Dist2D(dest.X(), dest.Z(), view.playerX, view.playerZ) > edge;
                const bool hullUnseen = (LiveHullOf(r) == 0 || ops->HullUnseen(r.hull, r.crewGroup)) &&
                                        (LiveEscortOf(r) == 0 || ops->HullUnseen(r.escort, r.escortCrewGroup));
                // every member, the dead included: a corpse in view never vanishes (reader defect 3)
                r.unseenFor = away && ops->BodiesUnseen(r.group) && hullUnseen ? r.unseenFor + elapsed : 0.0f;
                if (r.unseenFor >= VanishHold)
                {
                    const AutoArray<Vector3>& route = RouteOf(registry, r, ops);
                    const float fraction = RouteFractionNear(route, leader);
                    const float whole = LegTime(registry, r, ops);
                    ops->Remove(r.group); // the cargo first, then the crew and the hull
                    // L4: the infantry who died on the march are lost
                    if (alive < r.strength)
                    {
                        OccupierForces::Instance().NoteLoss(r.strength - alive, "killed on the march");
                    }
                    if (LiveHullOf(r) > 0)
                    {
                        // every living crewman (seated or not: HullUnseen
                        // waited for them all) is re-created with the hull;
                        // the dead are lost and come off the price (L3)
                        int seated = 0;
                        ops->HullState(r.hull, r.crewGroup, seated);
                        const int living = LivingCrew(ops, r.crewGroup, seated, r.crew);
                        if (living < r.crew)
                        {
                            OccupierForces::Instance().NoteLoss(r.crew - living, "crew");
                            r.priceUnits -= r.crew - living;
                            r.crew = living;
                        }
                        ops->RemoveHull(r.hull, r.crewGroup);
                        r.hull = nullptr;
                        r.crewGroup = nullptr;
                        // no refund: the record keeps hullClass and its price
                        // and re-creates the hull at the next birth (3.2)
                        Emit(registry, r, CAOpHullLost, 0, 0, "abstract travel: re-created at the next birth");
                    }
                    if (LiveEscortOf(r) > 0)
                    {
                        int seated = 0;
                        ops->HullState(r.escort, r.escortCrewGroup, seated);
                        const int escortAlive = LivingCrew(ops, r.escortCrewGroup, seated, r.escortCrew);
                        if (escortAlive < r.escortCrew)
                        {
                            OccupierForces::Instance().NoteLoss(r.escortCrew - escortAlive, "escort crew");
                            r.escortPriceUnits -= r.escortCrew - escortAlive;
                            r.escortCrew = escortAlive;
                        }
                        ops->RemoveHull(r.escort, r.escortCrewGroup); // the crew first, then the hull
                        r.escort = nullptr;
                        r.escortCrewGroup = nullptr;
                        // no refund: re-created with the carrier at the next birth
                        Emit(registry, r, CAOpHullLost, 0, 0, "abstract travel: escort re-created at the next birth");
                    }
                    ReturnToRoad(r, fraction, alive, whole);
                    LOG_INFO(Core,
                             "CounterAttack: the force on {} ({} soldier(s)) leaves the player's world {:.0f}% along"
                             " its road; abstract again, {:.0f}s from the town",
                             (const char*)r.target, alive, fraction * 100.0f, r.clock);
                    return false;
                }
            }
            r.clock += elapsed;
            if (r.leg != LegTown)
            {
                // on the road: leg by leg to the town edge
                March(registry, target, r, ops, leader, elapsed);
            }
            else
            {
                r.resend -= elapsed;
                if (r.resend <= 0)
                {
                    r.resend = ResendInterval;
                    if (Dist2D(leader.X(), leader.Z(), dest.X(), dest.Z()) > registry.Tuning().zoneArea * 0.5f)
                    {
                        ops->MoveOn(r.group, dest);
                    }
                }
            }
            if (r.clock >= _tuning.liveTimeout)
            {
                const float edge = registry.Tuning().cacheRadius + GarrisonCache::DespawnHysteresis;
                const bool unseen =
                    !view.playerValid || Dist2D(leader.X(), leader.Z(), view.playerX, view.playerZ) > edge;
                if (unseen)
                {
                    LOG_INFO(Core, "CounterAttack: the force on {} gives up after {:.0f}s; {} go home",
                             (const char*)r.target, r.clock, alive);
                    ReturnToSource(registry, view, r, alive);
                    ops->Remove(r.group);
                    AddEvent(fired, ZECounterRepelled, target);
                    OccupierForces::Instance().NoteLoss(r.strength - alive, "repelled");
                    Emit(registry, r, CAOpRepelled, alive, r.strength - alive, "withdrew");
                    return true;
                }
            }
            return false;
        }
        default:
            return true;
    }
}

bool CounterAttack::RevertedUnder(ZoneRegistry& registry, const CounterAttackView& view, int target, Record& r)
{
    // 5.4: a travelling force whose town reverted under it (a passer-by)
    // does not turn back: it tops the town up when its garrison is short,
    // else it goes home
    const ZoneRecord* town = registry.GetZone(target);
    const int live = target < view.garrisonLive.Size() ? view.garrisonLive[target] : 0;
    const float have = town ? floorf(town->garrison) + (float)town->pending + (float)live : 0.0f;
    if (town && !town->garrisonExempt && GarrisonCache::OccupierHolds(registry, *town) &&
        have < floorf(town->garrisonMax))
    {
        r.kind = CAKReinforce;
        LOG_INFO(Core, "CounterAttack: [op {}] {} reverted under the force; it goes on as a top-up column ({} of {})",
                 r.opId, (const char*)r.target, have, town->garrisonMax);
        return false;
    }
    LOG_INFO(Core, "CounterAttack: [op {}] {} reverted under the force; its garrison is whole, the force goes home",
             r.opId, (const char*)r.target);
    ReturnToSource(registry, view, r, r.strength);
    Emit(registry, r, CAOpAborted, r.strength, 0, "the town reverted under it");
    return true;
}

int CounterAttack::ArriveLive(ZoneRegistry& registry, const CounterAttackView& view, int target, Record& r,
                              CounterAttackOps* ops, AutoArray<ZoneEventRecord>& fired)
{
    // the birth search starts at the last route point at least the bound
    // from the town and runs back toward the base (the gate passes trivially
    // far from the player, so off-screen it is that point)
    const Vector3 dest = DestOf(registry, r);
    const bool haveDest = dest.X() != 0 || dest.Z() != 0;
    const AutoArray<Vector3>& route = RouteOf(registry, r, ops);
    const float len = RouteLength(route);
    float fraction = 0;
    if (haveDest && len > 0)
    {
        float arc = len;
        for (int i = route.Size() - 1; i > 0; i--)
        {
            if (Dist2D(route[i], dest) >= _tuning.birthMinToTarget + 1.0f)
            {
                fraction = Clamp01(arc / len);
                break;
            }
            arc -= Dist2D(route[i - 1], route[i]);
        }
    }
    const bool whole = r.wholeRoad;
    r.wholeRoad = false;
    r.arrivalLive = true;
    const int birth = GoLive(registry, view, target, r, ops, fraction, fired);
    r.wholeRoad = whole;
    if (birth != BirthDone)
    {
        r.arrivalLive = false;
        r.retry = RetryInterval;
        if (!r.deferLogged && !r.hiddenLogged)
        {
            LOG_INFO(Core, "CounterAttack: [op {}] the force at {} waits at the town edge (defenders present)", r.opId,
                     (const char*)r.target);
            r.deferLogged = true;
        }
    }
    return birth;
}

// is name one of the ';'-joined names in list (case-insensitive)?
static bool NameListed(const char* list, const char* name)
{
    const size_t len = strlen(name);
    const char* p = list;
    while (p && *p)
    {
        const char* end = strchr(p, ';');
        const size_t seg = end ? (size_t)(end - p) : strlen(p);
        if (seg == len && strnicmp(p, name, len) == 0)
        {
            return true;
        }
        p = end ? end + 1 : nullptr;
    }
    return false;
}

bool CounterAttack::Resource(ZoneRegistry& registry, const CounterAttackView& view, int target, Record& r,
                             CounterAttackOps* ops)
{
    if (r.IsAir())
    {
        return false; // phase 6A: air has no road to re-source; its birth search already ran off-map
    }
    (void)target;
    const Vector3 dest = DestOf(registry, r);
    const int a = registry.FindZoneIndex(r.source);
    if (dest.X() == 0 && dest.Z() == 0)
    {
        return false;
    }
    // the bases already used (D6: then each further base)
    const RString tried = r.tried.GetLength() > 0 ? r.tried + RString(";") + r.source : r.source;
    // the other occupier bases whose stored reserve can spare the whole force
    // (a base that cannot is passed over for the next), nearest the target
    // first by straight distance
    AutoArray<int> cands;
    AutoArray<float> dists;
    for (int i = 0; i < registry.NZones(); i++)
    {
        const ZoneRecord* z = registry.GetZone(i);
        if (!z || i == a || !CampaignEnd::IsMilitaryType(z->type) || stricmp(z->owner, registry.OccupierSide()) != 0 ||
            NameListed(tried, z->name))
        {
            continue;
        }
        if (Spare(_tuning, z->garrison, 0, z->garrisonMax, false) < r.strength)
        {
            continue;
        }
        if (!RoadRoute::SameLandmass(z->pos, dest))
        {
            LOG_INFO(Core, "CounterAttack: [op {}] {} passed over for {}: it stands on another landmass", r.opId,
                     (const char*)z->name, (const char*)r.target);
            continue;
        }
        const float d = Dist2D(z->pos, dest);
        cands.Add(i);
        dists.Add(d);
        for (int at = cands.Size() - 1; at > 0 && dists[at - 1] > dists[at]; at--)
        {
            const int ci = cands[at];
            cands[at] = cands[at - 1];
            cands[at - 1] = ci;
            const float di = dists[at];
            dists[at] = dists[at - 1];
            dists[at - 1] = di;
        }
    }
    // 3.1 step 3: the first of them whose route to the target has a point the
    // gate passes (the same search the birth will run: from the base when the
    // target is in the bubble, else the base point).  The route is the road
    // net's (ops->BuildRoute, the road A*), else the straight line
    // cross-country, so every base's route reaches the target; the gate is
    // what decides.  No ops (no world, no gate): the nearest, as before.
    const bool wholeRoad = InBubbleAt(registry, view, dest);
    int best = -1;
    AutoArray<Vector3> bestRoute;
    bool bestIsRoad = false;
    for (int c = 0; c < cands.Size() && best < 0; c++)
    {
        if (!ops)
        {
            best = cands[c];
            break;
        }
        Record probe;
        probe.target = r.target;
        probe.source = registry.GetZone(cands[c])->name;
        probe.kind = r.kind;
        probe.destPos = r.destPos;
        const AutoArray<Vector3>& route = RouteOf(registry, probe, ops);
        const HullGateScope hullGate(ops, (r.HasHullPackage() && !r.hullDone) || (r.HasEscort() && !r.escortDone));
        const CounterAttackSpawnPick pick = PickSpawnPoint(route, 0.0f, wholeRoad, _tuning.birthMinToTarget, *ops);
        if (pick.reason == CASpawnDeferred)
        {
            LOG_INFO(Core, "CounterAttack: [op {}] {} passed over for {}: its road is watched too", r.opId,
                     (const char*)probe.source, (const char*)r.target);
            continue;
        }
        best = cands[c];
        bestRoute = probe.route;
        bestIsRoad = probe.routeIsRoad;
    }
    if (best < 0)
    {
        return false;
    }
    // A is refunded, B pays
    // (room-capped, the surplus to the pool, L2)
    ReturnBodies(registry, view, a, r.strength);
    ZoneRecord* b = registry.GetZoneMutable(best);
    b->garrison = floorf(b->garrison) - (float)r.strength;
    if (b->garrison < 0)
    {
        b->garrison = 0;
    }
    LOG_INFO(Core,
             "CounterAttack: [op {}] every road from {} to {} is watched; re-sourced to {} ({} soldier(s); reserve "
             "{} there)",
             r.opId, (const char*)r.source, (const char*)r.target, (const char*)b->name, r.strength, b->garrison);
    r.tried = tried;
    r.source = b->name;
    r.route = bestRoute; // empty without ops: RouteOf rebuilds it
    r.routeIsRoad = bestIsRoad;
    r.travel = 0;
    r.clock = LegTime(registry, r, ops);
    r.travel = r.clock;
    r.deferTime = 0;
    r.retry = 0;
    r.hiddenLogged = false;
    r.deferLogged = false;
    r.wholeRoad = wholeRoad;
    Emit(registry, r, CAOpResourced, r.strength, 0, "next base back");
    return true;
}

void CounterAttack::CancelUnseen(ZoneRegistry& registry, const CounterAttackView& view, Record& r)
{
    // nothing could be born anywhere: the operation is called off while it
    // is still abstract (an unborn force vanishing off-map is no visible
    // turn-back, D6).  Its bodies go back to their current source, or the
    // pool when that base is gone; the director refunds the hull price and
    // starts the town's cooldown on the event.
    // (room-capped, the surplus to the pool, L2)
    ReturnBodies(registry, view, registry.FindZoneIndex(r.source), r.strength);
    LOG_INFO(Core, "CounterAttack: [op {}] cancelled unborn: every route to {} watched ({} soldier(s) back to {})",
             r.opId, (const char*)r.target, r.strength, (const char*)r.source);
    // the unsettled hull prices go back through the event: exactly what is
    // still owed (L3: crew already booked lost or refunded are not in it)
    int owed = r.OwedUnits();
    if (r.airReplanned && owed > 0)
    {
        OccupierForces::Instance().Refund(owed, "air re-plan cancelled"); // spent directly, refunded directly
        owed = 0;
    }
    r.hullDone = true;
    r.escortDone = true;
    Emit(registry, r, CAOpCancelledUnseen, r.strength, owed, "every route watched");
}

int CounterAttack::ReturnBodies(ZoneRegistry& registry, const CounterAttackView& view, int zone, int bodies)
{
    if (bodies <= 0)
    {
        return 0;
    }
    ZoneRecord* s = registry.GetZoneMutable(zone);
    if (!s || stricmp(s->owner, registry.OccupierSide()) != 0)
    {
        OccupierForces::Instance().Refund(bodies, "no base");
        return 0;
    }
    const int live = zone < view.garrisonLive.Size() ? view.garrisonLive[zone] : 0;
    const float room = s->garrisonMax - (float)live - floorf(s->garrison) - (float)s->pending;
    const int back = MinInt(bodies, room > 0 ? (int)floorf(room) : 0);
    GarrisonCache::AddReserve(registry, zone, back, zone, view.playerValid, view.playerX, view.playerZ);
    if (bodies > back)
    {
        OccupierForces::Instance().Refund(bodies - back, "surplus");
    }
    return back;
}

void CounterAttack::AddLeftover(const RString& source, Entity* hull, AIGroup* group, int living)
{
    if (!hull && !group)
    {
        return;
    }
    Leftover l;
    l.source = source;
    l.living = living > 0 ? living : 0;
    l.hull = hull;
    l.group = group;
    _leftovers.Add(l);
}

void CounterAttack::StepLeftovers(ZoneRegistry& registry, const CounterAttackView& view, float elapsed,
                                  CounterAttackOps* ops)
{
    if (!ops)
    {
        return;
    }
    const float edge = registry.Tuning().cacheRadius + GarrisonCache::DespawnHysteresis;
    for (int i = 0; i < _leftovers.Size();)
    {
        Leftover& l = _leftovers[i];
        l.retry -= elapsed;
        if (l.retry > 0)
        {
            i++;
            continue;
        }
        l.retry = RetryInterval;
        Entity* hull = l.hull;
        if (hull)
        {
            // an abandoned intact hull leaves where nobody sees it and
            // nobody sits in it (the player may have taken it); a wreck stays
            int seated = 0;
            if (ops->HullState(hull, nullptr, seated) != CounterAttackOps::HullIntact)
            {
                l.hull = nullptr;
            }
            else if (!ops->HullOccupied(hull) && ops->HullUnseen(hull, nullptr))
            {
                ops->RemoveHull(hull, nullptr);
                l.hull = nullptr;
            }
        }
        AIGroup* grp = l.group;
        if (grp)
        {
            // dead units stay group members, so such a group holds a slot of
            // the side's group budget: it goes with its corpses once they are
            // all unseen; living stragglers only away from the player too,
            // home as bodies, the dead among them lost
            Vector3 leader = VZero;
            const int alive = ops->Alive(grp, leader);
            const bool away = alive == 0 || !view.playerValid ||
                              Dist2D(leader.X(), leader.Z(), view.playerX, view.playerZ) > edge;
            if (away && ops->BodiesUnseen(grp))
            {
                ops->Remove(grp);
                const int home = MinInt(alive, l.living);
                if (l.living > home)
                {
                    OccupierForces::Instance().NoteLoss(l.living - home, "left behind");
                }
                CrewHome(registry, view, l.source, home);
                l.living = 0;
                l.group = nullptr;
            }
        }
        else if (l.living > 0)
        {
            // the group was deleted elsewhere: its stragglers never came home
            OccupierForces::Instance().NoteLoss(l.living, "left behind");
            l.living = 0;
        }
        if (!l.hull.GetLink() && !l.group.GetLink())
        {
            _leftovers.Delete(i);
            continue;
        }
        i++;
    }
}

void CounterAttack::CrewHome(ZoneRegistry& registry, const CounterAttackView& view, const char* source, int crew)
{
    if (crew <= 0)
    {
        return;
    }
    const int src = registry.FindZoneIndex(source);
    ZoneRecord* s = registry.GetZoneMutable(src);
    if (!s || stricmp(s->owner, registry.OccupierSide()) != 0)
    {
        OccupierForces::Instance().Refund(crew, "no base");
        return;
    }
    const int live = src < view.garrisonLive.Size() ? view.garrisonLive[src] : 0;
    const float room = s->garrisonMax - (float)live - floorf(s->garrison) - (float)s->pending;
    const int back = MinInt(crew, room > 0 ? (int)floorf(room) : 0);
    GarrisonCache::AddReserve(registry, src, back, src, view.playerValid, view.playerX, view.playerZ);
    if (crew > back)
    {
        OccupierForces::Instance().Refund(crew - back, "surplus");
    }
}

bool CounterAttack::CheckHull(ZoneRegistry& registry, Record& r, CounterAttackOps* ops)
{
    int crewAlive = 0;
    const int state = ops->HullState(r.hull, r.crewGroup, crewAlive);
    if (state == CounterAttackOps::HullIntact && crewAlive > 0)
    {
        return false;
    }
    // destroyed, deleted, or nobody left to drive it (abandoned where it
    // stands): lost.  The dead crew count lost to the pool (L4: only the
    // dead); the crew group and an abandoned intact hull go to the unseen
    // cleanup, living stragglers home as bodies from there (M3)
    const int price = r.HullPrice();
    const int living = LivingCrew(ops, r.crewGroup, crewAlive, r.crew);
    OccupierForces::Instance().NoteLoss(r.crew - living, "crew");
    LOG_INFO(Core, "CounterAttack: [op {}] the {} of the force on {} is lost ({}; {} of {} crew alive)", r.opId,
             (const char*)r.hullClass, (const char*)r.target,
             state == CounterAttackOps::HullDestroyed ? "destroyed"
                                                      : (state == CounterAttackOps::HullGone ? "gone" : "abandoned"),
             living, r.crew);
    if (!r.dismounted && r.group)
    {
        ops->Dismount(r.group, r.hull);
        r.dismounted = true;
    }
    AddLeftover(r.source, state == CounterAttackOps::HullIntact ? r.hull.GetLink() : nullptr, r.crewGroup, living);
    r.hull = nullptr;
    r.crewGroup = nullptr;
    r.hullDone = true;
    r.crew = 0;
    r.priceUnits = 0; // settled: nothing more owed on the carrier
    if (r.abstractSpeed > 0)
    {
        r.abstractSpeed = FootSpeed; // on foot from here
        r.travel = 0;                // L5: a later return to the road re-times the leg at foot pace
    }
    Emit(registry, r, CAOpHullLost, 0, price, state == CounterAttackOps::HullDestroyed ? "destroyed" : "abandoned");
    return true;
}

bool CounterAttack::CheckEscort(ZoneRegistry& registry, Record& r, CounterAttackOps* ops)
{
    int crewAlive = 0;
    const int state = ops->HullState(r.escort, r.escortCrewGroup, crewAlive);
    if (state == CounterAttackOps::HullIntact && crewAlive > 0)
    {
        return false;
    }
    // destroyed, deleted or crewless: lost, its dead crew to the losses,
    // the group and an abandoned intact hull to the unseen cleanup (M3).
    // The column carries on without it (the infantry rides the carrier).
    const int price = r.EscortHullPrice();
    const int living = LivingCrew(ops, r.escortCrewGroup, crewAlive, r.escortCrew);
    OccupierForces::Instance().NoteLoss(r.escortCrew - living, "escort crew");
    LOG_INFO(Core, "CounterAttack: [op {}] the escort {} of the force on {} is lost ({}; {} of {} crew alive)",
             r.opId, (const char*)r.escortClass, (const char*)r.target,
             state == CounterAttackOps::HullDestroyed ? "destroyed"
                                                      : (state == CounterAttackOps::HullGone ? "gone" : "abandoned"),
             living, r.escortCrew);
    AddLeftover(r.source, state == CounterAttackOps::HullIntact ? r.escort.GetLink() : nullptr, r.escortCrewGroup,
                living);
    r.escort = nullptr;
    r.escortCrewGroup = nullptr;
    r.escortDone = true;
    r.escortCrew = 0;
    r.escortPriceUnits = 0;
    Emit(registry, r, CAOpHullLost, 0, price,
         state == CounterAttackOps::HullDestroyed ? "escort destroyed" : "escort abandoned");
    return true;
}

void CounterAttack::FinishEscort(ZoneRegistry& registry, const CounterAttackView& view, Record& r,
                                 CounterAttackOps* ops)
{
    if (!r.HasEscort() || r.escortDone)
    {
        return;
    }
    r.escortDone = true;
    if (LiveEscortOf(r) > 0)
    {
        int crewAlive = 0;
        const int state = ops ? ops->HullState(r.escort, r.escortCrewGroup, crewAlive) : (int)CounterAttackOps::HullGone;
        const int living = LivingCrew(ops, r.escortCrewGroup, crewAlive, r.escortCrew);
        OccupierForces::Instance().NoteLoss(r.escortCrew - living, "escort crew");
        if (state == CounterAttackOps::HullIntact && crewAlive > 0)
        {
            // it drives home and is deleted only where nobody sees it; both
            // prices come back apart (each hull its own HomeHull).  Its whole
            // living crew goes with it (the home deletion waits for them all)
            SendHullHome(r, r.escort, r.escortCrewGroup, living, ops, registry, r.EscortHullPrice(), r.escortClass);
        }
        else
        {
            // lost: the group and an abandoned intact hull to the cleanup (M3)
            AddLeftover(r.source, state == CounterAttackOps::HullIntact ? r.escort.GetLink() : nullptr,
                        r.escortCrewGroup, living);
            Emit(registry, r, CAOpHullLost, 0, r.EscortHullPrice(), "escort lost at the end of the operation");
        }
        r.escort = nullptr;
        r.escortCrewGroup = nullptr;
        return;
    }
    // an abstract escort (never born, or back on the abstract road): home
    // unseen at once, its price back, its crew home as bodies
    CrewHome(registry, view, r.source, r.escortCrew);
    Emit(registry, r, CAOpHullRefund, r.escortCrew, r.EscortHullPrice(), "escort abstract: home");
}

void CounterAttack::FinishHull(ZoneRegistry& registry, const CounterAttackView& view, Record& r, CounterAttackOps* ops)
{
    FinishEscort(registry, view, r, ops);
    if (r.airId > 0)
    {
        SendAirHome(r); // phase 6A: OccupierAir flies it home and removes it unseen
    }
    if (!r.HasHullPackage() || r.hullDone)
    {
        return;
    }
    r.hullDone = true;
    if (LiveHullOf(r) > 0)
    {
        int crewAlive = 0;
        const int state = ops ? ops->HullState(r.hull, r.crewGroup, crewAlive) : (int)CounterAttackOps::HullGone;
        const int living = LivingCrew(ops, r.crewGroup, crewAlive, r.crew);
        OccupierForces::Instance().NoteLoss(r.crew - living, "crew");
        if (state == CounterAttackOps::HullIntact && crewAlive > 0)
        {
            // it drives home and is deleted only where nobody sees it (its
            // whole living crew with it: the home deletion waits for them all)
            SendHullHome(r, r.hull, r.crewGroup, living, ops, registry);
        }
        else
        {
            // lost: the group and an abandoned intact hull to the cleanup (M3)
            AddLeftover(r.source, state == CounterAttackOps::HullIntact ? r.hull.GetLink() : nullptr, r.crewGroup,
                        living);
            Emit(registry, r, CAOpHullLost, 0, r.HullPrice(), "lost at the end of the operation");
        }
        r.hull = nullptr;
        r.crewGroup = nullptr;
        return;
    }
    // an abstract hull (never born, or back on the abstract road) drives
    // home unseen at once: its price back, its crew home as bodies
    CrewHome(registry, view, r.source, r.crew);
    if (r.airReplanned)
    {
        OccupierForces::Instance().Refund(r.HullPrice(), "air re-plan hull home");
        Emit(registry, r, CAOpHullRefund, r.crew, 0, "abstract: home");
        return;
    }
    Emit(registry, r, CAOpHullRefund, r.crew, r.HullPrice(), "abstract: home");
}

void CounterAttack::SendHullHome(const Record& r, Entity* hull, AIGroup* crewGroup, int crewAlive,
                                 CounterAttackOps* ops, const ZoneRegistry& registry, int units, const char* hullClass)
{
    HomeHull h;
    h.opId = r.opId;
    h.kind = r.kind;
    h.origin = r.origin;
    h.target = r.target;
    h.source = r.source;
    h.units = units >= 0 ? units : r.HullPrice();
    h.crew = crewAlive;
    h.retry = 0;
    h.hull = hull;
    h.crewGroup = crewGroup;
    _homeHulls.Add(h);
    const ZoneRecord* src = registry.GetZone(registry.FindZoneIndex(r.source));
    if (ops && src)
    {
        ops->DriveOn(crewGroup, src->pos);
    }
    LOG_INFO(Core, "CounterAttack: [op {}] the {} drives home to {} ({} crew)", r.opId,
             hullClass ? hullClass : (const char*)r.hullClass, (const char*)r.source, crewAlive);
}

void CounterAttack::StepHomeHulls(ZoneRegistry& registry, const CounterAttackView& view, float elapsed,
                                  CounterAttackOps* ops)
{
    if (!ops)
    {
        return;
    }
    for (int i = 0; i < _homeHulls.Size();)
    {
        HomeHull& h = _homeHulls[i];
        Record ref;
        ref.opId = h.opId;
        ref.kind = h.kind;
        ref.origin = h.origin;
        ref.target = h.target;
        ref.source = h.source;
        int crewAlive = 0;
        const int state = ops->HullState(h.hull, h.crewGroup, crewAlive);
        const int living = LivingCrew(ops, h.crewGroup, crewAlive, h.crew);
        if (state != CounterAttackOps::HullIntact || crewAlive == 0)
        {
            // the dead crew lost; the group and an abandoned intact hull to
            // the unseen cleanup (M3)
            OccupierForces::Instance().NoteLoss(h.crew - living, "crew");
            AddLeftover(h.source, state == CounterAttackOps::HullIntact ? h.hull.GetLink() : nullptr, h.crewGroup,
                        living);
            LOG_INFO(Core, "CounterAttack: [op {}] the hull driving home to {} is lost", h.opId, (const char*)h.source);
            Emit(registry, ref, CAOpHullLost, 0, h.units, "lost on the way home");
            _homeHulls.Delete(i);
            continue;
        }
        h.retry -= elapsed;
        if (h.retry > 0)
        {
            i++;
            continue;
        }
        h.retry = RetryInterval;
        if (ops->HullUnseen(h.hull, h.crewGroup))
        {
            // every crewman is deleted with it (HullUnseen waited for them
            // all): the living home, the dead lost
            if (living < h.crew)
            {
                OccupierForces::Instance().NoteLoss(h.crew - living, "crew");
            }
            ops->RemoveHull(h.hull, h.crewGroup);
            CrewHome(registry, view, h.source, living);
            LOG_INFO(Core, "CounterAttack: [op {}] the hull is home unseen ({} crew back at {})", h.opId, living,
                     (const char*)h.source);
            Emit(registry, ref, CAOpHullRefund, living, h.units, "home");
            _homeHulls.Delete(i);
            continue;
        }
        const ZoneRecord* src = registry.GetZone(registry.FindZoneIndex(h.source));
        if (src)
        {
            ops->DriveOn(h.crewGroup, src->pos);
        }
        i++;
    }
}

void CounterAttack::AdoptLive(ZoneRegistry& registry, int target, Record& r, CounterAttackOps* ops, int alive)
{
    ZoneRecord* town = registry.GetZoneMutable(target);
    if (!town || !r.group)
    {
        return;
    }
    if (LiveHullOf(r) > 0 && !r.dismounted)
    {
        ops->Dismount(r.group, r.hull);
        r.dismounted = true;
    }
    if (alive < r.strength)
    {
        OccupierForces::Instance().NoteLoss(r.strength - alive, "killed in the retake"); // L4
    }
    ops->Adopt(target, r.group);
    // the squad IS (part of) the new garrison: it comes off the refill (U2),
    // never assigned over the reserve; the refill bodies it replaces were
    // pool-funded and go back to the pool (5.5, settled once)
    const int cut = SubtractAdopted(*town, alive);
    OccupierForces::Instance().Refund(cut, "adopted squad");
    LOG_INFO(Core,
             "CounterAttack: {} soldier(s) stay in {} as its garrison ({} taken off the refill; reserve {}, "
             "pending {})",
             alive, (const char*)r.target, cut, town->garrison, town->pending);
    Emit(registry, r, CAOpSettled, alive, cut, "adopted");
}

int CounterAttack::SubtractAdopted(ZoneRecord& town, int alive)
{
    // from the pending deposit first (where an in-bubble refill went), then
    // from the reserve; only as much as the refill added
    int cut = MinInt(alive > 0 ? alive : 0, town.revertRefill > 0 ? town.revertRefill : 0);
    const int fromPending = MinInt(cut, town.pending);
    town.pending -= fromPending;
    if (town.pending <= 0)
    {
        town.pending = 0;
        town.pendingOrigin = -1;
    }
    cut -= fromPending;
    const int fromReserve = MinInt(cut, (int)floorf(town.garrison > 0 ? town.garrison : 0));
    town.garrison = floorf(town.garrison > 0 ? town.garrison : 0) - (float)fromReserve;
    town.revertRefill = 0;
    return fromPending + fromReserve;
}

void CounterAttack::Step(ZoneRegistry& registry, const CounterAttackView& view, float elapsed, CounterAttackOps* ops,
                         AutoArray<ZoneEventRecord>& fired)
{
    if (view.mapSize > 0)
    {
        _mapSize = view.mapSize; // phase 6A: the off-map edge (RouteOf reads it)
    }
    if (!view.playerValid)
    {
        return; // a dead player freezes every campaign clock
    }
    if (view.won || view.lost)
    {
        return; // 7.4, 9.2: once the win (or lose) latches nothing in flight steps
    }
    for (int i = 0; i < _records.Size();)
    {
        if (Advance(registry, view, elapsed, ops, _records[i], fired))
        {
            // the operation ended: its hull drives home (or is refunded)
            FinishHull(registry, view, _records[i], ops);
            _records.Delete(i);
        }
        else
        {
            i++;
        }
    }
    StepHomeHulls(registry, view, elapsed, ops);
    StepAirHomes(registry, view, ops);
    StepLeftovers(registry, view, elapsed, ops);
}

// ---------------------------------------------------------------------------
// engine path
// ---------------------------------------------------------------------------

namespace
{

Vector3 Grounded(float x, float z, float y)
{
    if (GLandscape)
    {
        float dx, dz;
        y = GLOB_LAND->SurfaceYAboveWater(x, z, &dx, &dz);
    }
    return Vector3(x, y, z);
}

// m from a zone centre a road link may be to anchor the route there
constexpr float RouteSnapRadius = 300.0f;
// m above the ground the terrain ray aims (a standing man)
constexpr float InfantryLosHeight = 1.8f;
// ... and for a ground hull (a car roof, Traffic::LosTargetHeight)
constexpr float VehicleLosHeight = 2.5f;

float Dist2DSq(Vector3Par a, Vector3Par b)
{
    const float dx = a.X() - b.X();
    const float dz = a.Z() - b.Z();
    return dx * dx + dz * dz;
}

// the road link whose centre is nearest pos within radius (the Traffic road
// survey idiom), null when none
const RoadLink* NearestRoadLink(Vector3Par pos, float radius)
{
    if (!GRoadNet)
    {
        return nullptr;
    }
    const RoadLink* best = nullptr;
    float bestD2 = radius * radius;
    const int cells = toIntCeil(radius * InvLandGrid);
    const int cx = toIntFloor(pos.X() * InvLandGrid);
    const int cz = toIntFloor(pos.Z() * InvLandGrid);
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
                if (!link)
                {
                    continue;
                }
                const float d2 = Dist2DSq(link->GetCenter(), pos);
                if (d2 < bestD2)
                {
                    bestD2 = d2;
                    best = link;
                }
            }
        }
    }
    return best;
}

class EngineOps final : public CounterAttackOps
{
  public:
    // the per-step perception snapshot (Simulate, before Step)
    void Refresh(const CounterAttackView& view)
    {
        _cam = PerceptionCamera::Capture();
        const TrafficTuning& tt = Traffic::Instance().Tuning();
        _audible = tt.minSpawnDist;
        // infantry shows no headlights: the object cull alone bounds what is drawn
        _band = Traffic::EffectiveBand(tt, ENGINE_CONFIG.objectsZ, false, ENGINE_CONFIG.horizontZ);
        // a ground hull's AI crew lights up at night (M8): Traffic's
        // conservative lights bound, past the darkest headlight threshold
        bool night = false;
        if (GScene && GScene->MainLight())
        {
            night = GScene->MainLight()->NightEffect() > Traffic::HeadlightNightEffect;
        }
        _hullBand = Traffic::EffectiveBand(tt, ENGINE_CONFIG.objectsZ, night, ENGINE_CONFIG.horizontZ);
        _hullGate = false;
        _playerValid = view.playerValid;
        _player = Vector3(view.playerX, 0, view.playerZ);
    }

    void SetHullGate(bool on) override { _hullGate = on; }

    // Traffic's spawn gate (issue #53) for a squad on foot (a ground hull
    // while SetHullGate is on: the headlight band, a roof-high ray): the config
    // minSpawnDist is the hard audible floor (from the player), then the
    // exposure legs from the camera, tiered: past the cull + margin or
    // terrain-hidden = hidden (survives a turn-around), outside the view cone
    // only = cone-only.  Never in the sea.
    int Exposure(Vector3Par pos) const override
    {
        if (!_playerValid)
        {
            return CAExposeSeen;
        }
        if (GLandscape && GLandscape->SurfaceY(pos.X(), pos.Z()) < 0.5f)
        {
            return CAExposeSeen;
        }
        if (Dist2DSq(pos, _player) < _audible * _audible)
        {
            return CAExposeSeen; // audible even unseen
        }
        const float camD2 = Dist2DSq(pos, _cam.hasCamera ? _cam.camPos : _player);
        const TrafficEffectiveBand& band = _hullGate ? _hullBand : _band;
        if (camD2 >= band.minSpawn * band.minSpawn)
        {
            return CAExposeHidden; // past the draw distance (a hull's headlights at night)
        }
        // inside the draw distance: the terrain ray decides "hidden" whichever
        // way the camera looks (no camera: nothing hides anything)
        if (_cam.LosBlocked(pos, _hullGate ? VehicleLosHeight : InfantryLosHeight))
        {
            return CAExposeHidden;
        }
        return _cam.InFrustum(pos, Traffic::FrustumCosHalfAngle) ? CAExposeSeen : CAExposeConeOnly;
    }

    // the road route (RoadRoute.hpp): link centres from a link near the base
    // to a link near the town, then the town centre.  A base whose nearest
    // roads are a pocket of their own starts from the first link outside it;
    // a base whose roads the island never joined to the town's network
    // leaves along its own road, crosses country to the town network's best
    // link within RoadRouteServiceJoinRadius on its own landmass, and
    // follows the road from there, instead of a straight line through its
    // walls.  A base on another landmass (Malden's Southeast Compound, an
    // island of its own) has no route and is never a ground source
    // (PickSource, Resource, OccupierForces::BuildSources).
    void BuildRoute(Vector3Par from, Vector3Par to, AutoArray<Vector3>& route) override
    {
        route.Clear();
        RoadRouteParams params;
        params.snapRadius = RouteSnapRadius;
        params.joinRadius = RoadRouteServiceJoinRadius;
        AutoArray<Vector3> centres;
        RoadRouteStats st;
        if (!RoadRoute::Search(from, to, params, centres, &st))
        {
            if (st.goalCandidates == 0)
            {
                LOG_INFO(Core, "CounterAttack: no road within {:.0f} m of [{:.0f}, {:.0f}]; straight line",
                         RouteSnapRadius, to.X(), to.Z());
            }
            else
            {
                LOG_INFO(Core,
                         "CounterAttack: no road route from [{:.0f}, {:.0f}] to [{:.0f}, {:.0f}] ({} links searched); "
                         "straight line",
                         from.X(), from.Z(), to.X(), to.Z(), st.linksSearched);
            }
            return;
        }
        for (int i = 0; i < centres.Size(); i++)
        {
            route.Add(Grounded(centres[i].X(), centres[i].Z(), centres[i].Y()));
        }
        route.Add(Grounded(to.X(), to.Z(), to.Y()));
        if (st.joined)
        {
            LOG_INFO(Core,
                     "CounterAttack: the roads at [{:.0f}, {:.0f}] are not joined to the town's; the route leaves "
                     "them at [{:.0f}, {:.0f}] and crosses {:.0f} m of country to the road at [{:.0f}, {:.0f}]",
                     from.X(), from.Z(), st.joinFrom.X(), st.joinFrom.Z(), st.joinGap, st.joinTo.X(), st.joinTo.Z());
        }
        else if (st.startIndex > 0)
        {
            LOG_INFO(Core,
                     "CounterAttack: the road nearest [{:.0f}, {:.0f}] is not joined to the town's; the route "
                     "starts {:.0f} m from it",
                     from.X(), from.Z(), st.startOffset);
        }
    }

    // on the road: SAFE keeps the unit planner on the road net
    // (AIUnit::CreatePath -> CreateRoadPath and the planner's road bonus are
    // both off while cautious); the squad still fires at will, and contact
    // raises it to COMBAT on its own
    static void MarchPosture(AIGroup* grp)
    {
        grp->SetCombatModeMajor(CMSafe);
        if (grp->MainSubgroup())
        {
            grp->MainSubgroup()->SetFormation(AI::FormColumn);
        }
        SemaphoreAll(grp);
    }

    static void SemaphoreAll(AIGroup* grp)
    {
        grp->SetSemaphore(AI::SemaphoreRed);
        PackedBoolArray all;
        for (int i = 0; i < MAX_UNITS_PER_GROUP; i++)
        {
            if (grp->UnitWithID(i + 1))
            {
                all.Set(i, true);
            }
        }
        grp->SendSemaphore(AI::SemaphoreRed, all);
    }

    // an armed hull at the town edge: AWARE and firing, driven up to edgePos
    void Support(AIGroup* crewGroup, Vector3Par edgePos) override
    {
        if (!crewGroup)
        {
            return;
        }
        Posture(crewGroup);
        DriveOn(crewGroup, edgePos);
        GetNetworkManager().UpdateObject(crewGroup);
    }

    void Assault(AIGroup* grp, Vector3Par town) override
    {
        if (!grp)
        {
            return;
        }
        Posture(grp);
        MoveOn(grp, town);
        GetNetworkManager().UpdateObject(grp);
    }

    // in the town: AWARE, fire at will
    static void Posture(AIGroup* grp)
    {
        grp->SetCombatModeMajor(CMAware);
        SemaphoreAll(grp);
    }

    // the doMove idiom (Traffic::IssueRoute): one Command::Move to every
    // unit through IssueCommand, never an arcade waypoint or AIGroup::Move
    void MoveOn(AIGroup* grp, Vector3Par target) override
    {
        if (!grp)
        {
            return;
        }
        Command cmd;
        cmd._message = Command::Move;
        cmd._destination = Grounded(target.X(), target.Z(), target.Y());
        cmd._discretion = Command::Undefined;
        cmd._context = Command::CtxMission;
        cmd._id = grp->GetNextCmdId();
        PackedBoolArray all;
        for (int u = 0; u < MAX_UNITS_PER_GROUP; u++)
        {
            AIUnit* unit = grp->UnitWithID(u + 1);
            if (unit && unit->GetLifeState() == AIUnit::LSAlive)
            {
                all.Set(u, true);
            }
        }
        grp->IssueCommand(cmd, all);
    }

    AIGroup* SpawnSquad(int n, Vector3Par from, Vector3Par firstLeg) override
    {
        ZoneRegistry& registry = ZoneRegistry::Instance();
        const RString side = registry.OccupierSide();
        const float warLevel = OccupierForces::Instance().OccupierWarLevel();
        const RString tier = registry.FactionTierClass(side, warLevel);
        if (tier.GetLength() == 0)
        {
            return nullptr;
        }
        AICenter* center = EnsureSideCenter(side);
        AIGroup* grp = center ? CreateSideGroup(center) : nullptr;
        if (!grp)
        {
            return nullptr;
        }
        AutoArray<RString> classes;
        registry.FactionSquad(side, warLevel, n, classes);
        for (int u = 0; u < n; u++)
        {
            const RString cls = u < classes.Size() ? classes[u] : tier;
            const float x = from.X() + (GRandGen.RandomValue() * 2.0f - 1.0f) * 12.0f;
            const float z = from.Z() + (GRandGen.RandomValue() * 2.0f - 1.0f) * 12.0f;
            ::CreateUnit(grp, cls, Grounded(x, z, from.Y()), RString(), u == 0 ? 0.6f : 0.5f,
                         u == 0 ? RankSergeant : RankPrivate);
        }
        if (grp->NUnits() == 0)
        {
            grp->RemoveFromCenter();
            return nullptr;
        }
        MarchPosture(grp);
        MoveOn(grp, firstLeg);
        GetNetworkManager().UpdateObject(grp);
        return grp;
    }

    AIGroup* DetachFromGarrison(int sourceZone, int n, Vector3Par firstLeg, int& taken) override
    {
        taken = 0;
        const GarrisonCache& cache = GarrisonCache::Instance();
        // non-leaders first, last group first: the post keeps its sergeant
        OLinkArray<AIUnit> units;
        for (int pass = 0; pass < 2 && units.Size() < n; pass++)
        {
            for (int g = cache.NGroups(sourceZone) - 1; g >= 0 && units.Size() < n; g--)
            {
                AIGroup* grp = cache.GetGroup(sourceZone, g);
                if (!grp)
                {
                    continue;
                }
                for (int u = MAX_UNITS_PER_GROUP - 1; u >= 0 && units.Size() < n; u--)
                {
                    AIUnit* unit = grp->UnitWithID(u + 1);
                    if (!unit || unit->GetLifeState() != AIUnit::LSAlive)
                    {
                        continue;
                    }
                    const bool leader = unit == grp->Leader();
                    if (leader != (pass == 1))
                    {
                        continue;
                    }
                    units.Add(unit);
                }
            }
        }
        if (units.Size() == 0)
        {
            return nullptr;
        }
        AICenter* center = EnsureSideCenter(ZoneRegistry::Instance().OccupierSide());
        AIGroup* grp = center ? CreateSideGroup(center) : nullptr;
        if (!grp)
        {
            return nullptr;
        }
        ::ProcessJoinGroups(units, grp);
        taken = grp->NUnits();
        if (taken == 0)
        {
            grp->RemoveFromCenter();
            return nullptr;
        }
        MarchPosture(grp);
        MoveOn(grp, firstLeg);
        GetNetworkManager().UpdateObject(grp);
        return grp;
    }

    int AliveWithin(AIGroup* grp, Vector3Par centre, float radius) override
    {
        if (!grp)
        {
            return 0;
        }
        int n = 0;
        const float rSq = radius * radius;
        for (int u = 0; u < MAX_UNITS_PER_GROUP; u++)
        {
            AIUnit* unit = grp->UnitWithID(u + 1);
            if (unit && unit->GetLifeState() == AIUnit::LSAlive && Dist2DSq(unit->Position(), centre) <= rSq)
            {
                n++;
            }
        }
        return n;
    }

    int Alive(AIGroup* grp, Vector3& leaderPos) override
    {
        if (!grp)
        {
            return 0;
        }
        int alive = 0;
        for (int u = 0; u < MAX_UNITS_PER_GROUP; u++)
        {
            AIUnit* unit = grp->UnitWithID(u + 1);
            if (unit && unit->GetLifeState() == AIUnit::LSAlive)
            {
                if (alive == 0)
                {
                    leaderPos = unit->Position();
                }
                alive++;
            }
        }
        AIUnit* leader = grp->Leader();
        if (leader && leader->GetLifeState() == AIUnit::LSAlive)
        {
            leaderPos = leader->Position();
        }
        return alive;
    }

    void Remove(AIGroup* grp) override
    {
        if (!grp)
        {
            return;
        }
        // collect first - DeleteVehicle mutates the unit table (the
        // GarrisonCache::DespawnGarrison idiom)
        AutoArray<Person*> bodies;
        for (int u = 0; u < MAX_UNITS_PER_GROUP; u++)
        {
            AIUnit* unit = grp->UnitWithID(u + 1);
            if (unit && unit->GetPerson())
            {
                bodies.Add(unit->GetPerson());
            }
        }
        for (int b = 0; b < bodies.Size(); b++)
        {
            DeletePerson(bodies[b]); // a seated man leaves his seat first
        }
        if (grp->NUnits() == 0)
        {
            grp->RemoveFromCenter();
        }
    }

    void Adopt(int townZone, AIGroup* grp) override { GarrisonCache::Instance().AdoptGroup(townZone, grp); }

    // Traffic's despawn rule for every living body: outside the view cone
    // and (past the draw distance or terrain-hidden), never inside the
    // audible close hold.  No camera: the distance rule alone.
    bool SquadUnseen(AIGroup* grp) const override
    {
        if (!grp || !_playerValid)
        {
            return false;
        }
        for (int u = 0; u < MAX_UNITS_PER_GROUP; u++)
        {
            AIUnit* unit = grp->UnitWithID(u + 1);
            if (!unit || unit->GetLifeState() != AIUnit::LSAlive)
            {
                continue;
            }
            if (!PointUnseen(unit->Position()))
            {
                return false;
            }
        }
        return true;
    }

    // anyone aboard (crew or cargo, the player included)
    bool HullOccupied(Entity* hull) const override
    {
        const Transport* veh = dyn_cast<Transport>(hull);
        return veh && (veh->Driver() || veh->Gunner() || veh->Commander() || veh->GetManCargoSize() > 0);
    }

    // every member, living or dead (the leftover cleanup deletes corpses)
    bool BodiesUnseen(AIGroup* grp) const override
    {
        if (!grp || !_playerValid)
        {
            return false;
        }
        for (int u = 0; u < MAX_UNITS_PER_GROUP; u++)
        {
            AIUnit* unit = grp->UnitWithID(u + 1);
            Person* body = unit ? unit->GetPerson() : nullptr;
            if (body && !PointUnseen(body->Position()))
            {
                return false;
            }
        }
        return true;
    }

    // the despawn rule for one point (SquadUnseen, HullUnseen); hull: a
    // ground hull's point (the headlight band at night, a roof-high ray, M8)
    bool PointUnseen(Vector3Par pos, bool hull = false) const
    {
        if (!_playerValid)
        {
            return false;
        }
        const TrafficEffectiveBand& band = hull ? _hullBand : _band;
        if (!_cam.hasCamera)
        {
            return Dist2DSq(pos, _player) > band.despawnEdge * band.despawnEdge;
        }
        const float camD2 = Dist2DSq(pos, _cam.camPos);
        if (Dist2DSq(pos, _player) < band.closeHold * band.closeHold)
        {
            return false;
        }
        const bool inFrustum = _cam.InFrustum(pos, Traffic::FrustumCosHalfAngle);
        const bool losBlocked = !inFrustum && camD2 < band.minSpawn * band.minSpawn &&
                                _cam.LosBlocked(pos, hull ? VehicleLosHeight : InfantryLosHeight);
        return Traffic::CanExposeDespawn(camD2, losBlocked, inFrustum, band);
    }

    // ground hulls (OCCUPIER-FORCES.md 3.2) ----------------------------------

    // mirrors Traffic::CreateTrafficVehicle (VehCreate minus the script
    // parsing): the hull oriented along dir and dropped on the road surface
    static Transport* CreateHull(const RString& type, Vector3Par where, Vector3Par dir)
    {
        if (!GWorld || !GLandscape || type.GetLength() == 0)
        {
            return nullptr;
        }
        Ref<Entity> veh = NewNonAIVehicle(type, nullptr);
        if (!veh)
        {
            return nullptr;
        }
        Transport* transport = dyn_cast<Transport>(veh.GetRef());
        if (!transport || veh->GetNonAIType()->IsKindOf(GWorld->Preloaded(VTypeStatic)))
        {
            LOG_WARN(Core, "CounterAttack: class '{}' is not a vehicle hull", (const char*)type);
            return nullptr;
        }
        Vector3 pos = where;
        Vector3 normal = VUp;
        if (AIUnit::FindFreePosition(pos, normal, false, transport))
        {
            float dx, dz;
            pos[1] = GLOB_LAND->RoadSurfaceYAboveWater(pos[0], pos[2]);
            GLOB_LAND->SurfaceYAboveWater(pos[0], pos[2], &dx, &dz);
            normal = Vector3(-dx, 1, -dz);
        }
        Vector3 heading = dir;
        heading[1] = 0;
        if (heading.SquareSize() < 1e-4f)
        {
            heading = VForward;
        }
        Matrix3 orient;
        Matrix4 transform;
        transform.SetPosition(pos);
        orient.SetUpAndDirection(normal, heading.Normalized());
        transform.SetOrientation(orient);
        veh->PlaceOnSurface(transform);
        veh->SetTransform(transform);
        veh->Init(transform);
        GWorld->AddVehicle(veh);
        if (GWorld->GetMode() == GModeNetware)
        {
            GetNetworkManager().CreateVehicle(veh, VLTVehicle, "", -1);
        }
        return transport;
    }

    // one soldier of type in grp near the hull, seated at position (the
    // Traffic::CreateCrewman idiom); null when the class refused to
    // materialize.  seated reports whether the seat took him.
    static Person* CreateSeated(AIGroup* grp, const RString& type, Vector3Par near, Transport* veh, int position,
                                bool& seated, float skill, Rank rank)
    {
        seated = false;
        if (!grp || type.GetLength() == 0)
        {
            return nullptr;
        }
        AIUnit* before[MAX_UNITS_PER_GROUP];
        for (int i = 0; i < MAX_UNITS_PER_GROUP; i++)
        {
            before[i] = grp->UnitWithID(i + 1);
        }
        ::CreateUnit(grp, type, near, RString(), skill, rank);
        AIUnit* unit = nullptr;
        for (int i = 0; i < MAX_UNITS_PER_GROUP; i++)
        {
            AIUnit* u = grp->UnitWithID(i + 1);
            if (u && u != before[i])
            {
                unit = u;
                break;
            }
        }
        Person* person = unit ? unit->GetPerson() : nullptr;
        if (person && veh)
        {
            seated = ::NativeMoveIn(person, veh, (GetInPosition)position);
        }
        return person;
    }

    // a seated man is moved out of his vehicle (no sound, where the vehicle
    // stands: the caller deletes him and the vehicle unseen) before he is
    // deleted.  ::DeleteVehicle only flags a seated man: he stays in the
    // world's out-vehicle list, which never purges deleted entries, so every
    // crewman and rider deleted in his seat lived on as an invisible object
    // (and was written into every save).
    static void LeaveVehicle(Person* person)
    {
        AIUnit* unit = person ? person->Brain() : nullptr;
        Transport* veh = unit ? unit->GetVehicleIn() : nullptr;
        if (!veh || !person->IsLocal())
        {
            return;
        }
        const Matrix4 at = veh->WorldTransform();
        if (veh->Driver() == person)
        {
            veh->GetOutDriver(at, false);
        }
        else if (veh->Gunner() == person)
        {
            veh->GetOutGunner(at, false);
        }
        else if (veh->Commander() == person)
        {
            veh->GetOutCommander(at, false);
        }
        else
        {
            veh->GetOutCargo(person, at, false);
        }
    }

    static void DeletePerson(Person* person)
    {
        if (!person)
        {
            return;
        }
        LeaveVehicle(person);
        if (person->IsLocal())
        {
            ::DeleteVehicle(person);
        }
        else
        {
            GetNetworkManager().AskForDeleteVehicle(person);
        }
    }

    AIGroup* SpawnMounted(const RString& hullClass, int crew, int n, Vector3Par from, Vector3Par dir, Entity*& hullOut,
                          AIGroup*& crewOut) override
    {
        hullOut = nullptr;
        crewOut = nullptr;
        ZoneRegistry& registry = ZoneRegistry::Instance();
        const RString side = registry.OccupierSide();
        const float warLevel = OccupierForces::Instance().OccupierWarLevel();
        const RString crewType = registry.FactionTierClass(side, warLevel);
        AICenter* center = crewType.GetLength() > 0 ? EnsureSideCenter(side) : nullptr;
        if (!center)
        {
            return nullptr;
        }
        Transport* veh = CreateHull(hullClass, from, dir);
        if (!veh)
        {
            return nullptr;
        }
        AIGroup* crewGrp = CreateSideGroup(center);
        if (!crewGrp)
        {
            ::DeleteVehicle(veh);
            return nullptr;
        }
        Vector3 heading = dir;
        heading[1] = 0;
        heading = heading.SquareSize() > 1e-4f ? heading.Normalized() : VForward;
        const Vector3 foot = veh->Position() + heading * 4.0f;
        // the crew: a driver (no driver = no hull), then a gunner where the
        // hull has a turret; crew beyond the seats stay home
        bool seated = false;
        Person* driver = CreateSeated(crewGrp, crewType, foot, veh, GIPDriver, seated, 0.5f, RankCorporal);
        if (!driver || !seated || veh->Driver() != driver)
        {
            LOG_WARN(Core, "CounterAttack: could not seat a '{}' driver into '{}'", (const char*)crewType,
                     (const char*)hullClass);
            RemoveHull(veh, crewGrp);
            return nullptr;
        }
        // then a commander where the hull has a cupola (a tank, a BMP):
        // the crew count is hasDriver + hasGunner + hasCommander (6.5)
        int left = crew - 1;
        if (left > 0 && veh->Type()->HasGunner())
        {
            Person* gunner = CreateSeated(crewGrp, crewType, foot, veh, GIPGunner, seated, 0.5f, RankPrivate);
            if (gunner && !seated)
            {
                DeletePerson(gunner);
            }
            left -= gunner && seated ? 1 : 0;
        }
        if (left > 0 && veh->Type()->HasCommander())
        {
            Person* commander = CreateSeated(crewGrp, crewType, foot, veh, GIPCommander, seated, 0.5f, RankPrivate);
            if (commander && !seated)
            {
                DeletePerson(commander);
            }
        }
        // the infantry: a group of its own, boarded as cargo (TR UNLOAD and
        // the dismount address a foreign group's cargo)
        AIGroup* inf = CreateSideGroup(center);
        if (!inf)
        {
            RemoveHull(veh, crewGrp);
            return nullptr;
        }
        AutoArray<RString> classes;
        registry.FactionSquad(side, warLevel, n, classes);
        for (int u = 0; u < n; u++)
        {
            const RString cls = u < classes.Size() ? classes[u] : crewType;
            Person* man = CreateSeated(inf, cls, foot, veh, GIPCargo, seated, u == 0 ? 0.6f : 0.5f,
                                       u == 0 ? RankSergeant : RankPrivate);
            if (man && !seated)
            {
                DeletePerson(man); // no seat: he stays home (a partial spawn, refunded by the caller)
            }
            else if (man && man->Brain())
            {
                // seated as cargo the way the mission loader seats it
                // (AICenterImpl): the unit's state is InCargo, which the
                // dismount's DoGetOut requires (moveInCargo leaves it unset)
                man->Brain()->SetState(AIUnit::InCargo);
            }
        }
        if (inf->NUnits() == 0)
        {
            inf->RemoveFromCenter();
            RemoveHull(veh, crewGrp);
            return nullptr;
        }
        crewGrp->SetCombatModeMajor(CMSafe);
        SemaphoreAll(crewGrp);
        MarchPosture(inf);
        GetNetworkManager().UpdateObject(crewGrp);
        GetNetworkManager().UpdateObject(inf);
        hullOut = veh;
        crewOut = crewGrp;
        return inf;
    }

    // a column's escort (6.5, phase 5): the hull and its crew, no cargo; the
    // crew seated as SpawnMounted seats it (a driver, then a gunner and a
    // commander where the hull has them)
    Entity* SpawnEscort(const RString& hullClass, int crew, Vector3Par from, Vector3Par dir,
                        AIGroup*& crewOut) override
    {
        crewOut = nullptr;
        ZoneRegistry& registry = ZoneRegistry::Instance();
        const RString side = registry.OccupierSide();
        const float warLevel = OccupierForces::Instance().OccupierWarLevel();
        const RString crewType = registry.FactionTierClass(side, warLevel);
        AICenter* center = crewType.GetLength() > 0 ? EnsureSideCenter(side) : nullptr;
        if (!center)
        {
            return nullptr;
        }
        Transport* veh = CreateHull(hullClass, from, dir);
        if (!veh)
        {
            return nullptr;
        }
        AIGroup* crewGrp = CreateSideGroup(center);
        if (!crewGrp)
        {
            ::DeleteVehicle(veh);
            return nullptr;
        }
        Vector3 heading = dir;
        heading[1] = 0;
        heading = heading.SquareSize() > 1e-4f ? heading.Normalized() : VForward;
        const Vector3 foot = veh->Position() + heading * 4.0f;
        bool seated = false;
        Person* driver = CreateSeated(crewGrp, crewType, foot, veh, GIPDriver, seated, 0.5f, RankCorporal);
        if (!driver || !seated || veh->Driver() != driver)
        {
            LOG_WARN(Core, "CounterAttack: could not seat a '{}' driver into the escort '{}'", (const char*)crewType,
                     (const char*)hullClass);
            RemoveHull(veh, crewGrp);
            return nullptr;
        }
        int left = crew - 1;
        if (left > 0 && veh->Type()->HasGunner())
        {
            Person* gunner = CreateSeated(crewGrp, crewType, foot, veh, GIPGunner, seated, 0.5f, RankPrivate);
            if (gunner && !seated)
            {
                DeletePerson(gunner);
            }
            left -= gunner && seated ? 1 : 0;
        }
        if (left > 0 && veh->Type()->HasCommander())
        {
            Person* commander = CreateSeated(crewGrp, crewType, foot, veh, GIPCommander, seated, 0.5f, RankPrivate);
            if (commander && !seated)
            {
                DeletePerson(commander);
            }
        }
        crewGrp->SetCombatModeMajor(CMSafe);
        SemaphoreAll(crewGrp);
        GetNetworkManager().UpdateObject(crewGrp);
        crewOut = crewGrp;
        return veh;
    }

    // the Traffic::IssueRoute idiom: one Command::Move to the DRIVER through
    // IssueCommand (arcade waypoints never moved a seated crew in the
    // Traffic probes); SAFE keeps the planner on the road net
    void DriveOn(AIGroup* crewGroup, Vector3Par target) override
    {
        if (!crewGroup)
        {
            return;
        }
        Command cmd;
        cmd._message = Command::Move;
        cmd._destination = Grounded(target.X(), target.Z(), target.Y());
        cmd._discretion = Command::Undefined;
        cmd._context = Command::CtxMission;
        cmd._id = crewGroup->GetNextCmdId();
        PackedBoolArray drivers;
        bool any = false;
        for (int u = 0; u < MAX_UNITS_PER_GROUP; u++)
        {
            AIUnit* unit = crewGroup->UnitWithID(u + 1);
            if (!unit || unit->GetLifeState() != AIUnit::LSAlive)
            {
                continue;
            }
            Transport* in = unit->GetVehicleIn();
            if (in && in->DriverBrain() == unit)
            {
                drivers.Set(u, true);
                any = true;
            }
        }
        if (any)
        {
            crewGroup->IssueCommand(cmd, drivers);
        }
    }

    // the cargo gets out (the Traffic park dismount without UnassignVehicle,
    // which would clear every seat of the hull, the crew's included)
    void Dismount(AIGroup* grp, Entity* hull) override
    {
        if (!grp)
        {
            return;
        }
        for (int u = 0; u < MAX_UNITS_PER_GROUP; u++)
        {
            AIUnit* unit = grp->UnitWithID(u + 1);
            if (!unit || unit->GetLifeState() != AIUnit::LSAlive)
            {
                continue;
            }
            Transport* seat = unit->GetVehicleIn();
            unit->OrderGetIn(false);
            unit->AllowGetIn(false); // the squad fights on foot from here
            if (seat && (!hull || seat == hull))
            {
                unit->DoGetOut(seat, false);
            }
        }
        GetNetworkManager().UpdateObject(grp);
    }

    int HullState(Entity* hull, AIGroup* crewGroup, int& crewAlive) const override
    {
        crewAlive = 0;
        if (!hull)
        {
            return HullGone;
        }
        if (crewGroup)
        {
            for (int u = 0; u < MAX_UNITS_PER_GROUP; u++)
            {
                AIUnit* unit = crewGroup->UnitWithID(u + 1);
                if (unit && unit->GetLifeState() == AIUnit::LSAlive && unit->GetVehicleIn() == hull)
                {
                    crewAlive++;
                }
            }
        }
        return hull->IsDammageDestroyed() ? HullDestroyed : HullIntact;
    }

    float HullSpeed(Entity* hull) const override { return hull ? hull->Speed().Size() : 0.0f; }

    // the hull (vehicle band, M8) and every crewman outside it, living or
    // dead (M4: a gunner who bailed out next to the player is deleted with
    // the crew, so the crew waits until he too is unseen)
    bool HullUnseen(Entity* hull, AIGroup* crewGroup) const override
    {
        if (!hull || !PointUnseen(hull->Position(), true))
        {
            return false;
        }
        for (int u = 0; crewGroup && u < MAX_UNITS_PER_GROUP; u++)
        {
            AIUnit* unit = crewGroup->UnitWithID(u + 1);
            Person* body = unit ? unit->GetPerson() : nullptr;
            if (!body || (unit->GetLifeState() == AIUnit::LSAlive && unit->GetVehicleIn() == hull))
            {
                continue; // seated: the hull's point covers him
            }
            if (!PointUnseen(body->Position()))
            {
                return false;
            }
        }
        return true;
    }

    void RemoveHull(Entity* hull, AIGroup* crewGroup) override
    {
        // the crew FIRST: deleting a transport ejects its crew (3.2)
        if (crewGroup)
        {
            AutoArray<Person*> bodies;
            for (int u = 0; u < MAX_UNITS_PER_GROUP; u++)
            {
                AIUnit* unit = crewGroup->UnitWithID(u + 1);
                if (unit && unit->GetPerson())
                {
                    bodies.Add(unit->GetPerson());
                }
            }
            for (int b = 0; b < bodies.Size(); b++)
            {
                DeletePerson(bodies[b]);
            }
            if (crewGroup->NUnits() == 0)
            {
                crewGroup->RemoveFromCenter();
            }
        }
        if (hull)
        {
            if (hull->IsLocal())
            {
                ::DeleteVehicle(hull);
            }
            else
            {
                GetNetworkManager().AskForDeleteVehicle(hull);
            }
        }
    }

    // phase 6A air: OccupierAir's engine half ------------------------------
    int AirExposure(Vector3Par posAsl) const override
    {
        return OccupierAir::ExposureAt(OccupierAir::EngineViewer(), posAsl);
    }

    float GroundY(float x, float z) const override
    {
        return GLandscape ? GLOB_LAND->SurfaceYAboveWater(x, z) : 0.0f;
    }

    bool AirPickLz(Vector3Par target, const Vector3* awayFrom, Vector3& lz) override
    {
        RString why;
        return OccupierAir::Instance().PickLz(target, awayFrom, 0.0f, 0.0f, lz, why);
    }

    int AirInsert(const RString& hullClass, int n, Vector3Par birth, float altAgl, Vector3Par lz, Vector3Par target,
                  AIGroup*& cargo, Entity*& hull, AIGroup*& crewGroup) override
    {
        cargo = nullptr;
        hull = nullptr;
        crewGroup = nullptr;
        AirInsertionPlan plan;
        plan.heliClass = hullClass;
        plan.cargo = n;
        plan.birthPos = birth;
        plan.birthAlt = altAgl;
        plan.lz = lz;
        plan.target = target;
        plan.side = ZoneRegistry::Instance().OccupierSide();
        int id = -1;
        RString why;
        if (!OccupierAir::Instance().SpawnInsertion(plan, id, why) || id <= 0)
        {
            return 0;
        }
        const AirInsertion* rec = OccupierAir::Instance().FindInsertion(id);
        if (!rec || !rec->cargo)
        {
            OccupierAir::Instance().Abort(id, false); // no cargo: nothing to insert
            return 0;
        }
        cargo = rec->cargo;
        hull = rec->hull;
        crewGroup = rec->crew;
        return id;
    }

    int AirSortie(const RString& hullClass, Vector3Par birth, float altAgl, Vector3Par target, float sortieTime,
                  Entity*& hull, AIGroup*& crewGroup) override
    {
        hull = nullptr;
        crewGroup = nullptr;
        int id = -1;
        RString why;
        if (!OccupierAir::Instance().SpawnSortie(hullClass, birth, altAgl, target, sortieTime,
                                                 ZoneRegistry::Instance().OccupierSide(), id, why) ||
            id <= 0)
        {
            return 0;
        }
        const struct AirSortie* rec = OccupierAir::Instance().FindSortie(id);
        if (rec)
        {
            hull = rec->hull;
            crewGroup = rec->crew;
        }
        return id;
    }

    bool AirStatus(int airId, CounterAttackAirStatus& out) const override
    {
        out = CounterAttackAirStatus();
        const OccupierAir& air = OccupierAir::Instance();
        if (const AirInsertion* r = air.FindInsertion(airId))
        {
            out.state = r->core.state;
            out.landed = r->core.landed;
            out.landedOffset = r->core.landedOffset;
            out.landedAt = r->core.landedAt;
            out.failReason = r->core.failReason;
            out.crewRemoved = r->core.crewRemoved;
            out.gone = r->core.hullRemoved;
            out.hullLost = r->core.failReason == AFRHullLost;
            out.cargoBorn = r->cargoBorn;
            Transport* veh = dyn_cast<Transport>(r->hull.GetLink());
            for (int u = 0; r->cargo && u < MAX_UNITS_PER_GROUP; u++)
            {
                AIUnit* unit = r->cargo->UnitWithID(u + 1);
                if (unit && unit->GetLifeState() == AIUnit::LSAlive && veh && unit->GetVehicleIn() == veh)
                {
                    out.cargoSeated++;
                }
            }
            return true;
        }
        if (const struct AirSortie* r = air.FindSortie(airId))
        {
            out.sortie = true;
            out.state = r->core.state;
            out.failReason = r->core.failReason;
            out.loiter = r->core.state == ASSLoiter || (r->core.state >= ASSHome && r->core.failReason == AFRNone);
            out.crewRemoved = r->core.crewRemoved;
            out.gone = r->core.hullRemoved;
            out.hullLost = r->core.failReason == AFRHullLost;
            return true;
        }
        return false;
    }

    // the limited reveal: resistance soldiers (never the player) within
    // RevealRadius of a fresh reported position, at an accuracy below 1
    int AirRevealNear(int airId, Vector3Par pos, float ageSeconds) override
    {
        const ZoneRegistry& registry = ZoneRegistry::Instance();
        AICenter* center = FindSideCenter(registry.ResistanceSide());
        Person* player = GWorld ? GWorld->GetRealPlayer() : nullptr;
        int n = 0;
        for (int g = 0; center && g < center->NGroups() && n < RevealMax; g++)
        {
            AIGroup* grp = center->GetGroup(g);
            for (int u = 0; grp && u < MAX_UNITS_PER_GROUP && n < RevealMax; u++)
            {
                AIUnit* unit = grp->UnitWithID(u + 1);
                Person* man = unit ? unit->GetPerson() : nullptr;
                if (!man || man == player || unit->GetLifeState() != AIUnit::LSAlive ||
                    Dist2DSq(unit->Position(), pos) > RevealRadius * RevealRadius)
                {
                    continue;
                }
                n += OccupierAir::Instance().Reveal(airId, man, ageSeconds, RevealAccuracy) ? 1 : 0;
            }
        }
        return n;
    }

    static constexpr float RevealRadius = 100.0f;
    static constexpr float RevealAccuracy = 0.5f;
    static constexpr int RevealMax = 4;

  private:
    PerceptionCamera _cam;
    TrafficEffectiveBand _band;
    TrafficEffectiveBand _hullBand; // the night light band when AI headlights may be on (M8)
    bool _hullGate = false;         // Exposure rates points for a ground hull (SetHullGate)
    float _audible = 300.0f;
    bool _playerValid = false;
    Vector3 _player = VZero;
};

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
EngineOps& Ops()
{
    static EngineOps ops;
    return ops;
}
#pragma clang diagnostic pop

// the resistance HOLD presence within radius of pos (the HQ's defenders,
// CounterAttackView::hqDefenders): every living resistance unit but the
// real player, plus the player while PlayerHoldsPresence holds (8.1)
int HoldPresenceAt(const ZoneRegistry& registry, Vector3Par pos, float radius)
{
    World* world = GWorld;
    if (!world)
    {
        return 0;
    }
    Person* player = world->GetRealPlayer();
    const bool playerAlive = player && !player->IsDammageDestroyed();
    const float rSq = radius * radius;
    int n = 0;
    AICenter* center = FindSideCenter(registry.ResistanceSide());
    for (int g = 0; center && g < center->NGroups(); g++)
    {
        AIGroup* grp = center->GetGroup(g);
        if (!grp)
        {
            continue;
        }
        for (int u = 0; u < MAX_UNITS_PER_GROUP; u++)
        {
            AIUnit* unit = grp->UnitWithID(u + 1);
            if (!unit || unit->GetLifeState() != AIUnit::LSAlive || (playerAlive && unit->GetPerson() == player))
            {
                continue;
            }
            n += Dist2DSq(unit->Position(), pos) <= rSq ? 1 : 0;
        }
    }
    if (playerAlive && Dist2DSq(player->Position(), pos) <= rSq)
    {
        PlayerPresenceInput presence;
        registry.SamplePlayerPresence(player, presence);
        n += PlayerHoldsPresence(presence) ? 1 : 0;
    }
    return n;
}

} // namespace

int CounterAttack::HqAssaultPresence(Vector3Par hqPos, float radius) const
{
    return HqAssaultPresence(hqPos, radius, &Ops());
}

int CounterAttack::MountedOf(const Record& r)
{
    AIGroup* grp = r.group;
    Entity* hull = r.hull;
    int n = 0;
    for (int u = 0; grp && hull && u < MAX_UNITS_PER_GROUP; u++)
    {
        AIUnit* unit = grp->UnitWithID(u + 1);
        n += unit && unit->GetLifeState() == AIUnit::LSAlive && unit->GetVehicleIn() == hull ? 1 : 0;
    }
    return n;
}

int CounterAttack::AliveOf(const Record& r)
{
    Vector3 leader = VZero;
    return r.phase == CALive ? Ops().Alive(r.group, leader) : 0;
}

Entity* CounterAttack::LeaderOf(const Record& r)
{
    AIGroup* grp = r.group;
    AIUnit* leader = grp ? grp->Leader() : nullptr;
    return leader ? leader->GetPerson() : nullptr;
}

const CounterAttackGate& CounterAttack::EngineGate(bool playerValid, float px, float pz)
{
    CounterAttackView view;
    view.playerValid = playerValid;
    view.playerX = px;
    view.playerZ = pz;
    Ops().Refresh(view);
    return Ops();
}

void CounterAttack::OnZoneEvents(const AutoArray<ZoneEventRecord>& fired)
{
    ZoneRegistry& registry = ZoneRegistry::Instance();
    for (int i = 0; i < fired.Size(); i++)
    {
        const ZoneEventRecord& ev = fired[i];
        if (ev.type == ZECaptured)
        {
            // the force size reads the occupier-side war level (7.1)
            OnTownFlipped(registry, ev.zoneIndex, GarrisonCache::Instance().Tuning().groupSize,
                          OccupierForces::Instance().OccupierWarLevel(), GuerrillaBase::Instance().HqZone());
        }
        else if (ev.type == ZEHqLost)
        {
            // the HQ fell to an assault (GuerrillaBase::Destroy): every
            // HQ_ASSAULT record succeeded
            CounterAttackView view;
            BuildView(view);
            SettleHqAssaults(registry, view, &Ops());
        }
        else if (ev.type == ZETownLost || ev.type == ZEBaseLost)
        {
            // a live squad standing in the town (or base) it just retook
            // becomes its garrison before GarrisonCache materializes the
            // refill
            const ZoneRecord* z = registry.GetZone(ev.zoneIndex);
            const int r = z ? FindByTarget(z->name) : -1;
            if (r < 0 || _records[r].kind == CAKReinforce || _records[r].kind == CAKAlarm ||
                _records[r].kind == CAKStrike)
            {
                continue;
            }
            if (_records[r].phase == CALive)
            {
                Record& rec = _records[r];
                Vector3 leader = VZero;
                const int alive = Ops().Alive(rec.group, leader);
                if (alive > 0)
                {
                    AdoptLive(registry, ev.zoneIndex, rec, &Ops(), alive);
                }
                else
                {
                    OccupierForces::Instance().NoteLoss(rec.strength, "dead before the revert");
                    AddLeftover(rec.source, nullptr, rec.group, 0); // L7: its dead group leaves unseen
                    Emit(registry, rec, CAOpAborted, 0, rec.strength, "the town reverted; the squad is gone");
                }
                CounterAttackView view;
                BuildView(view);
                FinishHull(registry, view, rec, &Ops());
                _records.Delete(r);
            }
        }
    }
    // the director hears the tick's events after this service handled them
    if (GListener && !GDispatching)
    {
        GDispatching = true;
        GListener->OnZoneEvents(fired);
        GDispatching = false;
        DrainEvents();
    }
}

void CounterAttack::BuildView(CounterAttackView& view) const
{
    view = CounterAttackView();
    ZoneRegistry& registry = ZoneRegistry::Instance();
    World* world = GWorld;
    Person* player = world ? world->GetRealPlayer() : nullptr;
    if (player && !player->IsDammageDestroyed())
    {
        view.playerValid = true;
        view.playerX = player->Position().X();
        view.playerZ = player->Position().Z();
    }
    const GarrisonCache& cache = GarrisonCache::Instance();
    view.garrisonLive.Resize(registry.NZones());
    for (int i = 0; i < registry.NZones(); i++)
    {
        view.garrisonLive[i] = cache.LiveCount(i);
    }
    if (world)
    {
        SimDiag diag;
        GatherSimDiag(diag);
        view.liveBodies = diag.liveTotal;
        view.occGroups = diag.grpOcc;
    }
    view.hqZone = GuerrillaBase::Instance().HqZone();
    view.won = CampaignEnd::Instance().Won();
    view.lost = CampaignEnd::Instance().Lost();
    const GuerrillaBase& base = GuerrillaBase::Instance();
    view.hqEstablished = base.IsEstablished();
    view.hqPos = view.hqEstablished ? base.HqPos() : VZero;
    view.hqDefenders = view.hqEstablished ? HoldPresenceAt(registry, view.hqPos, registry.Tuning().zoneArea) : 0;
    // phase 6A: alert states (alarm answers, 8.2) and the map size (the off-map edge)
    view.alertState.Resize(registry.NZones());
    for (int i = 0; i < registry.NZones(); i++)
    {
        view.alertState[i] = AlertMachine::Instance().GetZoneState(i);
    }
    view.mapSize = GLandscape ? (float)GLandscape->GetLandRange() * GLandscape->GetLandGrid() : 0.0f;
    Ops().Refresh(view);
}

int CounterAttack::SettleHqAssaults(ZoneRegistry& registry, const CounterAttackView& view, CounterAttackOps* ops)
{
    int settled = 0;
    for (int i = 0; i < _records.Size();)
    {
        Record& r = _records[i];
        if (r.kind != CAKHqAssault)
        {
            i++;
            continue;
        }
        if (r.phase == CALive)
        {
            // a live squad leaves the world once nobody sees it (Advance)
            if (!r.hqDone)
            {
                r.hqDone = true;
                LOG_INFO(Core, "CounterAttack: [op {}] the HQ fell; the assault squad leaves once nobody sees it",
                         r.opId);
            }
            i++;
            continue;
        }
        LOG_INFO(Core, "CounterAttack: [op {}] the HQ fell; the {} soldier(s) of the assault go back to the pool",
                 r.opId, r.strength);
        OccupierForces::Instance().Refund(r.strength, "HQ assault done");
        Emit(registry, r, CAOpSettled, r.strength, r.strength, "the HQ fell");
        FinishHull(registry, view, r, ops);
        _records.Delete(i);
        settled++;
    }
    return settled;
}

CounterAttackOps* CounterAttack::EngineOps()
{
    return &Ops();
}

void CounterAttack::Simulate(float deltaT)
{
    ZoneRegistry& registry = ZoneRegistry::Instance();
    if (!registry.IsActive() || (_records.Size() == 0 && _homeHulls.Size() == 0 && _leftovers.Size() == 0))
    {
        _accum = 0;
        return;
    }
    _accum += deltaT;
    if (_accum < TickInterval)
    {
        return;
    }
    const float elapsed = _accum;
    _accum = 0;
    if (!GWorld)
    {
        return;
    }
    CounterAttackView view;
    BuildView(view);
    AutoArray<ZoneEventRecord> fired;
    Step(registry, view, elapsed, &Ops(), fired);
    registry.FireEvents(fired);
}

// ---------------------------------------------------------------------------
// serialization
// ---------------------------------------------------------------------------

LSError CounterAttack::Record::Serialize(ParamArchive& ar)
{
    PARAM_CHECK(ar.Serialize("target", target, 1, RString()))
    PARAM_CHECK(ar.Serialize("source", source, 1, RString()))
    PARAM_CHECK(ar.Serialize("phase", phase, 1, (int)CAScheduled))
    PARAM_CHECK(ar.Serialize("clock", clock, 1, 0.0f))
    PARAM_CHECK(ar.Serialize("travel", travel, 1, 0.0f))
    PARAM_CHECK(ar.Serialize("wholeRoad", wholeRoad, 1, false))
    PARAM_CHECK(ar.Serialize("deferTime", deferTime, 1, 0.0f))
    PARAM_CHECK(ar.Serialize("strength", strength, 1, 0))
    PARAM_CHECK(ar.Serialize("size", size, 1, 0))
    PARAM_CHECK(ar.Serialize("townsBefore", townsBefore, 1, 0))
    int seedValue = (int)seed;
    PARAM_CHECK(ar.Serialize("seed", seedValue, 1, 0))
    seed = (unsigned int)seedValue;
    PARAM_CHECK(ar.Serialize("salt", salt, 1, 2))
    PARAM_CHECK(ar.Serialize("resend", resend, 1, 0.0f))
    // a live squad's group resolves on the second load pass, after the
    // world's vehicle serializer recreated its bodies (GarrisonCache idiom)
    PARAM_CHECK(ar.SerializeRef("group", group, 1))
    // occupier forces (absent in older saves: a legacy foot retake, opId 0)
    PARAM_CHECK(ar.Serialize("opId", opId, 1, 0))
    PARAM_CHECK(ar.Serialize("kind", kind, 1, (int)CAKRetake))
    PARAM_CHECK(ar.Serialize("origin", origin, 1, (int)CAOLegacy))
    PARAM_CHECK(ar.Serialize("package", package, 1, (int)FPFoot))
    PARAM_CHECK(ar.Serialize("priceUnits", priceUnits, 1, 0))
    PARAM_CHECK(ar.Serialize("hullClass", hullClass, 1, RString()))
    PARAM_CHECK(ar.Serialize("crew", crew, 1, 0))
    PARAM_CHECK(ar.Serialize("urgent", urgent, 1, false))
    PARAM_CHECK(ar.Serialize("funded", funded, 1, false))
    PARAM_CHECK(ar.Serialize("abstractSpeed", abstractSpeed, 1, 0.0f))
    PARAM_CHECK(ar.Serialize("lethality", lethality, 1, 1.0f))
    PARAM_CHECK(ar.Serialize("arrivalLive", arrivalLive, 1, false))
    PARAM_CHECK(ar.Serialize("hullDone", hullDone, 1, false))
    PARAM_CHECK(ar.Serialize("tried", tried, 1, RString()))
    PARAM_CHECK(ar.Serialize("dismounted", dismounted, 1, false))
    // the hull and its crew ride the world's vehicle serializer; the refs
    // resolve on the second pass like the group
    PARAM_CHECK(ar.SerializeRef("hull", hull, 1))
    PARAM_CHECK(ar.SerializeRef("crewGroup", crewGroup, 1))
    // phase 4 (absent in older saves: the target zone's centre, not done)
    float destX = destPos.X();
    float destY = destPos.Y();
    float destZ = destPos.Z();
    PARAM_CHECK(ar.Serialize("destX", destX, 1, 0.0f))
    PARAM_CHECK(ar.Serialize("destY", destY, 1, 0.0f))
    PARAM_CHECK(ar.Serialize("destZ", destZ, 1, 0.0f))
    destPos = Vector3(destX, destY, destZ);
    PARAM_CHECK(ar.Serialize("hqDone", hqDone, 1, false))
    // phase 5 (absent in older saves: no escort); the escort hull and its
    // crew ride the vehicle serializer, the refs resolve on the second pass
    PARAM_CHECK(ar.Serialize("escortClass", escortClass, 1, RString()))
    PARAM_CHECK(ar.Serialize("escortCrew", escortCrew, 1, 0))
    PARAM_CHECK(ar.Serialize("escortPriceUnits", escortPriceUnits, 1, 0))
    PARAM_CHECK(ar.Serialize("escortDone", escortDone, 1, false))
    PARAM_CHECK(ar.SerializeRef("escort", escort, 1))
    PARAM_CHECK(ar.SerializeRef("escortCrewGroup", escortCrewGroup, 1))
    PARAM_CHECK(ar.Serialize("withdrawing", withdrawing, 1, false))
    // phase 6A (absent in older saves: no air, no strike, no alarm)
    PARAM_CHECK(ar.Serialize("airOrigin", airOrigin, 1, -1))
    PARAM_CHECK(ar.Serialize("strikeZone", strikeZone, 1, -1))
    float sx = strikePos.X();
    float sy = strikePos.Y();
    float sz = strikePos.Z();
    PARAM_CHECK(ar.Serialize("strikeX", sx, 1, 0.0f))
    PARAM_CHECK(ar.Serialize("strikeY", sy, 1, 0.0f))
    PARAM_CHECK(ar.Serialize("strikeZ", sz, 1, 0.0f))
    strikePos = Vector3(sx, sy, sz);
    PARAM_CHECK(ar.Serialize("alarmZone", alarmZone, 1, -1))
    float ax = alarmPos.X();
    float ay = alarmPos.Y();
    float az = alarmPos.Z();
    PARAM_CHECK(ar.Serialize("alarmX", ax, 1, 0.0f))
    PARAM_CHECK(ar.Serialize("alarmY", ay, 1, 0.0f))
    PARAM_CHECK(ar.Serialize("alarmZ", az, 1, 0.0f))
    alarmPos = Vector3(ax, ay, az);
    PARAM_CHECK(ar.Serialize("struck", struck, 1, false))
    PARAM_CHECK(ar.Serialize("airId", airId, 1, 0))
    float fx = airFrom.X();
    float fz = airFrom.Z();
    PARAM_CHECK(ar.Serialize("airFromX", fx, 1, 0.0f))
    PARAM_CHECK(ar.Serialize("airFromZ", fz, 1, 0.0f))
    airFrom = Vector3(fx, 0, fz);
    float lx = airLz.X();
    float ly = airLz.Y();
    float lz = airLz.Z();
    PARAM_CHECK(ar.Serialize("airLzX", lx, 1, 0.0f))
    PARAM_CHECK(ar.Serialize("airLzY", ly, 1, 0.0f))
    PARAM_CHECK(ar.Serialize("airLzZ", lz, 1, 0.0f))
    airLz = Vector3(lx, ly, lz);
    PARAM_CHECK(ar.Serialize("airReplans", airReplans, 1, 0))
    PARAM_CHECK(ar.Serialize("airLanded", airLanded, 1, false))
    PARAM_CHECK(ar.Serialize("airReplanned", airReplanned, 1, false))
    PARAM_CHECK(ar.Serialize("alarmQuiet", alarmQuiet, 1, 0.0f))
    PARAM_CHECK(ar.Serialize("answered", answered, 1, false))
    return LSOK;
}

LSError CounterAttack::HomeHull::Serialize(ParamArchive& ar)
{
    PARAM_CHECK(ar.Serialize("opId", opId, 1, 0))
    PARAM_CHECK(ar.Serialize("kind", kind, 1, (int)CAKRetake))
    PARAM_CHECK(ar.Serialize("origin", origin, 1, (int)CAOLegacy))
    PARAM_CHECK(ar.Serialize("target", target, 1, RString()))
    PARAM_CHECK(ar.Serialize("source", source, 1, RString()))
    PARAM_CHECK(ar.Serialize("units", units, 1, 0))
    PARAM_CHECK(ar.Serialize("crew", crew, 1, 0))
    PARAM_CHECK(ar.SerializeRef("hull", hull, 1))
    PARAM_CHECK(ar.SerializeRef("crewGroup", crewGroup, 1))
    return LSOK;
}

LSError CounterAttack::Leftover::Serialize(ParamArchive& ar)
{
    PARAM_CHECK(ar.Serialize("source", source, 1, RString()))
    PARAM_CHECK(ar.Serialize("living", living, 1, 0))
    PARAM_CHECK(ar.SerializeRef("hull", hull, 1))
    PARAM_CHECK(ar.SerializeRef("group", group, 1))
    return LSOK;
}

LSError CounterAttack::Serialize(ParamArchive& ar)
{
    // both load passes read the rows (the second resolves the group refs)
    PARAM_CHECK(ar.Serialize("Attacks", _records, 1))
    // towns taken so far (k of the next flip); absent in older saves: 0, and
    // the towns held at the next flip stand in for it
    PARAM_CHECK(ar.Serialize("townsTaken", _townsTaken, 1, 0))
    // intact hulls driving home after their operation (absent: none)
    PARAM_CHECK(ar.Serialize("HomeHulls", _homeHulls, 1))
    // lost hulls and dead or straggling groups awaiting an unseen deletion
    // (absent: none)
    PARAM_CHECK(ar.Serialize("Leftovers", _leftovers, 1))
    // phase 6A (absent: none): airframes flying home, and OccupierAir's
    // live records (a mid-flight save resumes: the hulls ride the vehicle
    // serializer, the refs resolve on the second pass)
    PARAM_CHECK(ar.Serialize("AirHomes", _airHomes, 1))
    PARAM_CHECK(OccupierAir::Instance().SerializeRecords(ar))
    if (ar.IsLoading() && ar.GetPass() == ParamArchive::PassSecond)
    {
        OccupierAir::Instance().AfterLoad();
    }
    if (ar.IsLoading())
    {
        for (int i = 0; i < _records.Size(); i++)
        {
            Record& r = _records[i];
            r.deferLogged = false;
            r.hiddenLogged = false;
            r.retry = 0;
            r.suspended = false;
            r.route.Clear();    // rebuilt from the zone positions on demand
            r.leg = LegUnknown; // a live squad re-derives its leg from its leader
            if (r.phase == CATravel && r.travel < r.clock)
            {
                r.travel = r.clock; // a save without the field: progress restarts at the base
            }
        }
        _accum = 0;
        if (ar.GetPass() == ParamArchive::PassSecond)
        {
            // the mission config was reparsed during the first pass
            LoadFromConfig();
        }
    }
    return LSOK;
}

} // namespace Poseidon::Guerrilla
