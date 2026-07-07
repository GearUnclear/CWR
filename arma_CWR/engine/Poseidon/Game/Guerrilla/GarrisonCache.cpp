#include <Poseidon/Game/Guerrilla/GarrisonCache.hpp>
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>

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

// zone is garrisonable by the occupier: not a CITY, owned by the campaign's
// resolved occupier side, and that side has spawn tiers configured (the
// data-driven form of the spawning.sqs owner == "EAST" rule)
static bool IsOccupierOwned(const ZoneRegistry& registry, const ZoneRecord& z)
{
    if (stricmp(z.type, "CITY") == 0)
    {
        return false;
    }
    if (stricmp(z.owner, registry.OccupierSide()) != 0)
    {
        return false;
    }
    return registry.FactionTierClass(z.owner, 1.0f).GetLength() > 0;
}

// script-owned war level; 1 when undefined (matches init.sqs default)
static float ReadWarLevel()
{
    if (!GWorld)
    {
        return 1.0f;
    }
    GameState* gstate = GWorld->GetGameState();
    if (!gstate)
    {
        return 1.0f;
    }
    GameValue value = gstate->VarGet("gmwarlevel");
    if (value.GetType() != GameScalar)
    {
        return 1.0f;
    }
    return (float)value;
}

static Vector3 RandomNear(Vector3Par center, float radius)
{
    float x = center.X() + (GRandGen.RandomValue() * 2.0f - 1.0f) * radius;
    float z = center.Z() + (GRandGen.RandomValue() * 2.0f - 1.0f) * radius;
    float y = center.Y();
    if (GLandscape)
    {
        float dx, dz;
        y = GLOB_LAND->SurfaceYAboveWater(x, z, &dx, &dz);
    }
    return Vector3(x, y, z);
}

// mirrors CenterCreate (GameStateExtWorldConfig.cpp): the occupier may have
// no seeded units, so its command center may not exist yet
static AICenter* EnsureCenter(const char* sideName)
{
    using Poseidon::Foundation::GetEnumValue;
    TargetSide side = GetEnumValue<TargetSide>(sideName);
    if (side == INT_MIN || !GWorld)
    {
        return nullptr;
    }
    AICenter* center = GWorld->GetCenter(side);
    if (!center)
    {
        center = GWorld->CreateCenter(side);
        if (center)
        {
            GetNetworkManager().CreateObject(center);
        }
    }
    return center;
}

// mirrors GroupCreate (GameStateExtWorldConfig.cpp:693)
static AIGroup* CreateGarrisonGroup(AICenter* center)
{
    if (!center || center->NGroups() >= MaxGroups)
    {
        return nullptr;
    }
    Ref<AIGroup> group = new AIGroup();
    center->AddGroup(group);
    group->AddFirstWaypoint(VZero);

    Mission mis;
    mis._action = Mission::Arcade;
    center->SendMission(group, mis);

    GetNetworkManager().CreateObject(group);
    return group;
}

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

    RString tierClass = registry.FactionTierClass(z->owner, warLevel);
    if (tierClass.GetLength() == 0)
    {
        return; // no tier table for the owner: leave the reserve untouched
    }
    RString officerClass = registry.FactionValue(z->owner, "officer");
    if (officerClass.GetLength() == 0)
    {
        officerClass = tierClass;
    }

    AICenter* center = EnsureCenter(z->owner);
    if (!center)
    {
        return;
    }

    AutoArray<int> takes;
    int reserve = toInt(z->garrison);
    PlanGroups(reserve, _tuning.groupSize, takes);

    const float spread = registry.Tuning().zoneArea / 3.0f;
    int spawned = 0;
    int remaining = reserve;
    bool firstGroup = true;
    for (int g = 0; g < takes.Size(); g++)
    {
        AIGroup* grp = CreateGarrisonGroup(center);
        if (!grp)
        {
            // AICenter group slots exhausted: stop, the rest stays in reserve
            RptF("GarrisonCache: group budget exhausted spawning zone '%s'", (const char*)z->name);
            break;
        }
        for (int u = 0; u < takes[g]; u++)
        {
            bool officer = firstGroup && u == 0;
            Vector3 pos = RandomNear(z->pos, spread);
            int before = grp->NUnits();
            ::CreateUnit(grp, officer ? officerClass : tierClass, pos, RString(), officer ? 0.6f : 0.5f,
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
    // anything the group budget refused stays behind as reserve
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

    // authoritative survivor write-back (spawning.sqs #dsDone)
    z->garrison = (float)survivors;
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

    const float warLevel = ReadWarLevel();
    const float cacheRadius = registry.Tuning().cacheRadius;

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

        GarrisonDecisionInput in;
        in.occupierOwned = IsOccupierOwned(registry, *z);
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
    }
    // handlers run only after the cache's own state mutation completed
    DispatchEvents(fired);
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
            // wiped out before the save - reconcile to an empty zone
            state.spawned = false;
            if (z)
            {
                z->garrison = 0;
                z->liveOccupiers = 0;
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
