#include <Poseidon/Game/Guerrilla/Traffic.hpp>
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>
#include <Poseidon/Game/Guerrilla/Undercover.hpp> // ClassifyWeaponShow

#include <Poseidon/IO/ParamFileExt.hpp> // Pars / ExtParsMission
#include <Poseidon/IO/Serialization/ParamArchive.hpp>

#include <Evaluator/express.hpp>                   // GameState / GameValue (event dispatch, war level)
#include <Poseidon/Game/Commands/GameStateExt.hpp> // GameValueExt

#include <Poseidon/World/World.hpp>
#include <Poseidon/World/Terrain/Landscape.hpp> // GLOB_LAND surface Y
#include <Poseidon/World/Terrain/Roads.hpp>     // GRoadNet
#include <Poseidon/World/Entities/Infantry/Person.hpp>
#include <Poseidon/World/Entities/Infantry/SoldierOld.hpp> // Man
#include <Poseidon/World/Entities/Vehicles/Transport.hpp>
#include <Poseidon/AI/AI.hpp>
#include <Poseidon/AI/AICore.hpp>              // MaxGroups
#include <Poseidon/AI/VehicleAI.hpp>           // Rank
#include <Poseidon/AI/Path/ArcadeWaypoint.hpp> // ArcadeWaypointInfo / CombatMode / SpeedMode
#include <Poseidon/Network/Network.hpp>        // GetNetworkManager / GetInPosition

#include <Random/randomGen.hpp>

#include <Poseidon/Foundation/Common/FltOpts.hpp> // toInt / Square
#include <Poseidon/Foundation/Containers/BoolArray.hpp>
#include <Poseidon/Foundation/Framework/DebugLog.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>
#include <Poseidon/Foundation/platform.hpp>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Shared command internals (Game/Commands) - the bodies of createUnit /
// deleteVehicle / moveInXxx without the script-value parsing.  Global
// namespace, same forward-declaration idiom as GarrisonCache.cpp.
void CreateUnit(AIGroup* group, RString type, Vector3Par position, RString init, float skill, Rank rank);
void DeleteVehicle(Entity* veh);
// Game/Commands/GameStateExtUi.cpp: seat a local person into a transport
// (driver / gunner / cargo by position); false when refused
bool NativeMoveIn(Poseidon::Person* soldier, Poseidon::Transport* veh, GetInPosition position);

