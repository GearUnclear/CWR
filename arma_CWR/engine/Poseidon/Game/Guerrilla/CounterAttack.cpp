#include <Poseidon/Game/Guerrilla/CounterAttack.hpp>

#include <Poseidon/Game/Guerrilla/CampaignEnd.hpp> // IsMilitaryType
#include <Poseidon/Game/Guerrilla/GarrisonCache.hpp>
#include <Poseidon/Game/Guerrilla/GuerrillaBase.hpp>  // HqZone: the HQ's town is never a target (N4a)
#include <Poseidon/Game/Guerrilla/OccupierForces.hpp> // the pool seams (5.5), pass-through while inactive
#include <Poseidon/Game/Guerrilla/Traffic.hpp>        // the shared perception gate (issue #53)
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>

#include <Poseidon/IO/ParamFileExt.hpp> // Pars / ExtParsMission
#include <Poseidon/IO/Serialization/ParamArchive.hpp>

#include <Poseidon/Core/Application.hpp>         // GApp (ENGINE_CONFIG)
#include <Poseidon/Core/Config/EngineConfig.hpp> // ENGINE_CONFIG.objectsZ / horizontZ (perception gate)

#include <Poseidon/World/World.hpp>
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
    t.forcesOn = c.ReadValue("occupierForces", 0.0f) != 0.0f;
    t.forcesOpsMax = (int)ReadRanged(c, "forcesOpsMax", (float)t.forcesOpsMax, 0.0f, 6.0f);
    t.forcesLiveHullsMax = (int)ReadRanged(c, "forcesLiveHullsMax", (float)t.forcesLiveHullsMax, 0.0f, 20.0f);
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
        if (stricmp(_records[i].target, town) == 0)
        {
            return i;
        }
    }
    return -1;
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

int CounterAttack::BornOps() const
{
    int n = 0;
    for (int i = 0; i < _records.Size(); i++)
    {
        const Record& r = _records[i];
        n += r.funded && (r.phase == CALive || LiveHullOf(r) > 0) ? 1 : 0;
    }
    return n;
}

