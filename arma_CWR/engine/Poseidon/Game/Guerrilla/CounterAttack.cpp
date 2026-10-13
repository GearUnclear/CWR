#include <Poseidon/Game/Guerrilla/CounterAttack.hpp>

#include <Poseidon/Game/Guerrilla/CampaignEnd.hpp> // IsMilitaryType
#include <Poseidon/Game/Guerrilla/GarrisonCache.hpp>
#include <Poseidon/Game/Guerrilla/GuerrillaBase.hpp> // HqZone: the HQ's town is never a target (N4a)
#include <Poseidon/Game/Guerrilla/Traffic.hpp> // the shared perception gate (issue #53)
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>

#include <Poseidon/IO/ParamFileExt.hpp> // Pars / ExtParsMission
#include <Poseidon/IO/Serialization/ParamArchive.hpp>

#include <Poseidon/Core/Application.hpp>         // GApp (ENGINE_CONFIG)
#include <Poseidon/Core/Config/EngineConfig.hpp> // ENGINE_CONFIG.objectsZ / horizontZ (perception gate)

#include <Poseidon/World/World.hpp>
#include <Poseidon/World/Terrain/Landscape.hpp> // surface Y, dry-land test
#include <Poseidon/World/Terrain/Roads.hpp>     // GRoadNet road snap
#include <Poseidon/World/Entities/Infantry/Person.hpp>
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
    if (!_tuning.enabled)
    {
        return false;
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
    if (zoneIndex == hqZone)
    {
        // the town the player's HQ stands in (N4a): never the object of a
        // flip answer before the director exists.  The flip still counts as
        // a town taken (only the target is skipped).
        LOG_INFO(Core, "CounterAttack: {} rose; it holds the player's headquarters, no answer",
                 (const char*)z->name);
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
    if (!_tuning.enabled || !town || !src || size < 1 || stricmp(town->type, "CITY") != 0 ||
        stricmp(town->owner, registry.ResistanceSide()) != 0 || town->garrisonExempt || target == view.hqZone ||
        stricmp(src->owner, registry.OccupierSide()) != 0 || FindByTarget(town->name) >= 0)
    {
        return false;
    }
    const int liveBodies = source < view.garrisonLive.Size() ? view.garrisonLive[source] : 0;
    const bool live = ops != nullptr && liveBodies > 0 && InBubble(registry, view, target) && RoomLive(registry, view);
    const int spare = Spare(_tuning, src->garrison, liveBodies, src->garrisonMax, live);
    const int n = MinInt(size, spare);
    if (n < 1)
    {
        return false;
    }
    Record r;
    r.target = town->name;
    r.townsBefore = _townsTaken;
    r.seed = MakeSeed(town->name, town->flipDay, town->flipMinute, _townsTaken) ^ 0x5A17C0DEu;
    r.size = size;
    if (!Launch(registry, target, source, n, live, ops, r, fired))
    {
        return false;
    }
    r.wholeRoad = r.phase == CATravel && InBubble(registry, view, target);
    _records.Add(r);
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
    const float edge = registry.Tuning().zoneArea + TownEdgeMargin;
    if (n == 0 || Dist2D(leader, town->pos) <= edge)
    {
        // the town edge: deploy and move in; the live timeout starts now
        r.leg = LegTown;
        if (r.clock < 0)
        {
            r.clock = 0;
        }
        r.resend = ResendInterval;
        ops->Assault(r.group, town->pos);
        LOG_INFO(Core, "CounterAttack: the force on {} reached the town edge ({:.0f} m out); it moves in",
                 (const char*)r.target, Dist2D(leader, town->pos));
        return;
    }
    // the route point nearest the leader, searched around the current leg
    // (the whole road after a reload)
    const bool known = r.leg == LegRoad;
    const int lo = known ? MaxInt(0, r.wp - 2 * WaypointStride) : 0;
    const int hi = known ? MinInt(n - 1, r.wp + 2 * WaypointStride) : n - 1;
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
        r.wp = MinInt(at + WaypointStride, n - 1);
        issue = true;
    }
    else if (r.wp >= 0 && r.wp < n && (at >= r.wp - 1 || Dist2D(leader, route[r.wp]) < WaypointReach))
    {
        // this leg is done: the next one ends WaypointStride points on
        const int next = MinInt(at + WaypointStride, n - 1);
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
        ops->MoveOn(r.group, route[r.wp]);
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
        LOG_INFO(Core, "CounterAttack: {} survivor(s) of the force on {} have no base to return to ('{}' is gone)",
                 bodies, (const char*)r.target, (const char*)r.source);
        return;
    }
    const int live = src < view.garrisonLive.Size() ? view.garrisonLive[src] : 0;
    // whole bodies (U3): the room is floored, the pending deposit counts
    // against it (S5 bodies are on their way into this reserve)
    const float room = s->garrisonMax - (float)live - floorf(s->garrison) - (float)s->pending;
    const int back = MinInt(bodies, room > 0 ? (int)floorf(room) : 0);
    // inside the player's deposit bubble the survivors are a pending deposit
    // (S5): they join the reserve unseen or once he leaves
    const bool pending = GarrisonCache::AddReserve(registry, src, back, src, view.playerValid, view.playerX,
                                                   view.playerZ);
    LOG_INFO(Core, "CounterAttack: {} of {} survivor(s) back at {}{} (reserve {}, pending {}, of {})", back, bodies,
             (const char*)s->name, pending ? " as a pending deposit" : "", s->garrison, s->pending, s->garrisonMax);
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
    const ZoneRecord* town = registry.GetZone(target);
    const int defenders = town ? town->lastGuer : 0;
    const float hold = HoldOdds(_tuning, defenders, r.strength);
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
        ReturnToSource(registry, view, r, r.strength - lost);
        AddEvent(fired, ZECounterRepelled, target);
        return true;
    }
    LOG_INFO(Core, "CounterAttack: {} attacker(s) retake {} from {} fighter(s) (odds {:.2f}, roll {:.2f})", r.strength,
             (const char*)r.target, defenders, hold, roll);
    registry.RevertTown(target, fired);
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
    const bool overLive = LiveSquads() >= _tuning.maxLive;
    const bool overBodies = zt.budgetLiveBodies > 0 && view.liveBodies + r.strength > zt.budgetLiveBodies;
    const bool overGroups = zt.budgetSideGroups > 0 && view.occGroups + 1 > zt.budgetSideGroups;
    if (overLive || overBodies || overGroups)
    {
        if (!r.deferLogged)
        {
            LOG_WARN(Core,
                     "CounterAttack: force on {} deferred (live squads {}/{}, live AI {}+{}/{}, occupier groups {}/{});"
                     " retrying every {:.0f}s",
                     (const char*)r.target, LiveSquads(), _tuning.maxLive, view.liveBodies, r.strength,
                     zt.budgetLiveBodies, view.occGroups, zt.budgetSideGroups, RetryInterval);
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
                     " perception; retrying every {:.0f}s, its clock stopped, turning back after {:.0f}s",
                     (const char*)r.target, (const char*)r.source,
                     r.wholeRoad ? "from the base" : "behind its progress point", RetryInterval, _tuning.birthGiveUp);
            r.hiddenLogged = true;
        }
        return BirthNoPoint;
    }
    const int n = route.Size();
    const int legEnd = MinInt(RouteIndexAt(route, pick.fraction) + WaypointStride, n - 1);
    AIGroup* grp = ops->SpawnSquad(r.strength, pick.pos, route[legEnd]);
    if (!grp)
    {
        return BirthWait;
    }
    const bool afterDeferral = r.deferLogged || r.hiddenLogged;
    r.group = grp;
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
             "{:.0f}% along {:.0f} m of road); they march on {}",
             r.strength, (const char*)r.source, pick.pos.X(), pick.pos.Z(), toPlayer,
             pick.reason == CASpawnBase ? "base" : "route point", afterDeferral ? ", after a deferral" : "",
             pick.fraction * 100.0f, RouteLength(route), (const char*)r.target);
    AddEvent(fired, ZECounterAttack, target);
    return BirthDone;
}

