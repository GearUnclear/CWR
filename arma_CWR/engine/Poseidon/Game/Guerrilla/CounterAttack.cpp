#include <Poseidon/Game/Guerrilla/CounterAttack.hpp>

#include <Poseidon/Game/Guerrilla/CampaignEnd.hpp> // IsMilitaryType
#include <Poseidon/Game/Guerrilla/GarrisonCache.hpp>
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>

#include <Poseidon/IO/ParamFileExt.hpp> // Pars / ExtParsMission
#include <Poseidon/IO/Serialization/ParamArchive.hpp>

#include <Poseidon/World/World.hpp>
#include <Poseidon/World/Terrain/Landscape.hpp> // surface Y, dry-land test
#include <Poseidon/World/Terrain/Roads.hpp>     // GRoadNet road snap
#include <Poseidon/World/Entities/Infantry/Person.hpp>
#include <Poseidon/AI/AI.hpp>
#include <Poseidon/AI/AICore.hpp>
#include <Poseidon/AI/VehicleAI.hpp> // Rank
#include <Poseidon/Network/Network.hpp>

#include <Random/randomGen.hpp>

#include <Poseidon/Foundation/Containers/BoolArray.hpp>
#include <Poseidon/Foundation/Framework/DebugLog.hpp>
#include <Poseidon/Foundation/platform.hpp>

#include <math.h>
#include <string.h>

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
    t.stageDistance = ReadRanged(c, "counterAttackStageDistance", t.stageDistance, 50.0f, 2000.0f);
    t.defenderWeight = ReadRanged(c, "counterAttackDefenderWeight", t.defenderWeight, 0.0f, 100.0f);
    t.repelLoss = ReadRanged(c, "counterAttackRepelLoss", t.repelLoss, 0.0f, 1.0f);
    t.liveTimeout = ReadRanged(c, "counterAttackLiveTimeout", t.liveTimeout, 60.0f, 86400.0f);
    t.maxLive = (int)ReadRanged(c, "counterAttackMaxLive", (float)t.maxLive, 0.0f, 8.0f);
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
    float leg = straightDistance - t.stageDistance;
    if (leg < 0)
    {
        leg = 0;
    }
    const float seconds = leg * t.roadFactor / (t.speed > 0 ? t.speed : 1.0f);
    return seconds < MinTravel ? MinTravel : seconds;
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
        if (_records[i].phase == CAHolding && stricmp(_records[i].target, z->name) == 0)
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

bool CounterAttack::OnTownFlipped(const ZoneRegistry& registry, int zoneIndex, int groupSize, float warLevel)
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
        stricmp(town->owner, registry.ResistanceSide()) != 0 || town->garrisonExempt ||
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

