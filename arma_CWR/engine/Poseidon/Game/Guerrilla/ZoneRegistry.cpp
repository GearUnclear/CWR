#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>

#include <Poseidon/Core/Global.hpp>      // Glob.header.worldname
#include <Poseidon/Core/SaveVersion.hpp> // GuerrillaSaveVersion
#include <Poseidon/Game/Guerrilla/AlertMachine.hpp>
#include <Poseidon/IO/ParamFileExt.hpp> // Pars / ExtParsMission
#include <Poseidon/IO/Serialization/ParamArchive.hpp>

#include <Evaluator/express.hpp> // GameState / GameValue (event dispatch)

#include <Poseidon/World/World.hpp>
#include <Poseidon/World/Entities/Infantry/Person.hpp>
#include <Poseidon/AI/AI.hpp>
#include <Poseidon/AI/AICore.hpp>              // markersMap
#include <Poseidon/AI/Path/ArcadeWaypoint.hpp> // ArcadeMarkerInfo

#include <Poseidon/Foundation/Enums/EnumNames.hpp> // GetEnumValue<TargetSide>
#include <Poseidon/Foundation/Framework/DebugLog.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>
#include <Poseidon/Foundation/platform.hpp>

#include <stdio.h>
#include <string.h>

namespace Poseidon::Guerrilla
{

// Defined in ZoneRegistryCommands.cpp.  Referencing it from here forces the
// command TU (whose only other content is an INIT_MODULE registration) into
// the link - same pattern as EnsureGameStateExtTestGettersLinked.
void EnsureZoneRegistryCommandsLinked();

// Process-lifetime singleton - no global constructor (see express.hpp's
// GGameState for the convention).
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
ZoneRegistry& ZoneRegistry::Instance()
{
    EnsureZoneRegistryCommandsLinked();
    static ZoneRegistry instance;
    return instance;
}
#pragma clang diagnostic pop

static float Dist2DSq(float ax, float az, float bx, float bz)
{
    float dx = ax - bx;
    float dz = az - bz;
    return dx * dx + dz * dz;
}

AICenter* FindSideCenter(const char* sideName)
{
    using Poseidon::Foundation::GetEnumValue;
    if (!sideName || !GWorld)
    {
        return nullptr;
    }
    TargetSide side = GetEnumValue<TargetSide>(sideName);
    if ((int)side < 0)
    {
        // unknown side name: Foundation's GetEnumValue returns -1 (NOT the
        // legacy INT_MIN sentinel). GetCenter would also fall through to
        // null for any unmatched value; the explicit check documents it.
        return nullptr;
    }
    return GWorld->GetCenter(side);
}

// script/campaign global published by the new-game UI (OptionsUIApp VarSets
// kGuerrillaVarOccupier / kGuerrillaVarResistance); read like GarrisonCache's
// ReadWarLevel - VarGet with the lowercased name, nil tolerated
static RString ReadSideSelection(const char* lowercaseName)
{
    if (!GWorld)
    {
        return RString();
    }
    GameState* gstate = GWorld->GetGameState();
    if (!gstate)
    {
        return RString();
    }
    GameValue value = gstate->VarGet(lowercaseName);
    if (value.GetType() != GameString)
    {
        return RString();
    }
    return (RString)value;
}

// ---------------------------------------------------------------------------
// lifecycle / config
// ---------------------------------------------------------------------------

void ZoneRegistry::Clear()
{
    _zones.Clear();
    _factions.Clear();
    _tuning = ZoneTuning();
    _occupierSide = "EAST";
    _resistanceSide = "GUER";
    for (int i = 0; i < NZoneEventTypes; i++)
    {
        _handlers[i] = RString();
    }
    _accum = 0;
    _pending.Clear();
    _pendingOccupierSide = RString();
    _pendingResistanceSide = RString();
    _pendingLoaded = false;
    _loadedSaveVersion = 0;
    // the alert layer lives and dies with the zone registry
    AlertMachine::Instance().Clear();
}

void ZoneRegistry::InitMission()
{
    Clear();
    LoadFromConfig();
}

void ZoneRegistry::LoadFromConfig()
{
    const ParamEntry* zones = ExtParsMission.FindEntry("CfgGuerrillaZones");
    if (!zones)
    {
        zones = Pars.FindEntry("CfgGuerrillaZones");
    }
    const ParamEntry* factions = ExtParsMission.FindEntry("CfgGuerrillaFactions");
    if (!factions)
    {
        factions = Pars.FindEntry("CfgGuerrillaFactions");
    }
    // new-game faction selections (nil outside a Guerrilla campaign)
    RString selOccupier = ReadSideSelection("gmseloccupier");
    RString selResistance = ReadSideSelection("gmselresistance");
    // the active world's named locations, for the optional CITY auto-seed
    // (same lookup the map drawing uses - Pars >> "CfgWorlds" >> worldname)
    const ParamEntry* names = nullptr;
    if (const ParamEntry* worlds = Pars.FindEntry("CfgWorlds"))
    {
        if (const ParamEntry* world = worlds->FindEntry(Glob.header.worldname))
        {
            names = world->FindEntry("Names");
        }
    }
    LoadFromParams(zones, factions, selOccupier, selResistance, names);
    // alert tunables share the CfgGuerrillaZones class; loading here (not in
    // LoadFromParams) keeps the testable core free of singleton side effects
    AlertMachine::Instance().LoadFromParams(zones);
}

void ZoneRegistry::LoadFromParams(const ParamEntry* zonesCfg, const ParamEntry* factionsCfg, const char* selOccupier,
                                  const char* selResistance, const ParamEntry* worldNamesCfg)
{
    // rebuilds the config-derived tables only; event handlers and any
    // pending savegame rows are preserved (see Serialize)
    _zones.Clear();
    _factions.Clear();
    _tuning = ZoneTuning();
    _occupierSide = "EAST";
    _resistanceSide = "GUER";
    _accum = 0;

    // factions first: the zone table's OCCUPIER/RESISTANCE owner tokens
    // resolve against the sides picked out of the faction table
    if (factionsCfg)
    {
        LoadFactions(*factionsCfg);
    }
    // side precedence: gmSel* selections (new-game UI) > the mission's
    // defaultOccupier/defaultResistance config keys (direct launches, no
    // UI) > the built-in EAST/GUER defaults.  ResolveSides overwrites only
    // on a faction match, so applying the config keys first and the
    // selections on top yields exactly that order.
    if (zonesCfg)
    {
        RString defOccupier = zonesCfg->ReadValue("defaultOccupier", RString());
        RString defResistance = zonesCfg->ReadValue("defaultResistance", RString());
        ResolveSides(defOccupier, defResistance);
    }
    ResolveSides(selOccupier, selResistance);
    if (zonesCfg)
    {
        LoadZones(*zonesCfg);
    }
    if (_tuning.seedCities && worldNamesCfg)
    {
        SeedCityZones(*worldNamesCfg);
    }
}

void ZoneRegistry::ResolveSides(const char* selOccupier, const char* selResistance)
{
    // a selection is a CfgGuerrillaFactions class name or a side string;
    // either way the resolved side is the matched faction's side field.
    // Unmatched or empty selections keep the current (default) sides.
    if (selOccupier && *selOccupier)
    {
        if (const FactionRecord* f = FindFaction(selOccupier))
        {
            _occupierSide = f->side;
        }
    }
    if (selResistance && *selResistance)
    {
        if (const FactionRecord* f = FindFaction(selResistance))
        {
            _resistanceSide = f->side;
        }
    }
}

RString ZoneRegistry::ResolveOwnerToken(const RString& owner) const
{
    if (stricmp(owner, "OCCUPIER") == 0)
    {
        return _occupierSide;
    }
    if (stricmp(owner, "RESISTANCE") == 0)
    {
        return _resistanceSide;
    }
    return owner;
}

void ZoneRegistry::ApplyOwnerTokens()
{
    for (int i = 0; i < _zones.Size(); i++)
    {
        _zones[i].owner = ResolveOwnerToken(_zones[i].ownerConfig);
    }
}

void ZoneRegistry::LoadZones(const ParamEntry& cfg)
{
    _tuning.tickInterval = cfg.ReadValue("tickInterval", _tuning.tickInterval);
    _tuning.zoneArea = cfg.ReadValue("zoneArea", _tuning.zoneArea);
    _tuning.revealRadius = cfg.ReadValue("revealRadius", _tuning.revealRadius);
    _tuning.cacheRadius = cfg.ReadValue("cacheRadius", _tuning.cacheRadius);
    _tuning.supportRate = cfg.ReadValue("supportRate", _tuning.supportRate);
    _tuning.supportFlip = cfg.ReadValue("supportFlip", _tuning.supportFlip);
    _tuning.heatCapSpike = cfg.ReadValue("heatCapSpike", _tuning.heatCapSpike);
    _tuning.defaultIncome = cfg.ReadValue("defaultIncome", _tuning.defaultIncome);
    _tuning.holdCount = cfg.ReadValue("holdCount", _tuning.holdCount);
    _tuning.seedCities = cfg.ReadValue("seedCities", _tuning.seedCities ? 1.0f : 0.0f) != 0.0f;
    _tuning.seedCitySupport = cfg.ReadValue("seedCitySupport", _tuning.seedCitySupport);

    const ParamEntry* zones = cfg.FindEntry("Zones");
    if (!zones)
    {
        return;
    }
    for (int i = 0; i < zones->GetEntryCount(); i++)
    {
        const ParamEntry& e = zones->GetEntry(i);
        if (!e.IsClass())
        {
            continue;
        }
        ZoneRecord z;
        z.name = e.ReadValue("name", RString(e.GetName()));
        z.type = e.ReadValue("type", RString("OUTPOST"));
        // owner accepts the generic "OCCUPIER"/"RESISTANCE" tokens next to
        // literal side strings; the raw value is kept for re-resolution
        z.ownerConfig = e.ReadValue("owner", RString("NEUTRAL"));
        z.owner = ResolveOwnerToken(z.ownerConfig);
        z.garrison = e.ReadValue("garrison", 0.0f);
        z.support = e.ReadValue("support", 0.0f);
        z.income = e.ReadValue("income", 0.0f);
        z.heat = e.ReadValue("heat", 0.0f);
        z.marker = e.ReadValue("marker", RString());
        const ParamEntry* pos = e.FindEntry("position");
        if (pos && pos->IsArray() && pos->GetSize() >= 2)
        {
            float easting = (*pos)[0];
            float northing = (*pos)[1];
            float elevation = pos->GetSize() >= 3 ? (float)(*pos)[2] : 0.0f;
            // position[] is getPos order [easting, northing, elevation];
            // engine Vector3 is (X=easting, Y=elevation, Z=northing)
            z.pos = Vector3(easting, elevation, northing);
        }
        _zones.Add(z);
    }
}

void ZoneRegistry::LoadFactions(const ParamEntry& cfg)
{
    for (int i = 0; i < cfg.GetEntryCount(); i++)
    {
        const ParamEntry& e = cfg.GetEntry(i);
        if (!e.IsClass())
        {
            continue;
        }
        FactionRecord f;
        f.className = e.GetName();
        f.side = e.ReadValue("side", RString(e.GetName()));
        f.vehicleThreshold = e.ReadValue("vehicleThreshold", f.vehicleThreshold);

        const ParamEntry* tiers = e.FindEntry("tiers");
        if (tiers && tiers->IsArray())
        {
            for (int k = 0; k < tiers->GetSize(); k++)
            {
                f.tiers.Add(RString((RStringB)(*tiers)[k]));
            }
        }
        const ParamEntry* thresholds = e.FindEntry("tierThresholds");
        if (thresholds && thresholds->IsArray())
        {
            for (int k = 0; k < thresholds->GetSize(); k++)
            {
                f.tierThresholds.Add((*thresholds)[k]);
            }
        }
        const ParamEntry* vehicles = e.FindEntry("vehicles");
        if (vehicles && vehicles->IsArray())
        {
            for (int k = 0; k < vehicles->GetSize(); k++)
            {
                f.vehicles.Add(RString((RStringB)(*vehicles)[k]));
            }
        }
        const ParamEntry* vehThresholds = e.FindEntry("vehicleThresholds");
        if (vehThresholds && vehThresholds->IsArray())
        {
            for (int k = 0; k < vehThresholds->GetSize(); k++)
            {
                f.vehicleThresholds.Add((*vehThresholds)[k]);
            }
        }

        // all remaining plain values (officer, holdClass, recruitFighter,
        // recruitSpecialist, companionClass, baseWeapon, baseMagazine, ...)
        for (int k = 0; k < e.GetEntryCount(); k++)
        {
            const ParamEntry& v = e.GetEntry(k);
            if (v.IsClass() || v.IsArray())
            {
                continue;
            }
            FactionRecord::NamedValue nv;
            nv.key = v.GetName();
            nv.value = v.GetValue();
            f.values.Add(nv);
        }
        _factions.Add(f);
    }
}

// CITY auto-seed from the world's named locations (CfgWorlds >> <world> >>
// Names).  OFP-era Names entries carry only name + a 2D position[] {x,z}
// (see CStaticMap::DrawName) - every entry is a town, so type-less entries
// are accepted; an Arma-style type entry, when present, must be a city-like
// location type.
void ZoneRegistry::SeedCityZones(const ParamEntry& namesCfg)
{
    const float dedupSq = 300.0f * 300.0f; // skip near explicit/seeded zones
    int seeded = 0;
    for (int i = 0; i < namesCfg.GetEntryCount(); i++)
    {
        const ParamEntry& e = namesCfg.GetEntry(i);
        if (!e.IsClass())
        {
            continue;
        }
        RString type = e.ReadValue("type", RString());
        if (type.GetLength() > 0 && stricmp(type, "NameCity") != 0 && stricmp(type, "NameCityCapital") != 0 &&
            stricmp(type, "NameVillage") != 0)
        {
            continue; // typed non-town location (rocks, hills, ...)
        }
        const ParamEntry* pos = e.FindEntry("position");
        if (!pos || !pos->IsArray() || pos->GetSize() < 2)
        {
            continue;
        }
        float easting = (*pos)[0];
        float northing = (*pos)[1];
        float elevation = pos->GetSize() >= 3 ? (float)(*pos)[2] : 0.0f;

        RString name = e.ReadValue("name", RString(e.GetName()));
        if (name.GetLength() == 0)
        {
            name = e.GetName(); // Names entries often ship name=""
        }
        // dedup: a location on top of a configured (or already seeded) zone
        // stays that zone's business; a name clash would break the name-keyed
        // savegame matching
        bool skip = FindZoneIndex(name) >= 0;
        for (int j = 0; j < _zones.Size() && !skip; j++)
        {
            skip = Dist2DSq(easting, northing, _zones[j].pos.X(), _zones[j].pos.Z()) < dedupSq;
        }
        if (skip)
        {
            continue;
        }
        if (_zones.Size() >= MaxZones)
        {
            RptF("ZoneRegistry: zone cap (%d) reached seeding cities - remaining Names entries skipped", MaxZones);
            return;
        }

        ZoneRecord z;
        z.name = name;
        z.type = "CITY";
        z.ownerConfig = "NEUTRAL";
        z.owner = "NEUTRAL";
        z.support = _tuning.seedCitySupport;
        char marker[32];
        snprintf(marker, sizeof(marker), "gmZoneCity_%d", seeded);
        z.marker = marker;
        z.pos = Vector3(easting, elevation, northing);
        _zones.Add(z);
        seeded++;
    }
}

// ---------------------------------------------------------------------------
// queries
// ---------------------------------------------------------------------------

const ZoneRecord* ZoneRegistry::GetZone(int index) const
{
    if (index < 0 || index >= _zones.Size())
    {
        return nullptr;
    }
    return &_zones[index];
}

ZoneRecord* ZoneRegistry::GetZoneMutable(int index)
{
    if (index < 0 || index >= _zones.Size())
    {
        return nullptr;
    }
    return &_zones[index];
}

int ZoneRegistry::FindZoneIndex(const char* name) const
{
    if (!name)
    {
        return -1;
    }
    for (int i = 0; i < _zones.Size(); i++)
    {
        if (stricmp(_zones[i].name, name) == 0)
        {
            return i;
        }
    }
    return -1;
}

void ZoneRegistry::HeatRaise(int index, float amount)
{
    ZoneRecord* z = GetZoneMutable(index);
    if (!z)
    {
        return;
    }
    z->heat += amount;
    if (z->heat > 100)
    {
        z->heat = 100;
    }
}

void ZoneRegistry::HeatDecay(int index, float amount)
{
    ZoneRecord* z = GetZoneMutable(index);
    if (!z)
    {
        return;
    }
    z->heat -= amount;
    if (z->heat < 0)
    {
        z->heat = 0;
    }
}

const FactionRecord* ZoneRegistry::FindFaction(const char* sideOrClass) const
{
    if (!sideOrClass)
    {
        return nullptr;
    }
    for (int i = 0; i < _factions.Size(); i++)
    {
        if (stricmp(_factions[i].side, sideOrClass) == 0)
        {
            return &_factions[i];
        }
    }
    for (int i = 0; i < _factions.Size(); i++)
    {
        if (stricmp(_factions[i].className, sideOrClass) == 0)
        {
            return &_factions[i];
        }
    }
    return nullptr;
}

RString ZoneRegistry::FactionTierClass(const char* side, float warLevel) const
{
    const FactionRecord* f = FindFaction(side);
    if (!f || f->tiers.Size() == 0)
    {
        return RString();
    }
    int tier = 0;
    for (int i = 0; i < f->tierThresholds.Size(); i++)
    {
        if (warLevel >= f->tierThresholds[i])
        {
            tier++;
        }
    }
    if (tier >= f->tiers.Size())
    {
        tier = f->tiers.Size() - 1;
    }
    return f->tiers[tier];
}

RString ZoneRegistry::FactionVehicle(const char* side, float warLevel) const
{
    const FactionRecord* f = FindFaction(side);
    if (!f || f->vehicles.Size() == 0)
    {
        return RString();
    }
    int index = 0;
    if (f->vehicleThresholds.Size() > 0)
    {
        // vehicleThresholds[] mirrors tierThresholds[]: ascending war levels,
        // vehicle i+1 unlocks at thresholds[i]
        for (int i = 0; i < f->vehicleThresholds.Size(); i++)
        {
            if (warLevel >= f->vehicleThresholds[i])
            {
                index++;
            }
        }
    }
    else
    {
        // legacy scalar key: a two-step ladder (vehicles[] past index 1 is
        // unreachable without vehicleThresholds[])
        index = warLevel >= f->vehicleThreshold ? 1 : 0;
    }
    if (index >= f->vehicles.Size())
    {
        index = f->vehicles.Size() - 1;
    }
    return f->vehicles[index];
}

RString ZoneRegistry::FactionValue(const char* side, const char* key) const
{
    const FactionRecord* f = FindFaction(side);
    if (!f || !key)
    {
        return RString();
    }
    for (int i = 0; i < f->values.Size(); i++)
    {
        if (stricmp(f->values[i].key, key) == 0)
        {
            return f->values[i].value;
        }
    }
    return RString();
}

// ---------------------------------------------------------------------------
// events
// ---------------------------------------------------------------------------

void ZoneRegistry::SetEventHandler(ZoneEventType type, RString handler)
{
    if (type < 0 || type >= NZoneEventTypes)
    {
        return;
    }
    _handlers[type] = handler;
}

RString ZoneRegistry::GetEventHandler(ZoneEventType type) const
{
    if (type < 0 || type >= NZoneEventTypes)
    {
        return RString();
    }
    return _handlers[type];
}

int ZoneRegistry::EventTypeFromName(const char* name)
{
    if (!name)
    {
        return -1;
    }
    if (stricmp(name, "captured") == 0)
    {
        return ZECaptured;
    }
    if (stricmp(name, "supportThreshold") == 0)
    {
        return ZESupportThreshold;
    }
    if (stricmp(name, "revealed") == 0)
    {
        return ZERevealed;
    }
    if (stricmp(name, "campaignLoaded") == 0)
    {
        return ZECampaignLoaded;
    }
    return -1;
}

void ZoneRegistry::MarkCampaignLoaded(int loadedVersion)
{
    _pendingLoaded = true;
    _loadedSaveVersion = loadedVersion;
}

bool ZoneRegistry::ConsumeCampaignLoaded(int& loadedVersion)
{
    if (!_pendingLoaded)
    {
        return false;
    }
    _pendingLoaded = false;
    loadedVersion = _loadedSaveVersion;
    return true;
}

// ---------------------------------------------------------------------------
// simulation
// ---------------------------------------------------------------------------

void ZoneRegistry::Simulate(float deltaT)
{
    if (!IsActive())
    {
        return;
    }
    // fire the queued campaignLoaded notification on the first tick after a
    // load - by now every guerrilla subsystem has finished deserializing
    int loadedVersion = 0;
    if (ConsumeCampaignLoaded(loadedVersion) && GWorld)
    {
        RString handler = GetEventHandler(ZECampaignLoaded);
        GameState* gstate = GWorld->GetGameState();
        if (gstate && handler.GetLength() > 0)
        {
            GameArrayType pars;
            pars.Resize(1);
            pars[0] = (float)loadedVersion;
            GameVarSpace local;
            gstate->BeginContext(&local);
            gstate->VarSetLocal("_this", GameValue(pars), true);
            gstate->Execute(handler);
            gstate->EndContext();
        }
    }
    // the alert layer runs on its own cadence (alertInterval), so it ticks
    // ahead of this registry's coarser throttle
    AlertMachine::Instance().Simulate(deltaT);
    _accum += deltaT;
    if (_accum < _tuning.tickInterval)
    {
        return;
    }
    _accum = 0;

    ZoneTickInputs in;
    GatherInputs(in);

    AutoArray<ZoneEventRecord> fired;
    EvaluateTick(in, fired);
    UpdateMarkers();
    // handlers run only after the registry's own state mutation completed
    DispatchEvents(fired);
}

void ZoneRegistry::EvaluateTick(const ZoneTickInputs& in, AutoArray<ZoneEventRecord>& fired)
{
    const int n = _zones.Size();
    const float revealSq = _tuning.revealRadius * _tuning.revealRadius;
    const float cacheSq = _tuning.cacheRadius * _tuning.cacheRadius;

    // fog-of-war: resistance-owned, or within revealRadius of a
    // resistance-owned zone
    for (int i = 0; i < n; i++)
    {
        ZoneRecord& z = _zones[i];
        bool revealed = stricmp(z.owner, _resistanceSide) == 0;
        for (int j = 0; !revealed && j < n; j++)
        {
            const ZoneRecord& other = _zones[j];
            if (stricmp(other.owner, _resistanceSide) != 0)
            {
                continue;
            }
            if (Dist2DSq(z.pos.X(), z.pos.Z(), other.pos.X(), other.pos.Z()) < revealSq)
            {
                revealed = true;
            }
        }
        if (revealed && !z.revealed)
        {
            ZoneEventRecord ev;
            ev.type = ZERevealed;
            ev.zoneIndex = i;
            fired.Add(ev);
        }
        z.revealed = revealed;
    }

    // capture math runs only near the player (the SQS distance-cache gate)
    if (!in.playerValid)
    {
        return;
    }

    for (int i = 0; i < n; i++)
    {
        ZoneRecord& z = _zones[i];
        if (Dist2DSq(z.pos.X(), z.pos.Z(), in.playerX, in.playerZ) > cacheSq)
        {
            continue;
        }

        bool guerHere = i < in.guerPresent.Size() && in.guerPresent[i];
        bool isCity = stricmp(z.type, "CITY") == 0;

        // military: flips when the occupier reserve and live occupiers are
        // gone and a live resistance unit stands in the zone area
        bool milCap =
            !isCity && stricmp(z.owner, _occupierSide) == 0 && guerHere && z.liveOccupiers < 1 && z.garrison < 1;

        // city: support accrues while a resistance unit is present, flips on
        // the threshold - never on kills
        bool cityCap = false;
        if (isCity && stricmp(z.owner, "NEUTRAL") == 0)
        {
            if (guerHere)
            {
                z.support += _tuning.supportRate;
                if (z.support > 100)
                {
                    z.support = 100;
                }
            }
            cityCap = z.support >= _tuning.supportFlip;
        }

        if (!milCap && !cityCap)
        {
            continue;
        }

        z.owner = _resistanceSide;
        z.heat += _tuning.heatCapSpike;
        if (z.heat > 100)
        {
            z.heat = 100;
        }
        if (milCap)
        {
            if (z.income < 1)
            {
                z.income = _tuning.defaultIncome;
            }
            z.garrison = 0;
            z.liveOccupiers = 0;
        }
        if (cityCap)
        {
            ZoneEventRecord ev;
            ev.type = ZESupportThreshold;
            ev.zoneIndex = i;
            fired.Add(ev);
        }
        ZoneEventRecord ev;
        ev.type = ZECaptured;
        ev.zoneIndex = i;
        fired.Add(ev);
    }
}

void ZoneRegistry::GatherInputs(ZoneTickInputs& in) const
{
    const int n = _zones.Size();
    in.guerPresent.Resize(n);
    for (int i = 0; i < n; i++)
    {
        in.guerPresent[i] = false;
    }

    World* world = GWorld;
    if (!world)
    {
        return;
    }

    Person* player = world->GetRealPlayer();
    if (player && !player->IsDammageDestroyed())
    {
        in.playerValid = true;
        in.playerX = player->Position().X();
        in.playerZ = player->Position().Z();
    }

    // enumerate the resistance side's live units; the player is included
    // here when playing that side.  No center yet == no units (do NOT
    // create one here - GarrisonCache::EnsureCenter is the creating path)
    AICenter* guer = FindSideCenter(_resistanceSide);
    if (!guer)
    {
        return;
    }
    const float areaSq = _tuning.zoneArea * _tuning.zoneArea;
    for (int g = 0; g < guer->NGroups(); g++)
    {
        AIGroup* grp = guer->GetGroup(g);
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
            Vector3 pos = unit->Position();
            for (int i = 0; i < n; i++)
            {
                if (in.guerPresent[i])
                {
                    continue;
                }
                if (Dist2DSq(pos.X(), pos.Z(), _zones[i].pos.X(), _zones[i].pos.Z()) < areaSq)
                {
                    in.guerPresent[i] = true;
                }
            }
        }
    }
}

