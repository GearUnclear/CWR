#include <Poseidon/Game/Guerrilla/GarrisonCache.hpp>
#include <Poseidon/Game/Guerrilla/BaseProps.hpp>
#include <Poseidon/Game/Guerrilla/CounterAttack.hpp> // CounterAttackGate (S5 pending release)
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>
#include <Poseidon/Game/Guerrilla/AlertMachine.hpp> // D2 recovery: zone alert GREEN
#include <Poseidon/Game/Guerrilla/OccupierForces.hpp> // the pool seams (D2 Draw, 5.5 refunds); pass-through while inactive

#include <Poseidon/IO/ParamFileExt.hpp> // Pars / ExtParsMission
#include <Poseidon/IO/Serialization/ParamArchive.hpp>

#include <Evaluator/express.hpp> // GameState / GameValue (event dispatch, war level)

#include <Poseidon/World/World.hpp>
#include <Poseidon/World/Terrain/Landscape.hpp> // GLOB_LAND surface Y
#include <Poseidon/World/Entities/Infantry/Person.hpp>
#include <Poseidon/AI/AI.hpp>
#include <Poseidon/AI/AICore.hpp>              // MaxGroups
#include <Poseidon/AI/VehicleAI.hpp>           // Rank
#include <Poseidon/AI/Path/ArcadeWaypoint.hpp> // ArcadeWaypointInfo / CombatMode
#include <Poseidon/Network/Network.hpp>        // GetNetworkManager

#include <Random/randomGen.hpp>

#include <Poseidon/Foundation/Common/FltOpts.hpp> // toInt
#include <Poseidon/Foundation/Containers/BoolArray.hpp>
#include <Poseidon/Foundation/Enums/EnumNames.hpp>
#include <Poseidon/Foundation/Framework/DebugLog.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>
#include <Poseidon/Foundation/platform.hpp>

#include <limits.h>
#include <math.h>
#include <string.h>

// Shared command internals from Game/Commands/GameStateExtWorld.cpp - the
// bodies of createUnit / deleteVehicle without the script-value parsing.
// Global namespace, same forward-declaration idiom as NetworkClientOnMessage.
void CreateUnit(AIGroup* group, RString type, Vector3Par position, RString init, float skill, Rank rank);
void DeleteVehicle(Entity* veh);
// waypoint re-evaluation hook, mirrored from WaypointSetType
// (GameStateExtWorldWaypoint.cpp); defined at global scope in AIArcade.cpp
void OnWaypointsUpdated(Poseidon::AIGroupContext* context);

