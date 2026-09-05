#include <Poseidon/Game/Guerrilla/Traffic.hpp>
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>
#include <Poseidon/Game/Guerrilla/AlertMachine.hpp> // GetZoneState (modulation)
#include <Poseidon/Game/Guerrilla/Undercover.hpp>   // ClassifyWeaponShow

#include <Poseidon/IO/ParamFileExt.hpp> // Pars / ExtParsMission
#include <Poseidon/IO/Serialization/ParamArchive.hpp>

#include <Evaluator/express.hpp>                   // GameState / GameValue (event dispatch, war level)
#include <Poseidon/Game/Commands/GameStateExt.hpp> // GameValueExt

#include <Poseidon/Core/Global.hpp> // Glob.clock (wall-clock modulation)

#include <Poseidon/Core/Application.hpp>          // GApp (ENGINE_CONFIG)
#include <Poseidon/Core/Config/EngineConfig.hpp>  // ENGINE_CONFIG.objectsZ / horizontZ (perception gate)
#include <Poseidon/World/Scene/Camera/Camera.hpp> // camera position/heading (perception gate)

#include <Poseidon/World/World.hpp>
#include <Poseidon/World/Scene/Scene.hpp>                  // GScene (sun darkness, camera)
#include <Poseidon/World/Terrain/Landscape.hpp>            // GLOB_LAND surface Y, rain
#include <Poseidon/World/Terrain/Roads.hpp>                // GRoadNet
#include <Poseidon/Graphics/Rendering/Lighting/Lights.hpp> // LightSun::NightEffect
#include <Poseidon/World/Entities/Infantry/Person.hpp>
#include <Poseidon/World/Entities/Infantry/SoldierOld.hpp> // Man
#include <Poseidon/World/Entities/Vehicles/Transport.hpp>
#include <Poseidon/World/Entities/Weapons/Weapons.hpp> // AmmoType (danger severity)
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