namespace Poseidon::Guerrilla
{

// Defined in TrafficCommands.cpp.  Referencing it from here forces the
// command TU into the link - same pattern as EnsureGarrisonCacheCommandsLinked.
void EnsureTrafficCommandsLinked();

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
Traffic& Traffic::Instance()
{
    EnsureTrafficCommandsLinked();
    static Traffic instance;
    return instance;
}
#pragma clang diagnostic pop

static float Dist2DSq(float ax, float az, float bx, float bz)
{
    float dx = ax - bx;
    float dz = az - bz;
    return dx * dx + dz * dz;
}

static float Dist2DSq(Vector3Par a, Vector3Par b)
{
    return Dist2DSq(a.X(), a.Z(), b.X(), b.Z());
}

// ---------------------------------------------------------------------------
// lifecycle / config
// ---------------------------------------------------------------------------

void Traffic::Clear()
{
    _tuning = TrafficTuning();
    _entries.Clear();
    _released.Clear();
    _fleeing.Clear();
    for (int i = 0; i < NTrafficEventTypes; i++)
    {
        _handlers[i] = RString();
    }
    _accum = 0;
    _subAccum = 0;
    _pending.Clear();
}

void Traffic::InitMission()
{
    Clear();
    LoadFromConfig();
}

void Traffic::LoadFromConfig()
{
    const ParamEntry* zones = ExtParsMission.FindEntry("CfgGuerrillaZones");
    if (!zones)
    {
        zones = Pars.FindEntry("CfgGuerrillaZones");
    }
    LoadFromParams(zones);
}

void Traffic::LoadFromParams(const ParamEntry* zonesCfg)
{
    _tuning = TrafficTuning();
    _accum = 0;
    _subAccum = 0;
    if (!zonesCfg)
    {
        return;
    }
    TrafficTuning& t = _tuning;
    t.enabled = zonesCfg->ReadValue("trafficEnabled", t.enabled ? 1.0f : 0.0f) != 0.0f;
    t.interval = zonesCfg->ReadValue("trafficInterval", t.interval);
    t.radius = zonesCfg->ReadValue("trafficRadius", t.radius);
    t.minSpawnDist = zonesCfg->ReadValue("trafficMinSpawnDist", t.minSpawnDist);
    t.despawnHysteresis = zonesCfg->ReadValue("trafficDespawnHysteresis", t.despawnHysteresis);
    t.maxCiv = toInt(zonesCfg->ReadValue("trafficMaxCiv", (float)t.maxCiv));
    t.maxPatrols = toInt(zonesCfg->ReadValue("trafficMaxPatrols", (float)t.maxPatrols));
    t.maxConvoys = toInt(zonesCfg->ReadValue("trafficMaxConvoys", (float)t.maxConvoys));
    t.civChance = zonesCfg->ReadValue("trafficCivChance", t.civChance);
    t.patrolChance = zonesCfg->ReadValue("trafficPatrolChance", t.patrolChance);
    t.convoyChance = zonesCfg->ReadValue("trafficConvoyChance", t.convoyChance);
    t.convoyWarScale = zonesCfg->ReadValue("trafficConvoyWarScale", t.convoyWarScale);
    t.stallTimeout = zonesCfg->ReadValue("trafficStallTimeout", t.stallTimeout);
    t.arriveRadius = zonesCfg->ReadValue("trafficArriveRadius", t.arriveRadius);
    t.maxLegs = toInt(zonesCfg->ReadValue("trafficMaxLegs", (float)t.maxLegs));
    t.commandeerRadius = zonesCfg->ReadValue("trafficCommandeerRadius", t.commandeerRadius);
    t.commandeerLaneHalfWidth = zonesCfg->ReadValue("trafficCommandeerLaneHalfWidth", t.commandeerLaneHalfWidth);
    t.commandeerStopDelay = zonesCfg->ReadValue("trafficCommandeerStopDelay", t.commandeerStopDelay);
    t.fleeDist = zonesCfg->ReadValue("trafficFleeDist", t.fleeDist);
    // sanity floors: a zero interval would tick every frame, negative caps
    // would read as "nothing ever spawns" (which 0 already says)
    if (t.interval < 0.5f)
    {
        t.interval = 0.5f;
    }
    if (t.maxCiv < 0)
    {
        t.maxCiv = 0;
    }
    if (t.maxPatrols < 0)
    {
        t.maxPatrols = 0;
    }
    if (t.maxConvoys < 0)
    {
        t.maxConvoys = 0;
    }
    if (t.maxLegs < 0)
    {
        t.maxLegs = 0;
    }
}

// ---------------------------------------------------------------------------
// queries
// ---------------------------------------------------------------------------

bool Traffic::IsActive() const
{
    return _tuning.enabled && ZoneRegistry::Instance().IsActive();
}

int Traffic::Count(int kind) const
{
    int n = 0;
    for (int i = 0; i < _entries.Size(); i++)
    {
        if (kind < 0 || _entries[i].kind == kind)
        {
            n++;
        }
    }
    return n;
}

Transport* Traffic::EntryVehicle(int i) const
{
    if (i < 0 || i >= _entries.Size())
    {
        return nullptr;
    }
    return _entries[i].vehicle;
}

bool Traffic::FindEntry(const Transport* veh, TrafficKind& kind, int& originIndex, int& destIndex,
                        TrafficState& state) const
{
    if (!veh)
    {
        return false;
    }
    for (int i = 0; i < _entries.Size(); i++)
    {
        const TrafficEntry& e = _entries[i];
        if (e.vehicle.GetLink() == veh || e.escort.GetLink() == veh)
        {
            kind = e.kind;
            originIndex = e.originIndex;
            destIndex = e.destIndex;
            state = e.state;
            return true;
        }
    }
    return false;
}

bool Traffic::IsTrafficGroup(const AIGroup* grp, int kind) const
{
    if (!grp)
    {
        return false;
    }
    for (int i = 0; i < _entries.Size(); i++)
    {
        const TrafficEntry& e = _entries[i];
        if (e.group.GetLink() == grp && (kind < 0 || e.kind == kind))
        {
            return true;
        }
    }
    return false;
}

const char* Traffic::KindName(int kind)
{
    switch (kind)
    {
        case TKCiv:
            return "civ";
        case TKPatrol:
            return "patrol";
        case TKConvoy:
            return "convoy";
        default:
            return "all";
    }
}

int Traffic::KindFromName(const char* name)
{
    if (!name)
    {
        return -1;
    }
    if (stricmp(name, "civ") == 0)
    {
        return TKCiv;
    }
    if (stricmp(name, "patrol") == 0)
    {
        return TKPatrol;
    }
    if (stricmp(name, "convoy") == 0)
    {
        return TKConvoy;
    }
    return -1;
}

// ---------------------------------------------------------------------------
// events
// ---------------------------------------------------------------------------

void Traffic::SetEventHandler(TrafficEventType type, RString handler)
{
    if (type < 0 || type >= NTrafficEventTypes)
    {
        return;
    }
    _handlers[type] = handler;
}

RString Traffic::GetEventHandler(TrafficEventType type) const
{
    if (type < 0 || type >= NTrafficEventTypes)
    {
        return RString();
    }
    return _handlers[type];
}

int Traffic::EventTypeFromName(const char* name)
{
    if (!name)
    {
        return -1;
    }
    if (stricmp(name, "spawned") == 0)
    {
        return TESpawned;
    }
    if (stricmp(name, "despawned") == 0)
    {
        return TEDespawned;
    }
    if (stricmp(name, "commandeered") == 0)
    {
        return TECommandeered;
    }
    if (stricmp(name, "arrived") == 0)
    {
        return TEArrived;
    }
    if (stricmp(name, "driverKilled") == 0)
    {
        return TEDriverKilled;
    }
    return -1;
}

// ---------------------------------------------------------------------------
// pure logic (unit-tested)
// ---------------------------------------------------------------------------

float Traffic::ConvoyChance(const TrafficTuning& tuning, float warLevel)
{
    float over = warLevel - 1.0f;
    if (over < 0)
    {
        over = 0;
    }
    float chance = tuning.convoyChance * (1.0f + tuning.convoyWarScale * over);
    if (chance < 0)
    {
        chance = 0;
    }
    if (chance > ConvoyChanceCap)
    {
        chance = ConvoyChanceCap;
    }
    return chance;
}

int Traffic::DecideSpawn(const TrafficDecisionInput& in, const TrafficTuning& tuning)
{
    if (!in.enabled || !tuning.enabled || !in.playerValid)
    {
        return -1;
    }
    float roll = in.roll;
    // rarest first; each eligible kind consumes its band of the roll
    if (in.hasConvoyRoute && in.liveConvoys < tuning.maxConvoys)
    {
        float chance = ConvoyChance(tuning, in.warLevel);
        if (roll < chance)
        {
            return TKConvoy;
        }
        roll -= chance;
    }
    if (in.hasPatrolRoute && in.livePatrols < tuning.maxPatrols)
    {
        if (roll < tuning.patrolChance)
        {
            return TKPatrol;
        }
        roll -= tuning.patrolChance;
    }
    if (in.hasCivRoute && in.liveCiv < tuning.maxCiv)
    {
        if (roll < tuning.civChance)
        {
            return TKCiv;
        }
    }
    return -1;
}

bool Traffic::ShouldDespawn(float playerDistSq, const TrafficTuning& tuning)
{
    float edge = tuning.radius + tuning.despawnHysteresis;
    return playerDistSq > edge * edge;
}

// origin eligibility per kind
static bool OriginEligible(int kind, const TrafficZoneCandidate& z)
{
    switch (kind)
    {
        case TKCiv:
            return z.isCity;
        case TKPatrol:
            return z.occupierOwned;
        case TKConvoy:
            return z.occupierOwned && !z.isCity;
        default:
            return false;
    }
}

static bool DestEligible(int kind, const TrafficZoneCandidate& z)
{
    switch (kind)
    {
        case TKCiv:
            return z.isCity;
        case TKPatrol:
        case TKConvoy:
            return z.occupierOwned;
        default:
            return false;
    }
}

static int PickDest(int kind, const AutoArray<TrafficZoneCandidate>& zones, int originIndex, float roll)
{
    const TrafficZoneCandidate* origin = nullptr;
    for (int i = 0; i < zones.Size(); i++)
    {
        if (zones[i].index == originIndex)
        {
            origin = &zones[i];
            break;
        }
    }
    if (!origin)
    {
        return -1;
    }
    AutoArray<int> preferred;
    AutoArray<int> fallback;
    for (int i = 0; i < zones.Size(); i++)
    {
        const TrafficZoneCandidate& z = zones[i];
        if (z.index == originIndex || !DestEligible(kind, z))
        {
            continue;
        }
        if (kind == TKCiv)
        {
            float d2 = Dist2DSq(z.x, z.z, origin->x, origin->z);
            if (d2 >= Square(Traffic::CivRouteMinDist) && d2 <= Square(Traffic::CivRouteMaxDist))
            {
                preferred.Add(z.index);
            }
            else
            {
                fallback.Add(z.index);
            }
        }
        else
        {
            preferred.Add(z.index);
        }
    }
    const AutoArray<int>& pool = preferred.Size() > 0 ? preferred : fallback;
    if (pool.Size() == 0)
    {
        return -1;
    }
    int pick = toIntFloor(roll * pool.Size());
    if (pick < 0)
    {
        pick = 0;
    }
    if (pick >= pool.Size())
    {
        pick = pool.Size() - 1;
    }
    return pool[pick];
}

bool Traffic::PickRoute(int kind, const AutoArray<TrafficZoneCandidate>& zones, float playerX, float playerZ,
                        const TrafficTuning& tuning, float roll, int& originIndex, int& destIndex, int originZone)
{
    originIndex = -1;
    destIndex = -1;
    if (kind < 0 || kind >= NTrafficKinds)
    {
        return false;
    }
    if (roll < 0)
    {
        roll = 0;
    }
    if (roll >= 1.0f)
    {
        roll = 0.999999f;
    }
    if (originZone >= 0)
    {
        // pinned origin (force spawn): only the destination is rolled
        for (int i = 0; i < zones.Size(); i++)
        {
            if (zones[i].index == originZone && OriginEligible(kind, zones[i]))
            {
                destIndex = PickDest(kind, zones, originZone, roll);
                if (destIndex >= 0)
                {
                    originIndex = originZone;
                    return true;
                }
            }
        }
        return false;
    }

    // origins: eligible zones within the player radius, each of which must
    // have at least one destination
    AutoArray<int> origins;
    float r2 = Square(tuning.radius);
    for (int i = 0; i < zones.Size(); i++)
    {
        const TrafficZoneCandidate& z = zones[i];
        if (!OriginEligible(kind, z))
        {
            continue;
        }
        if (Dist2DSq(z.x, z.z, playerX, playerZ) > r2)
        {
            continue;
        }
        if (PickDest(kind, zones, z.index, 0.0f) < 0)
        {
            continue;
        }
        origins.Add(z.index);
    }
    if (origins.Size() == 0)
    {
        return false;
    }
    float scaled = roll * origins.Size();
    int pick = toIntFloor(scaled);
    if (pick >= origins.Size())
    {
        pick = origins.Size() - 1;
    }
    float destRoll = scaled - (float)pick; // the fractional part re-rolls the destination
    originIndex = origins[pick];
    destIndex = PickDest(kind, zones, originIndex, destRoll);
    return destIndex >= 0;
}

int Traffic::SelectSpawnPoint(const AutoArray<Vector3>& roadPts, Vector3Par playerPos, const TrafficTuning& tuning)
{
    int best = -1;
    float bestD2 = -1;
    float minD2 = Square(tuning.minSpawnDist);
    float maxD2 = Square(tuning.radius);
    for (int i = 0; i < roadPts.Size(); i++)
    {
        float d2 = Dist2DSq(roadPts[i], playerPos);
        if (d2 < minD2 || d2 > maxD2)
        {
            continue;
        }
        if (d2 > bestD2)
        {
            bestD2 = d2;
            best = i;
        }
    }
    return best;
}

bool Traffic::CommandeerTriggered(const CommandeerObs& obs, const TrafficTuning& tuning)
{
    float dx = obs.playerPos.X() - obs.carPos.X();
    float dz = obs.playerPos.Z() - obs.carPos.Z();
    float d2 = dx * dx + dz * dz;
    if (d2 > Square(tuning.commandeerRadius))
    {
        return false;
    }
    if (d2 < 1e-4f)
    {
        return true; // standing on the bonnet
    }
    float d = sqrtf(d2);
    float tx = dx / d; // car -> player, unit 2D
    float tz = dz / d;

    // car heading, unit 2D
    float cx = obs.carDir.X();
    float cz = obs.carDir.Z();
    float cl = sqrtf(cx * cx + cz * cz);
    if (cl > 1e-4f)
    {
        cx /= cl;
        cz /= cl;
        float ahead = cx * tx + cz * tz;
        const float cos20 = 0.9397f;
        if (ahead > cos20)
        {
            // lateral offset from the car's line of travel
            float lateral = fabsf(cx * dz - cz * dx);
            if (lateral <= tuning.commandeerLaneHalfWidth)
            {
                return true;
            }
        }
    }

    if (obs.weaponInHands)
    {
        float px = obs.playerDir.X();
        float pz = obs.playerDir.Z();
        float pl = sqrtf(px * px + pz * pz);
        if (pl > 1e-4f)
        {
            px /= pl;
            pz /= pl;
            // player -> car is -t
            float aim = -(px * tx + pz * tz);
            const float cos15 = 0.9659f;
            if (aim > cos15)
            {
                return true;
            }
        }
    }
    return false;
}

bool Traffic::StallExpired(float stalledSeconds, const TrafficTuning& tuning)
{
    return tuning.stallTimeout > 0 && stalledSeconds >= tuning.stallTimeout;
}

// ---------------------------------------------------------------------------
// world-touching internals (engine path only)
// ---------------------------------------------------------------------------

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

void Traffic::BuildZoneCandidates(AutoArray<TrafficZoneCandidate>& out) const
{
    out.Clear();
    const ZoneRegistry& registry = ZoneRegistry::Instance();
    RString occ = registry.OccupierSide();
    for (int i = 0; i < registry.NZones(); i++)
    {
        const ZoneRecord* z = registry.GetZone(i);
        if (!z)
        {
            continue;
        }
        TrafficZoneCandidate c;
        c.index = i;
        c.x = z->pos.X();
        c.z = z->pos.Z();
        c.isCity = stricmp(z->type, "CITY") == 0;
        c.occupierOwned = stricmp(z->owner, occ) == 0;
        out.Add(c);
    }
}

int Traffic::DestForOrigin(int kind, const AutoArray<TrafficZoneCandidate>& zones, int originIndex, float roll) const
{
    return PickDest(kind, zones, originIndex, roll);
}

// every unlocked road link centre within SpawnScanRadius of center, with
// the link's travel direction (unit, either sign)
bool Traffic::CollectRoadSpots(Vector3Par center, AutoArray<Vector3>& pts, AutoArray<Vector3>& dirs) const
{
    pts.Clear();
    dirs.Clear();
    if (!GRoadNet || !GLandscape)
    {
        return false;
    }
    const float r2 = Square(SpawnScanRadius);
    int cells = toIntCeil(SpawnScanRadius * InvLandGrid);
    int cx = toIntFloor(center.X() * InvLandGrid);
    int cz = toIntFloor(center.Z() * InvLandGrid);
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
                if (!link || link->IsLocked())
                {
                    continue;
                }
                Vector3 c = link->GetCenter();
                if (Dist2DSq(c, center) > r2)
                {
                    continue;
                }
                Vector3 dir = VForward;
                if (link->NConnections() >= 2)
                {
                    const Vector3* con = link->PosConnections();
                    Vector3 d = con[1] - con[0];
                    d[1] = 0;
                    if (d.SquareSize() > 1e-4f)
                    {
                        dir = d.Normalized();
                    }
                }
                pts.Add(c);
                dirs.Add(dir);
            }
        }
    }
    return pts.Size() > 0;
}