void ZoneRegistry::UpdateMarkers()
{
    // mimics setMarkerColor / setMarkerText (GameStateExtWorld.cpp:569,
    // GameStateExtWorldConfig.cpp:773) - repaint only on change because
    // OnColorChanged re-reads CfgMarkerColors
    for (int i = 0; i < _zones.Size(); i++)
    {
        const ZoneRecord& z = _zones[i];
        if (z.marker.GetLength() == 0)
        {
            continue;
        }
        const char* color = "ColorBlack";
        if (z.revealed)
        {
            if (stricmp(z.owner, _resistanceSide) == 0)
            {
                color = "ColorGreen";
            }
            else if (stricmp(z.owner, _occupierSide) == 0)
            {
                color = "ColorRed";
            }
            else
            {
                color = "ColorYellow"; // NEUTRAL and third parties
            }
        }
        RString text = z.revealed ? z.name : RString("");

        for (int m = 0; m < markersMap.Size(); m++)
        {
            ArcadeMarkerInfo& mInfo = markersMap[m];
            if (stricmp(mInfo.name, z.marker) != 0)
            {
                continue;
            }
            if (stricmp(mInfo.colorName, color) != 0)
            {
                mInfo.colorName = color;
                mInfo.OnColorChanged();
            }
            if (strcmp(mInfo.text, text) != 0)
            {
                mInfo.text = text;
            }
            break;
        }
        // marker not in markersMap -> skip silently
    }
}