namespace Poseidon::Guerrilla
{

// Defined in GarrisonCacheCommands.cpp.  Referencing it from here forces the
// command TU (whose only other content is an INIT_MODULE registration) into
// the link - same pattern as EnsureZoneRegistryCommandsLinked.
void EnsureGarrisonCacheCommandsLinked();

// Process-lifetime singleton - no global constructor (see express.hpp's
// GGameState for the convention).
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
GarrisonCache& GarrisonCache::Instance()
{
    EnsureGarrisonCacheCommandsLinked();
    static GarrisonCache instance;
    return instance;
}
#pragma clang diagnostic pop

static float Dist2DSq(float ax, float az, float bx, float bz)
{
    float dx = ax - bx;
    float dz = az - bz;
    return dx * dx + dz * dz;
}

// ---------------------------------------------------------------------------
// lifecycle / config
// ---------------------------------------------------------------------------

void GarrisonCache::Clear()
{
    _tuning = GarrisonTuning();
    _states.Clear();
    for (int i = 0; i < NGarrisonEventTypes; i++)
    {
        _handlers[i] = RString();
    }
    _accum = 0;
    _pending.Clear();
}

void GarrisonCache::InitMission()
{
    Clear();
    LoadFromConfig();
}

void GarrisonCache::LoadFromConfig()
{
    const ParamEntry* zones = ExtParsMission.FindEntry("CfgGuerrillaZones");
    if (!zones)
    {
        zones = Pars.FindEntry("CfgGuerrillaZones");
    }
    LoadFromParams(zones);
}

void GarrisonCache::LoadFromParams(const ParamEntry* zonesCfg)
{
    _tuning = GarrisonTuning();
    _accum = 0;
    if (!zonesCfg)
    {
        return;
    }
    _tuning.cacheInterval = zonesCfg->ReadValue("cacheInterval", _tuning.cacheInterval);
    _tuning.groupSize = toInt(zonesCfg->ReadValue("groupSize", (float)_tuning.groupSize));
    _tuning.recoverInterval = zonesCfg->ReadValue("garrisonRecoverInterval", _tuning.recoverInterval);
    if (_tuning.groupSize < 1)
    {
        _tuning.groupSize = 1;
    }
}

// ---------------------------------------------------------------------------
// queries
// ---------------------------------------------------------------------------

bool GarrisonCache::IsActive() const
{
    return ZoneRegistry::Instance().IsActive();
}

bool GarrisonCache::IsSpawned(int zoneIndex) const
{
    if (zoneIndex < 0 || zoneIndex >= _states.Size())
    {
        return false;
    }
    return _states[zoneIndex].spawned;
}

int GarrisonCache::LiveCount(int zoneIndex) const
{
    if (zoneIndex < 0 || zoneIndex >= _states.Size() || !_states[zoneIndex].spawned)
    {
        return 0;
    }
    return CountAlive(_states[zoneIndex]);
}

int GarrisonCache::LiveCountWithin(int zoneIndex, Vector3Par pos, float radius) const
{
    if (zoneIndex < 0 || zoneIndex >= _states.Size() || !_states[zoneIndex].spawned)
    {
        return 0;
    }
    const float radiusSq = radius * radius;
    const GarrisonState& state = _states[zoneIndex];
    int alive = 0;
    for (int g = 0; g < state.groups.Size(); g++)
    {
        AIGroup* grp = state.groups[g];
        if (!grp)
        {
            continue;
        }
        for (int u = 0; u < MAX_UNITS_PER_GROUP; u++)
        {
            AIUnit* unit = grp->UnitWithID(u + 1);
            if (!unit || unit->GetLifeState() != AIUnit::LSAlive)
            {
                continue;
            }
            const Vector3 at = unit->Position();
            if (Dist2DSq(at.X(), at.Z(), pos.X(), pos.Z()) <= radiusSq)
            {
                alive++;
            }
        }
    }
    return alive;
}

int GarrisonCache::NGroups(int zoneIndex) const
{
    if (zoneIndex < 0 || zoneIndex >= _states.Size())
    {
        return 0;
    }
    return _states[zoneIndex].groups.Size();
}

AIGroup* GarrisonCache::GetGroup(int zoneIndex, int i) const
{
    if (zoneIndex < 0 || zoneIndex >= _states.Size())
    {
        return nullptr;
    }
    const GarrisonState& state = _states[zoneIndex];
    if (i < 0 || i >= state.groups.Size())
    {
        return nullptr;
    }
    return state.groups[i];
}

int GarrisonCache::CountAlive(const GarrisonState& state) const
{
    int alive = 0;
    for (int g = 0; g < state.groups.Size(); g++)
    {
        AIGroup* grp = state.groups[g];
        if (!grp)
        {
            continue;
        }
        for (int u = 0; u < MAX_UNITS_PER_GROUP; u++)
        {
            AIUnit* unit = grp->UnitWithID(u + 1);
            if (unit && unit->GetLifeState() == AIUnit::LSAlive)
            {
                alive++;
            }
        }
    }
    return alive;
}

// ---------------------------------------------------------------------------
// events
// ---------------------------------------------------------------------------

void GarrisonCache::SetEventHandler(GarrisonEventType type, RString handler)
{
    if (type < 0 || type >= NGarrisonEventTypes)
    {
        return;
    }
    _handlers[type] = handler;
}

RString GarrisonCache::GetEventHandler(GarrisonEventType type) const
{
    if (type < 0 || type >= NGarrisonEventTypes)
    {
        return RString();
    }
    return _handlers[type];
}

int GarrisonCache::EventTypeFromName(const char* name)
{
    if (!name)
    {
        return -1;
    }
    if (stricmp(name, "garrisonSpawned") == 0)
    {
        return GESpawned;
    }
    if (stricmp(name, "garrisonDespawned") == 0)
    {
        return GEDespawned;
    }
    return -1;
}

// ---------------------------------------------------------------------------
// pure decision logic (unit-tested)
// ---------------------------------------------------------------------------

GarrisonAction GarrisonCache::Decide(const GarrisonDecisionInput& in, float cacheRadius, float despawnHysteresis)
{
    if (!in.occupierOwned)
    {
        // zone flipped away from the occupier: any leftover cache groups
        // are released (survivor write-back lands on the flipped record)
        return in.spawned ? GActDespawn : GActNone;
    }
    if (!in.playerValid)
    {
        // no live player: freeze rather than despawn (spawning.sqs keeps
        // polling getPos aP; freezing is the safe native equivalent)
        return GActNone;
    }
    if (!in.spawned)
    {
        bool near = in.playerDistSq <= cacheRadius * cacheRadius;
        return near && in.reserve >= 1 ? GActSpawn : GActNone;
    }
    float rOut = cacheRadius + despawnHysteresis;
    return in.playerDistSq > rOut * rOut ? GActDespawn : GActNone;
}

int GarrisonCache::StepRecover(const GarrisonRecoverInput& in, float elapsed, float interval, float cacheRadius,
                               float despawnHysteresis, float& clock)
{
    if (interval <= 0)
    {
        clock = 0;
        return 0;
    }
    if (!in.playerValid)
    {
        return 0; // freeze, like every other zone clock while the player is dead
    }
    float rOut = cacheRadius + despawnHysteresis;
    bool eligible = in.occupierOwned && !in.spawned && in.playerDistSq > rOut * rOut && in.alertGreen &&
                    in.capture <= 0 && in.resistancePresent == 0 && in.reserve < in.reserveMax;
    if (!eligible)
    {
        clock = 0;
        return 0;
    }
    clock += elapsed;
    int gained = 0;
    while (clock >= interval && in.reserve + (float)gained < in.reserveMax)
    {
        clock -= interval;
        gained++;
    }
    if (in.reserve + (float)gained >= in.reserveMax)
    {
        clock = 0; // at strength: the next loss starts a fresh interval
    }
    return gained;
}

int GarrisonCache::ApplyRecover(ZoneRecord& z, int gained, float interval)
{
    if (gained <= 0)
    {
        return 0;
    }
    const int granted = OccupierForces::Instance().Draw(gained);
    z.garrison = floorf(z.garrison) + (float)granted;
    if (granted < gained)
    {
        // an empty pool grants nothing and the clock is clamped: the zone is
        // due again next pass, no owed bodies are banked
        z.recoverClock = interval > 0 ? interval : 0;
    }
    return granted;
}

void GarrisonCache::WriteBackSurvivors(ZoneRegistry& registry, int zone, int survivors, int dead)
{
    ZoneRecord* z = registry.GetZoneMutable(zone);
    if (!z)
    {
        return;
    }
    OccupierForces& forces = OccupierForces::Instance();
    if (dead > 0)
    {
        forces.NoteLoss(dead, "garrison"); // diagnostics (the `lost` field)
    }
    if (survivors > 0 && forces.Active() && !OccupierHolds(registry, *z))
    {
        // a flipped base's live garrison: with the occupier pool on it goes
        // back to the pool (5.5), not onto the new owner's record
        forces.Refund(survivors, "flipped base");
        LOG_INFO(Core, "Guerrilla: {} survivor(s) of the lost {} garrison go back to the occupier pool", survivors,
                 (const char*)z->name);
        return;
    }
    z->garrison = floorf(z->garrison) + (float)survivors;
}

// ---------------------------------------------------------------------------
// S5 pending deposits (OCCUPIER-FORCES.md 3.3; pure, unit-tested)
// ---------------------------------------------------------------------------

namespace
{
// the phase-2 refund seam (SetPendingRefundHook); constant-initialized, so
// no global constructor
GarrisonCache::PendingRefundHook GPendingRefundHook = nullptr;

int MinBodies(int a, int b)
{
    return a < b ? a : b;
}
} // namespace

void GarrisonCache::SetPendingRefundHook(PendingRefundHook hook)
{
    GPendingRefundHook = hook;
}

GarrisonCache::PendingRefundHook GarrisonCache::GetPendingRefundHook()
{
    return GPendingRefundHook;
}

bool GarrisonCache::OccupierHolds(const ZoneRegistry& registry, const ZoneRecord& z)
{
    const bool post = stricmp(z.type, "CITY") != 0 && stricmp(z.owner, registry.OccupierSide()) == 0;
    const bool town = registry.IsAdministeredTown(z) && !z.garrisonExempt;
    return post || town;
}

void GarrisonCache::Deposit(ZoneRegistry& registry, int zone, int bodies, int originZone)
{
    ZoneRecord* z = registry.GetZoneMutable(zone);
    if (!z || bodies < 1)
    {
        return;
    }
    if (z->pending <= 0)
    {
        z->pending = 0;
        z->pendingOrigin = originZone;
    }
    else if (z->pendingOrigin != originZone)
    {
        z->pendingOrigin = -1; // mixed origins: no single zone to refund to
    }
    z->pending += bodies;
    LOG_INFO(Core,
             "Guerrilla: {} body(ies) pending at {} (pending {}, reserve {}); they join the reserve once the player "
             "leaves or their spawn points are hidden",
             bodies, (const char*)z->name, z->pending, z->garrison);
}

void GarrisonCache::Deposit(int zone, int bodies, int originZone)
{
    Deposit(ZoneRegistry::Instance(), zone, bodies, originZone);
}

bool GarrisonCache::InDepositBubble(const ZoneRecord& z, bool playerValid, float px, float pz, float cacheRadius)
{
    if (!playerValid)
    {
        return false;
    }
    const float edge = cacheRadius + DespawnHysteresis;
    return Dist2DSq(px, pz, z.pos.X(), z.pos.Z()) <= edge * edge;
}

bool GarrisonCache::AddReserve(ZoneRegistry& registry, int zone, int bodies, int originZone, bool playerValid,
                               float px, float pz)
{
    ZoneRecord* z = registry.GetZoneMutable(zone);
    if (!z || bodies < 1)
    {
        return false;
    }
    if (InDepositBubble(*z, playerValid, px, pz, registry.Tuning().cacheRadius))
    {
        Deposit(registry, zone, bodies, originZone);
        return true;
    }
    z->garrison = floorf(z->garrison) + (float)bodies;
    return false;
}

Vector3 GarrisonCache::SpawnPoint(const char* zoneName, Vector3Par centre, float spread, int k)
{
    const unsigned int seed = CounterAttack::MakeSeed(zoneName, 0, 0, k);
    const float x = centre.X() + (CounterAttack::Roll01(seed, 0x51A7) * 2.0f - 1.0f) * spread;
    const float z = centre.Z() + (CounterAttack::Roll01(seed, 0x7E3D) * 2.0f - 1.0f) * spread;
    return Vector3(x, centre.Y(), z);
}

bool GarrisonCache::PendingSpawnHidden(const char* zoneName, Vector3Par centre, float spread, int bodies,
                                       const CounterAttackGate& gate)
{
    for (int dz = -1; dz <= 1; dz++)
    {
        for (int dx = -1; dx <= 1; dx++)
        {
            const Vector3 at(centre.X() + (float)dx * spread, centre.Y(), centre.Z() + (float)dz * spread);
            if (gate.Exposure(at) != CAExposeHidden)
            {
                return false;
            }
        }
    }
    // L11: the points the bodies will actually stand on
    for (int k = 0; k < bodies; k++)
    {
        if (gate.Exposure(SpawnPoint(zoneName, centre, spread, k)) != CAExposeHidden)
        {
            return false;
        }
    }
    return true;
}

int GarrisonCache::FlushPending(ZoneRegistry& registry, int zone, bool playerValid, float px, float pz,
                                const CounterAttackGate* gate)
{
    ZoneRecord* z = registry.GetZoneMutable(zone);
    if (!z || z->pending <= 0)
    {
        return PFNone;
    }
    if (!OccupierHolds(registry, *z))
    {
        RefundPending(registry, zone, playerValid, px, pz);
        return PFRefunded;
    }
    if (!playerValid)
    {
        return PFNone; // a dead player freezes the deposit like every other zone clock
    }
    int result = PFNone;
    if (!InDepositBubble(*z, playerValid, px, pz, registry.Tuning().cacheRadius))
    {
        result = PFReleasedBubble;
    }
    else if (gate && PendingSpawnHidden(z->name, z->pos, registry.Tuning().zoneArea / 3.0f,
                                        (int)floorf(z->garrison > 0 ? z->garrison : 0) + z->pending, *gate))
    {
        result = PFReleasedHidden;
    }
    if (result == PFNone)
    {
        return PFNone;
    }
    const int bodies = z->pending;
    z->garrison = floorf(z->garrison) + (float)bodies;
    z->pending = 0;
    z->pendingOrigin = -1;
    LOG_INFO(Core, "Guerrilla: {} pending body(ies) join the reserve at {} ({}; reserve {})", bodies,
             (const char*)z->name, result == PFReleasedBubble ? "the player left" : "every spawn point is hidden",
             z->garrison);
    return result;
}

void GarrisonCache::RefundPending(ZoneRegistry& registry, int zone, bool playerValid, float px, float pz)
{
    ZoneRecord* z = registry.GetZoneMutable(zone);
    if (!z || z->pending <= 0)
    {
        return;
    }
    const int bodies = z->pending;
    const int origin = z->pendingOrigin;
    z->pending = 0;
    z->pendingOrigin = -1;
    if (GPendingRefundHook && GPendingRefundHook(registry, zone, origin, bodies))
    {
        LOG_INFO(Core, "Guerrilla: {} changed hands with {} body(ies) pending; handed to the refund hook",
                 (const char*)z->name, bodies);
        return;
    }
    ZoneRecord* o = origin != zone ? registry.GetZoneMutable(origin) : nullptr;
    if (o && OccupierHolds(registry, *o))
    {
        // the origin's live garrison counts against its room too (L2)
        const float room =
            o->garrisonMax - (float)Instance().LiveCount(origin) - floorf(o->garrison) - (float)o->pending;
        const int back = MinBodies(bodies, room > 0 ? (int)floorf(room) : 0);
        AddReserve(registry, origin, back, origin, playerValid, px, pz);
        LOG_INFO(Core, "Guerrilla: {} changed hands with {} body(ies) pending; {} back to {}, {} discarded",
                 (const char*)z->name, bodies, back, (const char*)o->name, bodies - back);
        return;
    }
    LOG_INFO(Core, "Guerrilla: {} changed hands with {} body(ies) pending; discarded (no occupier-held origin)",
             (const char*)z->name, bodies);
}

int GarrisonCache::PlanGroups(int reserve, int groupSize, AutoArray<int>& takes)
{
    takes.Clear();
    if (groupSize < 1)
    {
        groupSize = 1;
    }
    int remaining = reserve;
    while (remaining > 0)
    {
        int take = remaining < groupSize ? remaining : groupSize;
        takes.Add(take);
        remaining -= take;
    }
    return takes.Size();
}

// ---------------------------------------------------------------------------
// world-touching internals (engine path only)
// ---------------------------------------------------------------------------

// the k-th garrison body's point (SpawnPoint, L11) on the surface
static Vector3 GarrisonPointOnSurface(const char* zoneName, Vector3Par center, float radius, int k)
{
    const Vector3 at = GarrisonCache::SpawnPoint(zoneName, center, radius, k);
    float y = center.Y();
    if (GLandscape)
    {
        float dx, dz;
        y = GLOB_LAND->SurfaceYAboveWater(at.X(), at.Z(), &dx, &dz);
    }
    return Vector3(at.X(), y, at.Z());
}

namespace
{
// The engine perception gate (CounterAttack::EngineGate) for pending-release
// samples: the gate's terrain ray aims from the ground under a point, so
// each sample is put on the surface first (the zone centre's elevation is
// not the ground 50 m away).
class GroundedGate final : public CounterAttackGate
{
  public:
    explicit GroundedGate(const CounterAttackGate& inner) : _inner(inner) {}
    int Exposure(Vector3Par pos) const override
    {
        Vector3 at = pos;
        if (GLandscape)
        {
            float dx, dz;
            at[1] = GLOB_LAND->SurfaceYAboveWater(at.X(), at.Z(), &dx, &dz);
        }
        return _inner.Exposure(at);
    }