Vector3 CounterAttack::StagePoint(const CounterAttackTuning& t, Vector3Par town, Vector3Par source)
{
    float dx = source.X() - town.X();
    float dz = source.Z() - town.Z();
    const float len = sqrtf(dx * dx + dz * dz);
    if (len < 1.0f)
    {
        dx = 1;
        dz = 0;
    }
    else
    {
        dx /= len;
        dz /= len;
    }
    const float reach = len > 1.0f && len < t.stageDistance ? len : t.stageDistance;
    // the full distance first, then shorter, so a coast between the town and
    // the base pulls the squad in rather than dropping it in the sea
    static const float shares[] = {1.0f, 0.6f, 0.3f};
    for (float share : shares)
    {
        Vector3 p(town.X() + dx * reach * share, town.Y(), town.Z() + dz * reach * share);
        if (GRoadNet)
        {
            const Vector3 road = GRoadNet->GetNearestRoadPoint(p);
            if (Dist2D(road.X(), road.Z(), p.X(), p.Z()) < 150.0f)
            {
                p = road;
            }
        }
        if (!GLandscape)
        {
            return p;
        }
        if (GLandscape->SurfaceY(p.X(), p.Z()) >= 0.5f)
        {
            p[1] = GLandscape->SurfaceY(p.X(), p.Z());
            return p;
        }
    }
    return town;
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
    float room = s->garrisonMax - (float)live - s->garrison;
    if (room < 0)
    {
        room = 0;
    }
    const float back = (float)bodies < room ? (float)bodies : room;
    s->garrison += back;
    LOG_INFO(Core, "CounterAttack: {} of {} survivor(s) back at {} (reserve {} of {})", back, bodies,
             (const char*)s->name, s->garrison, s->garrisonMax);
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

bool CounterAttack::GoLive(ZoneRegistry& registry, const CounterAttackView& view, int target, Record& r,
                           CounterAttackOps* ops, bool announce, AutoArray<ZoneEventRecord>& fired)
{
    const ZoneRecord* town = registry.GetZone(target);
    if (!ops || !town)
    {
        return false;
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
        return false;
    }
    const ZoneRecord* src = registry.GetZone(registry.FindZoneIndex(r.source));
    const Vector3 from = StagePoint(_tuning, town->pos, src ? src->pos : town->pos);
    AIGroup* grp = ops->SpawnSquad(r.strength, from, town->pos);
    if (!grp)
    {
        return false;
    }
    r.group = grp;
    r.phase = CALive;
    r.clock = 0;
    r.resend = ResendInterval;
    r.deferLogged = false;
    LOG_INFO(Core, "CounterAttack: {} soldier(s) from {} move on {} from [{:.0f}, {:.0f}]", r.strength,
             (const char*)r.source, (const char*)r.target, from.X(), from.Z());
    if (announce)
    {
        AddEvent(fired, ZECounterAttack, target);
    }
    return true;
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
        // the base is in the bubble: its own soldiers march out
        int taken = 0;
        AIGroup* grp = ops ? ops->DetachFromGarrison(src, n, town->pos, taken) : nullptr;
        if (!grp || taken <= 0)
        {
            return false;
        }
        r.strength = taken;
        r.group = grp;
        r.phase = CALive;
        r.clock = 0;
        r.resend = ResendInterval;
        LOG_INFO(Core, "CounterAttack: {} soldier(s) of the {} garrison march on {}", taken, (const char*)s->name,
                 (const char*)r.target);
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
            r.clock -= elapsed;
            if (r.clock > 0)
            {
                return false;
            }
            if (InBubble(registry, view, target))
            {
                if (!GoLive(registry, view, target, r, ops, true, fired))
                {
                    r.clock = RetryInterval;
                }
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
            if (InBubble(registry, view, target) && GoLive(registry, view, target, r, ops, false, fired))
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
            r.clock += elapsed;
            r.resend -= elapsed;
            if (r.resend <= 0)
            {
                r.resend = ResendInterval;
                if (Dist2D(leader.X(), leader.Z(), town->pos.X(), town->pos.Z()) > registry.Tuning().zoneArea * 0.5f)
                {
                    ops->MoveOn(r.group, town->pos);
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
    // the squad IS the new garrison; the stored reserve only tops it up
    float top = town->garrisonExempt ? 0.0f : town->garrisonMax - (float)alive;
    town->garrison = top > 0 ? top : 0.0f;
    LOG_INFO(Core, "CounterAttack: {} soldier(s) stay in {} as its garrison (reserve {})", alive, (const char*)r.target,
             town->garrison);
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

class EngineOps final : public CounterAttackOps
{
  public:
    static void Posture(AIGroup* grp)
    {
        grp->SetCombatModeMajor(CMAware);
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

    AIGroup* SpawnSquad(int n, Vector3Par from, Vector3Par target) override
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
        Posture(grp);
        MoveOn(grp, target);
        GetNetworkManager().UpdateObject(grp);
        return grp;
    }

    AIGroup* DetachFromGarrison(int sourceZone, int n, Vector3Par target, int& taken) override
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
        Posture(grp);
        MoveOn(grp, target);
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

void CounterAttack::OnZoneEvents(const AutoArray<ZoneEventRecord>& fired)
{
    ZoneRegistry& registry = ZoneRegistry::Instance();
    for (int i = 0; i < fired.Size(); i++)
    {
        const ZoneEventRecord& ev = fired[i];
        if (ev.type == ZECaptured)
        {
            OnTownFlipped(registry, ev.zoneIndex, GarrisonCache::Instance().Tuning().groupSize, ReadWarLevel());
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
            _records[i].deferLogged = false;
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