bool CounterAttack::RoomLive(const ZoneRegistry& registry, const CounterAttackView& view) const
{
    const ZoneTuning& zt = registry.Tuning();
    return LiveSquads() < _tuning.maxLive && (zt.budgetSideGroups <= 0 || view.occGroups + 1 <= zt.budgetSideGroups);
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
    r.clock = TravelTime(_tuning, Dist2D(s->pos.X(), s->pos.Z(), town->pos.X(), town->pos.Z()));
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
        return true;
    }
    const bool held = stricmp(town->owner, registry.ResistanceSide()) == 0;

    switch (r.phase)
    {
        case CAScheduled:
        {
            if (!held)
            {
                LOG_INFO(Core, "CounterAttack: {} fell before the force left; cancelled", (const char*)r.target);
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
                return true;
            }
            if (!Launch(registry, target, src, MinInt(r.size, spare), live, ops, r, fired))
            {
                r.clock = RetryInterval;
            }
            // launched with the town already in the bubble: born anywhere on
            // its road (from the base), not only behind its progress point
            r.wholeRoad = r.phase == CATravel && inBubble;
            return false;
        }
        case CATravel:
        {
            if (!held)
            {
                LOG_INFO(Core, "CounterAttack: {} fell while the force was on the road; it turns back",
                         (const char*)r.target);
                ReturnToSource(registry, view, r, r.strength);
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
                        LOG_INFO(Core,
                                 "CounterAttack: the force on {} found no unseen road for {:.0f}s; it turns back to {}",
                                 (const char*)r.target, r.deferTime, (const char*)r.source);
                        ReturnToSource(registry, view, r, r.strength);
                        return true;
                    }
                }
                if (r.retry > 0)
                {
                    r.retry -= elapsed;
                    return false;
                }
                const int birth = GoLive(registry, view, target, r, ops, Progress(r), fired);
                if (birth == BirthHold)
                {
                    // already at the town edge: it stays there, abstract and
                    // idle, until the player leaves or it can be born
                    r.retry = RetryInterval;
                }
                else if (birth != BirthDone)
                {
                    r.retry = RetryInterval;
                }
                return false;
            }
            r.retry = 0;
            r.deferTime = 0;
            r.hiddenLogged = false;
            r.clock -= elapsed;
            if (r.clock > 0)
            {
                return false;
            }
            // off-screen arrival
            AddEvent(fired, ZECounterAttack, target);
            if (town->lastGuer > 0)
            {
                return Fight(registry, view, target, r, fired);
            }
            r.phase = CAHolding;
            LOG_INFO(Core, "CounterAttack: {} attacker(s) walk into empty {} (support {})", r.strength,
                     (const char*)r.target, town->support);
            return false;
        }
        case CAHolding:
        {
            if (!held)
            {
                LOG_INFO(Core, "CounterAttack: {} retaken by the holding force", (const char*)r.target);
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
            if (town->lastGuer > 0)
            {
                return Fight(registry, view, target, r, fired);
            }
            return false;
        }
        case CALive:
        {
            Vector3 leader = VZero;
            const int alive = ops && r.group ? ops->Alive(r.group, leader) : 0;
            if (!held)
            {
                if (alive > 0 && ops)
                {
                    AdoptLive(registry, target, r, ops, alive);
                }
                return true;
            }
            if (alive == 0)
            {
                LOG_INFO(Core, "CounterAttack: the force on {} is gone; the town held", (const char*)r.target);
                AddEvent(fired, ZECounterRepelled, target);
                return true;
            }
            if (r.leg == LegRoad)
            {
                // a marching squad nobody can perceive, away from the
                // player's bubble, returns to the abstract road (Traffic's
                // despawn rule): its slot and budget are freed, its
                // survivors and its progress carried over
                const float edge = registry.Tuning().cacheRadius + GarrisonCache::DespawnHysteresis;
                const bool away = Dist2D(leader.X(), leader.Z(), view.playerX, view.playerZ) > edge &&
                                  Dist2D(town->pos.X(), town->pos.Z(), view.playerX, view.playerZ) > edge;
                r.unseenFor = away && ops->SquadUnseen(r.group) ? r.unseenFor + elapsed : 0.0f;
                if (r.unseenFor >= VanishHold)
                {
                    const AutoArray<Vector3>& route = RouteOf(registry, r, ops);
                    const float fraction = RouteFractionNear(route, leader);
                    const ZoneRecord* src = registry.GetZone(registry.FindZoneIndex(r.source));
                    const float whole =
                        src ? TravelTime(_tuning, Dist2D(src->pos.X(), src->pos.Z(), town->pos.X(), town->pos.Z()))
                            : MinTravel;
                    ops->Remove(r.group);
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
                    return true;
                }
            }
            return false;
        }
        default:
            return true;
    }
}