  private:
    const CounterAttackGate& _inner;
};
} // namespace

// Center + group creation are the shared Guerrilla helpers EnsureSideCenter /
// CreateSideGroup (ZoneRegistry.hpp) - the occupier may have no seeded units,
// so its command center is created on demand; the group mirrors GroupCreate
// (GameStateExtWorldConfig.cpp:693).

// hold-the-zone posture from spawning.sqs: waypoint on the zone (SENTRY for
// the officer group, GUARD otherwise), behaviour AWARE, combat mode YELLOW -
// mirrors WaypointAdd / WaypointSetType / GrpSetBehaviour / GrpSetCombatMode
static void SetHoldPosture(AIGroup* grp, Vector3Par zonePos, bool firstGroup)
{
    int index = grp->AddWaypoint();
    ArcadeWaypointInfo& wp = grp->GetWaypoint(index);
    wp.position = zonePos;
    wp.placement = 0;
    wp.type = firstGroup ? ACSENTRY : ACGUARD;
    if (grp->GetCurrent())
    {
        AIGroupContext context(grp);
        context._task = grp->GetCurrent()->_task;
        context._fsm = grp->GetCurrent()->_fsm;
        ::OnWaypointsUpdated(&context);
    }

    grp->SetCombatModeMajor(CMAware);
    grp->SetSemaphore(AI::SemaphoreYellow);
    PackedBoolArray all;
    for (int i = 0; i < MAX_UNITS_PER_GROUP; i++)
    {
        if (grp->UnitWithID(i + 1))
        {
            all.Set(i, true);
        }
    }
    grp->SendSemaphore(AI::SemaphoreYellow, all);

    GetNetworkManager().UpdateObject(grp);
}

void GarrisonCache::SyncStates()
{
    int n = ZoneRegistry::Instance().NZones();
    if (_states.Size() != n)
    {
        // config changed under us (mission (re)init); stale groups were
        // already dropped by Clear/InitMission
        _states.Clear();
        _states.Resize(n);
    }
}

void GarrisonCache::SpawnGarrison(int zoneIndex, float warLevel, AutoArray<GarrisonEventRecord>& fired)
{
    ZoneRegistry& registry = ZoneRegistry::Instance();
    ZoneRecord* z = registry.GetZoneMutable(zoneIndex);
    if (!z || zoneIndex >= _states.Size())
    {
        return;
    }
    GarrisonState& state = _states[zoneIndex];

    // the garrison always fields the campaign's occupier: a post's owner IS
    // that side, and an administered town is NEUTRAL (no roster of its own)
    const RString side = registry.OccupierSide();
    RString tierClass = registry.FactionTierClass(side, warLevel);
    if (tierClass.GetLength() == 0)
    {
        return; // no tier table for the occupier: leave the reserve untouched
    }
    RString officerClass = registry.FactionValue(side, "officer");
    if (officerClass.GetLength() == 0)
    {
        officerClass = tierClass;
    }

    AICenter* center = EnsureSideCenter(side);
    if (!center)
    {
        return;
    }

    AutoArray<int> takes;
    int reserve = toInt(z->garrison);
    PlanGroups(reserve, _tuning.groupSize, takes);

    const float spread = registry.Tuning().zoneArea / 3.0f;
    // garrison morale (OCCUPIER-FORCES.md 7.3, D21): a town that turned on
    // the occupier (standing < 25) fields its garrison at skill x 0.8
    const float morale = OccupierForces::MoraleFactor(*z);
    int spawned = 0;
    int remaining = reserve;
    bool firstGroup = true;
    int point = 0; // the k of SpawnPoint (L11): what the release test checked
    for (int g = 0; g < takes.Size(); g++)
    {
        AIGroup* grp = CreateSideGroup(center);
        if (!grp)
        {
            // AICenter group slots exhausted: stop, the rest stays in reserve
            LOG_WARN(Core, "GarrisonCache: group budget exhausted spawning zone '{}'", (const char*)z->name);
            break;
        }
        // role-diverse composition (plan 15): the squad template fills MG/
        // AT/medic/sniper slots from the faction's tier role arrays; a
        // descriptor without role arrays composes all-riflemen - the exact
        // pre-plan-15 squad
        AutoArray<RString> classes;
        registry.FactionSquad(side, warLevel, takes[g], classes);
        for (int u = 0; u < takes[g]; u++)
        {
            bool officer = firstGroup && u == 0;
            RString unitClass = u < classes.Size() ? classes[u] : tierClass;
            Vector3 pos = GarrisonPointOnSurface(z->name, z->pos, spread, point++);
            // a generated base's layout props (OCCUPIER-FORCES.md 5.7 step 2)
            // stand inside the spawn square: CreateUnit's FindFreePosition
            // only clears a body out of the props' geometry, so first push
            // the point out of every prop box (plus BaseProps::NudgeMargin)
            if (z->generated)
            {
                float nx = pos.X(), nz = pos.Z();
                if (BaseProps::Instance().NudgeClear(nx, nz))
                {
                    float sdx, sdz;
                    const float ny = GLandscape ? GLOB_LAND->SurfaceYAboveWater(nx, nz, &sdx, &sdz) : pos.Y();
                    pos = Vector3(nx, ny, nz);
                }
            }
            int before = grp->NUnits();
            ::CreateUnit(grp, officer ? officerClass : unitClass, pos, RString(), (officer ? 0.6f : 0.5f) * morale,
                         officer ? RankSergeant : RankPrivate);
            if (grp->NUnits() > before)
            {
                spawned++;
            }
        }
        remaining -= takes[g];
        if (grp->NUnits() == 0)
        {
            // nothing materialized (bad classname); drop the empty group
            grp->RemoveFromCenter();
            continue;
        }
        SetHoldPosture(grp, z->pos, firstGroup);
        state.groups.Add(grp);
        firstGroup = false;
    }

    if (spawned == 0)
    {
        state.groups.Clear();
        return; // reserve untouched; retried next tick
    }

    // the count now lives in the cache groups (see spawning.sqs #doSpawn);
    // anything the group budget refused stays behind as reserve.  Bodies
    // whose class failed to materialize inside a created group are NOT
    // returned to the reserve (a stored body beside a live garrison would
    // block a town's support gate forever); say so instead of dropping
    // them silently (issue #72).
    const int planned = reserve - (remaining > 0 ? remaining : 0);
    if (spawned < planned)
    {
        LOG_WARN(Core, "GarrisonCache: zone '{}' materialized {} of {} planned bodies; {} lost to unspawnable classes",
                 (const char*)z->name, spawned, planned, planned - spawned);
        // with the occupier pool on they go back to it (5.5)
        OccupierForces::Instance().Refund(planned - spawned, "partial spawn");
    }
    state.spawned = true;
    z->garrison = (float)(remaining > 0 ? remaining : 0);
    z->liveOccupiers = (float)spawned;

    GarrisonEventRecord ev;
    ev.type = GESpawned;
    ev.zoneIndex = zoneIndex;
    ev.count = spawned;
    fired.Add(ev);
}

void GarrisonCache::DespawnGarrison(int zoneIndex, AutoArray<GarrisonEventRecord>& fired)
{
    ZoneRegistry& registry = ZoneRegistry::Instance();
    ZoneRecord* z = registry.GetZoneMutable(zoneIndex);
    if (!z || zoneIndex >= _states.Size())
    {
        return;
    }
    GarrisonState& state = _states[zoneIndex];
    if (!state.spawned)
    {
        return;
    }

    int survivors = 0;
    int dead = 0;
    for (int g = 0; g < state.groups.Size(); g++)
    {
        AIGroup* grp = state.groups[g];
        if (!grp)
        {
            continue;
        }
        // collect first - DeleteVehicle mutates the group's unit table
        AutoArray<Person*> bodies;
        for (int u = 0; u < MAX_UNITS_PER_GROUP; u++)
        {
            AIUnit* unit = grp->UnitWithID(u + 1);
            if (!unit)
            {
                continue;
            }
            if (unit->GetLifeState() == AIUnit::LSAlive)
            {
                survivors++;
            }
            else
            {
                dead++;
            }
            Person* person = unit->GetPerson();
            if (person)
            {
                bodies.Add(person);
            }
        }
        // mirrors VehDelete / GroupDelete (Game/Commands)
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
    state.groups.Clear();
    state.spawned = false;

    // survivor write-back (spawning.sqs #dsDone), ADDED to the reserve still
    // stored (U1, OCCUPIER-FORCES.md 5.5): a group-budget remainder, D2
    // regrowth or a top-up written while the garrison was live (AdoptLive,
    // RevertTown, a pending release) is kept, never overwritten
    WriteBackSurvivors(registry, zoneIndex, survivors, dead);
    z->liveOccupiers = 0;

    GarrisonEventRecord ev;
    ev.type = GEDespawned;
    ev.zoneIndex = zoneIndex;
    ev.count = survivors;
    fired.Add(ev);
}

// ---------------------------------------------------------------------------
// simulation
// ---------------------------------------------------------------------------

void GarrisonCache::Simulate(float deltaT)
{
    ZoneRegistry& registry = ZoneRegistry::Instance();
    if (!registry.IsActive())
    {
        return;
    }
    _accum += deltaT;
    if (_accum < _tuning.cacheInterval)
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
    SyncStates();

    bool playerValid = false;
    float playerX = 0;
    float playerZ = 0;
    Person* player = world->GetRealPlayer();
    if (player && !player->IsDammageDestroyed())
    {
        playerValid = true;
        playerX = player->Position().X();
        playerZ = player->Position().Z();
    }

    const float warLevel = OccupierForces::Instance().OccupierWarLevel(); // the occupier-side level (7.1)
    const float cacheRadius = registry.Tuning().cacheRadius;
    // the perception gate for S5 releases: refreshed at most once per pass,
    // and only when some zone has a deposit pending
    const CounterAttackGate* engineGate = nullptr;

    AutoArray<GarrisonEventRecord> fired;
    for (int i = 0; i < registry.NZones(); i++)
    {
        ZoneRecord* z = registry.GetZoneMutable(i);
        if (!z || i >= _states.Size())
        {
            continue;
        }
        GarrisonState& state = _states[i];
        if (state.spawned)
        {
            // deleted groups read back as null links; drop them, then keep
            // the zone's transient live-occupier mirror fresh
            state.groups.Compact();
            z->liveOccupiers = (float)CountAlive(state);
        }

        // A reserve waiting beside a cache that holds no living body: the
        // garrison was wiped and the record refilled before this pass could
        // release it (a town retaken between two cache passes, issue #79),
        // or a group-budget remainder outlived the bodies that did spawn.
        // Release the dead groups (the same write-back as any despawn; under
        // U1 it adds 0 and keeps the reserve).  S5: with the player inside
        // the deposit bubble the waiting reserve becomes a PENDING deposit,
        // so it is not re-materialized beside the player who just killed the
        // last defender; it appears when its spawn points are hidden or he
        // leaves.  Outside the bubble Decide materializes it as before.
        if (state.spawned && z->garrison >= 1 && CountAlive(state) == 0 && registry.IsGarrisonable(*z))
        {
            DespawnGarrison(i, fired);
            if (InDepositBubble(*z, playerValid, playerX, playerZ, cacheRadius))
            {
                const int waiting = (int)floorf(z->garrison);
                z->garrison = 0;
                Deposit(registry, i, waiting, i);
            }
        }

        // S5: a pending deposit is checked before Decide (a released reserve
        // may materialize this pass, every spawn point hidden) and before
        // StepRecover (a deposit never sits beside a regrowing reserve)
        if (z->pending > 0)
        {
            if (!engineGate)
            {
                engineGate = &CounterAttack::EngineGate(playerValid, playerX, playerZ);
            }
            const GroundedGate grounded(*engineGate);
            FlushPending(registry, i, playerValid, playerX, playerZ, &grounded);
        }

        GarrisonDecisionInput in;
        // an occupier-held post or an occupier-administered town (issue #79
        // B2; the town model is documented at ZoneRegistry::IsAdministeredTown)
        in.occupierOwned = registry.IsGarrisonable(*z);
        in.spawned = state.spawned;
        in.reserve = z->garrison;
        in.playerValid = playerValid;
        in.playerDistSq = Dist2DSq(playerX, playerZ, z->pos.X(), z->pos.Z());

        switch (Decide(in, cacheRadius, DespawnHysteresis))
        {
            case GActSpawn:
                SpawnGarrison(i, warLevel, fired);
                break;
            case GActDespawn:
                DespawnGarrison(i, fired);
                break;
            default:
                break;
        }

        // D2 recovery on the stored reserve (issue #71): never while the
        // garrison is live, so a materialized post never grows a body
        GarrisonRecoverInput rec;
        rec.occupierOwned = in.occupierOwned;
        rec.spawned = state.spawned;
        rec.reserve = z->garrison;
        rec.reserveMax = z->garrisonMax;
        rec.playerValid = playerValid;
        rec.playerDistSq = in.playerDistSq;
        rec.alertGreen = AlertMachine::Instance().GetZoneState(i) == ASGreen;
        rec.capture = z->capture;
        rec.resistancePresent = z->lastGuer;
        int gained =
            StepRecover(rec, elapsed, _tuning.recoverInterval, cacheRadius, DespawnHysteresis, z->recoverClock);
        if (gained > 0)
        {
            // OCCUPIER-FORCES.md 5.3: regrowth draws on the pool (everything
            // granted while the pool is inactive, the legacy D2)
            const int granted = ApplyRecover(*z, gained, _tuning.recoverInterval);
            if (granted > 0)
            {
                LOG_INFO(Core, "Guerrilla: garrison at {} recovered {} (reserve {} of {})", (const char*)z->name,
                         granted, z->garrison, z->garrisonMax);
            }
        }
    }
    // handlers run only after the cache's own state mutation completed
    DispatchEvents(fired);
}

void GarrisonCache::AdoptGroup(int zoneIndex, AIGroup* grp)
{
    SyncStates();
    ZoneRecord* z = ZoneRegistry::Instance().GetZoneMutable(zoneIndex);
    if (!grp || !z || zoneIndex >= _states.Size())
    {
        return;
    }
    GarrisonState& state = _states[zoneIndex];
    state.groups.Compact();
    SetHoldPosture(grp, z->pos, state.groups.Size() == 0);
    state.groups.Add(grp);
    state.spawned = true;
    z->liveOccupiers = (float)CountAlive(state);
}

void GarrisonCache::ForceDespawn(int zoneIndex)
{
    SyncStates();
    AutoArray<GarrisonEventRecord> fired;
    DespawnGarrison(zoneIndex, fired);
    DispatchEvents(fired);
}

void GarrisonCache::MarkSpawnedForTest(int zoneIndex, bool spawned)
{
    SyncStates();
    if (zoneIndex < 0 || zoneIndex >= _states.Size())
    {
        return;
    }
    _states[zoneIndex].spawned = spawned;
}

void GarrisonCache::DispatchEvents(const AutoArray<GarrisonEventRecord>& fired)
{
    if (fired.Size() == 0 || !GWorld)
    {
        return;
    }
    GameState* gstate = GWorld->GetGameState();
    if (!gstate)
    {
        return;
    }
    ZoneRegistry& registry = ZoneRegistry::Instance();
    for (int i = 0; i < fired.Size(); i++)
    {
        const GarrisonEventRecord& ev = fired[i];
        RString handler = GetEventHandler(ev.type);
        if (handler.GetLength() == 0)
        {
            continue;
        }
        const ZoneRecord* z = registry.GetZone(ev.zoneIndex);
        if (!z)
        {
            continue;
        }

        GameArrayType pars;
        pars.Resize(3);
        pars[0] = (float)ev.zoneIndex;
        pars[1] = GameStringType(z->name);
        pars[2] = (float)ev.count;

        // dispatch idiom copied from ZoneRegistry::DispatchEvents
        GameVarSpace local;
        gstate->BeginContext(&local);
        gstate->VarSetLocal("_this", GameValue(pars), true);
        gstate->Execute(handler);
        gstate->EndContext();
    }
}

// ---------------------------------------------------------------------------
// serialization
// ---------------------------------------------------------------------------

LSError GarrisonCache::GarrisonSaveState::Serialize(ParamArchive& ar)
{
    PARAM_CHECK(ar.Serialize("name", name, 1, RString()))
    PARAM_CHECK(ar.Serialize("spawned", spawned, 1, false))
    // group refs resolve on the second load pass (SerializeRefs), after the
    // world's vehicle serializer has recreated the garrison units
    PARAM_CHECK(ar.SerializeRefs("Groups", groups, 1))
    return LSOK;
}

void GarrisonCache::ApplyPendingLoad()
{
    ZoneRegistry& registry = ZoneRegistry::Instance();
    SyncStates();
    for (int r = 0; r < _pending.Size(); r++)
    {
        const GarrisonSaveState& row = _pending[r];
        // rows are matched to the rebuilt zone table by NAME; rows without
        // a config zone are dropped
        int index = registry.FindZoneIndex(row.name);
        if (index < 0 || !row.spawned)
        {
            continue;
        }
        GarrisonState& state = _states[index];
        state.groups.Clear();
        for (int g = 0; g < row.groups.Size(); g++)
        {
            if (row.groups[g].GetLink())
            {
                state.groups.Add(row.groups[g]);
            }
        }
        ZoneRecord* z = registry.GetZoneMutable(index);
        if (state.groups.Size() > 0)
        {
            state.spawned = true;
            if (z)
            {
                z->liveOccupiers = (float)CountAlive(state);
            }
        }
        else
        {
            // spawned zone with no surviving group refs: the garrison was
            // wiped out before the save - reconcile to a despawned zone.  A
            // reserve stored beside it (a group-budget remainder or a top-up,
            // kept under U1) becomes a pending deposit (S5): it joins the
            // reserve on the first cache pass that finds the zone out of the
            // player's bubble or its spawn points hidden, never at his feet.
            state.spawned = false;
            if (z)
            {
                const int waiting = (int)floorf(z->garrison);
                z->garrison = 0;
                z->liveOccupiers = 0;
                Deposit(registry, index, waiting, index);
            }
        }
    }
}

LSError GarrisonCache::Serialize(ParamArchive& ar)
{
    if (ar.IsSaving())
    {
        _pending.Clear();
        const ZoneRegistry& registry = ZoneRegistry::Instance();
        for (int i = 0; i < _states.Size(); i++)
        {
            if (!_states[i].spawned)
            {
                continue; // despawned strength lives on the ZoneRegistry row
            }
            const ZoneRecord* z = registry.GetZone(i);
            if (!z)
            {
                continue;
            }
            GarrisonSaveState row;
            row.name = z->name;
            row.spawned = true;
            row.groups = _states[i].groups;
            _pending.Add(row);
        }
    }

    PARAM_CHECK(ar.Serialize("onGarrisonSpawned", _handlers[GESpawned], 1, RString()))
    PARAM_CHECK(ar.Serialize("onGarrisonDespawned", _handlers[GEDespawned], 1, RString()))
    PARAM_CHECK(ar.Serialize("Zones", _pending, 1))

    if (ar.IsSaving())
    {
        _pending.Clear();
    }
    else if (ar.GetPass() == ParamArchive::PassSecond)
    {
        // The mission config was reparsed during the load's first pass and
        // the ZoneRegistry rebuilt its tables just before this block runs
        // (its subclass precedes ours in World::Serialize), so rows can be
        // matched now; group refs were resolved by this pass's SerializeRefs.
        LoadFromConfig();
        ApplyPendingLoad();
        _pending.Clear();
    }
    return LSOK;
}

} // namespace Poseidon::Guerrilla
