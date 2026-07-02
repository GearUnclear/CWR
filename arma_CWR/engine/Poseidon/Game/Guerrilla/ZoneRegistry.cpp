#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>

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

#include <Poseidon/Foundation/Strings/RString.hpp>
#include <Poseidon/Foundation/platform.hpp>

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

// ---------------------------------------------------------------------------
// lifecycle / config
// ---------------------------------------------------------------------------

void ZoneRegistry::Clear()
{
    _zones.Clear();
    _factions.Clear();
    _tuning = ZoneTuning();
    for (int i = 0; i < NZoneEventTypes; i++)
    {
        _handlers[i] = RString();
    }
    _accum = 0;
    _pending.Clear();
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
    LoadFromParams(zones, factions);
    // alert tunables share the CfgGuerrillaZones class; loading here (not in
    // LoadFromParams) keeps the testable core free of singleton side effects
    AlertMachine::Instance().LoadFromParams(zones);
}

void ZoneRegistry::LoadFromParams(const ParamEntry* zonesCfg, const ParamEntry* factionsCfg)
{
    // rebuilds the config-derived tables only; event handlers and any
    // pending savegame rows are preserved (see Serialize)
    _zones.Clear();
    _factions.Clear();
    _tuning = ZoneTuning();
    _accum = 0;

    if (zonesCfg)
    {
        LoadZones(*zonesCfg);
    }
    if (factionsCfg)
    {
        LoadFactions(*factionsCfg);
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
        z.owner = e.ReadValue("owner", RString("NEUTRAL"));
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

const FactionRecord* ZoneRegistry::FindFaction(const char* side) const
{
    if (!side)
    {
        return nullptr;
    }
    for (int i = 0; i < _factions.Size(); i++)
    {
        if (stricmp(_factions[i].side, side) == 0)
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
    int index = warLevel >= f->vehicleThreshold ? 1 : 0;
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

    // fog-of-war: GUER-owned, or within revealRadius of a GUER-owned zone
    for (int i = 0; i < n; i++)
    {
        ZoneRecord& z = _zones[i];
        bool revealed = stricmp(z.owner, "GUER") == 0;
        for (int j = 0; !revealed && j < n; j++)
        {
            const ZoneRecord& other = _zones[j];
            if (stricmp(other.owner, "GUER") != 0)
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
        // gone and a live GUER unit stands in the zone area
        bool milCap = !isCity && stricmp(z.owner, "EAST") == 0 && guerHere && z.liveOccupiers < 1 && z.garrison < 1;

        // city: support accrues while a GUER unit is present, flips on the
        // threshold - never on kills
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

        z.owner = "GUER";
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

    // enumerate the guerrilla side's live units; the player is included
    // here when playing GUER
    AICenter* guer = world->GetGuerrilaCenter();
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
            if (stricmp(z.owner, "GUER") == 0)
            {
                color = "ColorGreen";
            }
            else if (stricmp(z.owner, "EAST") == 0)
            {
                color = "ColorRed";
            }
            else
            {
                color = "ColorYellow";
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

    PARAM_CHECK(ar.Serialize("onCaptured", _handlers[ZECaptured], 1, RString()))
    PARAM_CHECK(ar.Serialize("onSupportThreshold", _handlers[ZESupportThreshold], 1, RString()))
    PARAM_CHECK(ar.Serialize("onRevealed", _handlers[ZERevealed], 1, RString()))
    PARAM_CHECK(ar.Serialize("onCampaignLoaded", _handlers[ZECampaignLoaded], 1, RString()))
    PARAM_CHECK(ar.Serialize("Zones", _pending, 1))

    if (ar.IsSaving())
    {
        _pending.Clear();
    }
    else if (ar.GetPass() == ParamArchive::PassSecond)
    {
        // The mission config was reparsed during the load's first pass
        // (SetMission at the end of World::Serialize), so the static zone
        // table can be rebuilt now; the saved dynamic state then overlays
        // it.  Doing this on the first pass would read a stale or empty
        // ExtParsMission.
        LoadFromConfig();
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