void ZoneRegistry::DispatchEvents(const AutoArray<ZoneEventRecord>& fired)
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
        const ZoneEventRecord& ev = fired[i];
        RString handler = GetEventHandler(ev.type);
        if (handler.GetLength() == 0)
        {
            continue;
        }
        const ZoneRecord* z = GetZone(ev.zoneIndex);
        if (!z)
        {
            continue;
        }

        GameArrayType pars;
        pars.Resize(3);
        pars[0] = (float)ev.zoneIndex;
        pars[1] = GameStringType(z->name);
        pars[2] = GameStringType(z->owner);

        // dispatch idiom copied from EntityAI::OnEvent (VehicleAIPilot.cpp)
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

LSError ZoneRegistry::ZoneSaveState::Serialize(ParamArchive& ar)
{
    PARAM_CHECK(ar.Serialize("name", name, 1, RString()))
    PARAM_CHECK(ar.Serialize("owner", owner, 1, RString("NEUTRAL")))
    PARAM_CHECK(ar.Serialize("garrison", garrison, 1, 0.0f))
    PARAM_CHECK(ar.Serialize("support", support, 1, 0.0f))
    PARAM_CHECK(ar.Serialize("income", income, 1, 0.0f))
    PARAM_CHECK(ar.Serialize("heat", heat, 1, 0.0f))
    PARAM_CHECK(ar.Serialize("liveOccupiers", liveOccupiers, 1, 0.0f))
    PARAM_CHECK(ar.Serialize("revealed", revealed, 1, false))
    return LSOK;
}