// mirrors VehCreate (GameStateExtWorld.cpp) minus the script-value parsing;
// the hull is oriented along dir and dropped on the road surface
Transport* Traffic::CreateTrafficVehicle(RString type, Vector3Par where, Vector3Par dir) const
{
    if (!GWorld || !GLandscape)
    {
        return nullptr;
    }
    Ref<Entity> veh = NewNonAIVehicle(type, nullptr);
    if (!veh)
    {
        return nullptr;
    }
    Transport* transport = dyn_cast<Transport>(veh.GetRef());
    if (!transport)
    {
        LOG_WARN(Core, "Traffic: class '{}' is not a Transport - skipped", (const char*)type);
        return nullptr;
    }
    if (veh->GetNonAIType()->IsKindOf(GWorld->Preloaded(VTypeStatic)))
    {
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

// the shared Guerrilla group helper (ZoneRegistry.hpp) on the side's center,
// created on demand - mirrors GroupCreate (GameStateExtWorldConfig.cpp:693)
AIGroup* Traffic::CreateTrafficGroup(const char* sideName) const
{
    return CreateSideGroup(EnsureSideCenter(sideName));
}

// create one crewman in grp next to the hull and seat him; null when the
// class refused to materialize or the seat was refused
Person* Traffic::CreateCrewman(AIGroup* grp, RString type, Vector3Par near, Transport* veh, int position) const
{
    if (!grp || type.GetLength() == 0)
    {
        return nullptr;
    }
    // snapshot so the new unit can be told apart
    AIUnit* before[MAX_UNITS_PER_GROUP];
    for (int i = 0; i < MAX_UNITS_PER_GROUP; i++)
    {
        before[i] = grp->UnitWithID(i + 1);
    }
    ::CreateUnit(grp, type, near, RString(), 0.5f, RankPrivate);
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
    if (!unit)
    {
        return nullptr;
    }
    Person* person = unit->GetPerson();
    if (!person)
    {
        return nullptr;
    }
    if (veh)
    {
        if (!::NativeMoveIn(person, veh, (GetInPosition)position))
        {
            // seat refused: the body stays on foot next to the hull (the
            // caller decides whether that is acceptable)
            return person;
        }
    }
    return person;
}

// the one route leg.  Behaviour/speed/formation are set on the group, then
// a Command::Move goes to the DRIVERS through IssueCommand - the doMove idiom
// (GameStateExtGrp.cpp VehMove, silent=true).  NOT an arcade waypoint and NOT
// AIGroup::Move: both deliver through SendCommand (the radio channel), and
// that never reached a seated crew in probes (the car sat with unitReady
// false and speed 0 forever), while the direct issue drove at once.  CARELESS
// / SAFE keep the unit path planner on the road net (AIUnit::CreatePath ->
// CreateRoadPath when !IsCautious); gunners and cargo stay seated because
// only the drivers are addressed.
void Traffic::IssueRoute(TrafficEntry& e, Vector3Par dest, int combatMode, int speedMode, bool column)
{
    AIGroup* grp = e.group;
    if (!grp)
    {
        return;
    }
    grp->SetCombatModeMajor((CombatMode)combatMode);
    if (grp->MainSubgroup())
    {
        grp->MainSubgroup()->SetSpeedMode((SpeedMode)speedMode);
        if (column)
        {
            grp->MainSubgroup()->SetFormation(AI::FormColumn);
        }
    }
    Command cmd;
    cmd._message = Command::Move;
    cmd._destination = dest;
    cmd._discretion = Command::Undefined;
    cmd._context = Command::CtxMission;
    cmd._id = grp->GetNextCmdId();
    PackedBoolArray drivers;
    for (int u = 0; u < MAX_UNITS_PER_GROUP; u++)
    {
        AIUnit* unit = grp->UnitWithID(u + 1);
        if (!unit)
        {
            continue;
        }
        Transport* in = unit->GetVehicleIn();
        if (in && in->DriverBrain() == unit)
        {
            drivers.Set(u, true);
        }
    }
    grp->IssueCommand(cmd, drivers);
    GetNetworkManager().UpdateObject(grp);
    e.dest = dest;
}

// extra crew (gunner / cargo / escort seats): a refused seat leaves a body on
// foot next to the hull that no route ever addresses - delete it instead
static void SeatOrDelete(Person* person, Transport* veh)
{
    if (!person)
    {
        return;
    }
    AIUnit* unit = person->Brain();
    if (!unit || unit->GetVehicleIn() != veh)
    {
        ::DeleteVehicle(person);
    }
}

bool Traffic::SpawnEntry(int kind, int originIndex, int destIndex, Vector3Par playerPos, Transport*& outVeh,
                         AutoArray<TrafficEventRecord>& fired)
{
    outVeh = nullptr;
    ZoneRegistry& registry = ZoneRegistry::Instance();
    const ZoneRecord* origin = registry.GetZone(originIndex);
    const ZoneRecord* dest = registry.GetZone(destIndex);
    if (!origin || !dest || !GRoadNet)
    {
        return false;
    }

    // road placement near the origin, inside the player band
    AutoArray<Vector3> pts;
    AutoArray<Vector3> dirs;
    CollectRoadSpots(origin->pos, pts, dirs);
    int spot = SelectSpawnPoint(pts, playerPos, _tuning);
    if (spot < 0)
    {
        return false;
    }
    Vector3 spawnPos = pts[spot];
    Vector3 destPt = GRoadNet->GetNearestRoadPoint(dest->pos);
    // face the way that leads toward the destination
    Vector3 heading = dirs[spot];
    Vector3 toDest = destPt - spawnPos;
    toDest[1] = 0;
    if (heading.DotProduct(toDest) < 0)
    {
        heading = -heading;
    }

    // classes ---------------------------------------------------------------
    RString hullType;
    RString escortType;
    RString crewType;
    const char* sideName = nullptr;
    const float warLevel = ReadWarLevel();
    RString occ = registry.OccupierSide();
    if (kind == TKCiv)
    {
        AutoArray<RString> civVehicles;
        registry.FactionCivVehicles("CIV", civVehicles);
        if (civVehicles.Size() == 0)
        {
            return false; // no civilian hulls in this data package: civ traffic inert
        }
        int pick = toIntFloor(GRandGen.RandomValue() * civVehicles.Size());
        if (pick >= civVehicles.Size())
        {
            pick = civVehicles.Size() - 1;
        }
        hullType = civVehicles[pick];
        int nClasses = atoi((const char*)registry.FactionValue("CIV", "civClassCount"));
        if (nClasses > 0)
        {
            int c = toIntFloor(GRandGen.RandomValue() * nClasses) + 1;
            if (c > nClasses)
            {
                c = nClasses;
            }
            char key[32];
            snprintf(key, sizeof(key), "civClass%d", c);
            crewType = registry.FactionValue("CIV", key);
        }
        if (crewType.GetLength() == 0)
        {
            return false; // no civ driver class
        }
        sideName = "CIV";
    }
    else
    {
        const FactionRecord* f = registry.FindFactionForSide(occ);
        if (!f || f->vehicles.Size() == 0)
        {
            return false;
        }
        crewType = registry.FactionTierClass(occ, warLevel);
        if (crewType.GetLength() == 0)
        {
            return false;
        }
        if (kind == TKPatrol)
        {
            hullType = f->vehicles[0]; // the light rung, war-level independent
        }
        else
        {
            if (f->vehicles.Size() < 2)
            {
                return false; // convoy needs a truck rung and an escort rung
            }
            hullType = f->vehicles[1];
            escortType = f->vehicles[0];
        }
        sideName = occ;
    }

    // group + hull(s) + crew ------------------------------------------------
    AIGroup* grp = CreateTrafficGroup(sideName);
    if (!grp)
    {
        LOG_WARN(Core, "Traffic: group budget exhausted spawning {} traffic", KindName(kind));
        return false;
    }
    Transport* veh = CreateTrafficVehicle(hullType, spawnPos, heading);
    if (!veh)
    {
        grp->RemoveFromCenter();
        LOG_WARN(Core, "Traffic: could not create hull '{}' for {} traffic", (const char*)hullType, KindName(kind));
        return false;
    }
    Vector3 footPos = spawnPos + heading * 4.0f;
    Person* driver = CreateCrewman(grp, crewType, footPos, veh, GIPDriver);
    if (!driver || veh->Driver() != driver)
    {
        // no driver = no traffic; tear the hull down again
        DeleteCrew(grp);
        grp->RemoveFromCenter();
        ::DeleteVehicle(veh);
        LOG_WARN(Core, "Traffic: could not seat a '{}' driver into '{}'", (const char*)crewType, (const char*)hullType);
        return false;
    }
    Transport* escort = nullptr;
    if (kind == TKCiv)
    {
        // the kill ledger: the same expression civilians.sqs attaches to town
        // civilians, so a road murder writes the same [victim, killer, pos,
        // zoneIdx] tuple
        if (_handlers[TEDriverKilled].GetLength() > 0)
        {
            driver->AddEventHandler(EEKilled, _handlers[TEDriverKilled]);
        }
    }
    else
    {
        // minimal military crew: gunner when the hull has a turret, one
        // passenger when it has cargo
        if (veh->Type()->HasGunner())
        {
            SeatOrDelete(CreateCrewman(grp, crewType, footPos, veh, GIPGunner), veh);
        }
        if (veh->Type()->HasCargo())
        {
            SeatOrDelete(CreateCrewman(grp, crewType, footPos, veh, GIPCargo), veh);
        }
        if (kind == TKConvoy)
        {
            // escort behind the truck, its own driver + gunner, same group
            Vector3 escortPos = spawnPos - heading * 14.0f;
            escort = CreateTrafficVehicle(escortType, escortPos, heading);
            if (escort)
            {
                SeatOrDelete(CreateCrewman(grp, crewType, escortPos + heading * 4.0f, escort, GIPDriver), escort);
                if (escort->Type()->HasGunner())
                {
                    SeatOrDelete(CreateCrewman(grp, crewType, escortPos + heading * 4.0f, escort, GIPGunner), escort);
                }
                if (!escort->Driver())
                {
                    ::DeleteVehicle(escort);
                    escort = nullptr;
                }
            }
        }
    }

    TrafficEntry e;
    e.kind = (TrafficKind)kind;
    e.state = TSDriving;
    e.vehicle = veh;
    e.escort = escort;
    e.group = grp;
    e.driver = driver;
    e.originZone = origin->name;
    e.destZone = dest->name;
    e.originIndex = originIndex;
    e.destIndex = destIndex;
    e.legs = 1;
    e.lastPos = veh->Position();
    int combat = kind == TKCiv ? CMCareless : CMSafe;
    int speed = kind == TKPatrol ? SpeedNormal : SpeedLimited;
    IssueRoute(e, destPt, combat, speed, kind == TKConvoy);
    _entries.Add(e);
    outVeh = veh;

    TrafficEventRecord ev;
    ev.type = TESpawned;
    ev.kind = kind;
    ev.originIndex = originIndex;
    ev.destIndex = destIndex;
    ev.vehicle = veh;
    ev.driver = driver;
    fired.Add(ev);
    LOG_INFO(Core, "Traffic: spawned {} '{}' {} -> {}", KindName(kind), (const char*)hullType,
             (const char*)origin->name, (const char*)dest->name);
    return true;
}

// delete every body of a traffic group (mirrors GarrisonCache::DespawnGarrison)
void Traffic::DeleteCrew(AIGroup* grp) const
{
    if (!grp)
    {
        return;
    }
    AutoArray<Person*> bodies;
    for (int u = 0; u < MAX_UNITS_PER_GROUP; u++)
    {
        AIUnit* unit = grp->UnitWithID(u + 1);
        if (!unit)
        {
            continue;
        }
        Person* person = unit->GetPerson();
        if (person)
        {
            bodies.Add(person);
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
}

void Traffic::DespawnEntry(int index, const char* reason, bool keepHull, AutoArray<TrafficEventRecord>& fired)
{
    if (index < 0 || index >= _entries.Size())
    {
        return;
    }
    TrafficEntry e = _entries[index];
    _entries.Delete(index);

    AIGroup* grp = e.group;
    Transport* veh = e.vehicle;
    Transport* escort = e.escort;
    if (keepHull)
    {
        // the hull (and whatever is left of the crew) outlives the entry;
        // cleanup deletes it once the player is far, unless somebody boarded
        ReleasedEntry r;
        r.vehicle = veh;
        r.group = grp;
        if (grp)
        {
            for (int u = 0; u < MAX_UNITS_PER_GROUP; u++)
            {
                AIUnit* unit = grp->UnitWithID(u + 1);
                if (unit && unit->GetPerson())
                {
                    r.bodies.Add(unit->GetPerson());
                }
            }
        }
        _released.Add(r);
        if (escort)
        {
            ReleasedEntry r2;
            r2.vehicle = escort;
            _released.Add(r2);
        }
    }
    else
    {
        DeleteCrew(grp);
        if (grp && grp->NUnits() == 0)
        {
            grp->RemoveFromCenter();
        }
        if (veh)
        {
            ::DeleteVehicle(veh);
        }
        if (escort)
        {
            ::DeleteVehicle(escort);
        }
    }

    TrafficEventRecord ev;
    ev.type = TEDespawned;
    ev.kind = e.kind;
    ev.originIndex = e.originIndex;
    ev.destIndex = e.destIndex;
    ev.reason = reason;
    fired.Add(ev);
    LOG_INFO(Core, "Traffic: despawned {} ({})", KindName(e.kind), reason);
}

static bool DriverAlive(const Traffic* /*self*/, Transport* veh)
{
    if (!veh)
    {
        return false;
    }
    Person* d = veh->Driver();
    return d && !d->IsDammageDestroyed();
}

void Traffic::UpdateEntries(Vector3Par playerPos, bool playerValid, AutoArray<TrafficEventRecord>& fired)
{
    AutoArray<TrafficZoneCandidate> zones;
    for (int i = _entries.Size() - 1; i >= 0; i--)
    {
        TrafficEntry& e = _entries[i];
        Transport* veh = e.vehicle;
        if (!veh)
        {
            DespawnEntry(i, "vanished", false, fired);
            continue;
        }
        if (veh->IsDammageDestroyed())
        {
            DespawnEntry(i, "destroyed", true, fired); // wreck stays until the player is far
            continue;
        }
        if (e.state == TSStopping || e.state == TSExiting)
        {
            // the commandeer sub-tick owns this entry while the player is
            // near; if the player left before the driver bailed, the stop
            // is called off and the car drives on (the sub-tick only runs
            // inside CommandeerWatchRadius, so without this the car would
            // sit at its Stop forever)
            if (playerValid && Dist2DSq(veh->Position(), playerPos) > Square(CommandeerWatchRadius))
            {
                e.state = TSDriving;
                e.stateTime = 0;
                IssueRoute(e, e.dest, CMCareless, SpeedLimited, false);
            }
            continue;
        }
        if (!DriverAlive(this, veh))
        {
            // road murder (or a crash): the hull is the player's for the
            // taking; bodies stay until the player is far
            DespawnEntry(i, "crewDead", true, fired);
            continue;
        }
        if (!playerValid)
        {
            continue;
        }
        float playerD2 = Dist2DSq(veh->Position(), playerPos);
        if (ShouldDespawn(playerD2, _tuning))
        {
            DespawnEntry(i, "far", false, fired);
            continue;
        }
        bool playerNear = playerD2 <= Square(_tuning.minSpawnDist);

        // stall bookkeeping (never teleports)
        float moved2 = Dist2DSq(veh->Position(), e.lastPos);
        e.lastPos = veh->Position();
        if (moved2 < Square(StallMoveEpsilon))
        {
            e.stallTime += _tuning.interval;
        }
        else
        {
            e.stallTime = 0;
            if (e.state == TSStalled)
            {
                e.state = TSDriving;
            }
        }

        float destD2 = Dist2DSq(veh->Position(), e.dest);
        if (destD2 <= Square(_tuning.arriveRadius))
        {
            if (e.state != TSArrived)
            {
                e.state = TSArrived;
                TrafficEventRecord ev;
                ev.type = TEArrived;
                ev.kind = e.kind;
                ev.originIndex = e.originIndex;
                ev.destIndex = e.destIndex;
                ev.vehicle = veh;
                fired.Add(ev);
            }
            if (!playerNear)
            {
                DespawnEntry(i, "arrived", false, fired);
                continue;
            }
            if (e.legs < _tuning.maxLegs)
            {
                // the player is watching: drive on to another zone
                if (zones.Size() == 0)
                {
                    BuildZoneCandidates(zones);
                }
                int next = DestForOrigin(e.kind, zones, e.destIndex, GRandGen.RandomValue());
                const ZoneRecord* z = next >= 0 ? ZoneRegistry::Instance().GetZone(next) : nullptr;
                if (z && GRoadNet)
                {
                    e.originIndex = e.destIndex;
                    e.originZone = e.destZone;
                    e.destIndex = next;
                    e.destZone = z->name;
                    e.legs++;
                    e.state = TSDriving;
                    e.stallTime = 0;
                    int combat = e.kind == TKCiv ? CMCareless : CMSafe;
                    int speed = e.kind == TKPatrol ? SpeedNormal : SpeedLimited;
                    IssueRoute(e, GRoadNet->GetNearestRoadPoint(z->pos), combat, speed, e.kind == TKConvoy);
                }
            }
            continue;
        }

        if (StallExpired(e.stallTime, _tuning))
        {
            e.state = TSStalled;
            if (!playerNear)
            {
                DespawnEntry(i, "stalled", false, fired);
                continue;
            }
        }
    }
}

// the commandeer sub-tick: trigger -> Stop -> (delay) -> driver bails and
// flees -> hull released
void Traffic::UpdateCommandeer(float dt)
{
    if (!GWorld)
    {
        return;
    }
    Person* player = GWorld->GetRealPlayer();
    if (!player || player->IsDammageDestroyed())
    {
        return;
    }
    const Man* man = dyn_cast<Man>(player);
    bool armed = man && ClassifyWeaponShow(*man) == UCWInHands;
    Vector3 playerPos = player->Position();
    Vector3 playerDir = player->Direction();

    AutoArray<TrafficEventRecord> fired;
    for (int i = _entries.Size() - 1; i >= 0; i--)
    {
        TrafficEntry& e = _entries[i];
        if (e.kind != TKCiv)
        {
            continue;
        }
        Transport* veh = e.vehicle;
        AIGroup* grp = e.group;
        Person* driver = e.driver;
        if (!veh || !grp || !driver || driver->IsDammageDestroyed() || veh->Driver() != driver)
        {
            continue; // the main tick reconciles dead/vanished entries
        }
        if (e.state == TSDriving || e.state == TSArrived || e.state == TSStalled)
        {
            CommandeerObs obs;
            obs.carPos = veh->Position();
            obs.carDir = veh->Direction();
            obs.playerPos = playerPos;
            obs.playerDir = playerDir;
            obs.weaponInHands = armed;
            if (!CommandeerTriggered(obs, _tuning))
            {
                continue;
            }
            // Stop to the whole crew (issued like VehStop silent, GameStateExtGrp.cpp)
            Command cmd;
            cmd._message = Command::Stop;
            cmd._discretion = Command::Undefined;
            cmd._context = Command::CtxMission;
            cmd._id = grp->GetNextCmdId();
            PackedBoolArray all;
            for (int u = 0; u < MAX_UNITS_PER_GROUP; u++)
            {
                if (grp->UnitWithID(u + 1))
                {
                    all.Set(u, true);
                }
            }
            grp->IssueCommand(cmd, all); // the doStop idiom (direct, not radio)
            if (grp->MainSubgroup())
            {
                grp->MainSubgroup()->SetSpeedMode(SpeedLimited);
            }
            e.state = TSStopping;
            e.stateTime = 0;
            continue;
        }
        if (e.state == TSStopping)
        {
            e.stateTime += dt;
            if (e.stateTime < _tuning.commandeerStopDelay)
            {
                continue;
            }
            // bail: the crew stops wanting the car, the driver steps out and
            // runs from the player at full speed
            AIUnit* unit = driver->Brain();
            grp->UnassignVehicle(veh);
            if (unit)
            {
                unit->AllowGetIn(false);
                unit->DoGetOut(veh, false);
            }
            Vector3 away = driver->Position() - playerPos;
            away[1] = 0;
            if (away.SquareSize() < 1e-2f)
            {
                away = -veh->Direction();
                away[1] = 0;
            }
            Vector3 flee = driver->Position() + away.Normalized() * _tuning.fleeDist;
            if (GLandscape)
            {
                float dx, dz;
                flee[1] = GLOB_LAND->SurfaceYAboveWater(flee[0], flee[2], &dx, &dz);
            }
            if (unit)
            {
                Command mv;
                mv._message = Command::Move;
                mv._destination = flee;
                mv._discretion = Command::Undefined;
                mv._context = Command::CtxMission;
                mv._id = grp->GetNextCmdId();
                PackedBoolArray one;
                one.Set(unit->ID() - 1, true);
                grp->IssueCommand(mv, one); // doMove idiom
            }
            grp->SetCombatModeMajor(CMCareless);
            if (grp->MainSubgroup())
            {
                grp->MainSubgroup()->SetSpeedMode(SpeedFull);
            }
            GetNetworkManager().UpdateObject(grp);

            FleeingDriver fd;
            fd.person = driver;
            fd.group = grp;
            fd.age = 0;
            _fleeing.Add(fd);

            TrafficEventRecord ev;
            ev.type = TECommandeered;
            ev.kind = e.kind;
            ev.originIndex = e.originIndex;
            ev.destIndex = e.destIndex;
            ev.vehicle = veh;
            ev.driver = driver;
            fired.Add(ev);

            // registry half: the hull is released (no bodies - the driver is
            // alive and tracked by _fleeing)
            ReleasedEntry r;
            r.vehicle = veh;
            _released.Add(r);
            _entries.Delete(i);
            LOG_INFO(Core, "Traffic: civ car commandeered");
        }
    }
    DispatchEvents(fired);
}

bool Traffic::Release(Transport* veh)
{
    if (!veh)
    {
        return false;
    }
    for (int i = 0; i < _entries.Size(); i++)
    {
        if (_entries[i].vehicle.GetLink() == veh)
        {
            ReleasedEntry r;
            r.vehicle = veh;
            _released.Add(r);
            if (_entries[i].escort.GetLink())
            {
                ReleasedEntry r2;
                r2.vehicle = _entries[i].escort;
                _released.Add(r2);
            }
            _entries.Delete(i);
            return true;
        }
    }
    return false;
}

void Traffic::CleanupReleased(Vector3Par playerPos, bool playerValid)
{
    for (int i = _released.Size() - 1; i >= 0; i--)
    {
        ReleasedEntry& r = _released[i];
        Transport* veh = r.vehicle;
        if (!veh)
        {
            // hull gone (deleted by script / the world): drop the bodies too
            for (int b = 0; b < r.bodies.Size(); b++)
            {
                if (r.bodies[b].GetLink())
                {
                    ::DeleteVehicle(r.bodies[b]);
                }
            }
            AIGroup* grp = r.group;
            if (grp && grp->NUnits() == 0)
            {
                grp->RemoveFromCenter();
            }
            _released.Delete(i);
            continue;
        }
        if (!r.boarded && !veh->IsDammageDestroyed())
        {
            if (veh->Driver() || veh->Gunner() || veh->Commander() || veh->GetManCargoSize() > 0)
            {
                r.boarded = true;
            }
        }
        if (r.boarded)
        {
            // somebody took it: an ordinary world object from now on (the
            // persistent garage is cache-and-garage, #28)
            _released.Delete(i);
            continue;
        }
        if (!playerValid)
        {
            continue;
        }
        if (ShouldDespawn(Dist2DSq(veh->Position(), playerPos), _tuning))
        {
            for (int b = 0; b < r.bodies.Size(); b++)
            {
                if (r.bodies[b].GetLink())
                {
                    ::DeleteVehicle(r.bodies[b]);
                }
            }
            AIGroup* grp = r.group;
            if (grp && grp->NUnits() == 0)
            {
                grp->RemoveFromCenter();
            }
            ::DeleteVehicle(veh);
            _released.Delete(i);
        }
    }
}

void Traffic::CleanupFleeing(Vector3Par playerPos, bool playerValid, float dt)
{
    for (int i = _fleeing.Size() - 1; i >= 0; i--)
    {
        FleeingDriver& f = _fleeing[i];
        Person* p = f.person;
        f.age += dt;
        if (!p)
        {
            _fleeing.Delete(i);
            continue;
        }
        if (p->IsDammageDestroyed())
        {
            // murdered on the run: the body stays (the killed EH already
            // wrote the ledger tuple); forget him
            _fleeing.Delete(i);
            continue;
        }
        bool far = playerValid && Dist2DSq(p->Position(), playerPos) > Square(FleeDeleteDist);
        if (far || f.age >= FleeDeleteAfter)
        {
            AIGroup* grp = f.group;
            ::DeleteVehicle(p);
            if (grp && grp->NUnits() == 0)
            {
                grp->RemoveFromCenter();
            }
            _fleeing.Delete(i);
        }
    }
}

// ---------------------------------------------------------------------------
// simulation
// ---------------------------------------------------------------------------

void Traffic::Simulate(float deltaT)
{
    if (!IsActive() || !GWorld)
    {
        return;
    }

    // commandeer sub-tick: only while a civ car is within watch radius
    _subAccum += deltaT;
    if (_subAccum >= CommandeerSubTick)
    {
        float sub = _subAccum;
        _subAccum = 0;
        Person* player = GWorld->GetRealPlayer();
        if (player && !player->IsDammageDestroyed())
        {
            bool near = false;
            for (int i = 0; i < _entries.Size() && !near; i++)
            {
                const TrafficEntry& e = _entries[i];
                Transport* veh = e.vehicle;
                if (e.kind == TKCiv && veh &&
                    Dist2DSq(veh->Position(), player->Position()) <= Square(CommandeerWatchRadius))
                {
                    near = true;
                }
            }
            if (near)
            {
                UpdateCommandeer(sub);
            }
        }
    }

    _accum += deltaT;
    if (_accum < _tuning.interval)
    {
        return;
    }
    float tick = _accum;
    _accum = 0;

    bool playerValid = false;
    Vector3 playerPos = VZero;
    Person* player = GWorld->GetRealPlayer();
    if (player && !player->IsDammageDestroyed())
    {
        playerValid = true;
        playerPos = player->Position();
    }

    AutoArray<TrafficEventRecord> fired;
    UpdateEntries(playerPos, playerValid, fired);
    CleanupReleased(playerPos, playerValid);
    CleanupFleeing(playerPos, playerValid, tick);

    if (playerValid)
    {
        AutoArray<TrafficZoneCandidate> zones;
        BuildZoneCandidates(zones);
        int o, d;
        TrafficDecisionInput in;
        in.enabled = _tuning.enabled;
        in.playerValid = true;
        in.liveCiv = Count(TKCiv);
        in.livePatrols = Count(TKPatrol);
        in.liveConvoys = Count(TKConvoy);
        in.hasCivRoute = PickRoute(TKCiv, zones, playerPos.X(), playerPos.Z(), _tuning, 0.0f, o, d);
        in.hasPatrolRoute = PickRoute(TKPatrol, zones, playerPos.X(), playerPos.Z(), _tuning, 0.0f, o, d);
        in.hasConvoyRoute = PickRoute(TKConvoy, zones, playerPos.X(), playerPos.Z(), _tuning, 0.0f, o, d);
        in.warLevel = ReadWarLevel();
        in.roll = GRandGen.RandomValue();
        int kind = DecideSpawn(in, _tuning);
        if (kind >= 0 && PickRoute(kind, zones, playerPos.X(), playerPos.Z(), _tuning, GRandGen.RandomValue(), o, d))
        {
            Transport* veh = nullptr;
            SpawnEntry(kind, o, d, playerPos, veh, fired);
        }
    }
    // handlers run only after the service's own state mutation completed
    DispatchEvents(fired);
}

Transport* Traffic::ForceSpawn(int kind, int zoneIndex)
{
    if (!GWorld || kind < 0 || kind >= NTrafficKinds || !ZoneRegistry::Instance().IsActive())
    {
        return nullptr;
    }
    Person* player = GWorld->GetRealPlayer();
    Vector3 playerPos = player ? player->Position() : VZero;
    AutoArray<TrafficZoneCandidate> zones;
    BuildZoneCandidates(zones);
    int o, d;
    if (!PickRoute(kind, zones, playerPos.X(), playerPos.Z(), _tuning, GRandGen.RandomValue(), o, d, zoneIndex))
    {
        return nullptr;
    }
    AutoArray<TrafficEventRecord> fired;
    Transport* veh = nullptr;
    SpawnEntry(kind, o, d, playerPos, veh, fired);
    DispatchEvents(fired);
    return veh;
}

void Traffic::MarkEntryForTest(TrafficKind kind, const char* originZone, const char* destZone, int legs)
{
    TrafficEntry e;
    e.kind = kind;
    e.originZone = originZone;
    e.destZone = destZone;
    e.legs = legs;
    ZoneRegistry& registry = ZoneRegistry::Instance();
    e.originIndex = registry.FindZoneIndex(originZone);
    e.destIndex = registry.FindZoneIndex(destZone);
    _entries.Add(e);
}

void Traffic::DispatchEvents(const AutoArray<TrafficEventRecord>& fired)
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
    for (int i = 0; i < fired.Size(); i++)
    {
        const TrafficEventRecord& ev = fired[i];
        RString handler = GetEventHandler(ev.type);
        if (handler.GetLength() == 0)
        {
            continue;
        }
        GameArrayType pars;
        switch (ev.type)
        {
            case TESpawned:
                pars.Resize(4);
                pars[0] = GameValueExt(ev.vehicle.GetLink());
                pars[1] = GameStringType(KindName(ev.kind));
                pars[2] = (float)ev.originIndex;
                pars[3] = (float)ev.destIndex;
                break;
            case TEDespawned:
                pars.Resize(2);
                pars[0] = GameStringType(KindName(ev.kind));
                pars[1] = GameStringType(ev.reason);
                break;
            case TECommandeered:
                pars.Resize(2);
                pars[0] = GameValueExt(ev.vehicle.GetLink());
                pars[1] = GameValueExt(ev.driver.GetLink());
                break;
            case TEArrived:
                pars.Resize(3);
                pars[0] = GameValueExt(ev.vehicle.GetLink());
                pars[1] = GameStringType(KindName(ev.kind));
                pars[2] = (float)ev.destIndex;
                break;
            default:
                continue; // driverKilled is an entity EH, never dispatched here
        }
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

LSError Traffic::TrafficEntry::Serialize(ParamArchive& ar)
{
    int k = (int)kind;
    int s = (int)state;
    PARAM_CHECK(ar.Serialize("kind", k, 1, 0))
    PARAM_CHECK(ar.Serialize("state", s, 1, 0))
    kind = (TrafficKind)k;
    state = (TrafficState)s;
    PARAM_CHECK(ar.Serialize("originZone", originZone, 1, RString()))
    PARAM_CHECK(ar.Serialize("destZone", destZone, 1, RString()))
    PARAM_CHECK(ar.Serialize("dest", dest, 1, VZero))
    PARAM_CHECK(ar.Serialize("legs", legs, 1, 0))
    PARAM_CHECK(ar.Serialize("stallTime", stallTime, 1, 0.0f))
    PARAM_CHECK(ar.Serialize("lastPos", lastPos, 1, VZero))
    // object refs resolve on the second load pass, after the world's
    // vehicle serializer recreated the hulls and crews
    PARAM_CHECK(ar.SerializeRef("vehicle", vehicle, 1))
    PARAM_CHECK(ar.SerializeRef("escort", escort, 1))
    PARAM_CHECK(ar.SerializeRef("group", group, 1))
    PARAM_CHECK(ar.SerializeRef("driver", driver, 1))
    return LSOK;
}

LSError Traffic::ReleasedEntry::Serialize(ParamArchive& ar)
{
    PARAM_CHECK(ar.Serialize("boarded", boarded, 1, false))
    PARAM_CHECK(ar.SerializeRef("vehicle", vehicle, 1))
    PARAM_CHECK(ar.SerializeRef("group", group, 1))
    PARAM_CHECK(ar.SerializeRefs("Bodies", bodies, 1))
    return LSOK;
}

LSError Traffic::FleeingDriver::Serialize(ParamArchive& ar)
{
    PARAM_CHECK(ar.Serialize("age", age, 1, 0.0f))
    PARAM_CHECK(ar.SerializeRef("person", person, 1))
    PARAM_CHECK(ar.SerializeRef("group", group, 1))
    return LSOK;
}

void Traffic::ApplyPendingLoad()
{
    ZoneRegistry& registry = ZoneRegistry::Instance();
    _entries.Clear();
    for (int r = 0; r < _pending.Size(); r++)
    {
        TrafficEntry e = _pending[r];
        if (!e.vehicle.GetLink())
        {
            continue; // hull did not survive the save: drop the row
        }
        // zone indices are re-resolved by NAME against the rebuilt table;
        // a row whose zones no longer exist still drives to its saved dest
        e.originIndex = registry.FindZoneIndex(e.originZone);
        e.destIndex = registry.FindZoneIndex(e.destZone);
        // a commandeer in flight does not survive a save: restart the stop
        if (e.state == TSStopping || e.state == TSExiting)
        {
            e.state = TSDriving;
        }
        e.stateTime = 0;
        _entries.Add(e);
    }
    // released hulls / fleeing drivers: prune dead links
    for (int i = _released.Size() - 1; i >= 0; i--)
    {
        if (!_released[i].vehicle.GetLink())
        {
            _released.Delete(i);
        }
    }
    for (int i = _fleeing.Size() - 1; i >= 0; i--)
    {
        if (!_fleeing[i].person.GetLink())
        {
            _fleeing.Delete(i);
        }
    }
}

LSError Traffic::Serialize(ParamArchive& ar)
{
    if (ar.IsSaving())
    {
        _pending = _entries;
    }

    PARAM_CHECK(ar.Serialize("onSpawned", _handlers[TESpawned], 1, RString()))
    PARAM_CHECK(ar.Serialize("onDespawned", _handlers[TEDespawned], 1, RString()))
    PARAM_CHECK(ar.Serialize("onCommandeered", _handlers[TECommandeered], 1, RString()))
    PARAM_CHECK(ar.Serialize("onArrived", _handlers[TEArrived], 1, RString()))
    PARAM_CHECK(ar.Serialize("onDriverKilled", _handlers[TEDriverKilled], 1, RString()))
    PARAM_CHECK(ar.Serialize("Entries", _pending, 1))
    PARAM_CHECK(ar.Serialize("Released", _released, 1))
    PARAM_CHECK(ar.Serialize("Fleeing", _fleeing, 1))

    if (ar.IsSaving())
    {
        _pending.Clear();
    }
    else if (ar.GetPass() == ParamArchive::PassSecond)
    {
        // the mission config was reparsed during the load's first pass and
        // the ZoneRegistry rebuilt its tables before this block runs (its
        // subclass precedes ours in World::Serialize); object refs were
        // resolved by this pass's SerializeRef
        LoadFromConfig();
        ApplyPendingLoad();
        _pending.Clear();
    }
    return LSOK;
}

} // namespace Poseidon::Guerrilla