int CounterAttack::LiveHulls() const
{
    int n = 0;
    for (int i = 0; i < _records.Size(); i++)
    {
        n += LiveHullOf(_records[i]);
    }
    for (int i = 0; i < _homeHulls.Size(); i++)
    {
        n += _homeHulls[i].hull.GetLink() != nullptr ? 1 : 0;
    }
    return n;
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
        // town's new garrison when it reverts (5.4); its bodies are not lost
        if (_records[i].phase == CAHolding && stricmp(_records[i].target, z->name) == 0)
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
        // to the town until the player leaves
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
    if (zoneIndex == hqZone)
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
    const char* why = nullptr;
    if (view.won)
    {
        why = "the campaign is won";
    }
    else if (order.kind != CAKRetake)
    {
        why = "only town retakes move in this phase";
    }
    else if (!town || stricmp(town->type, "CITY") != 0 || stricmp(town->owner, registry.ResistanceSide()) != 0 ||
             town->garrisonExempt)
    {
        why = "the target is not a held town";
    }
    else if (order.target == view.hqZone)
    {
        why = "the target holds the player's headquarters";
    }
    else if (FindByTarget(town->name) >= 0)
    {
        why = "the town already has an operation";
    }
    else if (!src || stricmp(src->owner, registry.OccupierSide()) != 0)
    {
        why = "the source is not occupier-held";
    }
    else if (order.bodies < 1)
    {
        why = "no bodies ordered";
    }
    else if (BornOps() >= _tuning.forcesOpsMax)
    {
        why = "forcesOpsMax born operations are in flight";
    }
    if (why)
    {
        LOG_INFO(Core, "CounterAttack: [op {}] refused: {}", order.opId, why);
        return false;
    }
    Record r;
    r.target = town->name;
    r.townsBefore = _townsTaken;
    r.seed = MakeSeed(town->name, town->flipDay, town->flipMinute, _townsTaken) ^ (0x0F0F1E5Du + (unsigned)order.opId);
    r.size = order.bodies;
    r.opId = order.opId;
    r.kind = order.kind;
    r.origin = order.origin;
    r.package = order.package;
    r.priceUnits = order.priceUnits > 0 ? order.priceUnits : 0;
    r.hullClass = order.package != FPFoot ? order.hullClass : RString();
    r.crew = r.hullClass.GetLength() > 0 ? (order.crew > 0 ? order.crew : 1) : 0;
    r.urgent = order.urgent;
    r.funded = order.funded;
    r.abstractSpeed = order.abstractSpeed > 0 ? order.abstractSpeed : 0;
    r.lethality = order.lethality > 0 ? order.lethality : 1.0f;
    return LaunchCore(registry, view, order.target, order.source, order.bodies, r, ops, fired);
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
    const bool live = !r.HasHullPackage() && ops != nullptr && liveBodies > 0 && InBubble(registry, view, target) &&
                      RoomLive(registry, view) && (!r.funded || BornOps() < _tuning.forcesOpsMax);
    const int spare = Spare(_tuning, src->garrison, liveBodies, src->garrisonMax, live);
    const int n = MinInt(size, spare);
    if (n < 1)
    {
        if (r.opId != 0)
        {
            LOG_INFO(Core, "CounterAttack: [op {}] refused: {} cannot spare a man ({} stored, {} live)", r.opId,
                     (const char*)src->name, src->garrison, liveBodies);
        }
        return false;
    }
    if (!Launch(registry, target, source, n, live, ops, r, fired))
    {
        return false;
    }
    r.wholeRoad = r.phase == CATravel && InBubble(registry, view, target);
    _records.Add(r);
    const Record& added = _records[_records.Size() - 1];
    Emit(registry, added, CAOpLaunched, added.strength, added.priceUnits, live ? "live" : "abstract");
    if (added.phase == CALive)
    {
        Emit(registry, added, CAOpBorn, added.strength, 0, SpawnReasonName(added.spawnReason));
    }
    return true;
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
    const ZoneRecord* src = registry.GetZone(registry.FindZoneIndex(r.source));
    const ZoneRecord* town = registry.GetZone(registry.FindZoneIndex(r.target));
    if (!src || !town)
    {
        return r.route;
    }
    AutoArray<Vector3> raw;
    if (ops)
    {
        ops->BuildRoute(src->pos, town->pos, raw);
    }
    const float straight = Dist2D(src->pos, town->pos);
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
        raw.Add(town->pos);
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
    if (InBubble(registry, view, target))
    {
        return true;
    }
    // the player stands on the road ahead of (or at) the force
    const Vector3 at = RoutePoint(RouteOf(registry, r, ops), Progress(r));
    return Dist2D(at.X(), at.Z(), view.playerX, view.playerZ) <= registry.Tuning().cacheRadius;
}

void CounterAttack::March(const ZoneRegistry& registry, int target, Record& r, CounterAttackOps* ops, Vector3Par leader,
                          float elapsed)
{
    const ZoneRecord* town = registry.GetZone(target);
    if (!town || !ops)
    {
        return;
    }
    const AutoArray<Vector3>& route = RouteOf(registry, r, ops);
    const int n = route.Size();
    // a mounted force rides to counterAttackBirthMinToTarget and dismounts
    // there (3.2); men on foot deploy at the zone edge plus TownEdgeMargin
    const bool mounted = LiveHullOf(r) > 0 && !r.dismounted;
    const float edge = mounted ? _tuning.birthMinToTarget : registry.Tuning().zoneArea + TownEdgeMargin;
    if (n == 0 || Dist2D(leader, town->pos) <= edge)
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
            // back where it stands (an armed hull would support: a later
            // phase)
            ops->Dismount(r.group, r.hull);
            r.dismounted = true;
            r.stopping = -1;
        }
        ops->Assault(r.group, town->pos);
        LOG_INFO(Core, "CounterAttack: the force on {} reached the town edge ({:.0f} m out){}; it moves in",
                 (const char*)r.target, Dist2D(leader, town->pos), mounted ? " and dismounts" : "");
        Emit(registry, r, CAOpArrived, r.strength, 0, mounted ? "live, dismounted" : "live");
        return;
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
    // losses count bodies (A_bodies)
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
    registry.RevertTown(target, fired);
    Emit(registry, r, CAOpSettled, r.strength, 0, "abstract fight");
    return true;
}