void ZoneRegistry::ApplyPendingLoad()
{
    for (int i = 0; i < _pending.Size(); i++)
    {
        const ZoneSaveState& row = _pending[i];
        // saved rows are matched to the config table by zone NAME; rows
        // without a config zone are dropped, config zones without a row
        // keep their defaults
        int index = FindZoneIndex(row.name);
        if (index < 0)
        {
            continue;
        }
        ZoneRecord& z = _zones[index];
        z.owner = row.owner;
        z.garrison = row.garrison;
        z.support = row.support;
        z.income = row.income;
        z.heat = row.heat;
        z.liveOccupiers = row.liveOccupiers;
        z.revealed = row.revealed;
    }
}

LSError ZoneRegistry::Serialize(ParamArchive& ar)
{
    if (ar.IsSaving())
    {
        _pending.Clear();
        for (int i = 0; i < _zones.Size(); i++)
        {
            const ZoneRecord& z = _zones[i];
            ZoneSaveState row;
            row.name = z.name;
            row.owner = z.owner;
            row.garrison = z.garrison;
            row.support = z.support;
            row.income = z.income;
            row.heat = z.heat;
            row.liveOccupiers = z.liveOccupiers;
            row.revealed = z.revealed;
            _pending.Add(row);
        }
    }

    // campaign save format version - handed to the campaignLoaded script
    // event so mission scripts can migrate old campaign state
    int saveVersion = GuerrillaSaveVersion;
    PARAM_CHECK(ar.Serialize("guerrillaSaveVersion", saveVersion, 1, GuerrillaSaveVersion))

    // campaign faction sides - additive, presence-tolerant fields (no
    // GuerrillaSaveVersion bump): absent in older saves, which then keep
    // the config/var-resolved values applied by LoadFromConfig below.
    // Parked in members because scalar reads happen on the first load pass
    // only, while application must wait for the second (like _pending).
    if (ar.IsSaving())
    {
        _pendingOccupierSide = _occupierSide;
        _pendingResistanceSide = _resistanceSide;
    }
    PARAM_CHECK(ar.Serialize("occupierSide", _pendingOccupierSide, 1, RString()))
    PARAM_CHECK(ar.Serialize("resistanceSide", _pendingResistanceSide, 1, RString()))

    PARAM_CHECK(ar.Serialize("onCaptured", _handlers[ZECaptured], 1, RString()))
    PARAM_CHECK(ar.Serialize("onSupportThreshold", _handlers[ZESupportThreshold], 1, RString()))
    PARAM_CHECK(ar.Serialize("onRevealed", _handlers[ZERevealed], 1, RString()))
    PARAM_CHECK(ar.Serialize("onCampaignLoaded", _handlers[ZECampaignLoaded], 1, RString()))
    PARAM_CHECK(ar.Serialize("Zones", _pending, 1))

    if (ar.IsSaving())
    {
        _pending.Clear();
        _pendingOccupierSide = RString();
        _pendingResistanceSide = RString();
    }
    else if (ar.GetPass() == ParamArchive::PassSecond)
    {
        // The mission config was reparsed during the load's first pass
        // (SetMission at the end of World::Serialize), so the static zone
        // table can be rebuilt now; the saved dynamic state then overlays
        // it.  Doing this on the first pass would read a stale or empty
        // ExtParsMission.
        LoadFromConfig();
        // a campaign remembers its factions: the saved sides win over the
        // config/var-resolved ones, and the zone table's OCCUPIER/RESISTANCE
        // tokens are re-mapped before the dynamic state overlays it
        if (_pendingOccupierSide.GetLength() > 0)
        {
            _occupierSide = _pendingOccupierSide;
        }
        if (_pendingResistanceSide.GetLength() > 0)
        {
            _resistanceSide = _pendingResistanceSide;
        }
        ApplyOwnerTokens();
        _pendingOccupierSide = RString();
        _pendingResistanceSide = RString();
        ApplyPendingLoad();
        _pending.Clear();
        // queue the post-load script notification; the serialized handler
        // means no script has to re-arm anything - this replaces the
        // GM_SAVED sentinel + poll (and the Save action that re-execs its
        // own file as its handler)
        MarkCampaignLoaded(saveVersion);
    }

    // Alert layer state: optional nested subclass, absent both in saves
    // written before the AlertMachine existed and while loading them - the
    // IsSubclass gate keeps such saves loading.  Placed after the zone-table
    // rebuild above so the machine's second-pass name matching works.
    if (ar.IsSubclass("Alert"))
    {
        ParamArchive arAlert;
        if (ar.OpenSubclass("Alert", arAlert))
        {
            PARAM_CHECK(AlertMachine::Instance().Serialize(arAlert, *this))
            ar.CloseSubclass(arAlert);
        }
    }
    return LSOK;
}

} // namespace Poseidon::Guerrilla
