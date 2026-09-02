#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>

#include <Poseidon/Core/Global.hpp>      // Glob.header.worldname
#include <Poseidon/Core/SaveVersion.hpp> // GuerrillaSaveVersion
#include <Poseidon/Game/Guerrilla/AlertMachine.hpp>
#include <Poseidon/Asset/Addon/AddonClosure.hpp>       // faction addon closure (issue #54 C1)
#include <Poseidon/Game/Guerrilla/FactionSources.hpp> // global U island faction table (issue #54 A1)
#include <Poseidon/Game/Guerrilla/FactionTwins.hpp>   // sideTwin resolution (shared with the new-game UI)
#include <Poseidon/Game/Guerrilla/Undercover.hpp>
#include <Poseidon/IO/ParamFileExt.hpp>             // Pars / ExtParsMission
#include <Poseidon/IO/Serialization/ParamArchive.hpp>

#include <Evaluator/express.hpp> // GameState / GameValue (event dispatch)

#include <Poseidon/World/World.hpp>
#include <Poseidon/World/Entities/Infantry/Person.hpp>
#include <Poseidon/AI/AI.hpp>
#include <Poseidon/AI/AICore.hpp>              // markersMap
#include <Poseidon/AI/Path/ArcadeWaypoint.hpp> // ArcadeMarkerInfo
#include <Poseidon/Network/Network.hpp>        // GetNetworkManager (center creation)

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

AICenter* EnsureSideCenter(const char* sideName)
{
    using Poseidon::Foundation::GetEnumValue;
    if (!sideName || !GWorld)
    {
        return nullptr;
    }
    TargetSide side = GetEnumValue<TargetSide>(sideName);
    // same -1 (not INT_MIN) contract FindSideCenter documents; the upper bound
    // keeps the TSideUnknown..TEmpty tail out of CreateCenter, which would
    // otherwise build a center for a non-side
    if ((int)side < 0 || side >= TSideUnknown)
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

// mirrors GroupCreate (GameStateExtWorldConfig.cpp:693): a fresh group with
// its first waypoint at the origin and an Arcade mission, announced to the
// network layer.  Hoisted out of GarrisonCache/Traffic so every Guerrilla
// spawner (garrisons, traffic crews, dealers) builds groups the same way.
AIGroup* CreateSideGroup(AICenter* center)
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

// script-owned war level; 1 when undefined (matches the init.sqs default)
float ReadWarLevel()
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

// script/campaign global published by the new-game UI (OptionsUIApp VarSets
// kGuerrillaVarOccupier / kGuerrillaVarResistance); read like ReadWarLevel -
// VarGet with the lowercased name, nil tolerated
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

// the engine's ClassProbe (declared in the header - shared with the outfit
// seam): a classname exists when the merged game config (Pars, i.e. the
// loaded data package + addons) carries it under the bank
bool ParsClassProbe::Exists(const char* bank, const char* className) const
{
    if (!bank || !className || !*className)
    {
        return false;
    }
    const ParamEntry* bankEntry = Pars.FindEntry(bank);
    return bankEntry && bankEntry->FindEntry(className) != nullptr;
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
    _occupierFaction = RString();
    _resistanceFaction = RString();
    for (int i = 0; i < NZoneEventTypes; i++)
    {
        _handlers[i] = RString();
    }
    _accum = 0;
    _pending.Clear();
    _pendingOccupierSide = RString();
    _pendingResistanceSide = RString();
    _pendingOccupierFaction = RString();
    _pendingResistanceFaction = RString();
    _pendingLoaded = false;
    _loadedSaveVersion = 0;
    _friendshipApplied = false;
    // the alert and undercover layers live and die with the zone registry
    AlertMachine::Instance().Clear();
    UndercoverSystem::Instance().Clear();
}

void ZoneRegistry::InitMission()
{
    Clear();
    LoadFromConfig();
    ActivateFactionAddons();
}

// Engine wrapper over CollectFactionAddons: the additive World::ActivateAddon
// grant (the same runtime-only visibility the player-body seam uses, never
// written into the template's addOns[]), logged once at INFO so a missing
// pbo shows up as "activated X" or does not show up at all.
void ZoneRegistry::ActivateFactionAddons() const
{
    if (!GWorld || !IsActive())
    {
        return;
    }
    FindArrayRStringCI addons;
    CollectFactionAddons(Pars.FindEntry("CfgVehicles"), Pars.FindEntry("CfgWeapons"), Pars.FindEntry("CfgMagazines"),
                         addons);
    RString added;
    int nAdded = 0;
    for (int i = 0; i < addons.Size(); i++)
    {
        if (GWorld->IsAddonActive(addons[i]))
        {
            continue;
        }
        GWorld->ActivateAddon(addons[i]);
        added = added + (nAdded > 0 ? RString(", ") : RString()) + addons[i];
        nAdded++;
    }
    if (nAdded > 0)
    {
        LOG_INFO(Core, "ZoneRegistry: activated {} addon(s) the faction descriptors need beyond the mission's addOns[]: {}",
                 nAdded, (const char*)added);
    }
}

const FactionRecord* ZoneRegistry::GetFaction(int index) const
{
    if (index < 0 || index >= _factions.Size())
    {
        return nullptr;
    }
    return &_factions[index];
}


void ZoneRegistry::LoadFromConfig()
{
    const ParamEntry* zones = ExtParsMission.FindEntry("CfgGuerrillaZones");
    if (!zones)
    {
        zones = Pars.FindEntry("CfgGuerrillaZones");
    }
    // the faction table is the UNION of the global config (addon faction
    // packs, bin/config-extra.cpp) and the mission's own description.ext
    // block, island winning on a name collision (issue #54 A1). Built here
    // and consumed by LoadFromParams, which copies what it needs into
    // FactionRecords, so the merged ParamFile may die with this frame.
    FactionSources factionSources;
    BuildFactionSourcesFromEngine(factionSources);
    const ParamEntry* factions = factionSources.Factions();
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
    // the engine path always resolves descriptor classnames against the
    // loaded data package (plan 15) - a descriptor authored for one package
    // degrades with a logged substitution on another instead of producing
    // fatal or silently-sterile spawns
    ParsClassProbe probe;
    LoadFromParams(zones, factions, selOccupier, selResistance, names, &probe);
    // alert and undercover tunables share the CfgGuerrillaZones class;
    // loading here (not in LoadFromParams) keeps the testable core free of
    // singleton side effects
    AlertMachine::Instance().LoadFromParams(zones);
    UndercoverSystem::Instance().LoadFromParams(zones);
}

void ZoneRegistry::LoadFromParams(const ParamEntry* zonesCfg, const ParamEntry* factionsCfg, const char* selOccupier,
                                  const char* selResistance, const ParamEntry* worldNamesCfg, const ClassProbe* probe)
{
    // rebuilds the config-derived tables only; event handlers and any
    // pending savegame rows are preserved (see Serialize)
    _zones.Clear();
    _factions.Clear();
    _tuning = ZoneTuning();
    _occupierSide = "EAST";
    _resistanceSide = "GUER";
    _occupierFaction = RString();
    _resistanceFaction = RString();
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
        // read here, not with the rest of the tuning in LoadZones: LoadZones
        // runs below, AFTER the pass this key gates
        _tuning.playerSide = zonesCfg->ReadValue("playerSide", _tuning.playerSide);
    }
    ResolveSides(selOccupier, selResistance);
    // strictly after the precedence chain above (the sides it resolves are
    // this pass's input) and strictly before both of the passes below:
    // ResolveFactionClasses keys its fallback list off FactionRecord::side,
    // and LoadZones resolves each zone's owner token off the campaign sides -
    // either one running first would see pre-rebase sides
    ResolveSideCollisions(factionsCfg);
    if (probe)
    {
        ResolveFactionClasses(*probe);
    }
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
            _occupierFaction = f->className;
        }
    }
    if (selResistance && *selResistance)
    {
        if (const FactionRecord* f = FindFaction(selResistance))
        {
            _resistanceSide = f->side;
            _resistanceFaction = f->className;
        }
    }
}