// Fast gate for the frozen-core danger hooks (the GUndercoverActive
// precedent): FireWeaponEffects fires per round from every entity, so the
// inactive case must cost one bool read.  Synced by Simulate and Clear.
bool GTrafficDangerArmed = false;

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
    _percept = Perception();
    _ambushes.Clear();
    _danger.Clear();
    _dangerNow.Clear();
    GTrafficDangerArmed = false;
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
    t.civNightScale = zonesCfg->ReadValue("trafficCivNightScale", t.civNightScale);
    t.dayStart = zonesCfg->ReadValue("trafficDayStart", t.dayStart);
    t.dayEnd = zonesCfg->ReadValue("trafficDayEnd", t.dayEnd);
    t.alertPatrolBoost = zonesCfg->ReadValue("trafficAlertPatrolBoost", t.alertPatrolBoost);
    t.curfewWarLevel = zonesCfg->ReadValue("trafficCurfewWarLevel", t.curfewWarLevel);
    t.curfewPatrolBoost = zonesCfg->ReadValue("trafficCurfewPatrolBoost", t.curfewPatrolBoost);
    t.rainCivFade = zonesCfg->ReadValue("trafficRainCivFade", t.rainCivFade);
    t.stallTimeout = zonesCfg->ReadValue("trafficStallTimeout", t.stallTimeout);
    t.arriveRadius = zonesCfg->ReadValue("trafficArriveRadius", t.arriveRadius);
    t.maxLegs = toInt(zonesCfg->ReadValue("trafficMaxLegs", (float)t.maxLegs));
    t.commandeerRadius = zonesCfg->ReadValue("trafficCommandeerRadius", t.commandeerRadius);
    t.commandeerLaneHalfWidth = zonesCfg->ReadValue("trafficCommandeerLaneHalfWidth", t.commandeerLaneHalfWidth);
    t.commandeerStopDelay = zonesCfg->ReadValue("trafficCommandeerStopDelay", t.commandeerStopDelay);
    t.fleeDist = zonesCfg->ReadValue("trafficFleeDist", t.fleeDist);
    t.parkChance = zonesCfg->ReadValue("trafficParkChance", t.parkChance);
    t.parkDwellMin = zonesCfg->ReadValue("trafficParkDwellMin", t.parkDwellMin);
    t.parkDwellMax = zonesCfg->ReadValue("trafficParkDwellMax", t.parkDwellMax);
    t.exposeMargin = zonesCfg->ReadValue("trafficExposeMargin", t.exposeMargin);
    t.despawnDeferMax = zonesCfg->ReadValue("trafficDespawnDeferMax", t.despawnDeferMax);
    t.scaleCaps = zonesCfg->ReadValue("trafficScaleCaps", t.scaleCaps ? 1.0f : 0.0f) != 0.0f;
    t.combatStaleAfter = zonesCfg->ReadValue("trafficCombatStaleAfter", t.combatStaleAfter);
    t.combatHoldMax = zonesCfg->ReadValue("trafficCombatHoldMax", t.combatHoldMax);
    t.bailCombatWindow = zonesCfg->ReadValue("trafficBailCombatWindow", t.bailCombatWindow);
    t.dangerRadius = zonesCfg->ReadValue("trafficDangerRadius", t.dangerRadius);
    t.dangerCloseRadius = zonesCfg->ReadValue("trafficDangerCloseRadius", t.dangerCloseRadius);
    t.dangerCooldown = zonesCfg->ReadValue("trafficDangerCooldown", t.dangerCooldown);
    t.dangerTtl = zonesCfg->ReadValue("trafficDangerTtl", t.dangerTtl);
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
    if (t.parkChance < 0)
    {
        t.parkChance = 0;
    }
    if (t.parkChance > 1)
    {
        t.parkChance = 1;
    }
    if (t.parkDwellMin < 0)
    {
        t.parkDwellMin = 0;
    }
    if (t.parkDwellMax < t.parkDwellMin)
    {
        t.parkDwellMax = t.parkDwellMin;
    }
    if (t.exposeMargin < 0)
    {
        t.exposeMargin = 0;
    }
    if (t.despawnDeferMax < 0)
    {
        t.despawnDeferMax = 0;
    }
    if (t.combatStaleAfter < 0)
    {
        t.combatStaleAfter = 0;
    }
    if (t.combatHoldMax < 0)
    {
        t.combatHoldMax = 0;
    }
    if (t.bailCombatWindow < 0)
    {
        t.bailCombatWindow = 0;
    }
    if (t.dangerRadius < 0)
    {
        t.dangerRadius = 0; // 0 (and below) = danger response off
    }
    if (t.dangerCloseRadius < 0)
    {
        t.dangerCloseRadius = 0;
    }
    if (t.dangerCloseRadius > t.dangerRadius)
    {
        t.dangerCloseRadius = t.dangerRadius; // the close band lives inside the reaction band
    }
    if (t.dangerCooldown < 0)
    {
        t.dangerCooldown = 0;
    }
    if (t.dangerTtl < 0)
    {
        t.dangerTtl = 0;
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

bool Traffic::EntryDest(const Transport* veh, Vector3& out) const
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
            out = e.dest;
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
    if (stricmp(name, "parked") == 0)
    {
        return TEParked;
    }
    if (stricmp(name, "departed") == 0)
    {
        return TEDeparted;
    }
    if (stricmp(name, "bailed") == 0)
    {
        return TEBailed;
    }
    if (stricmp(name, "panicked") == 0)
    {
        return TEPanicked;
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

// time-of-day trapezoid: civNightScale outside [dayStart, dayEnd], a linear
// ramp just inside each edge, 1 on the plateau between the ramps
static float DayTrapezoid(float dayFraction, const TrafficTuning& tuning)
{
    if (dayFraction <= tuning.dayStart || dayFraction >= tuning.dayEnd)
    {
        return tuning.civNightScale;
    }
    float s = 1.0f;
    float rise = (dayFraction - tuning.dayStart) / Traffic::DayRampFraction;
    if (rise < s)
    {
        s = rise;
    }
    float fall = (tuning.dayEnd - dayFraction) / Traffic::DayRampFraction;
    if (fall < s)
    {
        s = fall;
    }
    return tuning.civNightScale + (1.0f - tuning.civNightScale) * s;
}

void Traffic::ModulationFactors(const TrafficModulationInput& in, const TrafficTuning& tuning, float& civScale,
                                float& patrolScale)
{
    civScale = DayTrapezoid(in.dayFraction, tuning);
    patrolScale = 1.0f;

    // alert on the civ route origin: RED empties the roads, YELLOW thins
    // them; either boosts the patrols
    if (in.originAlertCiv >= ASRed)
    {
        civScale = 0;
    }
    else if (in.originAlertCiv == ASYellow)
    {
        civScale *= AlertYellowCivScale;
    }
    if (in.originAlertCiv >= ASYellow)
    {
        patrolScale *= 1.0f + tuning.alertPatrolBoost;
    }

    // curfew: an occupied origin, after dark, late in the war.  Darkness is
    // NightEffect, not the wall clock, so curfew and the AI headlights agree
    // on what "night" is.
    if (in.warLevel >= tuning.curfewWarLevel && in.nightEffect > CurfewNightEffect && in.originOccupied)
    {
        civScale = 0;
        patrolScale *= tuning.curfewPatrolBoost;
    }

    // rain thins the civilians
    civScale *= 1.0f - tuning.rainCivFade * in.rain;

    if (civScale < 0)
    {
        civScale = 0;
    }
    if (civScale > 1)
    {
        civScale = 1;
    }
    if (patrolScale < 0)
    {
        patrolScale = 0;
    }
}

// one modulated band: chance * scale kept inside [0,1] so the roll shifting
// below stays exact
static float ScaledChance(float chance, float scale)
{
    float c = chance * scale;
    if (c < 0)
    {
        c = 0;
    }
    if (c > 1)
    {
        c = 1;
    }
    return c;
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
        float chance = ScaledChance(tuning.patrolChance, in.patrolScale);
        if (roll < chance)
        {
            return TKPatrol;
        }
        roll -= chance;
    }
    if (in.hasCivRoute && in.liveCiv < tuning.maxCiv)
    {
        if (roll < ScaledChance(tuning.civChance, in.civScale))
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

bool Traffic::ShouldDespawn(float playerDistSq, const TrafficEffectiveBand& band)
{
    return playerDistSq > Square(band.despawnEdge);
}

TrafficEffectiveBand Traffic::ConfigBand(const TrafficTuning& tuning)
{
    TrafficEffectiveBand b;
    b.minSpawn = tuning.minSpawnDist;
    b.radius = tuning.radius;
    b.despawnEdge = tuning.radius + tuning.despawnHysteresis;
    b.closeHold = tuning.minSpawnDist;
    return b;
}

TrafficEffectiveBand Traffic::EffectiveBand(const TrafficTuning& tuning, float objectsZ, bool lightsOn, float horizontZ)
{
    // the cull that actually hides things: the object mesh cull, raised to
    // the night light bound when the vehicle would show headlights (lights
    // draw out to horizontZ + NightLightCullMargin, farther than the mesh)
    float cull = objectsZ;
    if (lightsOn)
    {
        float lightCull = horizontZ + NightLightCullMargin;
        if (lightCull > cull)
        {
            cull = lightCull;
        }
    }
    float safe = cull + tuning.exposeMargin;
    TrafficEffectiveBand b = ConfigBand(tuning);
    if (safe > b.minSpawn)
    {
        b.minSpawn = safe;
    }
    // a pushed-out floor first NARROWS the band (the radius and despawn edge
    // stay at their config values, preserving the pre-#53 behaviour at the
    // daytime default view distance); the band only widens once less than a
    // usable width remains above the floor - never past the config width,
    // never past a road-scan radius (candidates come from SpawnScanRadius
    // scans, a thinner annulus starves the spawn pick), and never empty, or
    // high view distances get zero traffic
    float width = tuning.radius - tuning.minSpawnDist;
    if (width < 0)
    {
        width = 0;
    }
    if (width > SpawnScanRadius)
    {
        width = SpawnScanRadius;
    }
    if (b.minSpawn + width > b.radius)
    {
        b.radius = b.minSpawn + width;
    }
    // the despawn edge follows the (possibly widened) band so a fresh spawn
    // at the band cap is never instantly beyond it, and never sits inside
    // the always-safe distance
    b.despawnEdge = b.radius + tuning.despawnHysteresis;
    if (b.despawnEdge < safe)
    {
        b.despawnEdge = safe;
    }
    return b;
}

bool Traffic::CanExposeSpawn(float dist2, bool losBlocked, bool inFrustum, const TrafficEffectiveBand& band)
{
    return dist2 >= Square(band.minSpawn) || losBlocked || !inFrustum;
}

bool Traffic::CanExposeDespawn(float dist2, bool losBlocked, bool inFrustum, const TrafficEffectiveBand& band)
{
    if (inFrustum)
    {
        // never delete out of the player's forward view: even a car the
        // terrain hides right now is one crest away from being missed
        return false;
    }
    if (dist2 < Square(band.closeHold))
    {
        // inside the config minSpawnDist an idling engine is audible even
        // hidden: the pre-#53 close-range hold, kept
        return false;
    }
    // the distance leg is the imperceptibility bound (band.minSpawn: past
    // the cull + margin nothing is drawn), NOT the far-despawn edge - an
    // in-band ending (arrival, stall, linger) must be able to tear down on
    // open terrain once the car is beyond draw range, or invisible lingerers
    // pin the spawn caps
    return losBlocked || dist2 >= Square(band.minSpawn);
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
                        const TrafficTuning& tuning, float roll, int& originIndex, int& destIndex, int originZone,
                        const TrafficEffectiveBand* band)
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

    // origins: eligible zones within the player radius (the live band's when
    // the perception gate pushed it out - the origin gate, the spawn band and
    // the despawn edge must move together), each of which must have at least
    // one destination
    AutoArray<int> origins;
    float r2 = Square(band ? band->radius : tuning.radius);
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

int Traffic::SelectSpawnPoint(const AutoArray<Vector3>& roadPts, Vector3Par playerPos, const TrafficTuning& tuning,
                              const TrafficEffectiveBand& band, const AutoArray<TrafficExposeObs>* obs,
                              const Vector3* originPos, bool preferOrigin)
{
    int best = -1;
    int bestTier = -1;
    float bestD2 = -1;
    // the CONFIG minSpawnDist stays a hard floor - a close spawn is audible
    // even unseen; the cap and the exposure floor come from the live band
    float minD2 = Square(tuning.minSpawnDist);
    float maxD2 = Square(band.radius);
    float floor2 = Square(band.minSpawn);
    float origin2 = Square(AlibiOriginRadius);
    for (int i = 0; i < roadPts.Size(); i++)
    {
        float d2 = Dist2DSq(roadPts[i], playerPos);
        if (d2 < minD2 || d2 > maxD2)
        {
            continue;
        }
        TrafficExposeObs o;
        if (obs && i < obs->Size())
        {
            o = (*obs)[i];
        }
        // the exposure distance leg measures from the CAMERA when known (a
        // scripted camera can sit far from the player, and it is the viewer);
        // the band membership above stays the player's gameplay bubble
        float exposeD2 = o.camDist2 >= 0 ? o.camDist2 : d2;
        if (!CanExposeSpawn(exposeD2, o.losBlocked, o.inFrustum, band))
        {
            continue;
        }
        // tiered scoring (issue #53 T2): a pass that survives an immediate
        // turn-around (beyond the exposure floor, or terrain-hidden - both
        // hold whichever way the camera swings) beats a cone-only pass; an
        // in-zone candidate beats both for the kinds that plausibly pull out
        // of their own base.  Farthest from the player within a tier.
        int tier = (exposeD2 >= floor2 || o.losBlocked) ? 1 : 0;
        if (preferOrigin && originPos && Dist2DSq(roadPts[i], *originPos) <= origin2)
        {
            tier += 2;
        }
        if (tier > bestTier || (tier == bestTier && d2 > bestD2))
        {
            bestTier = tier;
            bestD2 = d2;
            best = i;
        }
    }
    return best;
}

int Traffic::SelectAlibiPoint(const AutoArray<Vector3>& roadPts, Vector3Par playerPos, Vector3Par originPos,
                              const TrafficTuning& tuning, const AutoArray<TrafficExposeObs>* obs)
{
    int best = -1;
    int bestTier = -1;
    float bestD2 = -1;
    float minD2 = Square(tuning.minSpawnDist); // still the audible hard floor
    float origin2 = Square(AlibiOriginRadius);
    for (int i = 0; i < roadPts.Size(); i++)
    {
        if (Dist2DSq(roadPts[i], originPos) > origin2)
        {
            continue; // the alibi only works inside the town
        }
        float d2 = Dist2DSq(roadPts[i], playerPos);
        if (d2 < minD2)
        {
            continue;
        }
        // a curb pull-out is plausible even watched, so visibility only
        // ranks: hidden (terrain or cone) over visible, then farthest
        TrafficExposeObs o;
        if (obs && i < obs->Size())
        {
            o = (*obs)[i];
        }
        int tier = (o.losBlocked || !o.inFrustum) ? 1 : 0;
        if (tier > bestTier || (tier == bestTier && d2 > bestD2))
        {
            bestTier = tier;
            bestD2 = d2;
            best = i;
        }
    }
    return best;
}

int Traffic::ScaleCap(int cap, float effRadius, float configRadius)
{
    if (cap <= 0 || configRadius <= 0 || effRadius <= configRadius)
    {
        return cap; // identity at (or below) the config band; 0 stays "never"
    }
    int scaled = toIntFloor((float)cap * (effRadius / configRadius) + 0.5f);
    return scaled > cap ? scaled : cap;
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

TrafficBlockedAction Traffic::DecideBlocked(float stalledSeconds, int retries, bool nearStopped,
                                            const TrafficTuning& tuning)
{
    if (tuning.stallTimeout <= 0 || !nearStopped || retries >= MaxBlockedRetries)
    {
        return TBlockNone;
    }
    float threshold = tuning.stallTimeout * (float)(retries + 1) / 3.0f;
    if (stalledSeconds < threshold)
    {
        return TBlockNone;
    }
    return retries == 0 ? TBlockRetryLeg : TBlockUTurn;
}

// ---------------------------------------------------------------------------
// danger response (civ danger response) - pure core
// ---------------------------------------------------------------------------

void Traffic::AddDangerEvent(AutoArray<TrafficDangerEvent>& buf, Vector3Par pos, float severity, bool playerCaused)
{
    // coalesce first: automatic fire must collapse into one refreshed
    // episode, not fill the ring one round at a time
    for (int i = 0; i < buf.Size(); i++)
    {
        if (Dist2DSq(buf[i].pos, pos) <= Square(DangerCoalesceRadius))
        {
            if (severity > buf[i].severity)
            {
                buf[i].severity = severity;
            }
            buf[i].playerCaused = buf[i].playerCaused || playerCaused;
            buf[i].age = 0; // the episode is fresh again
            return;
        }
    }
    TrafficDangerEvent e;
    e.pos = pos;
    e.severity = severity;
    e.playerCaused = playerCaused;
    e.age = 0;
    if (buf.Size() >= MaxDangerEvents)
    {
        // full ring: the oldest episode makes room
        int oldest = 0;
        for (int i = 1; i < buf.Size(); i++)
        {
            if (buf[i].age > buf[oldest].age)
            {
                oldest = i;
            }
        }
        buf[oldest] = e;
        return;
    }
    buf.Add(e);
}

void Traffic::AgeDangerEvents(AutoArray<TrafficDangerEvent>& buf, float dt, float ttl)
{
    for (int i = buf.Size() - 1; i >= 0; i--)
    {
        buf[i].age += dt;
        if (buf[i].age >= ttl)
        {
            buf.Delete(i);
        }
    }
}

int Traffic::NearestDanger(const AutoArray<TrafficDangerEvent>& buf, Vector3Par pos, float& outDist)
{
    int best = -1;
    float bestD2 = 0;
    for (int i = 0; i < buf.Size(); i++)
    {
        float d2 = Dist2DSq(buf[i].pos, pos);
        if (best < 0 || d2 < bestD2)
        {
            best = i;
            bestD2 = d2;
        }
    }
    outDist = best >= 0 ? sqrtf(bestD2) : -1.0f;
    return best;
}

float Traffic::DangerBandScale(float severity)
{
    float scale = sqrtf(severity > 0 ? severity : 0.0f);
    saturate(scale, DangerScaleMin, DangerScaleMax);
    return scale;
}

int Traffic::LoudestDanger(const AutoArray<TrafficDangerEvent>& buf, Vector3Par pos, float& outDist)
{
    // rank = (distance / bandScale)^2: how deep inside its own reaction
    // band each source sits, the same sqrt(severity) model
    // DecideDangerReaction bands with - a source is in band iff its rank
    // <= dangerRadius^2, so the loudest is in band whenever any source is
    int best = -1;
    float bestD2 = 0;
    float bestRank = 0;
    for (int i = 0; i < buf.Size(); i++)
    {
        float d2 = Dist2DSq(buf[i].pos, pos);
        float rank = d2 / Square(DangerBandScale(buf[i].severity));
        if (best < 0 || rank < bestRank)
        {
            best = i;
            bestD2 = d2;
            bestRank = rank;
        }
    }
    outDist = best >= 0 ? sqrtf(bestD2) : -1.0f;
    return best;
}

bool Traffic::DangerEscalates(float latchedSeverity, float severity)
{
    return latchedSeverity > 0 && severity >= latchedSeverity * DangerEscalationRatio;
}

bool Traffic::DangerLatchHolds(float cooldownLeft, bool cowering, float latchedSeverity, float severity)
{
    if (DangerEscalates(latchedSeverity, severity))
    {
        return false; // louder than what armed the latch: the decision re-opens
    }
    // a cowering car is latched for as long as it cowers (the clock never
    // re-rolls it against the same fight); anything else waits the cooldown
    return cowering || cooldownLeft > 0;
}

TrafficDangerReaction Traffic::DecideDangerReaction(float distance, float severity, int kind, TrafficState state,
                                                    float cooldownLeft, float roll, const TrafficTuning& tuning)
{
    if (tuning.dangerRadius <= 0 || severity <= 0 || kind != TKCiv || cooldownLeft > 0)
    {
        return TDRNone;
    }
    // the commandeer owns TSStopping/TSExiting, and a cowering car already
    // spent its reaction; everything else is eligible
    bool parked = state == TSParking || state == TSDwelling || state == TSDeparting;
    bool ended = state == TSStalled || state == TSLingering;
    bool driving = state == TSDriving || state == TSArrived;
    if (!parked && !ended && !driving)
    {
        return TDRNone;
    }
    float scale = DangerBandScale(severity);
    if (distance > tuning.dangerRadius * scale)
    {
        return TDRNone;
    }
    if (parked || ended)
    {
        // no live leg left to drive: the driver wants nothing more to do
        // with the car
        return TDRBail;
    }
    if (distance <= tuning.dangerCloseRadius * scale)
    {
        // point-blank: slam the brakes and duck, abandon the car outright,
        // or floor it back the way it came
        if (roll < DangerCloseCowerBand)
        {
            return TDRCower;
        }
        if (roll < DangerCloseBailBand)
        {
            return TDRBail;
        }
        return TDRUTurn;
    }
    // audible but not on top of the car: turn for home, speed past, or freeze
    if (roll < DangerFarUTurnBand)
    {
        return TDRUTurn;
    }
    if (roll < DangerFarRushBand && state != TSArrived)
    {
        // rushing is meaningless at the destination (the arrival ladder
        // overrides the same-leg re-issue): an arrived car freezes instead
        return TDRRush;
    }
    return TDRCower;
}

float Traffic::DangerSeverityFromAudible(float audibleFire)
{
    // the one-constant recalibration point: DangerRifleAudibleFire is the
    // audibleFire that reads as severity 1 (CfgAmmo magnitudes are not
    // readable offline; the in-game probe retunes the constant, nothing
    // else moves)
    return audibleFire * (1.0f / DangerRifleAudibleFire);
}

float Traffic::DangerSeverityFromBlast(bool explosive, float indirectHit, float indirectHitRange)
{
    // ExplosionDammage runs for EVERY projectile impact (ShotShell ground
    // and object hits included), not just detonations: only a real
    // explosive with meaningful indirect damage registers an episode
    if (!explosive || indirectHit * indirectHitRange < DangerBlastPowerMin)
    {
        return 0;
    }
    return DangerExplosionSeverity;
}

bool Traffic::WreckDangerLive(float wreckAge, const TrafficTuning& tuning)
{
    return tuning.dangerTtl > 0 && wreckAge < tuning.dangerTtl * WreckDangerTtlScale;
}

const char* Traffic::DangerReactionName(int reaction)
{
    switch (reaction)
    {
        case TDRCower:
            return "cower";
        case TDRUTurn:
            return "uturn";
        case TDRRush:
            return "rush";
        case TDRBail:
            return "bail";
        default:
            return "none";
    }
}

bool Traffic::DecidePark(float roll, const TrafficTuning& tuning)
{
    return tuning.parkChance > 0 && roll < tuning.parkChance;
}

TrafficState Traffic::LoadedParkState(TrafficState saved, bool driverSeated)
{
    switch (saved)
    {
        case TSParking:
            return driverSeated ? TSParking : TSDwelling;
        case TSDwelling:
            return driverSeated ? TSParking : TSDwelling; // seated dweller = degenerate row, re-park
        case TSDeparting:
            // on foot: re-dwell - the get-in flags cannot be assumed to have
            // survived the load, so the depart transition re-issues them
            return driverSeated ? TSDeparting : TSDwelling;
        case TSLingering:
            // the crew never dismounts while lingering, so seated is the
            // invariant; an on-foot driver is a degenerate row - restart it
            // driving and let the shared guards reconcile it
            return driverSeated ? TSLingering : TSDriving;
        default:
            return saved;
    }
}

TrafficEndAction Traffic::ArrivedEndAction(bool despawnSafe, int legs, int maxLegs)
{
    if (despawnSafe)
    {
        return TEndDespawn; // unobserved: the pre-#53 semantics unchanged
    }
    if (legs < maxLegs)
    {
        return TEndReLeg; // the player is watching: drive on to another zone
    }
    return TEndLinger; // legs spent: pull over and wait to be unobserved
}

TrafficEndAction Traffic::StalledEndAction(bool despawnSafe, int kind)
{
    if (despawnSafe)
    {
        return TEndDespawn;
    }
    // the walk-off machinery is single-driver-shaped (the commandeer bail);
    // patrol and convoy crews stay seated instead of dismounting sentries
    return kind == TKCiv ? TEndAbandon : TEndLinger;
}

TrafficCombatGate Traffic::CombatGateAction(bool inCombat, float sinceDisclosed, float heldTime,
                                            const TrafficTuning& tuning)
{
    if (tuning.combatHoldMax <= 0)
    {
        return TCGClear; // gate disabled
    }
    // negative = a raw Time underflow slipped past the world layer's
    // never-disclosed mapping: treat as stale, never as recent
    if (!inCombat && (sinceDisclosed < 0 || sinceDisclosed >= tuning.combatStaleAfter))
    {
        return TCGClear; // the episode is over: nothing hot, nothing recent
    }
    if (heldTime >= tuning.combatHoldMax)
    {
        // bounded escape: the budget stays spent until the episode clears,
        // so an exhausted gate never flips back to holding
        return TCGExhausted;
    }
    return TCGHold;
}

bool Traffic::ConvoyBailTriggered(bool escortExisted, bool escortDead, float sinceDisclosed,
                                  const TrafficTuning& tuning)
{
    if (!escortExisted || !escortDead)
    {
        return false;
    }
    // a quiet loss (a crash, a hit long forgotten) is not a rout; negative
    // sinceDisclosed (a raw Time underflow for never-disclosed) is not recent
    return tuning.bailCombatWindow > 0 && sinceDisclosed >= 0 && sinceDisclosed <= tuning.bailCombatWindow;
}

TrafficCrewDisposal Traffic::CrewDisposal(bool personDead, bool seated)
{
    if (personDead)
    {
        return TCDBody; // only the actual dead ride the bodies table
    }
    return seated ? TCDDismountFlee : TCDFlee;
}

// ---------------------------------------------------------------------------
// world-touching internals (engine path only)
// ---------------------------------------------------------------------------

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
    // IssueCommand pulled the addressed drivers (and, via AddUnitWithCargo,
    // their whole crews) into a command subgroup; a fresh one constructs as
    // FormWedge + SpeedNormal, so the MainSubgroup settings above never
    // reach a crew that was split off - every convoy, from its first leg.
    // Apply them to the subgroup the drivers actually live in, so convoys
    // really drive SpeedLimited in column (FormationPilot's follow branch
    // then tails GetFormationPrevious with the don't-overtake clamp).
    for (int u = 0; u < MAX_UNITS_PER_GROUP; u++)
    {
        if (!drivers.Get(u))
        {
            continue;
        }
        AIUnit* unit = grp->UnitWithID(u + 1);
        AISubgroup* sub = unit ? unit->GetSubgroup() : nullptr;
        if (!sub || sub == grp->MainSubgroup())
        {
            continue; // main already set above
        }
        sub->SetSpeedMode((SpeedMode)speedMode);
        if (column)
        {
            sub->SetFormation(AI::FormColumn);
        }
    }
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
                         AutoArray<TrafficEventRecord>& fired, bool forced)
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
    int spot;
    bool alibi = false;
    if (forced)
    {
        // gmTrafficForceSpawn: config band, no perception gate (mirrors the
        // chance-roll bypass - the tests spawn deliberately close)
        spot = SelectSpawnPoint(pts, playerPos, _tuning);
    }
    else
    {
        AutoArray<TrafficExposeObs> obs;
        BuildExposeObs(pts, playerPos, obs);
        // patrols/convoys prefer a point inside their origin zone (a car
        // appearing inside its own base is a plausible birth, watched or not)
        spot = SelectSpawnPoint(pts, playerPos, _tuning, _percept.band, &obs, &origin->pos, kind != TKCiv);
        if (spot < 0 && kind == TKCiv)
        {
            // alibi fallback (issue #53 T2): the origin town is near the
            // player and no hidden point exists - spawn PARKED at a curb
            // inside the town and pull out via the depart machinery (the
            // machinery is civ-shaped: the steal watch reads any gunner as
            // a theft, so the military kinds skip the fallback)
            spot = SelectAlibiPoint(pts, playerPos, origin->pos, _tuning, &obs);
            alibi = spot >= 0;
        }
    }
    if (spot < 0)
    {
        return false;
    }
    Vector3 spawnPos = pts[spot];
    Vector3 destPt = GRoadNet->GetNearestRoadPoint(dest->pos);
    // face the way that leads toward the destination (an alibi car is parked
    // at the curb - its destination is re-rolled at the pull-out, so the
    // road direction as scanned is the honest heading)
    Vector3 heading = dirs[spot];
    if (!alibi)
    {
        Vector3 toDest = destPt - spawnPos;
        toDest[1] = 0;
        if (heading.DotProduct(toDest) < 0)
        {
            heading = -heading;
        }
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
                // 1-2 cargo riflemen, so the combat dismount (AIGroup::
                // Disclose orders unloadInCombat cargo out) has bodies to
                // fight with; gated on the hull actually having cargo seats
                int riflemen = escort->GetMaxManCargo();
                if (riflemen > 2)
                {
                    riflemen = 2;
                }
                for (int c = 0; c < riflemen; c++)
                {
                    SeatOrDelete(CreateCrewman(grp, crewType, escortPos + heading * 4.0f, escort, GIPCargo), escort);
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
    if (alibi)
    {
        // parked at the curb, about to leave: enter the 911b724 depart
        // machinery directly.  destIndex is pinned to the ORIGIN so the
        // depart transition (which advances origin := dest and rolls a fresh
        // destination FROM it) departs from the town the car is actually in
        e.state = TSDeparting;
        e.stateTime = 0;
        e.destIndex = originIndex;
        e.destZone = origin->name;
        e.dest = spawnPos;
    }
    else
    {
        int combat = kind == TKCiv ? CMCareless : CMSafe;
        int speed = kind == TKPatrol ? SpeedNormal : SpeedLimited;
        IssueRoute(e, destPt, combat, speed, kind == TKConvoy);
    }
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

static void IssueSoloMove(AIGroup* grp, AIUnit* unit, Vector3 tgt); // defined with the park logic below

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
    // a violent end (wreck, murdered crew, a bail under fire) is
    // player-caused in practice - ambient traffic has no other enemies - and
    // its remains get the longer memory (issue #53 T4); the quiet releases
    // (abandon, script release) stay ambient set dressing
    bool violent = strcmp(reason, "destroyed") == 0 || strcmp(reason, "crewDead") == 0 || strcmp(reason, "bailed") == 0;
    if (violent && e.kind != TKCiv)
    {
        // a wiped patrol or convoy is an alert stimulus: queue for the
        // AlertMachine tick (ConsumeAmbushes), which floors the attributed
        // zone's knowledge and points its lastKnown at the wreck ("bailed"
        // qualifies too - a convoy only bails after losing its escort to
        // recent combat)
        TrafficAmbush am;
        am.pos = veh ? veh->Position() : e.lastPos;
        am.kind = e.kind;
        am.originZone = e.originZone;
        _ambushes.Add(am);
    }
    if (keepHull)
    {
        // the hull (and whatever is left of the crew) outlives the entry;
        // cleanup deletes it once the player is far, unless somebody boarded
        ReleasedEntry r;
        r.vehicle = veh;
        r.group = grp;
        r.playerCaused = violent;
        if (grp)
        {
            // revoke standing re-board orders once, for both hulls - a
            // crewman dismounted from the ESCORT must not be ordered back
            // aboard either
            if (veh)
            {
                grp->UnassignVehicle(veh);
            }
            if (escort)
            {
                grp->UnassignVehicle(escort);
            }
            for (int u = 0; u < MAX_UNITS_PER_GROUP; u++)
            {
                AIUnit* unit = grp->UnitWithID(u + 1);
                if (!unit || !unit->GetPerson())
                {
                    continue;
                }
                Person* person = unit->GetPerson();
                Transport* seat = unit->GetVehicleIn();
                TrafficCrewDisposal d = CrewDisposal(person->IsDammageDestroyed(), seat != nullptr);
                if (d == TCDBody)
                {
                    r.bodies.Add(person);
                    continue;
                }
                // a LIVING person (a park-state driver on foot, or a seated
                // survivor of the truck's end) must not be filed as a
                // corpse - ReleasedEntry::bodies is deleted wholesale.  Hand
                // him to the fleeing table, which respects life, with a walk
                // away from the (likely burning) hull; a seated survivor
                // steps out first
                unit->OrderGetIn(false);
                unit->AllowGetIn(false);
                if (d == TCDDismountFlee)
                {
                    unit->DoGetOut(seat, false);
                }
                Transport* hull = seat ? seat : veh;
                if (hull)
                {
                    Vector3 away = person->Position() - hull->Position();
                    away[1] = 0;
                    if (away.SquareSize() < 1e-2f)
                    {
                        away = -hull->Direction();
                        away[1] = 0;
                    }
                    if (away.SquareSize() < 1e-2f)
                    {
                        away = VForward;
                    }
                    IssueSoloMove(grp, unit, person->Position() + away.Normalized() * (2.0f * ParkWanderRadius));
                }
                FleeingDriver fd;
                fd.person = person;
                fd.group = grp;
                fd.age = 0;
                _fleeing.Add(fd);
            }
        }
        _released.Add(r);
        if (escort)
        {
            ReleasedEntry r2;
            r2.vehicle = escort;
            r2.playerCaused = violent;
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

// seconds since the group's last AIGroup::Disclose.  Never disclosed is
// TIME_MIN, where raw Time subtraction underflows to a large NEGATIVE float
// (and asserts in debug builds) - map it to huge-positive = stale instead
static float SinceDisclosed(const AIGroup* grp)
{
    if (!grp)
    {
        return 1e9f;
    }
    Foundation::Time disc = grp->GetDisclosed();
    if (disc == TIME_MIN)
    {
        return 1e9f;
    }
    return Glob.time - disc;
}

// brake the whole crew to a stop: Stop to every unit (the doStop idiom, like
// VehStop silent) + SpeedLimited; shared by the park roll, the commandeer
// trigger and the linger transition
static void IssueGroupStop(AIGroup* grp)
{
    if (!grp)
    {
        return;
    }
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
    grp->IssueCommand(cmd, all);
    if (grp->MainSubgroup())
    {
        grp->MainSubgroup()->SetSpeedMode(SpeedLimited);
    }
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
        // the one-reaction-per-episode danger latch ticks in EVERY state
        // (cowering, commandeer-owned, parked, headless): 45 s means 45 s
        if (e.dangerCooldown > 0)
        {
            e.dangerCooldown -= _tuning.interval;
            if (e.dangerCooldown < 0)
            {
                e.dangerCooldown = 0;
            }
        }
        // escort watch: the truck checks above have no eyes on the escort
        // hull.  An escort lost while the fight is still fresh breaks the
        // crew - they bail and the load is the player's for the taking (the
        // loot moment).  A quiet loss (a crash, a hit long forgotten) just
        // releases the escort hull and the truck drives on unescorted; its
        // dead crew stays with the shared group and is filed when the entry
        // itself ends.
        if (e.kind == TKConvoy)
        {
            Transport* esc = e.escort;
            bool escortDead = esc && (esc->IsDammageDestroyed() || !DriverAlive(this, esc));
            if (ConvoyBailTriggered(esc != nullptr, escortDead, SinceDisclosed(e.group), _tuning))
            {
                AbandonEntry(i, "bailed", fired, true);
                continue;
            }
            if (escortDead)
            {
                ReleasedEntry r;
                r.vehicle = esc;
                r.playerCaused = true; // a dead escort is a violent end
                // file the escort's own dead, still seated in it, with its
                // hull so the cleanup deletes them together (and the
                // boarded probe is not fooled by corpses); crew that died
                // outside stays with the group and is filed when the entry
                // itself ends
                Person* seats[3] = {esc->Driver(), esc->Gunner(), esc->Commander()};
                for (int s = 0; s < 3; s++)
                {
                    if (seats[s] && seats[s]->IsDammageDestroyed())
                    {
                        r.bodies.Add(seats[s]);
                    }
                }
                const ManCargo& cargo = esc->GetManCargo();
                for (int m = 0; m < cargo.Size(); m++)
                {
                    Person* p = cargo[m];
                    if (p && p->IsDammageDestroyed())
                    {
                        r.bodies.Add(p);
                    }
                }
                _released.Add(r);
                e.escort = nullptr;
            }
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
        if (e.state == TSParking || e.state == TSDwelling || e.state == TSDeparting)
        {
            if (zones.Size() == 0)
            {
                BuildZoneCandidates(zones); // cheap, reused by the depart leg
            }
            UpdateParked(i, playerPos, playerValid, zones, fired);
            // gates the DriverAlive check (driver is on foot), the stall
            // accrual/expiry (a parked car never moves), the arrival branch
            // (it sits inside arriveRadius permanently) and the shared
            // far-despawn (re-run inside UpdateParked)
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
        if (ShouldDespawn(playerD2, _percept.band))
        {
            if (GateDespawn(e, veh->Position(), playerD2, _tuning.radius + _tuning.despawnHysteresis))
            {
                DespawnEntry(i, "far", false, fired);
                continue;
            }
        }
        else
        {
            // back inside the band: the defer budget is per-episode, stale
            // blocked seconds must not carry into a later wanted despawn
            e.exposeDefer = 0;
        }
        // danger-reaction tier (civ only).  The commandeer always wins: its
        // sub-tick runs first and parks a fronted car in TSStopping, which
        // the early branch above already skipped.  One reaction per cooldown
        // episode, decided against the LOUDEST per-pass source (issue #55:
        // the one deepest inside its own band, so a nearer wreck no longer
        // shadows a grenade fight), and the latch yields to a danger that
        // escalates past the one it was armed against - a car that cowered
        // at a wreck still jumps at rifle fire.  A cowering car re-decides
        // for an escalation only, as the driving/arrived car it was; its
        // hold runs on below otherwise
        bool cowering = e.state == TSPanicked;
        bool atDest = Dist2DSq(veh->Position(), e.dest) <= Square(_tuning.arriveRadius);
        TrafficDangerReaction react = TDRNone;
        int src = -1;
        if (e.kind == TKCiv && _dangerNow.Size() > 0)
        {
            float dangerDist = -1;
            src = LoudestDanger(_dangerNow, veh->Position(), dangerDist);
            if (src >= 0 && !DangerLatchHolds(e.dangerCooldown, cowering, e.latchSeverity, _dangerNow[src].severity))
            {
                TrafficState decideAs = cowering ? (atDest ? TSArrived : TSDriving) : e.state;
                react = DecideDangerReaction(dangerDist, _dangerNow[src].severity, e.kind, decideAs, 0.0f,
                                             GRandGen.RandomValue(), _tuning);
            }
        }
        if (cowering && react == TDRNone)
        {
            // cower hold: braked with the crew ducked; drive on once the
            // hold ran out AND the ring has gone quiet nearby (a re-CHECK,
            // never a re-roll - the cooldown latch stops order ping-pong).
            // Deliberately the RING only, not the wreck sources: a static
            // wreck would pin the car (and the maxCiv cap) here forever
            e.stateTime += _tuning.interval;
            if (e.stateTime >= DangerCowerHold)
            {
                float quietDist = -1;
                int near = NearestDanger(_danger, veh->Position(), quietDist);
                if (near < 0 || quietDist > _tuning.dangerRadius * DangerScaleMax)
                {
                    // resume where the trip left off; a car that cowered
                    // inside its arrival radius returns to TSArrived so the
                    // arrival ladder is not re-armed (no duplicate arrived
                    // event, no second park roll)
                    e.state = atDest ? TSArrived : TSDriving;
                    e.stateTime = 0;
                    e.stallTime = 0;
                    e.blockedRetries = 0;
                    e.lastPos = veh->Position();
                    if (!atDest)
                    {
                        IssueRoute(e, e.dest, CMCareless, SpeedLimited, false);
                    }
                }
            }
            continue; // no stall accrual, no arrival: a cowering car holds in place
        }
        if (react != TDRNone)
        {
            e.dangerCooldown = _tuning.dangerCooldown;
            e.latchSeverity = _dangerNow[src].severity;
            if (cowering)
            {
                LOG_INFO(Core, "Traffic: cowering civ car escalates to {}", DangerReactionName(react));
            }
            Vector3 dangerPos = _dangerNow[src].pos;
            TrafficEventRecord ev;
            ev.type = TEPanicked;
            ev.kind = e.kind;
            ev.originIndex = e.originIndex;
            ev.destIndex = e.destIndex;
            ev.vehicle = veh;
            ev.driver = e.driver;
            if (react == TDRBail)
            {
                // the driver brakes, abandons the car and runs from the
                // danger; the panicked event precedes the abandon's
                // despawned one.  The hull keeps the at-gunpoint memory
                // only when the player's own fire forced the bail (the
                // commandeer-bail precedent: playerCaused = the player
                // made this happen, intact hull or not)
                ev.reason = DangerReactionName(TDRBail);
                fired.Add(ev);
                IssueGroupStop(e.group); // brake before the bail, the commandeer shape
                AbandonEntry(i, "panicked", fired, false, &dangerPos, _dangerNow[src].playerCaused);
                LOG_INFO(Core, "Traffic: civ driver panicked and bailed");
                continue;
            }
            if (react == TDRUTurn)
            {
                // the blocked-recovery U-turn at flee pace: swap the
                // endpoints and run for home; an unresolvable origin
                // downgrades to the cower
                const ZoneRecord* back = e.originIndex >= 0 ? ZoneRegistry::Instance().GetZone(e.originIndex) : nullptr;
                if (back && GRoadNet)
                {
                    int backIndex = e.originIndex;
                    RString backZone = e.originZone;
                    e.originIndex = e.destIndex;
                    e.originZone = e.destZone;
                    e.destIndex = backIndex;
                    e.destZone = backZone;
                    e.legs++;
                    e.state = TSDriving;
                    e.stateTime = 0;
                    // a panic U-turn is a fresh leg: the stall clock and
                    // the blocked retries restart with it, the
                    // depart/re-leg convention
                    e.stallTime = 0;
                    e.blockedRetries = 0;
                    IssueRoute(e, GRoadNet->GetNearestRoadPoint(back->pos), CMCareless, SpeedFull, false);
                    // the TEDeparted convention: destIdx = the NEW destination
                    ev.originIndex = e.originIndex;
                    ev.destIndex = e.destIndex;
                    LOG_INFO(Core, "Traffic: civ car panicked, U-turning to {}", (const char*)backZone);
                }
                else
                {
                    react = TDRCower;
                }
            }
            if (react == TDRRush)
            {
                // same leg, floored; the fresh command restarts the stall
                // clock, and a cowering car is back on the road (the rush
                // is never rolled for TSArrived, so TSDriving is the state
                // it cowered from).  Whether a CMCareless driver really
                // accelerates is probe-gated (see DangerFarRushBand)
                if (cowering)
                {
                    e.state = TSDriving;
                    e.stateTime = 0;
                    e.blockedRetries = 0;
                    e.lastPos = veh->Position();
                }
                e.stallTime = 0;
                IssueRoute(e, e.dest, CMCareless, SpeedFull, false);
                LOG_INFO(Core, "Traffic: civ car panicked, speeding past");
            }
            else if (react == TDRCower)
            {
                IssueGroupStop(e.group);
                e.state = TSPanicked;
                e.stateTime = 0;
                e.stallTime = 0;
                LOG_INFO(Core, "Traffic: civ car panicked, cowering");
            }
            ev.reason = DangerReactionName(react);
            fired.Add(ev);
            continue;
        }
        if (e.state == TSLingering)
        {
            // observed ending, parked in place with the crew seated: stable
            // set dressing, no defer timeout - tear down the moment the
            // player genuinely cannot perceive it (or, headless, is beyond
            // the old close-range hold)
            if (DespawnSafe(veh->Position(), playerD2, _tuning.minSpawnDist))
            {
                RString why = e.lingerReason.GetLength() > 0 ? e.lingerReason : RString("arrived");
                DespawnEntry(i, why, false, fired);
            }
            continue; // no stall accrual, no arrival re-trigger for a held car
        }

        // combat gate (convoy discipline under fire): while a patrol/convoy
        // group is fighting or was disclosed moments ago, the native AI owns
        // the vehicles - it halts, dismounts unloadInCombat cargo and fights,
        // and going cautious breaks convoy-follow into combat formation - so
        // the trip ladder below (stall accrual, arrival/stall endings,
        // re-legs) must neither issue orders over the fight nor tear it
        // down.  Bounded: a stale disclosure clears the gate, the hold
        // budget caps a pathologically hot group, and a dead crew never
        // reaches here (the destroyed/DriverAlive guards above run every
        // pass).  The far-despawn and its perception gate above stay live.
        if (e.kind == TKPatrol || e.kind == TKConvoy)
        {
            AIGroup* g = e.group;
            bool inCombat = g && g->GetCombatModeMinor() >= CMCombat;
            TrafficCombatGate gate = CombatGateAction(inCombat, SinceDisclosed(g), e.combatHold, _tuning);
            if (gate == TCGHold)
            {
                e.combatHold += _tuning.interval; // interval-quantized, like stallTime
                e.lastPos = veh->Position();      // keep the stall baseline honest for the release
                continue;
            }
            if (gate == TCGClear)
            {
                e.combatHold = 0; // a fresh budget for the next episode
            }
            // TCGExhausted: still hot but the budget ran out - fall through,
            // the normal ladder is the bounded escape (a linger order landing
            // mid-fight beats a leaked entry)
        }

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
            e.blockedRetries = 0; // a later blockage gets its own recovery attempts
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
                // civ arrival: roll park-vs-drive-on regardless of playerNear
                // (towns accumulate parked cars while the player is 300-1800 m
                // out - the stated payoff); the losing roll and the other
                // kinds fall through to the pre-park code untouched
                AIGroup* pgrp = e.group;
                if (e.kind == TKCiv && pgrp && DecidePark(GRandGen.RandomValue(), _tuning))
                {
                    // brake exactly like the commandeer stop
                    IssueGroupStop(pgrp);
                    e.state = TSParking;
                    e.stateTime = 0;
                    continue;
                }
            }
            // losing (or no) park roll: the observed-endings ladder (#53) -
            // unobserved teardown, re-leg while legs remain, else linger
            TrafficEndAction act =
                ArrivedEndAction(DespawnSafe(veh->Position(), playerD2, _tuning.minSpawnDist), e.legs, _tuning.maxLegs);
            if (act == TEndDespawn)
            {
                DespawnEntry(i, "arrived", false, fired);
                continue;
            }
            if (act == TEndReLeg)
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
                    e.blockedRetries = 0;
                    e.exposeDefer = 0; // the story continues: a fresh defer budget for the next ending
                    int combat = e.kind == TKCiv ? CMCareless : CMSafe;
                    int speed = e.kind == TKPatrol ? SpeedNormal : SpeedLimited;
                    IssueRoute(e, GRoadNet->GetNearestRoadPoint(z->pos), combat, speed, e.kind == TKConvoy);
                    continue;
                }
                // no destination left anywhere: fall through to the linger
                // ending rather than idling exposed in TSArrived
            }
            EnterLinger(e, "arrived");
            continue;
        }

        // blocked-reaction tier (before the expiry ladder): a car boxed in
        // mid-leg first re-plans the same leg - stopped hulls lock their
        // road links and the planner detours around locked links, so this
        // alone clears most blockages - then U-turns for home; only a car
        // that defeats both retries reaches the stallTimeout ladder, at the
        // unchanged timeout.  The speed guard keeps slow-but-moving cars
        // (crawling detours, queues behind a car that is recovering) out
        bool nearStopped = veh->Speed().SquareSize() < Square(BlockedSpeedEpsilon);
        TrafficBlockedAction blocked = DecideBlocked(e.stallTime, e.blockedRetries, nearStopped, _tuning);
        if (blocked != TBlockNone)
        {
            e.blockedRetries++;
            int combat = e.kind == TKCiv ? CMCareless : CMSafe;
            int speed = e.kind == TKPatrol ? SpeedNormal : SpeedLimited;
            if (blocked == TBlockRetryLeg)
            {
                LOG_INFO(Core, "Traffic: {} blocked, re-planning its leg", KindName(e.kind));
                IssueRoute(e, e.dest, combat, speed, e.kind == TKConvoy);
                continue;
            }
            // TBlockUTurn: swap the endpoints and drive back where it came
            // from; a row whose origin no longer resolves just spends the
            // retry and leaves the expiry ladder to finish the story
            const ZoneRecord* back = e.originIndex >= 0 ? ZoneRegistry::Instance().GetZone(e.originIndex) : nullptr;
            if (back && GRoadNet)
            {
                int backIndex = e.originIndex;
                RString backZone = e.originZone;
                e.originIndex = e.destIndex;
                e.originZone = e.destZone;
                e.destIndex = backIndex;
                e.destZone = backZone;
                e.legs++;
                LOG_INFO(Core, "Traffic: {} still blocked, U-turning to {}", KindName(e.kind), (const char*)backZone);
                IssueRoute(e, GRoadNet->GetNearestRoadPoint(back->pos), combat, speed, e.kind == TKConvoy);
                continue;
            }
        }

        if (StallExpired(e.stallTime, _tuning))
        {
            e.state = TSStalled;
            TrafficEndAction act =
                StalledEndAction(DespawnSafe(veh->Position(), playerD2, _tuning.minSpawnDist), e.kind);
            if (act == TEndDespawn)
            {
                DespawnEntry(i, "stalled", false, fired);
                continue;
            }
            if (act == TEndAbandon)
            {
                // observed civ stall: the driver steps out, has a look and
                // walks off; the hull stays behind as set dressing
                AbandonEntry(i, "abandoned", fired);
                continue;
            }
            EnterLinger(e, "stalled");
            continue;
        }
    }
}

// one Move to a single unit through IssueCommand - the doMove idiom (never
// arcade waypoints / AIGroup::Move, which a seated or freshly seated crew
// ignores); shared by the dwell stroll and the steal walk-off
static void IssueSoloMove(AIGroup* grp, AIUnit* unit, Vector3 tgt)
{
    if (!grp || !unit)
    {
        return;
    }
    if (GLandscape)
    {
        float dx, dz;
        tgt[1] = GLOB_LAND->SurfaceYAboveWater(tgt[0], tgt[2], &dx, &dz);
    }
    Command mv;
    mv._message = Command::Move;
    mv._destination = tgt;
    mv._discretion = Command::Undefined;
    mv._context = Command::CtxMission;
    mv._id = grp->GetNextCmdId();
    PackedBoolArray one;
    one.Set(unit->ID() - 1, true);
    grp->IssueCommand(mv, one);
}

// observed trip end for a crew that stays seated (issue #53): brake to a
// stop and hold in TSLingering; UpdateEntries tears the entry down on the
// first pass DespawnSafe allows.  Deliberately indefinite - a stopped car
// with its crew aboard is stable set dressing, and the far-despawn edge
// still applies through the shared gate.
void Traffic::EnterLinger(TrafficEntry& e, const char* reason)
{
    IssueGroupStop(e.group);
    e.state = TSLingering;
    e.stateTime = 0;
    e.stallTime = 0;
    e.exposeDefer = 0;
    e.lingerReason = reason;
}

// observed civ stall (issue #53): the living crew dismounts and walks off -
// the commandeer bail at walking pace - the hull joins the released set
// dressing, and the perception-gated cleanups delete both once unobserved.
// panic is the under-fire flavor (escort lost): everyone RUNS from the
// player at full speed, the remains keep the player-caused memory, and
// TEBailed marks the loot moment for scripts.
// NOT DespawnEntry(keepHull): that would file the living driver as a corpse.
// With fleeFrom (the danger bail) the crew RUNS fleeDist away from the
// danger instead, and playerCaused marks the released hull's memory.
void Traffic::AbandonEntry(int index, const char* reason, AutoArray<TrafficEventRecord>& fired, bool panic,
                           const Vector3* fleeFrom, bool playerCaused)
{
    if (index < 0 || index >= _entries.Size())
    {
        return;
    }
    TrafficEntry e = _entries[index];
    Transport* veh = e.vehicle;
    AIGroup* grp = e.group;
    Person* driver = e.driver;
    bool sprint = panic || fleeFrom != nullptr; // both bail flavors run, the stall walk-off walks
    bool caused = panic || playerCaused;
    if (!veh || !grp || !driver || driver->IsDammageDestroyed())
    {
        // nobody left to walk off: the plain released teardown (which hands
        // any other living crew to the fleeing table itself).  The loot
        // moment still fires first - a bail with the driver already dead is
        // still a bail, and "bailed" rides DespawnEntry's violent set
        if (panic && veh)
        {
            TrafficEventRecord bail;
            bail.type = TEBailed;
            bail.kind = e.kind;
            bail.originIndex = e.originIndex;
            bail.destIndex = e.destIndex;
            bail.vehicle = veh;
            bail.driver = driver;
            fired.Add(bail);
        }
        DespawnEntry(index, reason, true, fired);
        return;
    }
    Transport* escort = e.escort;
    grp->UnassignVehicle(veh);
    if (escort)
    {
        grp->UnassignVehicle(escort);
    }
    // the point everyone flees FROM: the danger when one is given, the
    // player when he caused this, else the (likely burning) hull itself
    Vector3 threat = veh->Position();
    if (fleeFrom)
    {
        threat = *fleeFrom;
    }
    else if (panic && GWorld)
    {
        Person* player = GWorld->GetRealPlayer();
        if (player && !player->IsDammageDestroyed())
        {
            threat = player->Position();
        }
    }
    float fleeDist = sprint ? _tuning.fleeDist : 2.0f * ParkWanderRadius;
    ReleasedEntry r;
    r.vehicle = veh;
    r.group = grp;
    r.playerCaused = caused;
    for (int u = 0; u < MAX_UNITS_PER_GROUP; u++)
    {
        AIUnit* unit = grp->UnitWithID(u + 1);
        if (!unit || !unit->GetPerson())
        {
            continue;
        }
        Person* person = unit->GetPerson();
        Transport* seat = unit->GetVehicleIn();
        TrafficCrewDisposal d = CrewDisposal(person->IsDammageDestroyed(), seat != nullptr);
        if (d == TCDBody)
        {
            r.bodies.Add(person); // deleted with the hull once unobserved
            continue;
        }
        unit->OrderGetIn(false);
        unit->AllowGetIn(false); // he leaves the traffic system for good
        if (d == TCDDismountFlee)
        {
            unit->DoGetOut(seat, false);
        }
        // a walk-off past the car (or a sprint from the player/danger), with
        // the seated crewman's no-offset fallback along his OWN hull's
        // -direction
        Transport* hull = seat ? seat : veh;
        Vector3 away = person->Position() - threat;
        away[1] = 0;
        if (away.SquareSize() < 1e-2f)
        {
            away = -hull->Direction();
            away[1] = 0;
        }
        if (away.SquareSize() < 1e-2f)
        {
            away = VForward;
        }
        IssueSoloMove(grp, unit, person->Position() + away.Normalized() * fleeDist);
        if (sprint)
        {
            // the commandeer sprint: the solo Move pulled the unit into a
            // command subgroup, and a fresh one constructs as SpeedNormal
            AISubgroup* sub = unit->GetSubgroup();
            if (sub)
            {
                sub->SetSpeedMode(SpeedFull);
            }
        }
        FleeingDriver fd;
        fd.person = person;
        fd.group = grp;
        fd.age = 0;
        _fleeing.Add(fd);
    }
    grp->SetCombatModeMajor(CMCareless); // run, don't fight
    if (grp->MainSubgroup())
    {
        // the stall walk-off stays a walk; both bail flavors sprint
        grp->MainSubgroup()->SetSpeedMode(sprint ? SpeedFull : SpeedLimited);
    }
    GetNetworkManager().UpdateObject(grp);

    _released.Add(r);
    if (escort)
    {
        ReleasedEntry r2;
        r2.vehicle = escort;
        r2.playerCaused = caused;
        _released.Add(r2);
    }

    if (panic)
    {
        TrafficEventRecord bail;
        bail.type = TEBailed;
        bail.kind = e.kind;
        bail.originIndex = e.originIndex;
        bail.destIndex = e.destIndex;
        bail.vehicle = veh;
        bail.driver = driver;
        fired.Add(bail);
    }
    TrafficEventRecord ev;
    ev.type = TEDespawned;
    ev.kind = e.kind;
    ev.originIndex = e.originIndex;
    ev.destIndex = e.destIndex;
    ev.reason = reason;
    fired.Add(ev);
    _entries.Delete(index);
    if (panic)
    {
        LOG_INFO(Core, "Traffic: convoy crew bailed");
    }
    else
    {
        LOG_INFO(Core, "Traffic: civ car abandoned ({})", reason);
    }
}

// one entry in TSParking / TSDwelling / TSDeparting: brake wait -> dismount
// -> dwell (with the occasional stroll) -> re-board -> depart to a fresh
// destination.  The caller guarantees e.vehicle is non-null and alive; every
// shared UpdateEntries guard after the park dispatch is re-implemented here
// where still wanted (dead driver, far-despawn) and deliberately absent
// where not (stall, arrival).
void Traffic::UpdateParked(int i, Vector3Par playerPos, bool playerValid, AutoArray<TrafficZoneCandidate>& zones,
                           AutoArray<TrafficEventRecord>& fired)
{
    TrafficEntry& e = _entries[i];
    Transport* veh = e.vehicle;
    AIGroup* grp = e.group;
    Person* driver = e.driver;
    e.stateTime += _tuning.interval; // same accrual style as stallTime

    // 1. dead driver: same outcome as the shared DriverAlive guard, but read
    //    e.driver, NOT veh->Driver() (null while he is on foot)
    if (!driver || driver->IsDammageDestroyed() || !grp)
    {
        DespawnEntry(i, "crewDead", true, fired); // hull + body released, kill EH already fired
        return;
    }
    // 2. far edge: identical policy to the shared check; DeleteCrew deletes
    //    the on-foot driver too, so the plain despawn is already correct
    float parkedD2 = playerValid ? Dist2DSq(veh->Position(), playerPos) : 0.0f;
    if (playerValid && ShouldDespawn(parkedD2, _percept.band))
    {
        if (GateDespawn(e, veh->Position(), parkedD2, _tuning.radius + _tuning.despawnHysteresis))
        {
            DespawnEntry(i, "far", false, fired);
            return;
        }
    }
    else
    {
        // inside the band (or headless): per-episode defer budget, as the
        // shared far gate resets it
        e.exposeDefer = 0;
    }
    AIUnit* unit = driver->Brain();

    // 3. steal watch: any occupant while our driver is not the seated driver
    //    hands the hull to the released table (boarded=true: it is the
    //    player's, an ordinary world object from the next cleanup on) and the
    //    walking driver to the fleeing table.  NOT DespawnEntry(keepHull):
    //    that would file the LIVING driver under ReleasedEntry::bodies and
    //    later delete him as a corpse.
    Person* occ = veh->Driver();
    bool someoneAboard = (occ && occ != driver) || veh->Gunner() || veh->Commander() || veh->GetManCargoSize() > 0;
    if (e.state != TSParking && someoneAboard)
    {
        // revoke the standing re-board orders FIRST (the dwell-expiry
        // transition issued AllowGetIn/OrderGetIn/AssignAsDriver/AddVehicle):
        // without this the group's AssignVehicles/GetInVehicles think keeps
        // ordering the walking driver back into the now-stolen car.
        // AllowGetIn(false) is safe here - unlike the park dismount, this
        // driver leaves the traffic system for good (the commandeer bail
        // does the same)
        grp->UnassignVehicle(veh);
        if (unit)
        {
            unit->OrderGetIn(false);
            unit->AllowGetIn(false);
            if (veh->Driver() == driver || unit->GetVehicleIn() == veh)
            {
                // our own driver is seated (or completed a late GetIn) with a
                // foreign passenger aboard: force him out - a Move to a
                // seated driver makes him DRIVE the car, passenger and all
                // (the doMove idiom)
                unit->DoGetOut(veh, false);
            }
            // the driver walks off ~2x wander radius, CMCareless/SpeedLimited
            // (both already set) - a mild walk, not the commandeer flee
            Vector3 away = driver->Position() - veh->Position();
            away[1] = 0;
            if (away.SquareSize() < 1e-2f)
            {
                away = -veh->Direction();
                away[1] = 0;
            }
            IssueSoloMove(grp, unit, driver->Position() + away.Normalized() * (2.0f * ParkWanderRadius));
        }
        FleeingDriver fd;
        fd.person = driver;
        fd.group = grp;
        fd.age = 0;
        _fleeing.Add(fd);
        ReleasedEntry r;
        r.vehicle = veh;
        r.boarded = true;
        _released.Add(r);
        TrafficEventRecord ev;
        ev.type = TEDespawned;
        ev.kind = e.kind;
        ev.originIndex = e.originIndex;
        ev.destIndex = e.destIndex;
        ev.reason = "stolen";
        fired.Add(ev);
        _entries.Delete(i);
        LOG_INFO(Core, "Traffic: parked civ car stolen");
        return;
    }

    // danger: a parked-family driver (seated braking, dwelling on foot, or
    // re-boarding) abandons the trip and flees.  AbandonEntry's revoke
    // sequence (OrderGetIn/AllowGetIn off BEFORE the fleeing hand-off) is
    // mandatory here - without it the group keeps re-ordering the walking
    // driver back into the car.  The steal watch above stays senior; the
    // cooldown latch already ticked in UpdateEntries' shared decrement.
    if (_dangerNow.Size() > 0)
    {
        float dangerDist = -1;
        int src = LoudestDanger(_dangerNow, veh->Position(), dangerDist);
        // roll 0: the parked family maps every in-band source to TDRBail,
        // the roll is unused.  A latch armed while still driving (say a
        // U-turn at a wreck) yields to a louder source here too (issue #55)
        if (src >= 0 && !DangerLatchHolds(e.dangerCooldown, false, e.latchSeverity, _dangerNow[src].severity) &&
            DecideDangerReaction(dangerDist, _dangerNow[src].severity, e.kind, e.state, 0.0f, 0.0f, _tuning) == TDRBail)
        {
            TrafficEventRecord ev;
            ev.type = TEPanicked;
            ev.kind = e.kind;
            ev.originIndex = e.originIndex;
            ev.destIndex = e.destIndex;
            ev.vehicle = veh;
            ev.driver = driver;
            ev.reason = DangerReactionName(TDRBail);
            fired.Add(ev);
            Vector3 dangerPos = _dangerNow[src].pos;
            AbandonEntry(i, "panicked", fired, false, &dangerPos, _dangerNow[src].playerCaused);
            LOG_INFO(Core, "Traffic: parked civ driver panicked and fled");
            return;
        }
    }

    if (e.state == TSParking)
    {
        if (e.stateTime < _tuning.commandeerStopDelay)
        {
            return; // one 5 s tick suffices
        }
        // dismount = the commandeer bail MINUS AllowGetIn(false) (which would
        // block every future GetIn) and MINUS the flee move
        grp->UnassignVehicle(veh);
        if (unit)
        {
            unit->DoGetOut(veh, false);
        }
        grp->SetCombatModeMajor(CMCareless);
        if (grp->MainSubgroup())
        {
            grp->MainSubgroup()->SetSpeedMode(SpeedLimited);
        }
        GetNetworkManager().UpdateObject(grp);
        e.state = TSDwelling;
        e.stateTime = 0;
        e.dwellDuration = 0; // rolled lazily below (also covers load-restarts)
        TrafficEventRecord ev;
        ev.type = TEParked;
        ev.kind = e.kind;
        ev.originIndex = e.originIndex;
        ev.destIndex = e.destIndex;
        ev.vehicle = veh;
        fired.Add(ev);
        return;
    }

    if (e.state == TSDwelling)
    {
        if (veh->Driver() == driver)
        {
            // seated dweller: the no-destination retry enters TSDwelling with
            // the driver seated, and a DepartTimeout re-dwell can be seated
            // by a late-completing GetIn.  The stroll Move would make him
            // DRIVE the "parked" car around town - re-park instead, mirroring
            // LoadedParkState's seated-dweller policy
            e.state = TSParking;
            e.stateTime = 0;
            return;
        }
        if (e.dwellDuration <= 0)
        {
            e.dwellDuration =
                _tuning.parkDwellMin + GRandGen.RandomValue() * (_tuning.parkDwellMax - _tuning.parkDwellMin);
        }
        if (e.stateTime < e.dwellDuration)
        {
            // occasional stroll: 1-in-ParkWanderOdds per tick, target within
            // ParkWanderRadius of the car
            if (unit && GRandGen.RandomValue() * ParkWanderOdds < 1.0f)
            {
                float a = GRandGen.RandomValue() * 2.0f * H_PI;
                float r = 5.0f + GRandGen.RandomValue() * (ParkWanderRadius - 5.0f);
                IssueSoloMove(grp, unit, veh->Position() + Vector3(sinf(a) * r, 0, cosf(a) * r));
            }
            return;
        }
        // dwell over: order the re-board.  AssignAsDriver does NOT restore the
        // hull to the group's vehicle list (UnassignVehicle removed it at the
        // dismount), so AddVehicle is required - the arcade GETIN idiom.  The
        // group's own think then runs AssignVehicles+GetInVehicles and sends
        // the stock Command::GetIn to the free soldier.
        if (unit)
        {
            unit->AllowGetIn(true); // explicit reset (belt: the dismount never cleared it)
            unit->OrderGetIn(true);
            unit->AssignAsDriver(veh); // false only if a foreign driver is seated
            grp->AddVehicle(veh);
        }
        e.state = TSDeparting;
        e.stateTime = 0;
        return;
    }

    // TSDeparting
    if (unit && veh->DriverBrain() == unit) // never IssueRoute mid-boarding
    {
        int next = DestForOrigin(e.kind, zones, e.destIndex, GRandGen.RandomValue());
        const ZoneRecord* z = next >= 0 ? ZoneRegistry::Instance().GetZone(next) : nullptr;
        if (z && GRoadNet)
        {
            e.originIndex = e.destIndex;
            e.originZone = e.destZone;
            e.destIndex = next;
            e.destZone = z->name;
            e.legs = 1; // a fresh trip, the park cycle can chain
            e.state = TSDriving;
            e.stallTime = 0;
            e.blockedRetries = 0;
            e.stateTime = 0;
            e.lastPos = veh->Position();
            IssueRoute(e, GRoadNet->GetNearestRoadPoint(z->pos), CMCareless, SpeedLimited, false);
            TrafficEventRecord ev;
            ev.type = TEDeparted;
            ev.kind = e.kind;
            ev.originIndex = e.originIndex;
            ev.destIndex = e.destIndex;
            ev.vehicle = veh;
            ev.driver = driver;
            fired.Add(ev);
        }
        else
        {
            // no destination left anywhere: quiet teardown when unobserved,
            // else re-dwell and retry later.  Plain DespawnSafe, NOT
            // GateDespawn: the entry can be arbitrarily close and centered
            // in view here, and the defer timeout would eventually vanish it
            // in plain sight - the re-dwell loop IS the defer (the far gate
            // above stays the bounded backstop)
            if (playerValid && DespawnSafe(veh->Position(), Dist2DSq(veh->Position(), playerPos), _tuning.minSpawnDist))
            {
                DespawnEntry(i, "arrived", false, fired);
            }
            else
            {
                e.state = TSDwelling;
                e.stateTime = 0;
                e.dwellDuration = 0;
            }
        }
        return;
    }
    if (e.stateTime >= DepartTimeout)
    {
        // NativeMoveIn refuses a soldier already aboard, so the DriverBrain
        // branch above must run first.  The instant seat is a teleport-shaped
        // event: perception-gated like a despawn (no defer - the retry loop
        // below IS the defer)
        bool playerFar =
            playerValid && DespawnSafe(veh->Position(), Dist2DSq(veh->Position(), playerPos), _tuning.minSpawnDist);
        if (unit && playerFar && ::NativeMoveIn(driver, veh, GIPDriver))
        {
            return; // seated instantly; the next tick takes the DriverBrain branch
        }
        // observed (or refused): back to dwelling, retry later; the far
        // despawn is the backstop
        e.state = TSDwelling;
        e.stateTime = 0;
        e.dwellDuration = 0;
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
        if (e.state == TSDriving || e.state == TSArrived || e.state == TSStalled || e.state == TSLingering ||
            e.state == TSPanicked)
        {
            // a lingering car (observed trip end, crew seated) is exactly a
            // stopped civ car: commandeerable like the rest - and so is a
            // panicked one cowering at the roadside
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
            IssueGroupStop(grp);
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
            // alive and tracked by _fleeing); the player forced this car off
            // the road at gunpoint, so its hull gets the longer memory
            ReleasedEntry r;
            r.vehicle = veh;
            r.playerCaused = true;
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
            TrafficEntry& e = _entries[i];
            // a park-state entry can hold a living ON-FOOT driver; without
            // this he is orphaned by the car forever (the group never torn
            // down) until the far cleanup deletes the hull from under him.
            // Hand him to the fleeing table, as the steal watch does, after
            // revoking any standing TSDeparting re-board orders
            Person* d = e.driver;
            AIGroup* grp = e.group;
            if ((e.state == TSParking || e.state == TSDwelling || e.state == TSDeparting) && d &&
                !d->IsDammageDestroyed() && grp && veh->Driver() != d)
            {
                AIUnit* unit = d->Brain();
                grp->UnassignVehicle(veh);
                if (unit)
                {
                    unit->OrderGetIn(false);
                    unit->AllowGetIn(false);
                }
                FleeingDriver fd;
                fd.person = d;
                fd.group = grp;
                fd.age = 0;
                _fleeing.Add(fd);
            }
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
        r.wreckAge += _tuning.interval; // the danger-source cutoff clock (WreckDangerLive)
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
            // a LIVING occupant only: a crewDead hull keeps its corpses
            // seated, and counting them as "boarded" would drop the row and
            // leak hull + bodies forever
            Person* d = veh->Driver();
            Person* g = veh->Gunner();
            Person* c = veh->Commander();
            bool living =
                (d && !d->IsDammageDestroyed()) || (g && !g->IsDammageDestroyed()) || (c && !c->IsDammageDestroyed());
            if (!living)
            {
                const ManCargo& cargo = veh->GetManCargo();
                for (int m = 0; m < cargo.Size() && !living; m++)
                {
                    Person* p = cargo[m];
                    living = p && !p->IsDammageDestroyed();
                }
            }
            if (living)
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
        // wrecks and bodies share the live entries' edge and perception gate;
        // no defer timer (a watched wreck just waits for a later pass - it is
        // set dressing, not a leaked simulation).  Player-caused remains keep
        // a PlayerCausedLingerScale wider edge (issue #53 T4): returning to
        // clean asphalt where you ambushed a patrol is a memory tell
        float relD2 = Dist2DSq(veh->Position(), playerPos);
        float relEdge = _tuning.radius + _tuning.despawnHysteresis;
        TrafficEffectiveBand relBand = _percept.band;
        if (r.playerCaused)
        {
            relEdge *= PlayerCausedLingerScale;
            relBand.despawnEdge *= PlayerCausedLingerScale;
        }
        if (ShouldDespawn(relD2, relBand) && DespawnSafe(veh->Position(), relD2, relEdge, &relBand))
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
            // never delete a person out of a vehicle seat (a steal-marked
            // driver whose late GetIn completed before the orders were
            // revoked, or one who boarded something else while fleeing)
            AIUnit* brain = p->Brain();
            if (brain && brain->GetVehicleIn())
            {
                continue;
            }
            // the age trigger alone must not vanish a civilian in front of
            // the player; wait until he is out of close range (the far
            // trigger already implies distance)
            if (!far && playerValid && Dist2DSq(p->Position(), playerPos) <= Square(_tuning.minSpawnDist))
            {
                continue;
            }
            // issue #53: with a camera, never delete a person the player can
            // perceive, whatever the trigger (headless keeps the legacy
            // rules above verbatim); no defer timer - a watched walker just
            // waits for a later pass
            if (_percept.hasCamera &&
                !CanExposeDespawn(Dist2DSq(p->Position(), _percept.camPos), PointLosBlocked(p->Position()),
                                  PointInFrustum(p->Position()), _percept.band))
            {
                continue;
            }
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
// perception (world layer, issue #53) - one snapshot per traffic pass.  The
// camera is GScene's (external cams and effects included; single-viewer,
// matching the GetRealPlayer scope).  BAND membership (which points are in
// the spawn annulus, when the far-despawn is wanted) measures from the
// PLAYER - the band is a gameplay bubble around him; every EXPOSURE leg
// (distance, cone, terrain ray) measures from the camera, the actual viewer,
// which sits at the player outside cutscenes.
// ---------------------------------------------------------------------------

void Traffic::RefreshPerception()
{
    _percept = Perception();
    _percept.band = ConfigBand(_tuning);
    const Camera* cam = GScene ? GScene->GetCamera() : nullptr;
    if (!cam)
    {
        // dedicated server / no scene: no perception - the config band and
        // the pre-perception distance rules apply verbatim
        return;
    }
    _percept.hasCamera = true;
    _percept.camPos = cam->Position();
    Vector3 heading = cam->Direction();
    heading[1] = 0;
    if (heading.SquareSize() > 1e-6f)
    {
        _percept.camDir = heading.Normalized();
        _percept.camDirValid = true;
    }
    // near-vertical camera: no usable 2D heading - camDirValid stays false
    // and the cone test treats everything as in frustum (conservative both
    // ways: no despawn legalized by the cone, no cone-escape spawns)
    // conservative lights bound: past the darkest randomized threshold ANY
    // AI car may already show headlights (TransportCore's per-vehicle roll)
    bool lightsOn = false;
    if (GScene->MainLight())
    {
        lightsOn = GScene->MainLight()->NightEffect() > HeadlightNightEffect;
    }
    _percept.band = EffectiveBand(_tuning, ENGINE_CONFIG.objectsZ, lightsOn, ENGINE_CONFIG.horizontZ);
}

bool Traffic::PointInFrustum(Vector3Par pos) const
{
    if (!_percept.hasCamera || !_percept.camDirValid)
    {
        return true; // no information (or no usable heading): never legalizes a despawn
    }
    float dx = pos.X() - _percept.camPos.X();
    float dz = pos.Z() - _percept.camPos.Z();
    float len2 = dx * dx + dz * dz;
    if (len2 < 1.0f)
    {
        return true; // on top of the camera
    }
    float dot = _percept.camDir.X() * dx + _percept.camDir.Z() * dz;
    return dot >= FrustumCosHalfAngle * sqrtf(len2);
}

bool Traffic::PointLosBlocked(Vector3Par pos) const
{
    if (!_percept.hasCamera || !GLandscape)
    {
        return false; // nothing hides anything
    }
    Vector3 tgt = pos;
    tgt[1] += LosTargetHeight;
    Vector3 delta = tgt - _percept.camPos;
    float dist = delta.Size();
    if (dist < 1.0f)
    {
        return false;
    }
    Vector3 dirN = delta * (1.0f / dist);
    // the canonical LOS idiom (InGameUIMenuSim): a ground hit before the
    // target means the terrain hides it
    Vector3 hit;
    float isect = GLandscape->IntersectWithGroundOrSea(&hit, _percept.camPos, dirN, 0, dist * 1.1f);
    return isect <= dist;
}

void Traffic::BuildExposeObs(const AutoArray<Vector3>& pts, Vector3Par playerPos,
                             AutoArray<TrafficExposeObs>& obs) const
{
    obs.Clear();
    obs.Resize(pts.Size()); // default-constructed: the no-information case
    if (!_percept.hasCamera)
    {
        return; // defaults: distance-only, the pre-perception behaviour
    }
    // a ray only matters for a point that fails BOTH cheap legs: inside the
    // exposure floor (measured from the camera, the actual viewer) AND in
    // the view cone
    float floor2 = Square(_percept.band.minSpawn);
    AutoArray<int> needRay;
    for (int i = 0; i < pts.Size(); i++)
    {
        obs[i].inFrustum = PointInFrustum(pts[i]);
        obs[i].camDist2 = Dist2DSq(pts[i], _percept.camPos);
        if (obs[i].inFrustum && obs[i].camDist2 < floor2)
        {
            needRay.Add(i);
        }
    }
    // farthest first: the selection prefers far points, spend the rays there
    for (int n = 0; n < MaxLosProbes && needRay.Size() > 0; n++)
    {
        int best = 0;
        float bestD2 = -1;
        for (int k = 0; k < needRay.Size(); k++)
        {
            float d2 = Dist2DSq(pts[needRay[k]], playerPos);
            if (d2 > bestD2)
            {
                bestD2 = d2;
                best = k;
            }
        }
        int idx = needRay[best];
        needRay.Delete(best);
        obs[idx].losBlocked = PointLosBlocked(pts[idx]);
    }
}

bool Traffic::DespawnSafe(Vector3Par pos, float playerD2, float legacyEdge, const TrafficEffectiveBand* band) const
{
    if (!_percept.hasCamera)
    {
        return playerD2 > Square(legacyEdge); // the pre-perception rule verbatim
    }
    float camD2 = Dist2DSq(pos, _percept.camPos);
    return CanExposeDespawn(camD2, PointLosBlocked(pos), PointInFrustum(pos), band ? *band : _percept.band);
}

bool Traffic::GateDespawn(TrafficEntry& e, Vector3Par pos, float playerD2, float legacyEdge)
{
    if (DespawnSafe(pos, playerD2, legacyEdge))
    {
        e.exposeDefer = 0;
        return true;
    }
    if (!_percept.hasCamera)
    {
        return false; // the legacy rules have no defer timeout
    }
    e.exposeDefer += _tuning.interval; // one accrual per traffic pass
    if (e.exposeDefer >= _tuning.despawnDeferMax)
    {
        // hard bound: staring at a blocked despawn must not leak the entry
        // forever (0 = no defer at all, the pre-perception teardown timing)
        e.exposeDefer = 0;
        return true;
    }
    return false;
}

// ---------------------------------------------------------------------------
// danger response (civ danger response) - world layer
// ---------------------------------------------------------------------------

// The core feed.  Reached per ROUND (via the FireWeaponEffects hook) while
// the fast gate is armed: keep the early-outs ahead of any work.
void Traffic::NotifyDanger(Vector3Par pos, float severity, bool playerCaused)
{
    if (_entries.Size() == 0 || severity <= 0)
    {
        return;
    }
    if (_tuning.dangerRadius <= 0 || _tuning.dangerTtl <= 0)
    {
        return;
    }
    // spatial filter: a battle across the island must not evict the local
    // ring episodes - only events inside the traffic band (where entries
    // live) can affect anybody
    if (GWorld)
    {
        Person* player = GWorld->GetRealPlayer();
        if (player && Dist2DSq(pos, player->Position()) > Square(_tuning.radius + _tuning.despawnHysteresis))
        {
            return;
        }
    }
    AddDangerEvent(_danger, pos, severity, playerCaused);
}

void Traffic::NotifyShot(EntityAI* shooter, float audibleFire)
{
    if (!shooter)
    {
        return;
    }
    // the player-caused test is the AutoReload idiom: the local player's
    // brain commands the firing entity (his own rifle or his vehicle seat)
    bool playerCaused = false;
    if (GWorld && GWorld->PlayerOn() && GWorld->PlayerOn()->Brain())
    {
        playerCaused = GWorld->PlayerOn()->Brain() == shooter->CommanderUnit();
    }
    NotifyDanger(shooter->Position(), DangerSeverityFromAudible(audibleFire), playerCaused);
}

void Traffic::NotifyExplosion(AIUnit* ownerUnit, Vector3Par pos, const AmmoType* type)
{
    if (!type)
    {
        return;
    }
    // the severity mapping filters out the non-blasts (ExplosionDammage
    // runs for every projectile impact); real blasts max the reaction band
    float severity = DangerSeverityFromBlast(type->explosive, type->indirectHit, type->indirectHitRange);
    if (severity <= 0)
    {
        return;
    }
    NotifyDanger(pos, severity, ownerUnit && ownerUnit->IsPlayer());
}

void Traffic::BuildDangerSources()
{
    _dangerNow = _danger;
    for (int i = 0; i < _released.Size(); i++)
    {
        const ReleasedEntry& r = _released[i];
        Transport* hull = r.vehicle;
        if (r.playerCaused && hull && hull->IsDammageDestroyed() && WreckDangerLive(r.wreckAge, _tuning))
        {
            // a wreck the player made is a danger tell of its own - and the
            // released table already knows it, no engine hook required
            TrafficDangerEvent w;
            w.pos = hull->Position();
            w.severity = WreckDangerSeverity;
            w.playerCaused = true;
            _dangerNow.Add(w);
        }
    }
}

// ---------------------------------------------------------------------------
// simulation
// ---------------------------------------------------------------------------

void Traffic::Simulate(float deltaT)
{
    bool active = IsActive() && GWorld;
    // arm/disarm the frozen-core danger hooks (shots land per round, so the
    // idle gate must be one global bool read; see TrafficNotifyShotFast)
    GTrafficDangerArmed = active && _entries.Size() > 0 && _tuning.dangerRadius > 0 && _tuning.dangerTtl > 0;
    if (!active)
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

    // one perception snapshot per pass: camera, lights bound, effective band
    RefreshPerception();

    // age the danger ring (it carries shots between passes) and snapshot the
    // per-pass source list for the reaction tier
    AgeDangerEvents(_danger, tick, _tuning.dangerTtl);
    BuildDangerSources();

    AutoArray<TrafficEventRecord> fired;
    UpdateEntries(playerPos, playerValid, fired);
    CleanupReleased(playerPos, playerValid);
    CleanupFleeing(playerPos, playerValid, tick);

    if (playerValid)
    {
        AutoArray<TrafficZoneCandidate> zones;
        BuildZoneCandidates(zones);
        int o, d;
        int civOrigin = -1, civDest = -1;
        TrafficDecisionInput in;
        in.enabled = _tuning.enabled;
        in.playerValid = true;
        in.liveCiv = Count(TKCiv);
        in.livePatrols = Count(TKPatrol);
        in.liveConvoys = Count(TKConvoy);
        // the civ route is rolled once here, so modulation assesses the same
        // origin a civ spawn will actually use (a deterministic availability
        // probe followed by a spawn-time re-roll could disagree on the origin)
        in.hasCivRoute = PickRoute(TKCiv, zones, playerPos.X(), playerPos.Z(), _tuning, GRandGen.RandomValue(),
                                   civOrigin, civDest, -1, &_percept.band);
        in.hasPatrolRoute =
            PickRoute(TKPatrol, zones, playerPos.X(), playerPos.Z(), _tuning, 0.0f, o, d, -1, &_percept.band);
        in.hasConvoyRoute =
            PickRoute(TKConvoy, zones, playerPos.X(), playerPos.Z(), _tuning, 0.0f, o, d, -1, &_percept.band);
        in.warLevel = ReadWarLevel();

        TrafficModulationInput mod;
        mod.dayFraction = Glob.clock.GetTimeOfDay();
        if (GScene && GScene->MainLight())
        {
            mod.nightEffect = GScene->MainLight()->NightEffect();
        }
        else
        {
            // no scene (dedicated server): night is the wall clock outside
            // the day window
            mod.nightEffect = (mod.dayFraction <= _tuning.dayStart || mod.dayFraction >= _tuning.dayEnd) ? 1.0f : 0.0f;
        }
        if (GLandscape)
        {
            mod.rain = GLandscape->GetRainDensity();
        }
        mod.warLevel = in.warLevel;
        if (in.hasCivRoute)
        {
            mod.originAlertCiv = AlertMachine::Instance().GetZoneState(civOrigin);
            for (int i = 0; i < zones.Size(); i++)
            {
                if (zones[i].index == civOrigin)
                {
                    mod.originOccupied = zones[i].occupierOwned;
                    break;
                }
            }
        }
        ModulationFactors(mod, _tuning, in.civScale, in.patrolScale);

        in.roll = GRandGen.RandomValue();
        // density scaling (issue #53 T6): a widened band would otherwise be
        // a ghost town at the config caps - 3 civ cars in a 3000 m circle.
        // Linear with the band radius (traffic lives on roads), never below
        // the config, identity when the band is not widened; convoys stay
        // at their config cap (rare by design)
        TrafficTuning decide = _tuning;
        if (_tuning.scaleCaps)
        {
            decide.maxCiv = ScaleCap(_tuning.maxCiv, _percept.band.radius, _tuning.radius);
            decide.maxPatrols = ScaleCap(_tuning.maxPatrols, _percept.band.radius, _tuning.radius);
        }
        int kind = DecideSpawn(in, decide);
        if (kind == TKCiv)
        {
            // reuse the modulated route: the spawn happens from the origin
            // whose alert/occupation state the decision was scaled by
            Transport* veh = nullptr;
            SpawnEntry(kind, civOrigin, civDest, playerPos, veh, fired);
        }
        else if (kind >= 0 && PickRoute(kind, zones, playerPos.X(), playerPos.Z(), _tuning, GRandGen.RandomValue(), o,
                                        d, -1, &_percept.band))
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
    SpawnEntry(kind, o, d, playerPos, veh, fired, true); // forced: no perception gate
    DispatchEvents(fired);
    return veh;
}

void Traffic::ConsumeAmbushes(AutoArray<TrafficAmbush>& out)
{
    out.Clear();
    for (int i = 0; i < _ambushes.Size(); i++)
    {
        out.Add(_ambushes[i]);
    }
    _ambushes.Clear();
}

void Traffic::QueueAmbushForTest(Vector3Par pos, TrafficKind kind, const char* originZone)
{
    TrafficAmbush am;
    am.pos = pos;
    am.kind = kind;
    am.originZone = originZone;
    _ambushes.Add(am);
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

void Traffic::PerceptProbe(Vector3Par pos, bool& hasCamera, bool& losBlocked, bool& inFrustum,
                           TrafficEffectiveBand& band)
{
    // a fresh snapshot rather than the last pass's: the probe answers "what
    // would the NEXT pass see", and inside an event handler (dispatched at
    // the end of a pass, camera unmoved since its start) the two agree
    RefreshPerception();
    hasCamera = _percept.hasCamera;
    losBlocked = PointLosBlocked(pos);
    inFrustum = PointInFrustum(pos);
    band = _percept.band;
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
            case TEParked:
            case TEDeparted:
            case TEBailed:
                pars.Resize(3);
                pars[0] = GameValueExt(ev.vehicle.GetLink());
                pars[1] = GameStringType(KindName(ev.kind));
                pars[2] = (float)ev.destIndex;
                break;
            case TEPanicked:
                pars.Resize(3);
                pars[0] = GameValueExt(ev.vehicle.GetLink());
                pars[1] = GameStringType(KindName(ev.kind));
                pars[2] = GameStringType(ev.reason); // the reaction: cower/uturn/rush/bail
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

LSError TrafficAmbush::Serialize(ParamArchive& ar)
{
    int k = (int)kind;
    PARAM_CHECK(ar.Serialize("pos", pos, 1, VZero))
    PARAM_CHECK(ar.Serialize("kind", k, 1, (int)TKPatrol))
    kind = (TrafficKind)k;
    PARAM_CHECK(ar.Serialize("originZone", originZone, 1, RString()))
    return LSOK;
}

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
    // absent from older saves: default 0 hands a loaded stall its full retries
    PARAM_CHECK(ar.Serialize("blockedRetries", blockedRetries, 1, 0))
    // absent from pre-#53 saves: the empty default despawns as "arrived"
    PARAM_CHECK(ar.Serialize("lingerReason", lingerReason, 1, RString()))
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
    // absent from pre-#53 saves: defaults to ambient (the short memory)
    PARAM_CHECK(ar.Serialize("playerCaused", playerCaused, 1, false))
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
        // a commandeer in flight does not survive a save, and neither does a
        // panic cower (the ring buffer is transient): restart the drive
        if (e.state == TSStopping || e.state == TSExiting || e.state == TSPanicked)
        {
            e.state = TSDriving;
        }
        // park/linger states downgrade on whether the driver came back seated
        // (the crews were rebuilt by the world serializer before this pass):
        // the brake wait restarts, a dwell restarts with a fresh roll, an
        // on-foot departer re-dwells (get-in flags re-issued at expiry), a
        // seated departer resumes departing, a seated lingerer keeps waiting
        // to be unobserved
        if (e.state == TSParking || e.state == TSDwelling || e.state == TSDeparting || e.state == TSLingering)
        {
            Transport* v = e.vehicle; // non-null (null rows dropped above)
            Person* d = e.driver;
            // a null driver (his body deleted in the window before the
            // entry's own crewDead despawn tick) KEEPS the row: dropping it
            // would leave the hull in the world untracked forever, while
            // UpdateParked's dead-driver guard releases hull+bodies with the
            // normal crewDead despawn on the first post-load tick
            e.state = LoadedParkState(e.state, d && v->Driver() == d);
            e.dwellDuration = 0;
        }
        e.stateTime = 0;
        _entries.Add(e);
    }
    // the danger ring is transient: no episode survives a load
    _danger.Clear();
    _dangerNow.Clear();
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
    PARAM_CHECK(ar.Serialize("onParked", _handlers[TEParked], 1, RString()))
    PARAM_CHECK(ar.Serialize("onDeparted", _handlers[TEDeparted], 1, RString()))
    // absent from pre-convoy-discipline saves: no handler
    PARAM_CHECK(ar.Serialize("onBailed", _handlers[TEBailed], 1, RString()))
    PARAM_CHECK(ar.Serialize("onPanicked", _handlers[TEPanicked], 1, RString()))
    PARAM_CHECK(ar.Serialize("Entries", _pending, 1))
    PARAM_CHECK(ar.Serialize("Released", _released, 1))
    PARAM_CHECK(ar.Serialize("Fleeing", _fleeing, 1))
    // an ambush queued in the tick-interval window before the save must
    // survive the load (same contract as the UndercoverSystem's pending
    // compromises); absent from older saves, which load an empty queue
    PARAM_CHECK(ar.Serialize("Ambushes", _ambushes, 1))

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