void CounterAttack::AdoptLive(ZoneRegistry& registry, int target, Record& r, CounterAttackOps* ops, int alive)
{
    ZoneRecord* town = registry.GetZoneMutable(target);
    if (!town || !r.group)
    {
        return;
    }
    ops->Adopt(target, r.group);
    // the squad IS (part of) the new garrison: it comes off the refill (U2),
    // never assigned over the reserve
    const int cut = SubtractAdopted(*town, alive);
    LOG_INFO(Core, "CounterAttack: {} soldier(s) stay in {} as its garrison ({} taken off the refill; reserve {}, "
                   "pending {})",
             alive, (const char*)r.target, cut, town->garrison, town->pending);
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
    for (int i = 0; i < _records.Size();)
    {
        if (Advance(registry, view, elapsed, ops, _records[i], fired))
        {
            _records.Delete(i);
        }
        else
        {
            i++;
        }
    }
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
    // too short for a base-to-town road): link centres from the link nearest
    // the base to the link nearest the town, then the town centre
    void BuildRoute(Vector3Par from, Vector3Par to, AutoArray<Vector3>& route) override
    {
        route.Clear();
        const RoadLink* start = NearestRoadLink(from, RouteSnapRadius);
        const RoadLink* goal = NearestRoadLink(to, RouteSnapRadius);
        if (!start || !goal)
        {
            LOG_INFO(Core,
                     "CounterAttack: no road within {:.0f} m of [{:.0f}, {:.0f}] or [{:.0f}, {:.0f}]; straight line",
                     RouteSnapRadius, from.X(), from.Z(), to.X(), to.Z());
            return;
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
        const Vector3 goalC = goal->GetCenter();
        nodes.push_back(Node{start, -1, 0.0f, false});
        index[start] = 0;
        open.push(Entry(Dist2D(start->GetCenter(), goalC), 0));
        int found = -1;
        int expanded = 0;
        while (!open.empty() && expanded < MaxRouteNodes)
        {
            const int cur = open.top().second;
            open.pop();
            if (nodes[cur].closed)
            {
                continue;
            }
            nodes[cur].closed = true;
            expanded++;
            const RoadLink* link = nodes[cur].link;
            if (link == goal)
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
            LOG_INFO(Core,
                     "CounterAttack: no road route from [{:.0f}, {:.0f}] to [{:.0f}, {:.0f}] ({} links searched); "
                     "straight line",
                     from.X(), from.Z(), to.X(), to.Z(), expanded);
            return;
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
            const Vector3 pos = unit->Position();
            if (!_cam.hasCamera)
            {
                if (Dist2DSq(pos, _player) <= _band.despawnEdge * _band.despawnEdge)
                {
                    return false;
                }
                continue;
            }
            const float camD2 = Dist2DSq(pos, _cam.camPos);
            if (Dist2DSq(pos, _player) < _band.closeHold * _band.closeHold)
            {
                return false;
            }
            const bool inFrustum = _cam.InFrustum(pos, Traffic::FrustumCosHalfAngle);
            const bool losBlocked =
                !inFrustum && camD2 < _band.minSpawn * _band.minSpawn && _cam.LosBlocked(pos, InfantryLosHeight);
            if (!Traffic::CanExposeDespawn(camD2, losBlocked, inFrustum, _band))
            {
                return false;
            }
        }
        return true;
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
            if (r < 0)
            {
                continue;
            }
            if (_records[r].phase == CALive)
            {
                Vector3 leader = VZero;
                const int alive = Ops().Alive(_records[r].group, leader);
                if (alive > 0)
                {
                    AdoptLive(registry, ev.zoneIndex, _records[r], &Ops(), alive);
                }
                _records.Delete(r);
            }
        }
    }
}

void CounterAttack::Simulate(float deltaT)
{
    ZoneRegistry& registry = ZoneRegistry::Instance();
    if (!registry.IsActive() || _records.Size() == 0)
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
    World* world = GWorld;
    if (!world)
    {
        return;
    }

    CounterAttackView view;
    Person* player = world->GetRealPlayer();
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
    SimDiag diag;
    GatherSimDiag(diag);
    view.liveBodies = diag.liveTotal;
    view.occGroups = diag.grpOcc;
    view.hqZone = GuerrillaBase::Instance().HqZone();

    AutoArray<ZoneEventRecord> fired;
    Ops().Refresh(view);
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
    return LSOK;
}

LSError CounterAttack::Serialize(ParamArchive& ar)
{
    // both load passes read the rows (the second resolves the group refs)
    PARAM_CHECK(ar.Serialize("Attacks", _records, 1))
    // towns taken so far (k of the next flip); absent in older saves: 0, and
    // the towns held at the next flip stand in for it
    PARAM_CHECK(ar.Serialize("townsTaken", _townsTaken, 1, 0))
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