// "Auto-rebase the resistance", aimed at the thing that can actually move.
//
// The player's unit is authored into the template's mission.sqm on ONE side,
// and GatherInputs counts the resistance out of THAT side's center - so a
// resistance side that walks away from the player is a resistance the player
// is not part of, and nothing he does captures anything.  The resistance side
// is a per-template constant; what a faction pick chooses is the ROSTER.  So:
// pin the picked resistance roster onto playerSide, and move the occupier off
// that side if the two collide.  Prefer the config-clean path (a sideTwin: the
// same roster the author already re-declared on the other side) and override a
// descriptor's side only where the data offers no twin.
//
// Idempotent by construction - a pure function of (playerSide, selections,
// faction table) - which matters because LoadFromConfig re-runs on every
// savegame load pass.
void ZoneRegistry::ResolveSideCollisions(const ParamEntry* factionsCfg)
{
    // legacy templates: no pin, no rebase, behaviour exactly as before.  The
    // new-game UI blocks a same-side pair for these instead.
    if (_tuning.playerSide.GetLength() == 0)
    {
        return;
    }

    // ---- resistance: pin to the player's side
    if (stricmp(_resistanceSide, _tuning.playerSide) != 0)
    {
        RString twin = TwinOnSide(factionsCfg, _resistanceFaction, _tuning.playerSide);
        if (twin.GetLength() > 0)
        {
            _resistanceFaction = twin; // already declared on that side: no override needed
        }
        else
        {
            LOG_INFO(Core, "Guerrilla: resistance faction '{}' rebased from side {} to {} (no sideTwin on that side)",
                     (const char*)_resistanceFaction, (const char*)_resistanceSide, (const char*)_tuning.playerSide);
        }
        _resistanceSide = _tuning.playerSide;
    }

    // ---- occupier: step off the resistance's side
    if (stricmp(_occupierSide, _resistanceSide) == 0)
    {
        RString twin = TwinOffSide(factionsCfg, _occupierFaction, _resistanceSide);
        if (twin.GetLength() > 0)
        {
            _occupierFaction = twin;
            _occupierSide = FactionSideOf(factionsCfg, twin);
        }
        else
        {
            RString freeSide = FirstFreeWarSide(_resistanceSide);
            if (freeSide.GetLength() == 0)
            {
                // unreachable with three war sides and one pinned; kept as the
                // honest floor.  Degrade to the pre-existing contested
                // deadlock rather than inventing a side or aborting a load.
                LOG_WARN(Core, "Guerrilla: occupier '{}' shares side {} with the resistance and no war side is free",
                         (const char*)_occupierFaction, (const char*)_occupierSide);
            }
            else
            {
                LOG_INFO(Core, "Guerrilla: occupier faction '{}' rebased from side {} to {} (no sideTwin off that side)",
                         (const char*)_occupierFaction, (const char*)_occupierSide, (const char*)freeSide);
                _occupierSide = freeSide;
            }
        }
    }

    RebindFactionSides();
}

// The faction table has to agree with the resolved sides: FindFaction matches
// side BEFORE class name, so a record left on its authored side after a rebase
// makes FindFaction(newSide) miss the descriptor the player picked while
// FindFaction(oldSide) still hands it to the other pick.  It also silently
// drops the descriptor's authored `flag` key, which TownFlags looks up by SIDE
// string (registry.FactionValue(z->owner, "flag")).
//
// Doing every side mutation here rather than inline keeps the twin path
// override-free for free: a twin already declares the side being forced onto
// it, so this writes back what the config says.
void ZoneRegistry::RebindFactionSides()
{
    // ...but only ONE record per pick, or the second write eats the first
    DivergeAliasedFactions();
    if (FactionRecord* f = FindFactionByClassMutable(_occupierFaction))
    {
        f->side = _occupierSide;
    }
    if (FactionRecord* f = FindFactionByClassMutable(_resistanceFaction))
    {
        f->side = _resistanceSide;
    }
}

// A mirror match - the same faction picked for both cyclers, or a template
// whose defaultOccupier and defaultResistance name one descriptor - leaves
// _occupierFaction == _resistanceFaction.  ResolveSideCollisions still steps
// the occupier onto a free side, so the two picks want that ONE FactionRecord
// to be on two sides at once.  Without this the occupier's `side` write in
// RebindFactionSides is immediately clobbered by the resistance's: the record
// ends on the resistance's side, ResolveFactionClasses resolves the occupier's
// roster against FallbackListForSide(the WRONG side), and
// FindFaction(_occupierSide) finds no record at all.
//
// The roster is what a faction pick chooses and the side is a separate axis
// (see ResolveSideCollisions), so hand the occupier its own copy of the roster
// on its own side rather than refusing the pair.  Both sides then field the
// identical order of battle, which is exactly what a mirror match is.
//
// The clone's name is a pure function of (class, side), so an identical clone
// reappears on every LoadFromConfig - that is what lets Serialize restore
// _occupierFaction by name across a save/load.  Idempotent: the second call
// finds the clone already there, and once _occupierFaction points at the clone
// the alias test no longer fires.
void ZoneRegistry::DivergeAliasedFactions()
{
    if (_occupierFaction.GetLength() == 0 || stricmp(_occupierFaction, _resistanceFaction) != 0)
    {
        return;
    }
    if (stricmp(_occupierSide, _resistanceSide) == 0)
    {
        // an unresolved collision (ResolveSideCollisions' honest floor, or a
        // legacy template with no playerSide): one side, one record, nothing
        // to split - splitting here would invent a side nobody resolved
        return;
    }
    const FactionRecord* src = FindFactionByClass(_occupierFaction);
    if (!src)
    {
        return;
    }
    RString cloneName = _occupierFaction + RString("@") + _occupierSide;
    if (!FindFactionByClass(cloneName))
    {
        FactionRecord clone = *src; // by value: _factions.Add may reallocate
        clone.className = cloneName;
        clone.side = _occupierSide;
        _factions.Add(clone);
        LOG_INFO(
            Core,
            "Guerrilla: occupier and resistance both picked '{}'; occupier fields a copy of that roster on side {}",
            (const char*)_occupierFaction, (const char*)_occupierSide);
    }
    _occupierFaction = cloneName;
}