int CounterAttack::GoLive(ZoneRegistry& registry, const CounterAttackView& view, int target, Record& r,
                          CounterAttackOps* ops, float fraction, AutoArray<ZoneEventRecord>& fired)
{
    const ZoneRecord* town = registry.GetZone(target);
    if (!ops || !town)
    {
        return BirthWait;
    }
    const ZoneTuning& zt = registry.Tuning();
    // a hull package is born mounted (its hull re-created after a return to
    // abstract travel), except at the town edge, where the infantry is
    // already on foot (6.5 arrival births) or once the hull was lost
    const bool mounted = r.HasHullPackage() && !r.hullDone && !r.arrivalLive;
    const int groups = mounted ? 2 : 1;
    const int bodies = r.strength + (mounted ? r.crew : 0);
    const bool overLive = LiveSquads() >= _tuning.maxLive;
    const bool overBodies = zt.budgetLiveBodies > 0 && view.liveBodies + bodies > zt.budgetLiveBodies;
    const bool overGroups = zt.budgetSideGroups > 0 && view.occGroups + groups > zt.budgetSideGroups;
    // forcesOpsMax counts born operations; an unborn one waits for a slot
    const bool overOps = r.funded && BornOps() >= _tuning.forcesOpsMax;
    const bool overHulls = mounted && LiveHulls() >= _tuning.forcesLiveHullsMax;
    if (overLive || overBodies || overGroups || overOps || overHulls)
    {
        if (!r.deferLogged)
        {
            LOG_WARN(
                Core,
                "CounterAttack: force on {} deferred (live squads {}/{}, live AI {}+{}/{}, occupier groups {}+{}/{},"
                " born ops {}/{}, live hulls {}/{}); retrying every {:.0f}s",
                (const char*)r.target, LiveSquads(), _tuning.maxLive, view.liveBodies, bodies, zt.budgetLiveBodies,
                view.occGroups, groups, zt.budgetSideGroups, BornOps(), _tuning.forcesOpsMax, LiveHulls(),
                _tuning.forcesLiveHullsMax, RetryInterval);
            r.deferLogged = true;
        }
        return BirthWait;
    }
    // the user's rule: a force is only ever born where the player cannot
    // perceive it, on its road between the base and the town, and walks
    // the rest
    const AutoArray<Vector3>& route = RouteOf(registry, r, ops);
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
                     r.funded ? "the next base back" : "turning back", _tuning.birthGiveUp);
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
            OccupierForces::Instance().Refund(r.crew - crewAlive, "partial spawn");
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
    const bool afterDeferral = r.deferLogged || r.hiddenLogged;
    r.group = grp;
    r.hull = hull;
    r.crewGroup = crewGroup;
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
    const ZoneRecord* town = registry.GetZone(registry.FindZoneIndex(r.target));
    if (!src || !town)
    {
        return MinTravel;
    }
    const float straight = Dist2D(src->pos.X(), src->pos.Z(), town->pos.X(), town->pos.Z());
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
    if (!town || !s || n < 1)
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
    const int target = registry.FindZoneIndex(r.target);
    ZoneRecord* town = registry.GetZoneMutable(target);
    if (!town)
    {
        if (r.phase == CALive && ops && r.group)
        {
            ops->Remove(r.group);
        }
        Emit(registry, r, CAOpAborted, r.strength, 0, "target gone");
        return true;
    }
    const bool held = stricmp(town->owner, registry.ResistanceSide()) == 0;
    // a top-up column (5.4) goes to a town the occupier holds; everything
    // else goes to a town the resistance holds
    const bool reinforce = r.kind == CAKReinforce;
    const bool valid = reinforce ? GarrisonCache::OccupierHolds(registry, *town) : held;

    switch (r.phase)
    {
        case CAScheduled:
        {
            if (!held)
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
                if (reinforce)
                {
                    LOG_INFO(Core, "CounterAttack: {} is no longer the occupier's; the top-up column goes home",
                             (const char*)r.target);
                    ReturnToSource(registry, view, r, r.strength);
                    Emit(registry, r, CAOpAborted, r.strength, 0, "the top-up target was lost");
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
                if (r.hiddenLogged)
                {
                    r.deferTime += elapsed;
                    if (r.deferTime >= _tuning.birthGiveUp)
                    {
                        if (r.funded)
                        {
                            // the birth chain (3.1, D6): the next base back,
                            // then cancel unseen; never a visible turn-back
                            if (Resource(registry, view, target, r, ops))
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
                ReturnToSource(registry, view, r, r.strength - in);
                Emit(registry, r, CAOpSettled, in, 0, "topped up");
                return true;
            }
            const bool defenders = town->lastGuer > 0 || (_tuning.forcesOn && town->lastGuerHold > 0);
            if (defenders)
            {
                if (!_tuning.forcesOn)
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
            LOG_INFO(Core, "CounterAttack: {} attacker(s) walk into empty {} (support {})", r.strength,
                     (const char*)r.target, town->support);
            return false;
        }
        case CAHolding:
        {
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
            r.suspended = InBubble(registry, view, target);
            if (r.suspended)
            {
                return false;
            }
            if (!_tuning.forcesOn)
            {
                if (town->lastGuer > 0)
                {
                    return Fight(registry, view, target, r, fired);
                }
                return false;
            }
            if (town->lastGuer > 0 || town->lastGuerHold > 0)
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
            Vector3 leader = VZero;
            const int alive = ops && r.group ? ops->Alive(r.group, leader) : 0;
            if (LiveHullOf(r) > 0 && ops)
            {
                CheckHull(registry, r, ops);
            }
            if (!valid)
            {
                if (reinforce)
                {
                    LOG_INFO(Core, "CounterAttack: {} is no longer the occupier's; the top-up squad goes home",
                             (const char*)r.target);
                    if (ops && r.group)
                    {
                        ops->Remove(r.group);
                    }
                    ReturnToSource(registry, view, r, alive);
                    Emit(registry, r, CAOpAborted, alive, 0, "the top-up target was lost");
                    return true;
                }
                if (alive > 0 && ops)
                {
                    AdoptLive(registry, target, r, ops, alive);
                }
                else
                {
                    OccupierForces::Instance().NoteLoss(r.strength, "dead before the revert");
                    Emit(registry, r, CAOpAborted, 0, r.strength, "the town reverted; the squad is gone");
                }
                return true;
            }
            if (alive == 0)
            {
                LOG_INFO(Core, "CounterAttack: the force on {} is gone; the town held", (const char*)r.target);
                AddEvent(fired, ZECounterRepelled, target);
                OccupierForces::Instance().NoteLoss(r.strength, "repelled");
                Emit(registry, r, CAOpRepelled, 0, r.strength, "wiped out");
                return true;
            }
            if (reinforce && r.leg == LegTown)
            {
                // the top-up squad reached the town: part of its garrison now
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
                                  Dist2D(town->pos.X(), town->pos.Z(), view.playerX, view.playerZ) > edge;
                const bool hullUnseen = LiveHullOf(r) == 0 || ops->HullUnseen(r.hull, r.crewGroup);
                r.unseenFor = away && ops->SquadUnseen(r.group) && hullUnseen ? r.unseenFor + elapsed : 0.0f;
                if (r.unseenFor >= VanishHold)
                {
                    const AutoArray<Vector3>& route = RouteOf(registry, r, ops);
                    const float fraction = RouteFractionNear(route, leader);
                    const float whole = LegTime(registry, r, ops);
                    ops->Remove(r.group); // the cargo first, then the crew and the hull
                    if (LiveHullOf(r) > 0)
                    {
                        int crewAlive = 0;
                        ops->HullState(r.hull, r.crewGroup, crewAlive);
                        if (crewAlive < r.crew)
                        {
                            OccupierForces::Instance().NoteLoss(r.crew - crewAlive, "crew");
                            r.crew = crewAlive;
                        }
                        ops->RemoveHull(r.hull, r.crewGroup);
                        r.hull = nullptr;
                        r.crewGroup = nullptr;
                        // no refund: the record keeps hullClass and its price
                        // and re-creates the hull at the next birth (3.2)
                        Emit(registry, r, CAOpHullLost, 0, 0, "abstract travel: re-created at the next birth");
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
                    if (Dist2D(leader.X(), leader.Z(), town->pos.X(), town->pos.Z()) >
                        registry.Tuning().zoneArea * 0.5f)
                    {
                        ops->MoveOn(r.group, town->pos);
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

void CounterAttack::ArriveLive(ZoneRegistry& registry, const CounterAttackView& view, int target, Record& r,
                               CounterAttackOps* ops, AutoArray<ZoneEventRecord>& fired)
{
    // the birth search starts at the last route point at least the bound
    // from the town and runs back toward the base (the gate passes trivially
    // far from the player, so off-screen it is that point)
    const ZoneRecord* town = registry.GetZone(target);
    const AutoArray<Vector3>& route = RouteOf(registry, r, ops);
    const float len = RouteLength(route);
    float fraction = 0;
    if (town && len > 0)
    {
        float arc = len;
        for (int i = route.Size() - 1; i > 0; i--)
        {
            if (Dist2D(route[i], town->pos) >= _tuning.birthMinToTarget + 1.0f)
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
    const ZoneRecord* town = registry.GetZone(target);
    const int a = registry.FindZoneIndex(r.source);
    if (!town)
    {
        return false;
    }
    // the bases already used (D6: then each further base)
    const RString tried = r.tried.GetLength() > 0 ? r.tried + RString(";") + r.source : r.source;
    // the nearest other occupier base whose stored reserve can spare the
    // whole force (a base that cannot is passed over for the next)
    int best = -1;
    float bestDist = 0;
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
        const float d = Dist2D(z->pos, town->pos);
        if (best < 0 || d < bestDist)
        {
            best = i;
            bestDist = d;
        }
    }
    if (best < 0)
    {
        return false;
    }
    // A is refunded, B pays
    ZoneRecord* oldBase = registry.GetZoneMutable(a);
    if (oldBase && stricmp(oldBase->owner, registry.OccupierSide()) == 0)
    {
        GarrisonCache::AddReserve(registry, a, r.strength, a, view.playerValid, view.playerX, view.playerZ);
    }
    else
    {
        OccupierForces::Instance().Refund(r.strength, "no base");
    }
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
    r.route.Clear();
    r.travel = 0;
    r.clock = LegTime(registry, r, ops);
    r.travel = r.clock;
    r.deferTime = 0;
    r.retry = 0;
    r.hiddenLogged = false;
    r.deferLogged = false;
    r.wholeRoad = InBubble(registry, view, target);
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
    const int src = registry.FindZoneIndex(r.source);
    ZoneRecord* s = registry.GetZoneMutable(src);
    if (s && stricmp(s->owner, registry.OccupierSide()) == 0)
    {
        GarrisonCache::AddReserve(registry, src, r.strength, src, view.playerValid, view.playerX, view.playerZ);
    }
    else
    {
        OccupierForces::Instance().Refund(r.strength, "no base");
    }
    LOG_INFO(Core, "CounterAttack: [op {}] cancelled unborn: every route to {} watched ({} soldier(s) back to {})",
             r.opId, (const char*)r.target, r.strength, (const char*)r.source);
    r.hullDone = true; // the hull price goes back through the event
    Emit(registry, r, CAOpCancelledUnseen, r.strength, r.priceUnits, "every route watched");
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
    // stands): lost.  The whole crew counts lost to the pool; a crewman who
    // walked away from a wreck is left to the world.
    const int price = r.HullPrice();
    OccupierForces::Instance().NoteLoss(r.crew, "crew");
    LOG_INFO(Core, "CounterAttack: [op {}] the {} of the force on {} is lost ({}; {} of {} crew alive)", r.opId,
             (const char*)r.hullClass, (const char*)r.target,
             state == CounterAttackOps::HullDestroyed ? "destroyed"
                                                      : (state == CounterAttackOps::HullGone ? "gone" : "abandoned"),
             crewAlive, r.crew);
    if (!r.dismounted && r.group)
    {
        ops->Dismount(r.group, r.hull);
        r.dismounted = true;
    }
    r.hull = nullptr;
    r.crewGroup = nullptr;
    r.hullDone = true;
    r.crew = 0;
    if (r.abstractSpeed > 0)
    {
        r.abstractSpeed = FootSpeed; // on foot from here
    }
    Emit(registry, r, CAOpHullLost, 0, price, state == CounterAttackOps::HullDestroyed ? "destroyed" : "abandoned");
    return true;
}

void CounterAttack::FinishHull(ZoneRegistry& registry, const CounterAttackView& view, Record& r, CounterAttackOps* ops)
{
    if (!r.HasHullPackage() || r.hullDone)
    {
        return;
    }
    r.hullDone = true;
    if (LiveHullOf(r) > 0)
    {
        int crewAlive = 0;
        const int state = ops ? ops->HullState(r.hull, r.crewGroup, crewAlive) : (int)CounterAttackOps::HullGone;
        if (crewAlive < r.crew)
        {
            OccupierForces::Instance().NoteLoss(r.crew - crewAlive, "crew");
        }
        if (state == CounterAttackOps::HullIntact && crewAlive > 0)
        {
            // it drives home and is deleted only where nobody sees it
            SendHullHome(r, r.hull, r.crewGroup, crewAlive, ops, registry);
        }
        else
        {
            Emit(registry, r, CAOpHullLost, 0, r.HullPrice(), "lost at the end of the operation");
        }
        r.hull = nullptr;
        r.crewGroup = nullptr;
        return;
    }
    // an abstract hull (never born, or back on the abstract road) drives
    // home unseen at once: its price back, its crew home as bodies
    CrewHome(registry, view, r.source, r.crew);
    Emit(registry, r, CAOpHullRefund, r.crew, r.HullPrice(), "abstract: home");
}

void CounterAttack::SendHullHome(const Record& r, Entity* hull, AIGroup* crewGroup, int crewAlive,
                                 CounterAttackOps* ops, const ZoneRegistry& registry)
{
    HomeHull h;
    h.opId = r.opId;
    h.kind = r.kind;
    h.origin = r.origin;
    h.target = r.target;
    h.source = r.source;
    h.units = r.HullPrice();
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
    LOG_INFO(Core, "CounterAttack: [op {}] the {} drives home to {} ({} crew)", r.opId, (const char*)r.hullClass,
             (const char*)r.source, crewAlive);
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
        if (state != CounterAttackOps::HullIntact || crewAlive == 0)
        {
            OccupierForces::Instance().NoteLoss(h.crew, "crew");
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
            if (crewAlive < h.crew)
            {
                OccupierForces::Instance().NoteLoss(h.crew - crewAlive, "crew");
            }
            ops->RemoveHull(h.hull, h.crewGroup);
            CrewHome(registry, view, h.source, crewAlive);
            LOG_INFO(Core, "CounterAttack: [op {}] the hull is home unseen ({} crew back at {})", h.opId, crewAlive,
                     (const char*)h.source);
            Emit(registry, ref, CAOpHullRefund, crewAlive, h.units, "home");
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
    if (!view.playerValid)
    {
        return; // a dead player freezes every campaign clock
    }
    if (view.won)
    {
        return; // 7.4: once the win latches nothing in flight steps
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
// road-graph search budget (links expanded); an island's whole net fits
constexpr int MaxRouteNodes = 60000;
// start links tried (nearest first) when the nearest road is cut off; links
// up to RouteStartWide from the base are candidates after the near ones
constexpr size_t MaxRouteStarts = 96;
constexpr float RouteStartWide = 900.0f;
// m above the ground the terrain ray aims (a standing man)
constexpr float InfantryLosHeight = 1.8f;

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
        _playerValid = view.playerValid;
        _player = Vector3(view.playerX, 0, view.playerZ);
    }

    // Traffic's spawn gate (issue #53) for a squad on foot: the config
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
        if (camD2 >= _band.minSpawn * _band.minSpawn)
        {
            return CAExposeHidden; // past the draw distance
        }
        // inside the draw distance: the terrain ray decides "hidden" whichever
        // way the camera looks (no camera: nothing hides anything)
        if (_cam.LosBlocked(pos, InfantryLosHeight))
        {
            return CAExposeHidden;
        }
        return _cam.InFrustum(pos, Traffic::FrustumCosHalfAngle) ? CAExposeSeen : CAExposeConeOnly;
    }

    // A* over the road links (RoadNet::SearchPath gives up after 100 nodes,
    // too short for a base-to-town road): link centres from a link near the
    // base to a link near the town, then the town centre.  The links within
    // RouteSnapRadius of either end are candidates, nearest first: a base
    // whose nearest road is an island of its own (a walled compound's yard
    // roads, not joined to the island's net) starts from the next link
    // outside it instead of falling back to a straight line through its
    // walls.  A search that exhausts a component marks it, so no later
    // candidate in the same component is searched again.
    void BuildRoute(Vector3Par from, Vector3Par to, AutoArray<Vector3>& route) override
    {
        route.Clear();
        std::vector<const RoadLink*> starts;
        std::vector<const RoadLink*> goals;
        RoadLinksNear(from, RouteSnapRadius, starts);
        if (!starts.empty())
        {
            // then the links farther out (RouteStartWide): a base whose whole
            // nearby road is a yard network reaches the island's net there
            std::vector<const RoadLink*> wide;
            RoadLinksNear(from, RouteStartWide, wide);
            std::unordered_map<const RoadLink*, bool> have;
            for (size_t i = 0; i < starts.size(); i++)
            {
                have[starts[i]] = true;
            }
            for (size_t i = 0; i < wide.size(); i++)
            {
                if (have.count(wide[i]) == 0)
                {
                    starts.push_back(wide[i]);
                }
            }
        }
        RoadLinksNear(to, RouteSnapRadius, goals);
        if (starts.empty() || goals.empty())
        {
            LOG_INFO(Core,
                     "CounterAttack: no road within {:.0f} m of [{:.0f}, {:.0f}] or [{:.0f}, {:.0f}]; straight line",
                     RouteSnapRadius, from.X(), from.Z(), to.X(), to.Z());
            return;
        }
        std::unordered_map<const RoadLink*, int> goalSet;
        for (size_t g = 0; g < goals.size(); g++)
        {
            goalSet[goals[g]] = (int)g;
        }
        const Vector3 goalC = goals[0]->GetCenter();
        std::unordered_map<const RoadLink*, bool> exhausted;
        int expandedAll = 0;
        for (size_t s = 0; s < starts.size() && s < MaxRouteStarts && expandedAll < MaxRouteNodes; s++)
        {
            const RoadLink* start = starts[s];
            if (exhausted.count(start) > 0)
            {
                continue;
            }
            struct Node
            {
                const RoadLink* link;
                int parent;
                float g;
                bool closed;
            };
            std::vector<Node> nodes;
            std::unordered_map<const RoadLink*, int> index;
            typedef std::pair<float, int> Entry;
            std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> open;
            nodes.push_back(Node{start, -1, 0.0f, false});
            index[start] = 0;
            open.push(Entry(Dist2D(start->GetCenter(), goalC), 0));
            int found = -1;
            while (!open.empty() && expandedAll < MaxRouteNodes)
            {
                const int cur = open.top().second;
                open.pop();
                if (nodes[cur].closed)
                {
                    continue;
                }
                nodes[cur].closed = true;
                expandedAll++;
                const RoadLink* link = nodes[cur].link;
                if (goalSet.count(link) > 0)
                {
                    found = cur;
                    break;
                }
                const Vector3 c = link->GetCenter();
                for (int k = 0; k < link->NConnections(); k++)
                {
                    const RoadLink* next = link->Connections()[k];
                    if (!next)
                    {
                        continue;
                    }
                    const Vector3 nc = next->GetCenter();
                    const float g = nodes[cur].g + Dist2D(c, nc);
                    int ni;
                    auto it = index.find(next);
                    if (it == index.end())
                    {
                        ni = (int)nodes.size();
                        nodes.push_back(Node{next, cur, g, false});
                        index[next] = ni;
                    }
                    else
                    {
                        ni = it->second;
                        if (nodes[ni].closed || g >= nodes[ni].g)
                        {
                            continue;
                        }
                        nodes[ni].g = g;
                        nodes[ni].parent = cur;
                    }
                    open.push(Entry(g + Dist2D(nc, goalC), ni));
                }
            }
            if (found < 0)
            {
                if (open.empty())
                {
                    // the whole component of this start is not joined to the
                    // town's roads: skip its other links
                    for (size_t i = 0; i < nodes.size(); i++)
                    {
                        exhausted[nodes[i].link] = true;
                    }
                }
                continue;
            }
            AutoArray<Vector3> back;
            for (int i = found; i >= 0; i = nodes[i].parent)
            {
                back.Add(nodes[i].link->GetCenter());
            }
            for (int i = back.Size() - 1; i >= 0; i--)
            {
                route.Add(Grounded(back[i].X(), back[i].Z(), back[i].Y()));
            }
            route.Add(Grounded(to.X(), to.Z(), to.Y()));
            if (s > 0)
            {
                LOG_INFO(Core,
                         "CounterAttack: the road nearest [{:.0f}, {:.0f}] is not joined to the town's; the route "
                         "starts {:.0f} m from it",
                         from.X(), from.Z(), Dist2D(start->GetCenter(), from));
            }
            return;
        }
        LOG_INFO(Core,
                 "CounterAttack: no road route from [{:.0f}, {:.0f}] to [{:.0f}, {:.0f}] ({} links searched); "
                 "straight line",
                 from.X(), from.Z(), to.X(), to.Z(), expandedAll);
    }

    // the road links whose centres lie within radius of pos, nearest first
    // (the Traffic road survey idiom)
    static void RoadLinksNear(Vector3Par pos, float radius, std::vector<const RoadLink*>& out)
    {
        out.clear();
        if (!GRoadNet)
        {
            return;
        }
        std::vector<std::pair<float, const RoadLink*>> found;
        const float r2 = radius * radius;
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
                    if (d2 < r2)
                    {
                        found.push_back(std::make_pair(d2, link));
                    }
                }
            }
        }
        std::sort(found.begin(), found.end(),
                  [](const std::pair<float, const RoadLink*>& a, const std::pair<float, const RoadLink*>& b)
                  { return a.first < b.first; });
        for (size_t i = 0; i < found.size(); i++)
        {
            if (out.empty() || out.back() != found[i].second)
            {
                out.push_back(found[i].second);
            }
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
        const float warLevel = ReadWarLevel();
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
            if (bodies[b]->IsLocal())
            {
                ::DeleteVehicle(bodies[b]);
            }
            else
            {
                GetNetworkManager().AskForDeleteVehicle(bodies[b]);
            }
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

    // the despawn rule for one point (SquadUnseen, HullUnseen)
    bool PointUnseen(Vector3Par pos) const
    {
        if (!_playerValid)
        {
            return false;
        }
        if (!_cam.hasCamera)
        {
            return Dist2DSq(pos, _player) > _band.despawnEdge * _band.despawnEdge;
        }
        const float camD2 = Dist2DSq(pos, _cam.camPos);
        if (Dist2DSq(pos, _player) < _band.closeHold * _band.closeHold)
        {
            return false;
        }
        const bool inFrustum = _cam.InFrustum(pos, Traffic::FrustumCosHalfAngle);
        const bool losBlocked =
            !inFrustum && camD2 < _band.minSpawn * _band.minSpawn && _cam.LosBlocked(pos, InfantryLosHeight);
        return Traffic::CanExposeDespawn(camD2, losBlocked, inFrustum, _band);
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

    static void DeletePerson(Person* person)
    {
        if (!person)
        {
            return;
        }
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
        const float warLevel = ReadWarLevel();
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
        if (crew > 1 && veh->Type()->HasGunner())
        {
            Person* gunner = CreateSeated(crewGrp, crewType, foot, veh, GIPGunner, seated, 0.5f, RankPrivate);
            if (gunner && !seated)
            {
                DeletePerson(gunner);
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

    bool HullUnseen(Entity* hull, AIGroup* crewGroup) const override
    {
        (void)crewGroup; // the crew sit inside the hull
        return hull && PointUnseen(hull->Position());
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

  private:
    PerceptionCamera _cam;
    TrafficEffectiveBand _band;
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

} // namespace

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
            OnTownFlipped(registry, ev.zoneIndex, GarrisonCache::Instance().Tuning().groupSize, ReadWarLevel(),
                          GuerrillaBase::Instance().HqZone());
        }
        else if (ev.type == ZETownLost)
        {
            // a live squad standing in the town it just retook becomes its
            // garrison before GarrisonCache materializes the refill
            const ZoneRecord* z = registry.GetZone(ev.zoneIndex);
            const int r = z ? FindByTarget(z->name) : -1;
            if (r < 0 || _records[r].kind == CAKReinforce)
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
    Ops().Refresh(view);
}

CounterAttackOps* CounterAttack::EngineOps()
{
    return &Ops();
}

void CounterAttack::Simulate(float deltaT)
{
    ZoneRegistry& registry = ZoneRegistry::Instance();
    if (!registry.IsActive() || (_records.Size() == 0 && _homeHulls.Size() == 0))
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

LSError CounterAttack::Serialize(ParamArchive& ar)
{
    // both load passes read the rows (the second resolves the group refs)
    PARAM_CHECK(ar.Serialize("Attacks", _records, 1))
    // towns taken so far (k of the next flip); absent in older saves: 0, and
    // the towns held at the next flip stand in for it
    PARAM_CHECK(ar.Serialize("townsTaken", _townsTaken, 1, 0))
    // intact hulls driving home after their operation (absent: none)
    PARAM_CHECK(ar.Serialize("HomeHulls", _homeHulls, 1))
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