// WEST and GUER are ALLIES out of the box: ArcadeIntel::Init seeds
// friends[TWest][TGuerrila] = friends[TGuerrila][TWest] = 1.0 and
// AICenter::IsEnemy is `_friends[side] < 0.6`.  Both Guerrilla templates ship
// an empty `class Intel {}`, so those defaults apply verbatim - an Abel
// campaign with a WEST occupier against the GUER resistance would be a
// permanent ceasefire: two armies coexisting peacefully, campaign unwinnable,
// nothing logged anywhere.  Sinai only works today because IDF(WEST) vs
// EgyptFrontier(EAST) happens to default to 0.0.
//
// Runs once per campaign load (Simulate), not per tick, so a mission script's
// setFriend stays authoritative afterwards.  The third war side is left
// exactly as the template set it.
//
// The return is "settled", not "succeeded": the caller latches on true, so the
// two refusals below - which no later tick can un-refuse, since both are pure
// functions of sides fixed at load - must report true and say why ONCE, or the
// guarded call re-runs two string->enum lookups every tick for the life of the
// campaign.  Only a world that has not finished building returns false.
bool ZoneRegistry::ApplyCampaignFriendship()
{
    using Poseidon::Foundation::GetEnumValue;
    if (!GWorld)
    {
        return false; // transient: no world yet
    }
    // (const char*), not the RString: GetEnumValue overloads on const char* AND
    // const RStringB&, and RString converts to both — the raw call is ambiguous
    TargetSide occ = GetEnumValue<TargetSide>((const char*)_occupierSide);
    TargetSide res = GetEnumValue<TargetSide>((const char*)_resistanceSide);
    if ((int)occ < 0 || (int)res < 0 || occ >= TSideUnknown || res >= TSideUnknown)
    {
        LOG_WARN(Core,
                 "Guerrilla: campaign sides '{}'/'{}' do not both name a war side; friendship left at the "
                 "template's defaults",
                 (const char*)_occupierSide, (const char*)_resistanceSide);
        return true; // permanent: the strings will not change under us
    }
    if (occ == res)
    {
        // unresolved collision: never weld a side as its own enemy
        LOG_WARN(Core,
                 "Guerrilla: occupier and resistance are both side {}; friendship left at the template's defaults",
                 (const char*)_occupierSide);
        return true; // permanent
    }
    AICenter* occCenter = EnsureSideCenter(_occupierSide);
    AICenter* resCenter = EnsureSideCenter(_resistanceSide);
    if (!occCenter || !resCenter)
    {
        return false;
    }
    occCenter->SetFriendship(res, 0.0f);
    occCenter->SetFriendship(occ, 1.0f);
    occCenter->SetFriendship(TCivilian, 1.0f);
    resCenter->SetFriendship(occ, 0.0f);
    resCenter->SetFriendship(res, 1.0f);
    resCenter->SetFriendship(TCivilian, 1.0f);
    // the population is nobody's enemy; the ambience layer's civilians are not
    // a third army
    if (AICenter* civCenter = GWorld->GetCenter(TCivilian))
    {
        civCenter->SetFriendship(occ, 1.0f);
        civCenter->SetFriendship(res, 1.0f);
        civCenter->SetFriendship(TCivilian, 1.0f);
    }
    return true;
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
    _tuning.captureRate = cfg.ReadValue("captureRate", _tuning.captureRate);
    _tuning.captureCrewCap = cfg.ReadValue("captureCrewCap", _tuning.captureCrewCap);
    _tuning.captureDecayDefended = cfg.ReadValue("captureDecayDefended", _tuning.captureDecayDefended);
    _tuning.captureDecayAbandoned = cfg.ReadValue("captureDecayAbandoned", _tuning.captureDecayAbandoned);
    _tuning.supportDecayOccupied = cfg.ReadValue("supportDecayOccupied", _tuning.supportDecayOccupied);
    _tuning.supportDecayFloor = cfg.ReadValue("supportDecayFloor", _tuning.supportDecayFloor);
    _tuning.contestOutnumberRatio = cfg.ReadValue("contestOutnumberRatio", _tuning.contestOutnumberRatio);
    // already read (and acted on) in LoadFromParams; re-read so the tuning
    // struct is complete for anyone inspecting it
    _tuning.playerSide = cfg.ReadValue("playerSide", _tuning.playerSide);

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
        // per-zone pacing: big installations author a slower rate (0 = tuning)
        z.captureRateOverride = e.ReadValue("captureRate", 0.0f);
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

        auto readStringArray = [&e](const char* key, AutoArray<RString>& out)
        {
            const ParamEntry* arr = e.FindEntry(key);
            if (arr && arr->IsArray())
            {
                for (int k = 0; k < arr->GetSize(); k++)
                {
                    out.Add(RString((RStringB)(*arr)[k]));
                }
            }
        };
        readStringArray("tiers", f.tiers);
        // per-tier role variants (plan 15); optional, parallel to tiers[]
        readStringArray("tiersMG", f.tiersMG);
        readStringArray("tiersAT", f.tiersAT);
        readStringArray("tiersMedic", f.tiersMedic);
        readStringArray("tiersSniper", f.tiersSniper);
        // civilian-outfit ladder (issue #25): the rung for AI garrisons,
        // guards, town militia and part-time fighters; optional, indexed by
        // the same tierThresholds[] as tiers[] (clamped to its own length)
        readStringArray("civTier", f.civTiers);
        const ParamEntry* thresholds = e.FindEntry("tierThresholds");
        if (thresholds && thresholds->IsArray())
        {
            for (int k = 0; k < thresholds->GetSize(); k++)
            {
                f.tierThresholds.Add((*thresholds)[k]);
            }
        }
        readStringArray("vehicles", f.vehicles);
        // civilian traffic hulls (Traffic): optional, CIV descriptor only in
        // practice, no ladder / thresholds
        readStringArray("civVehicles", f.civVehicles);
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

// ---------------------------------------------------------------------------
// plan-15 descriptor resolution: unknown incoming faction data never reaches
// a spawn path.  Substitutions are logged; ABSENT keys keep their semantics
// (only present-but-unresolvable values are rewritten).
// ---------------------------------------------------------------------------

namespace
{

// built-in last-resort candidates per side, probed in order.  The lists are
// verified against the three local data packages (tmp/class-survey/): every
// name is scope-accessible and model-complete in the package that ships it.
// GUER degrades cross-side on GUER-less packages (Demo [Remaster], #13):
// bodies spawned into GUER-side groups fight as resistance regardless of
// their config side (the civilians.sqs side-comes-from-group mechanism).
const char* const kFallbackWest[] = {"SoldierWB", nullptr};
const char* const kFallbackEast[] = {"SoldierEB", nullptr};
const char* const kFallbackGuer[] = {"SoldierGB", "SoldierGFakeE", "SoldierEB", "SoldierWB", nullptr};
const char* const kFallbackCiv[] = {"Civilian", "SoldierWCaptive", nullptr};
// the civilian-OUTFIT ladder (issue #25): armed-fighter-in-civilian-clothes
// bodies for the civTier[] rung.  Deliberately NOT kFallbackCiv - its
// SoldierWCaptive entry is a WEST-side unarmed captive class.  BIS's own
// plainclothes pair first; plain Civilian last (it costs the civilian-kill
// stat line and has Man-grade weapon slots, but it is a spawnable body).
const char* const kFallbackCivOutfit[] = {"SoldierGFakeC", "SoldierGFakeC2", "Civilian", nullptr};

const char* const* FallbackListForSide(const char* side)
{
    if (stricmp(side, "WEST") == 0)
    {
        return kFallbackWest;
    }
    if (stricmp(side, "EAST") == 0)
    {
        return kFallbackEast;
    }
    if (stricmp(side, "GUER") == 0)
    {
        return kFallbackGuer;
    }
    if (stricmp(side, "CIV") == 0)
    {
        return kFallbackCiv;
    }
    return nullptr;
}

// keys valued with a CfgVehicles unit classname
bool IsUnitClassKey(const char* key)
{
    if (stricmp(key, "officer") == 0 || stricmp(key, "holdClass") == 0 || stricmp(key, "recruitFighter") == 0 ||
        stricmp(key, "recruitSpecialist") == 0 || stricmp(key, "companionClass") == 0 ||
        stricmp(key, "fallbackClass") == 0)
    {
        return true;
    }
    // the Civ outfit-family scalars (issue #25): the civilian-outfit bodies
    // for the player and the player-adjacent AI. They ride the same plan-15
    // resolution; a missing class degrades to the warrior-side fallback
    // (tier 0 / sideFallback), so gmFactionValue hands scripts a spawnable
    // warrior body automatically (accepted trade-off, issue #25 Part 4).
    // The player seam is unaffected: it reads raw config before the registry
    // loads and keeps the authored class on probe failure.
    if (stricmp(key, "playerClassWarrior") == 0 || stricmp(key, "playerClassCiv") == 0 ||
        stricmp(key, "recruitFighterCiv") == 0 || stricmp(key, "recruitSpecialistCiv") == 0 ||
        stricmp(key, "companionClassCiv") == 0 || stricmp(key, "holdClassCiv") == 0)
    {
        return true;
    }
    // civClass1..civClassN (civClassCount is numeric, not a classname)
    if (strnicmp(key, "civClass", 8) == 0 && key[8] >= '0' && key[8] <= '9')
    {
        return true;
    }
    return false;
}

// keys valued with a CfgWeapons classname (OFP-era magazines are CfgWeapons
// entries too, so both kinds probe the same bank)
bool IsWeaponKey(const char* key)
{
    if (stricmp(key, "baseWeapon") == 0 || stricmp(key, "baseMagazine") == 0)
    {
        return true;
    }
    if (strnicmp(key, "loot", 4) != 0)
    {
        return false;
    }
    size_t len = strlen(key);
    return (len > 6 && stricmp(key + len - 6, "Weapon") == 0) || (len > 3 && stricmp(key + len - 3, "Mag") == 0);
}

} // namespace

void ZoneRegistry::CollectFactionAddons(const ParamEntry* vehiclesCfg, const ParamEntry* weaponsCfg,
                                        const ParamEntry* magazinesCfg, FindArrayRStringCI& addons) const
{
    for (int fi = 0; fi < _factions.Size(); fi++)
    {
        const FactionRecord& f = _factions[fi];
        const AutoArray<RString>* unitLadders[] = {&f.tiers,       &f.tiersMG,  &f.tiersAT,  &f.tiersMedic,
                                                   &f.tiersSniper, &f.civTiers, &f.vehicles, &f.civVehicles};
        for (const AutoArray<RString>* ladder : unitLadders)
        {
            for (int i = 0; i < ladder->Size(); i++)
            {
                CollectVehicleClassAddons(vehiclesCfg, weaponsCfg, magazinesCfg, (*ladder)[i], addons);
            }
        }
        for (int k = 0; k < f.values.Size(); k++)
        {
            const char* key = f.values[k].key;
            const RString& value = f.values[k].value;
            if (value.GetLength() == 0)
            {
                continue;
            }
            // the *Civ twins of the unit keys (recruitFighterCiv, holdClassCiv,
            // ...) and the CIV descriptor's civClass1..N are unit classes too
            size_t len = strlen(key);
            bool civUnit = (len > 3 && stricmp(key + len - 3, "Civ") == 0) ||
                           (len > 8 && strnicmp(key, "civClass", 8) == 0 && stricmp(key, "civClassCount") != 0);
            if (IsUnitClassKey(key) || civUnit || stricmp(key, "playerClassWarrior") == 0 ||
                stricmp(key, "playerClassCiv") == 0)
            {
                CollectVehicleClassAddons(vehiclesCfg, weaponsCfg, magazinesCfg, value, addons);
            }
            else if (IsWeaponKey(key))
            {
                // "...Mag" keys are CfgMagazines classes, the rest CfgWeapons
                if (len > 3 && stricmp(key + len - 3, "Mag") == 0)
                {
                    CollectMagazineAddons(magazinesCfg, value, addons);
                }
                else
                {
                    CollectWeaponAddons(weaponsCfg, magazinesCfg, value, addons);
                }
            }
        }
    }
}

void ZoneRegistry::ResolveFactionClasses(const ClassProbe& probe)
{
    const char* kVeh = "CfgVehicles";
    const char* kWpn = "CfgWeapons";
    for (int fi = 0; fi < _factions.Size(); fi++)
    {
        FactionRecord& f = _factions[fi];

        // side fallback: the descriptor's own fallbackClass key first, then
        // the built-in per-side candidate list; "" when nothing resolves
        RString sideFallback;
        for (int k = 0; k < f.values.Size() && sideFallback.GetLength() == 0; k++)
        {
            if (stricmp(f.values[k].key, "fallbackClass") == 0 && probe.Exists(kVeh, f.values[k].value))
            {
                sideFallback = f.values[k].value;
            }
        }
        if (const char* const* candidates = FallbackListForSide(f.side))
        {
            for (int k = 0; candidates[k] && sideFallback.GetLength() == 0; k++)
            {
                if (probe.Exists(kVeh, candidates[k]))
                {
                    sideFallback = candidates[k];
                }
            }
        }

        // substitutions must be visible in release logs, so route through
        // the real logger (LOG_WARN)
        auto logSub = [&f](const char* key, const char* bad, const char* sub)
        {
            LOG_WARN(Core, "ZoneRegistry: faction '{}' key '{}': class '{}' not in the loaded data package - using '{}'",
                     (const char*)f.className, key, bad, (sub && *sub) ? sub : "<none>");
        };

        // ---- tiers[]: nearest lower resolved tier, then higher, then the
        // side fallback (an all-unresolved ladder collapses to the fallback
        // or, failing that, empties - FactionTierClass "" = systems inert)
        AutoArray<bool> ok;
        ok.Resize(f.tiers.Size());
        bool anyTier = false;
        for (int i = 0; i < f.tiers.Size(); i++)
        {
            ok[i] = f.tiers[i].GetLength() > 0 && probe.Exists(kVeh, f.tiers[i]);
            anyTier |= ok[i];
        }
        for (int i = 0; i < f.tiers.Size(); i++)
        {
            if (ok[i])
            {
                continue;
            }
            RString sub;
            for (int j = i - 1; j >= 0 && sub.GetLength() == 0; j--)
            {
                if (ok[j])
                {
                    sub = f.tiers[j];
                }
            }
            for (int j = i + 1; j < f.tiers.Size() && sub.GetLength() == 0; j++)
            {
                if (ok[j])
                {
                    sub = f.tiers[j];
                }
            }
            if (sub.GetLength() == 0)
            {
                sub = sideFallback;
            }
            logSub("tiers[]", f.tiers[i], sub);
            f.tiers[i] = sub;
        }
        if (!anyTier && f.tiers.Size() > 0 && sideFallback.GetLength() == 0)
        {
            f.tiers.Clear(); // nothing spawnable: honest inert beats sterile retry loops
        }

        // ---- civTier[] (issue #25): nearest lower resolved civ tier, then
        // higher, then the civilian-OUTFIT ladder (kFallbackCivOutfit), then
        // the (already resolved) tiers[0] warrior rung.  An all-unresolved
        // ladder with no candidate empties - "" = the rung is inert, callers
        // keep their warrior classes.
        {
            AutoArray<bool> civOk;
            civOk.Resize(f.civTiers.Size());
            bool anyCivTier = false;
            for (int i = 0; i < f.civTiers.Size(); i++)
            {
                civOk[i] = f.civTiers[i].GetLength() > 0 && probe.Exists(kVeh, f.civTiers[i]);
                anyCivTier |= civOk[i];
            }
            RString civOutfitFallback;
            if (f.civTiers.Size() > 0 && !anyCivTier)
            {
                for (int k = 0; kFallbackCivOutfit[k] && civOutfitFallback.GetLength() == 0; k++)
                {
                    if (probe.Exists(kVeh, kFallbackCivOutfit[k]))
                    {
                        civOutfitFallback = kFallbackCivOutfit[k];
                    }
                }
                if (civOutfitFallback.GetLength() == 0 && f.tiers.Size() > 0)
                {
                    civOutfitFallback = f.tiers[0];
                }
            }
            bool anyKept = false;
            for (int i = 0; i < f.civTiers.Size(); i++)
            {
                if (civOk[i])
                {
                    anyKept = true;
                    continue;
                }
                RString sub;
                for (int j = i - 1; j >= 0 && sub.GetLength() == 0; j--)
                {
                    if (civOk[j])
                    {
                        sub = f.civTiers[j];
                    }
                }
                for (int j = i + 1; j < f.civTiers.Size() && sub.GetLength() == 0; j++)
                {
                    if (civOk[j])
                    {
                        sub = f.civTiers[j];
                    }
                }
                if (sub.GetLength() == 0)
                {
                    sub = civOutfitFallback;
                }
                logSub("civTier[]", f.civTiers[i], sub);
                f.civTiers[i] = sub;
                anyKept |= sub.GetLength() > 0;
            }
            if (!anyKept)
            {
                f.civTiers.Clear(); // honest inert beats sterile spawns
            }
        }

        // ---- role tiers: an unresolvable entry blanks to "" (query-time
        // fallback to the tier rifleman keeps the no-probe path identical)
        AutoArray<RString>* roleArrays[] = {&f.tiersMG, &f.tiersAT, &f.tiersMedic, &f.tiersSniper};
        const char* roleNames[] = {"tiersMG[]", "tiersAT[]", "tiersMedic[]", "tiersSniper[]"};
        for (int r = 0; r < 4; r++)
        {
            AutoArray<RString>& arr = *roleArrays[r];
            for (int i = 0; i < arr.Size(); i++)
            {
                if (arr[i].GetLength() > 0 && !probe.Exists(kVeh, arr[i]))
                {
                    logSub(roleNames[r], arr[i], "<tier rifleman>");
                    arr[i] = RString();
                }
            }
        }

        // ---- vehicles[]: drop unresolvable rungs and compact the ladder.
        // The thresholds are the LADDER's escalation gates (rung k>0 unlocks
        // at vehicleThresholds[k-1]), not properties of a specific hull - so
        // the surviving vehicles slide down into the dropped rungs and the
        // pacing stays authored (at WL6 SOMETHING still upgrades)
        {
            AutoArray<RString> keptVehicles;
            for (int i = 0; i < f.vehicles.Size(); i++)
            {
                bool exists = f.vehicles[i].GetLength() > 0 && probe.Exists(kVeh, f.vehicles[i]);
                if (!exists)
                {
                    logSub("vehicles[]", f.vehicles[i], "<dropped>");
                    continue;
                }
                keptVehicles.Add(f.vehicles[i]);
            }
            if (keptVehicles.Size() != f.vehicles.Size())
            {
                f.vehicles = keptVehicles;
                int gates = keptVehicles.Size() > 0 ? keptVehicles.Size() - 1 : 0;
                if (f.vehicleThresholds.Size() > gates)
                {
                    f.vehicleThresholds.Resize(gates);
                }
            }
        }

        // ---- civVehicles[]: drop unresolvable hulls (no ladder, no
        // thresholds - an empty result simply leaves civilian traffic inert)
        {
            AutoArray<RString> keptCiv;
            for (int i = 0; i < f.civVehicles.Size(); i++)
            {
                bool exists = f.civVehicles[i].GetLength() > 0 && probe.Exists(kVeh, f.civVehicles[i]);
                if (!exists)
                {
                    logSub("civVehicles[]", f.civVehicles[i], "<dropped>");
                    continue;
                }
                keptCiv.Add(f.civVehicles[i]);
            }
            if (keptCiv.Size() != f.civVehicles.Size())
            {
                f.civVehicles = keptCiv;
            }
        }

        // ---- plain keys: units fall back to tier 0 / the side fallback;
        // weapon keys fall back to the (resolved) baseWeapon; mag keys to
        // their weapon sibling (the OFP self-magazine convention).  Resolve
        // baseWeapon first - it anchors the rest.
        RString baseWeapon;
        for (int k = 0; k < f.values.Size(); k++)
        {
            FactionRecord::NamedValue& v = f.values[k];
            if (stricmp(v.key, "baseWeapon") != 0)
            {
                continue;
            }
            if (v.value.GetLength() > 0 && !probe.Exists(kWpn, v.value))
            {
                logSub(v.key, v.value, "");
                v.value = RString();
            }
            baseWeapon = v.value;
        }
        int civResolved = 0;
        RString civSub; // first resolvable civClass value, for sibling substitution
        for (int k = 0; k < f.values.Size(); k++)
        {
            const FactionRecord::NamedValue& v = f.values[k];
            bool isCiv = strnicmp(v.key, "civClass", 8) == 0 && v.key[8] >= '0' && v.key[8] <= '9';
            if (isCiv && v.value.GetLength() > 0 && probe.Exists(kVeh, v.value))
            {
                civResolved++;
                if (civSub.GetLength() == 0)
                {
                    civSub = v.value;
                }
            }
        }
        for (int k = 0; k < f.values.Size(); k++)
        {
            FactionRecord::NamedValue& v = f.values[k];
            if (v.value.GetLength() == 0)
            {
                continue;
            }
            if (IsUnitClassKey(v.key))
            {
                if (probe.Exists(kVeh, v.value))
                {
                    continue;
                }
                bool isCiv = strnicmp(v.key, "civClass", 8) == 0 && v.key[8] >= '0' && v.key[8] <= '9';
                RString sub = isCiv ? civSub : (f.tiers.Size() > 0 ? f.tiers[0] : RString());
                if (sub.GetLength() == 0)
                {
                    sub = sideFallback;
                }
                logSub(v.key, v.value, sub);
                v.value = sub;
                if (isCiv && sub.GetLength() > 0)
                {
                    civResolved++;
                }
            }
            else if (IsWeaponKey(v.key) && stricmp(v.key, "baseWeapon") != 0)
            {
                if (probe.Exists(kWpn, v.value))
                {
                    continue;
                }
                // a mag key first tries its weapon sibling (lootXMag ->
                // lootXWeapon, baseMagazine -> baseWeapon), then baseWeapon
                RString sub;
                size_t len = strlen(v.key);
                bool isMag = stricmp(v.key, "baseMagazine") == 0 || (len > 3 && stricmp(v.key + len - 3, "Mag") == 0);
                if (isMag)
                {
                    char weaponKey[64];
                    if (stricmp(v.key, "baseMagazine") == 0)
                    {
                        snprintf(weaponKey, sizeof(weaponKey), "baseWeapon");
                    }
                    else
                    {
                        snprintf(weaponKey, sizeof(weaponKey), "%.*sWeapon", (int)(len - 3), (const char*)v.key);
                    }
                    for (int m = 0; m < f.values.Size() && sub.GetLength() == 0; m++)
                    {
                        if (stricmp(f.values[m].key, weaponKey) == 0 && f.values[m].value.GetLength() > 0 &&
                            probe.Exists(kWpn, f.values[m].value))
                        {
                            sub = f.values[m].value;
                        }
                    }
                }
                if (sub.GetLength() == 0)
                {
                    sub = baseWeapon;
                }
                logSub(v.key, v.value, sub);
                v.value = sub;
            }
        }
        // a CIV roster that resolved to nothing soft-disables the ambience
        // layer: civilians.sqs treats civClassCount 0 / "" as "no CIV layer"
        if (civResolved == 0)
        {
            for (int k = 0; k < f.values.Size(); k++)
            {
                if (stricmp(f.values[k].key, "civClassCount") == 0 && f.values[k].value.GetLength() > 0 &&
                    stricmp(f.values[k].value, "0") != 0)
                {
                    LOG_WARN(Core, "ZoneRegistry: faction '{}': no civClass<N> resolved - civClassCount forced to 0",
                         (const char*)f.className);
                    f.values[k].value = "0";
                }
            }
        }
    }
}

// CITY auto-seed from the world's named locations (CfgWorlds >> <world> >>
// Names).  OFP-era Names entries carry only name + a 2D position[] {x,z}
// (see CStaticMap::DrawName) - every entry is a town, so type-less entries
// are accepted; an Arma-style type entry, when present, must be a city-like
// location type.
// the 300 m dedup radius shared by SeedCityZones and CollectTownNames
static constexpr float SeedDedupDistSq = 300.0f * 300.0f;

bool ZoneRegistry::NamesEntryIsTown(const ParamEntry& e, RString& name, Vector3& pos)
{
    if (!e.IsClass())
    {
        return false;
    }
    RString type = e.ReadValue("type", RString());
    if (type.GetLength() > 0 && stricmp(type, "NameCity") != 0 && stricmp(type, "NameCityCapital") != 0 &&
        stricmp(type, "NameVillage") != 0)
    {
        return false; // typed non-town location (rocks, hills, ...)
    }
    const ParamEntry* position = e.FindEntry("position");
    if (!position || !position->IsArray() || position->GetSize() < 2)
    {
        return false;
    }
    float easting = (*position)[0];
    float northing = (*position)[1];
    float elevation = position->GetSize() >= 3 ? (float)(*position)[2] : 0.0f;
    name = e.ReadValue("name", RString(e.GetName()));
    if (name.GetLength() == 0)
    {
        name = e.GetName(); // Names entries often ship name=""
    }
    pos = Vector3(easting, elevation, northing);
    return true;
}

void ZoneRegistry::SeedCityZones(const ParamEntry& namesCfg)
{
    int seeded = 0;
    for (int i = 0; i < namesCfg.GetEntryCount(); i++)
    {
        RString name;
        Vector3 pos;
        if (!NamesEntryIsTown(namesCfg.GetEntry(i), name, pos))
        {
            continue;
        }
        // dedup: a location on top of a configured (or already seeded) zone
        // stays that zone's business; a name clash would break the name-keyed
        // savegame matching
        bool skip = FindZoneIndex(name) >= 0;
        for (int j = 0; j < _zones.Size() && !skip; j++)
        {
            skip = Dist2DSq(pos.X(), pos.Z(), _zones[j].pos.X(), _zones[j].pos.Z()) < SeedDedupDistSq;
        }
        if (skip)
        {
            continue;
        }
        if (_zones.Size() >= MaxZones)
        {
            LOG_WARN(Core, "ZoneRegistry: zone cap ({}) reached seeding cities - remaining Names entries skipped",
                     MaxZones);
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
        z.pos = pos;
        _zones.Add(z);
        seeded++;
    }
}

void ZoneRegistry::CollectTownNames(const ParamEntry* zonesCfg, const ParamEntry* namesCfg, AutoArray<RString>& out)
{
    out.Clear();
    // every authored zone counts for the dedup (a Names town on top of an
    // authored OUTPOST is skipped by SeedCityZones too); only CITY ones are
    // towns the player can start in
    AutoArray<RString> zoneNames;
    AutoArray<Vector3> zonePos;
    bool seedCities = false;
    if (zonesCfg)
    {
        seedCities = zonesCfg->ReadValue("seedCities", 0.0f) != 0.0f;
        if (const ParamEntry* zones = zonesCfg->FindEntry("Zones"))
        {
            for (int i = 0; i < zones->GetEntryCount(); i++)
            {
                const ParamEntry& e = zones->GetEntry(i);
                if (!e.IsClass())
                {
                    continue;
                }
                RString name = e.ReadValue("name", RString(e.GetName()));
                RString type = e.ReadValue("type", RString("OUTPOST"));
                Vector3 pos = VZero;
                const ParamEntry* position = e.FindEntry("position");
                if (position && position->IsArray() && position->GetSize() >= 2)
                {
                    pos = Vector3((float)(*position)[0], 0.0f, (float)(*position)[1]);
                }
                zoneNames.Add(name);
                zonePos.Add(pos);
                if (stricmp(type, "CITY") == 0)
                {
                    out.Add(name);
                }
            }
        }
    }
    if (!seedCities || !namesCfg)
    {
        return;
    }
    for (int i = 0; i < namesCfg->GetEntryCount(); i++)
    {
        RString name;
        Vector3 pos;
        if (!NamesEntryIsTown(namesCfg->GetEntry(i), name, pos))
        {
            continue;
        }
        bool skip = false;
        for (int j = 0; j < zoneNames.Size() && !skip; j++)
        {
            skip = stricmp(zoneNames[j], name) == 0 ||
                   Dist2DSq(pos.X(), pos.Z(), zonePos[j].X(), zonePos[j].Z()) < SeedDedupDistSq;
        }
        if (skip)
        {
            continue;
        }
        if (zoneNames.Size() >= MaxZones)
        {
            return; // SeedCityZones stops here too
        }
        zoneNames.Add(name);
        zonePos.Add(pos);
        out.Add(name);
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

const FactionRecord* ZoneRegistry::FindFactionByClass(const char* className) const
{
    if (!className || !*className)
    {
        return nullptr;
    }
    for (int i = 0; i < _factions.Size(); i++)
    {
        if (stricmp(_factions[i].className, className) == 0)
        {
            return &_factions[i];
        }
    }
    return nullptr;
}

FactionRecord* ZoneRegistry::FindFactionByClassMutable(const char* className)
{
    return const_cast<FactionRecord*>(FindFactionByClass(className));
}

const FactionRecord* ZoneRegistry::FindFactionForSide(const char* side) const
{
    if (side)
    {
        if (_occupierFaction.GetLength() > 0 && stricmp(side, _occupierSide) == 0)
        {
            if (const FactionRecord* f = FindFactionByClass(_occupierFaction))
            {
                return f;
            }
        }
        if (_resistanceFaction.GetLength() > 0 && stricmp(side, _resistanceSide) == 0)
        {
            if (const FactionRecord* f = FindFactionByClass(_resistanceFaction))
            {
                return f;
            }
        }
    }
    return FindFaction(side);
}

int ZoneRegistry::TierIndex(const FactionRecord& f, float warLevel)
{
    int tier = 0;
    for (int i = 0; i < f.tierThresholds.Size(); i++)
    {
        if (warLevel >= f.tierThresholds[i])
        {
            tier++;
        }
    }
    if (tier >= f.tiers.Size())
    {
        tier = f.tiers.Size() - 1;
    }
    return tier;
}

RString ZoneRegistry::FactionTierClass(const char* side, float warLevel) const
{
    const FactionRecord* f = FindFactionForSide(side);
    if (!f || f->tiers.Size() == 0)
    {
        return RString();
    }
    return f->tiers[TierIndex(*f, warLevel)];
}

RString ZoneRegistry::FactionCivTierClass(const char* side, float warLevel) const
{
    const FactionRecord* f = FindFactionForSide(side);
    if (!f || f->civTiers.Size() == 0)
    {
        return RString();
    }
    // the same threshold walk as TierIndex, clamped to the CIV ladder's own
    // length (civTier[] may author fewer rungs than tiers[])
    int tier = 0;
    for (int i = 0; i < f->tierThresholds.Size(); i++)
    {
        if (warLevel >= f->tierThresholds[i])
        {
            tier++;
        }
    }
    if (tier >= f->civTiers.Size())
    {
        tier = f->civTiers.Size() - 1;
    }
    return f->civTiers[tier];
}

void ZoneRegistry::FactionSquad(const char* side, float warLevel, int count, AutoArray<RString>& out) const
{
    out.Clear();
    const FactionRecord* f = FindFactionForSide(side);
    if (!f || f->tiers.Size() == 0 || count <= 0)
    {
        return;
    }
    int tier = TierIndex(*f, warLevel);
    const RString& rifleman = f->tiers[tier];
    // a tier fields a role only when its slot is authored non-empty; every
    // absent role becomes a rifleman ("only as makes realistic sense" - the
    // template never invents a specialist the faction does not have)
    auto role = [tier, &rifleman](const AutoArray<RString>& arr) -> const RString&
    { return (tier < arr.Size() && arr[tier].GetLength() > 0) ? arr[tier] : rifleman; };

    // deterministic military template (no RNG: same inputs, same squad -
    // save/load and test friendly).  Gates by squad size:
    //   MG     1 from 3 men, 2 from 9
    //   AT     1 from 5 men, 2 from 11
    //   medic  1 from 6 men
    //   sniper 1 from 10 men, only when the tier authors one
    int mg = count >= 3 ? (count >= 9 ? 2 : 1) : 0;
    int at = count >= 5 ? (count >= 11 ? 2 : 1) : 0;
    int medic = count >= 6 ? 1 : 0;
    bool tierHasSniper = tier < f->tiersSniper.Size() && f->tiersSniper[tier].GetLength() > 0;
    int sniper = (count >= 10 && tierHasSniper) ? 1 : 0;

    // slot 0 is the leader slot (spawned as a rifleman here; garrison/QRF
    // callers substitute the faction officer); specials interleave with
    // riflemen so partial groups still come out mixed
    AutoArray<RString> specials;
    if (mg >= 1)
    {
        specials.Add(role(f->tiersMG));
    }
    if (at >= 1)
    {
        specials.Add(role(f->tiersAT));
    }
    if (medic >= 1)
    {
        specials.Add(role(f->tiersMedic));
    }
    if (mg >= 2)
    {
        specials.Add(role(f->tiersMG));
    }
    if (at >= 2)
    {
        specials.Add(role(f->tiersAT));
    }
    if (sniper >= 1)
    {
        specials.Add(role(f->tiersSniper));
    }

    out.Add(rifleman); // leader slot
    int nextSpecial = 0;
    while (out.Size() < count)
    {
        if (nextSpecial < specials.Size())
        {
            out.Add(specials[nextSpecial]);
            nextSpecial++;
            if (out.Size() < count && nextSpecial < specials.Size())
            {
                out.Add(rifleman); // breathing room between specialists
            }
        }
        else
        {
            out.Add(rifleman);
        }
    }
}

RString ZoneRegistry::FactionVehicle(const char* side, float warLevel) const
{
    const FactionRecord* f = FindFactionForSide(side);
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

void ZoneRegistry::FactionCivVehicles(const char* side, AutoArray<RString>& out) const
{
    out.Clear();
    const FactionRecord* f = FindFactionForSide(side);
    if (!f)
    {
        return;
    }
    for (int i = 0; i < f->civVehicles.Size(); i++)
    {
        out.Add(f->civVehicles[i]);
    }
}

RString ZoneRegistry::FactionValue(const char* side, const char* key) const
{
    const FactionRecord* f = FindFactionForSide(side);
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
    if (stricmp(name, "captureStarted") == 0)
    {
        return ZECaptureStarted;
    }
    if (stricmp(name, "contested") == 0)
    {
        return ZEContested;
    }
    if (stricmp(name, "captureLost") == 0)
    {
        return ZECaptureLost;
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
    // once per campaign load, as soon as the world has centers to weld: the
    // two campaign sides are allies by default on half the possible pairings
    // (see ApplyCampaignFriendship).  It returns true once the question is
    // SETTLED - welded, or permanently unweldable - so this latches either way
    // and only a still-building world is retried.
    if (!_friendshipApplied && ApplyCampaignFriendship())
    {
        _friendshipApplied = true;
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
            // outside the bubble the meters FREEZE (no gain, no decay) -
            // consistent with the world-bubble design: nothing happens to
            // ground the simulation is not looking at
            continue;
        }

        int guer = i < in.guerCount.Size() ? in.guerCount[i] : 0;
        int occ = i < in.occCount.Size() ? in.occCount[i] : 0;
        bool isCity = stricmp(z.type, "CITY") == 0;
        bool milEligible = !isCity && stricmp(z.owner, _occupierSide) == 0;
        // an occupier-ADMINISTERED town accrues underground support like a
        // neutral one (administration collapse, not assault); third-party
        // towns stay out of reach
        bool cityEligible = isCity && (stricmp(z.owner, "NEUTRAL") == 0 || stricmp(z.owner, _occupierSide) == 0);
        if (!milEligible && !cityEligible)
        {
            z.contestedLastTick = false;
            continue;
        }

        bool milCap = false;
        bool cityCap = false;

        if (milEligible)
        {
            // military: a consolidation hold, not a touch.  The capture meter
            // climbs only while resistance units hold the zone area with NO
            // live occupier unit inside it - positional presence (occCount)
            // covers garrison, QRF, patrols and mission-placed troops alike;
            // the bookkeeping mirror liveOccupiers decides nothing.  An
            // unspawned reserve (garrison) also defends: within the bubble
            // GarrisonCache materializes it within one pass.
            bool attackers = guer > 0;
            bool defenders = occ > 0 || z.garrison >= 1;
            bool contested = attackers && defenders;
            // straggler mitigation: overwhelming defenders re-secure their
            // post instead of being held hostage by one hidden attacker.
            // Applies only toward DEFENDED - a single live defender always
            // blocks SECURING, whatever the attacker superiority.
            if (contested && _tuning.contestOutnumberRatio > 0 &&
                (float)occ >= _tuning.contestOutnumberRatio * (float)guer)
            {
                contested = false;
                attackers = false;
            }

            float before = z.capture;
            if (attackers && !defenders)
            {
                // SECURING: rate scales with crew up to the cap
                float rate = z.captureRateOverride > 0 ? z.captureRateOverride : _tuning.captureRate;
                float crew = (float)guer;
                if (crew > _tuning.captureCrewCap)
                {
                    crew = _tuning.captureCrewCap;
                }
                z.capture += rate * crew;
                if (z.capture > 100)
                {
                    z.capture = 100;
                }
                if (before <= 0 && z.capture > 0)
                {
                    ZoneEventRecord ev;
                    ev.type = ZECaptureStarted;
                    ev.zoneIndex = i;
                    fired.Add(ev);
                }
                milCap = z.capture >= 100;
            }
            else if (contested)
            {
                // CONTESTED: frozen - the firefight resolves it; standing to
                // fight is never punished with decay
                if (!z.contestedLastTick && z.capture > 0)
                {
                    ZoneEventRecord ev;
                    ev.type = ZEContested;
                    ev.zoneIndex = i;
                    fired.Add(ev);
                }
            }
            else
            {
                // DEFENDED (occupiers re-secure fast) or ABANDONED (slow fade)
                float decay = (occ > 0 || z.garrison >= 1) ? _tuning.captureDecayDefended : _tuning.captureDecayAbandoned;
                z.capture -= decay;
                if (z.capture < 0)
                {
                    z.capture = 0;
                }
                if (before > 0 && z.capture <= 0)
                {
                    ZoneEventRecord ev;
                    ev.type = ZECaptureLost;
                    ev.zoneIndex = i;
                    fired.Add(ev);
                }
            }
            z.contestedLastTick = contested;
        }

        if (cityEligible)
        {
            // city: support, not force - and never while the occupier is
            // watching.  Occupier-only presence intimidates support back
            // toward the floor (it can suppress expression, not erase the
            // sentiment); script-side writers (GM_fnSupportAdd) are separate
            // and unfloored.  An unspawned garrison reserve counts as
            // occupation, mirroring the military predicate: the zone tick
            // runs before GarrisonCache materializes it inside the bubble.
            bool guerHere = guer > 0;
            bool occHere = occ > 0 || z.garrison >= 1;
            if (guerHere && !occHere)
            {
                z.support += _tuning.supportRate;
                if (z.support > 100)
                {
                    z.support = 100;
                }
            }
            else if (occHere && !guerHere && z.support > _tuning.supportDecayFloor)
            {
                z.support -= _tuning.supportDecayOccupied;
                if (z.support < _tuning.supportDecayFloor)
                {
                    z.support = _tuning.supportDecayFloor;
                }
            }
            z.contestedLastTick = guerHere && occHere;

            // threshold decoupled from flip: crossing supportFlip announces
            // the town is READY; the flip itself needs resistance fighters
            // standing in an occupier-free town on the same tick
            if (z.support >= _tuning.supportFlip)
            {
                if (!z.supportReadyNotified)
                {
                    z.supportReadyNotified = true;
                    ZoneEventRecord ev;
                    ev.type = ZESupportThreshold;
                    ev.zoneIndex = i;
                    fired.Add(ev);
                }
                cityCap = guerHere && !occHere;
            }
            else
            {
                z.supportReadyNotified = false; // re-arm if support fell back
            }
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
        z.capture = 0;
        z.contestedLastTick = false;
        // the occupier presence is gone either way; only military zones open
        // the income tap (CITY income stays with the economy script)
        z.garrison = 0;
        z.liveOccupiers = 0;
        if (milCap && z.income < 1)
        {
            z.income = _tuning.defaultIncome;
        }
        ZoneEventRecord ev;
        ev.type = ZECaptured;
        ev.zoneIndex = i;
        fired.Add(ev);
    }
}

// count one side's live units within areaSq of each zone center - the
// positional presence signal both capture directions run on.  excludePerson
// (may be null) is skipped: the undercover real player, whom the AI cannot
// engage, is not an armed force securing ground.  out must be pre-sized and
// zeroed by the caller; a null center leaves it untouched (no units).
//
// Civilian-OUTFIT policy (issue #25 M3.4, decided 2026-07-29): units whose
// class came through a Civ-family key (playerClassCiv, recruit*Civ,
// holdClassCiv, civTier[]) COUNT here, exactly like warrior bodies.  The
// undercover-player rationale does not transfer: the AI can and does engage
// a disguised AI fighter once the vanilla side-resolve identifies him
// (~27 m, or instantly when he fires - Target.cpp fired-at branch), unlike
// the gmUndercover player whom the AI can never engage.  Excluding the
// family would also make zone capture impossible for a civilian-outfit
// campaign (the player's whole force wears Civ bodies) and would break
// issue #16's militia-holds-its-own-town case.  The "count only while not
// currently reading civilian to any occupier observer" position stays
// infeasible until the Phase-3 N-subject undercover generalization
// (#19/#20/#21) exists; revisit there.
static void CountSidePresence(AICenter* center, const AutoArray<ZoneRecord>& zones, float areaSq,
                              const Person* excludePerson, AutoArray<int>& out)
{
    if (!center)
    {
        return;
    }
    for (int g = 0; g < center->NGroups(); g++)
    {
        AIGroup* grp = center->GetGroup(g);
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
            if (excludePerson && unit->GetPerson() == excludePerson)
            {
                continue;
            }
            Vector3 pos = unit->Position();
            for (int i = 0; i < zones.Size(); i++)
            {
                if (Dist2DSq(pos.X(), pos.Z(), zones[i].pos.X(), zones[i].pos.Z()) < areaSq)
                {
                    out[i]++;
                }
            }
        }
    }
}

void ZoneRegistry::GatherInputs(ZoneTickInputs& in) const
{
    const int n = _zones.Size();
    in.guerCount.Resize(n);
    in.occCount.Resize(n);
    for (int i = 0; i < n; i++)
    {
        in.guerCount[i] = 0;
        in.occCount[i] = 0;
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

    // undercover global is script-owned; nil (mission never set it) == false
    // (same read as AlertMachine::GatherInputs)
    if (GameState* gstate = world->GetGameState())
    {
        GameValue undercover = gstate->VarGet("gmundercover");
        in.playerUndercover = undercover.GetType() == GameBool && (GameBoolType)undercover;
    }

    // enumerate each side's live units; the player is included in his side's
    // count except while undercover.  No center yet == no units (do NOT
    // create one here - GarrisonCache::EnsureCenter is the creating path)
    const float areaSq = _tuning.zoneArea * _tuning.zoneArea;
    const Person* exclude = in.playerUndercover ? player : nullptr;
    CountSidePresence(FindSideCenter(_resistanceSide), _zones, areaSq, exclude, in.guerCount);
    CountSidePresence(FindSideCenter(_occupierSide), _zones, areaSq, nullptr, in.occCount);
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
        RString text = "";
        if (z.revealed)
        {
            if (z.contestedLastTick)
            {
                // one stable contested state - no flashing (ColorWhite is
                // verified present in the 1.99 CfgMarkerColors; ColorOrange
                // is not)
                color = "ColorWhite";
            }
            else if (stricmp(z.owner, _resistanceSide) == 0)
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

            // progress feedback on the map label; capture % quantized to 10s
            // so a full solo capture rewrites the text ~10 times, not 17
            bool isCity = stricmp(z.type, "CITY") == 0;
            char label[256];
            if (z.contestedLastTick)
            {
                snprintf(label, sizeof(label), "%s - CONTESTED", (const char*)z.name);
                text = label;
            }
            else if (!isCity && z.capture > 0 && stricmp(z.owner, _occupierSide) == 0)
            {
                // 10%-quantized, clamped away from the lying edges: never
                // "0%" on a moving meter, never "100%" before the flip
                int pct = (int)((z.capture + 5.0f) / 10.0f) * 10;
                if (pct < 10)
                {
                    pct = 10;
                }
                if (pct > 90)
                {
                    pct = 90;
                }
                snprintf(label, sizeof(label), "%s - securing %d%%", (const char*)z.name, pct);
                text = label;
            }
            else if (isCity && (stricmp(z.owner, "NEUTRAL") == 0 || stricmp(z.owner, _occupierSide) == 0) &&
                     z.support >= _tuning.supportFlip)
            {
                snprintf(label, sizeof(label), "%s - READY", (const char*)z.name);
                text = label;
            }
            else if (isCity && (stricmp(z.owner, "NEUTRAL") == 0 || stricmp(z.owner, _occupierSide) == 0) &&
                     z.support > _tuning.seedCitySupport)
            {
                snprintf(label, sizeof(label), "%s - support %d/%d", (const char*)z.name, (int)z.support,
                         (int)_tuning.supportFlip);
                text = label;
            }
            else
            {
                text = z.name;
            }
        }

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
    // presence-tolerant: absent in pre-consolidation saves, defaulting to
    // "no capture in progress" - semantically correct, no version bump
    PARAM_CHECK(ar.Serialize("capture", capture, 1, 0.0f))
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
        z.capture = row.capture;
        z.revealed = row.revealed;
        // reconstruct the one-shot threshold edge from the loaded value: a
        // town saved in the READY-waiting state must not re-announce itself
        // on every reload
        z.supportReadyNotified = z.support >= _tuning.supportFlip;
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
            row.capture = z.capture;
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
        _pendingOccupierFaction = _occupierFaction;
        _pendingResistanceFaction = _resistanceFaction;
    }
    PARAM_CHECK(ar.Serialize("occupierSide", _pendingOccupierSide, 1, RString()))
    PARAM_CHECK(ar.Serialize("resistanceSide", _pendingResistanceSide, 1, RString()))
    // the sides alone no longer name the campaign's picks: several rosters may
    // share a side, and a rebased one is not on its authored side at all.
    // Same additive, presence-tolerant contract as the two above.
    PARAM_CHECK(ar.Serialize("occupierFaction", _pendingOccupierFaction, 1, RString()))
    PARAM_CHECK(ar.Serialize("resistanceFaction", _pendingResistanceFaction, 1, RString()))

    PARAM_CHECK(ar.Serialize("onCaptured", _handlers[ZECaptured], 1, RString()))
    PARAM_CHECK(ar.Serialize("onSupportThreshold", _handlers[ZESupportThreshold], 1, RString()))
    PARAM_CHECK(ar.Serialize("onRevealed", _handlers[ZERevealed], 1, RString()))
    PARAM_CHECK(ar.Serialize("onCampaignLoaded", _handlers[ZECampaignLoaded], 1, RString()))
    // absent in pre-consolidation saves: the slots load empty and the events
    // fire into nothing (SQS scripts resume their serialized text, so no
    // script-side re-arm can reach an old campaign - accepted degradation:
    // mechanics apply, the extra narration does not)
    PARAM_CHECK(ar.Serialize("onCaptureStarted", _handlers[ZECaptureStarted], 1, RString()))
    PARAM_CHECK(ar.Serialize("onContested", _handlers[ZEContested], 1, RString()))
    PARAM_CHECK(ar.Serialize("onCaptureLost", _handlers[ZECaptureLost], 1, RString()))
    PARAM_CHECK(ar.Serialize("Zones", _pending, 1))

    if (ar.IsSaving())
    {
        _pending.Clear();
        _pendingOccupierSide = RString();
        _pendingResistanceSide = RString();
        _pendingOccupierFaction = RString();
        _pendingResistanceFaction = RString();
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
        if (_pendingOccupierFaction.GetLength() > 0)
        {
            _occupierFaction = _pendingOccupierFaction;
        }
        if (_pendingResistanceFaction.GetLength() > 0)
        {
            _resistanceFaction = _pendingResistanceFaction;
        }
        // the faction table above came from a FRESH LoadFromConfig, so its
        // side fields describe the config's idea of the campaign, not the
        // save's - re-point them at the restored picks before anything reads
        // a roster off a side
        RebindFactionSides();
        ApplyOwnerTokens();
        _pendingOccupierSide = RString();
        _pendingResistanceSide = RString();
        _pendingOccupierFaction = RString();
        _pendingResistanceFaction = RString();
        ApplyPendingLoad();
        _pending.Clear();
        _friendshipApplied = false; // world state: re-welded on the next tick
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

    // Undercover layer state: same optional-subclass contract as "Alert"
    // (absent in saves written before the UndercoverSystem existed)
    if (ar.IsSubclass("Undercover"))
    {
        ParamArchive arUndercover;
        if (ar.OpenSubclass("Undercover", arUndercover))
        {
            PARAM_CHECK(UndercoverSystem::Instance().Serialize(arUndercover))
            ar.CloseSubclass(arUndercover);
        }
    }
    return LSOK;
}

} // namespace Poseidon::Guerrilla
