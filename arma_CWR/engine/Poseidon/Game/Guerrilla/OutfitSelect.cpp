#include <Poseidon/Game/Guerrilla/OutfitSelect.hpp>

#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>   // ClassProbe / ParsClassProbe
#include <Poseidon/Game/Guerrilla/FactionSources.hpp> // global U island faction table (issue #54 A1)

#include <Poseidon/AI/ArcadeTemplate.hpp>
#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/IO/ParamFileExt.hpp>     // Pars / ExtParsMission / GetShapeName
#include <Poseidon/IO/Streams/QBStream.hpp> // QIFStreamB::FileExist (shape-file probe)

#include <Evaluator/express.hpp> // GameState / GameValue (gmSel* reads)

#include <Poseidon/World/World.hpp>

#include <Poseidon/Foundation/Framework/DebugLog.hpp>
#include <Poseidon/Foundation/platform.hpp> // stricmp

namespace Poseidon::Guerrilla
{

namespace
{

// script/campaign global published by the new-game UI - the same VarGet
// convention ZoneRegistry's ReadSideSelection uses (lowercased name, nil
// tolerated)
RString ReadMenuSelection(const char* lowercaseName)
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

// Optional string array off a config class. FindEntry (not
// FindEntryNoInheritance) so an array INHERITED from a base class is found:
// ParamClass::FindEntry resolves through _base, and most soldier bodies derive
// their weapons[]/magazines[] from a base class rather than restating them. Null
// when the key is absent or is not an array (ParamEntry::GetSize on a non-array
// raises an EMError, so the IsArray gate is not merely defensive).
const ParamEntry* FindClassArray(const ParamEntry* cls, const char* name)
{
    if (!cls)
    {
        return nullptr;
    }
    const ParamEntry* arr = cls->FindEntry(name);
    if (!arr || !arr->IsArray())
    {
        return nullptr;
    }
    return arr;
}

// Record the addon that owns `entry`. An EMPTY owner means base-game content,
// which ParamOwnerList reports visible unconditionally, so it is skipped: adding
// it would put a meaningless empty name into the active list.
void AddOwnerOf(const ParamEntry* entry, FindArrayRStringCI& addons)
{
    if (!entry)
    {
        return;
    }
    const RStringB& owner = entry->GetOwner();
    if (owner.GetLength() > 0)
    {
        addons.AddUnique(RString(owner));
    }
}

} // namespace

const ParamEntry* FindGuerrillaFactionEntry(const ParamEntry* factionsCfg, const char* selection)
{
    if (!factionsCfg || !selection || !*selection)
    {
        return nullptr;
    }
    for (int i = 0; i < factionsCfg->GetEntryCount(); i++)
    {
        const ParamEntry& e = factionsCfg->GetEntry(i);
        if (!e.IsClass())
        {
            continue;
        }
        RString side = e.ReadValue("side", RString(e.GetName()));
        if (side.GetLength() > 0 && stricmp(side, selection) == 0)
        {
            return &e;
        }
    }
    for (int i = 0; i < factionsCfg->GetEntryCount(); i++)
    {
        const ParamEntry& e = factionsCfg->GetEntry(i);
        if (e.IsClass() && stricmp(e.GetName(), selection) == 0)
        {
            return &e;
        }
    }
    return nullptr;
}

RString ResolveCivilianPlayerClass(const ParamEntry* zonesCfg, const ParamEntry* factionsCfg, const char* selOutfit,
                                   const char* selResistance, const ClassProbe& probe)
{
    // the warrior value IS the authored class: only "civilian" substitutes
    if (!selOutfit || stricmp(selOutfit, "civilian") != 0)
    {
        return RString();
    }
    if (!factionsCfg)
    {
        // issue #46: an explicit civilian pick that cannot be resolved is a
        // DROPPED selection, so every rung of this ladder says so. It costs
        // nothing in the normal case - the seam only gets here when the player
        // actually picked CIVILIAN, "warrior" and nil having returned above.
        LOG_WARN(Core, "Guerrilla outfit: gmSelOutfit=civilian pending, but neither the mission's description.ext nor "
                       "the loaded config carries CfgGuerrillaFactions - keeping the authored player class");
        return RString();
    }
    // resistance-block precedence, mirroring ZoneRegistry::LoadFromParams:
    // the published selection > the mission's defaultResistance key (a key of
    // CfgGuerrillaZones, not of the factions class) > the built-in GUER side
    const ParamEntry* faction = FindGuerrillaFactionEntry(factionsCfg, selResistance);
    if (!faction && zonesCfg)
    {
        RString defResistance = zonesCfg->ReadValue("defaultResistance", RString());
        faction = FindGuerrillaFactionEntry(factionsCfg, defResistance);
    }
    if (!faction)
    {
        faction = FindGuerrillaFactionEntry(factionsCfg, "GUER");
    }
    if (!faction)
    {
        LOG_WARN(Core,
                 "Guerrilla outfit: gmSelOutfit=civilian pending, but CfgGuerrillaFactions carries no block for "
                 "gmSelResistance '{}', for defaultResistance, or for the built-in GUER side - keeping the authored "
                 "player class",
                 (selResistance && *selResistance) ? selResistance : "<none>");
        return RString();
    }
    RString civClass = faction->ReadValue("playerClassCiv", RString());
    if (civClass.GetLength() == 0)
    {
        // The new-game UI hides the CIVILIAN row for a descriptor with no
        // playerClassCiv, so reaching here means the menu and the mission
        // resolved DIFFERENT resistance blocks (see GuerrillaNewGame's
        // per-island description.ext peek vs this seam's ExtParsMission read).
        LOG_WARN(Core,
                 "Guerrilla outfit: gmSelOutfit=civilian pending, but faction '{}' authors no playerClassCiv - keeping "
                 "the authored player class",
                 (const char*)faction->GetName());
        return RString(); // the descriptor offers no civilian outfit
    }
    if (!probe.Exists("CfgVehicles", civClass))
    {
        // plan-15 shaped degrade, stricter than the registry's: the player
        // seam NEVER substitutes a fallback body - the authored mission.sqm
        // class is the fallback
        LOG_WARN(Core,
                 "Guerrilla outfit: faction '{}' playerClassCiv '{}' not in the loaded data package - keeping the "
                 "authored player class",
                 (const char*)faction->GetName(), (const char*)civClass);
        return RString();
    }
    return civClass;
}

RString ResolveWarriorPlayerClass(const ParamEntry* zonesCfg, const ParamEntry* factionsCfg, const char* selResistance,
                                  const ClassProbe& probe)
{
    if (!selResistance || !*selResistance || !factionsCfg)
    {
        return RString(); // nothing picked, or nothing to resolve it against
    }
    const ParamEntry* picked = FindGuerrillaFactionEntry(factionsCfg, selResistance);
    if (!picked)
    {
        // issue #46 shape: a published pick that resolves to nothing is a
        // DROPPED selection and says so. The registry will fall back to the
        // template defaults for the sides; the body follows the same rule.
        LOG_WARN(Core,
                 "Guerrilla body: gmSelResistance '{}' names no CfgGuerrillaFactions block - keeping the authored "
                 "player class",
                 selResistance);
        return RString();
    }
    // The template's OWN default resistance is the roster mission.sqm was
    // authored against: its playerClassWarrior documents the authored class,
    // so there is nothing to substitute and nothing dropped (no log).
    const ParamEntry* authored = nullptr;
    if (zonesCfg)
    {
        RString defResistance = zonesCfg->ReadValue("defaultResistance", RString());
        authored = FindGuerrillaFactionEntry(factionsCfg, defResistance);
    }
    if (!authored)
    {
        authored = FindGuerrillaFactionEntry(factionsCfg, "GUER");
    }
    if (picked == authored)
    {
        return RString();
    }
    RString warrior = picked->ReadValue("playerClassWarrior", RString());
    if (warrior.GetLength() == 0)
    {
        LOG_WARN(Core,
                 "Guerrilla body: resistance '{}' authors no playerClassWarrior - keeping the authored player class "
                 "(the squad still spawns from its tiers[])",
                 (const char*)picked->GetName());
        return RString();
    }
    if (!probe.Exists("CfgVehicles", warrior))
    {
        // the menu greys this faction out (GuerrillaFactionIssue probes the
        // same key), so reaching here means a direct launch or a stale bank
        LOG_WARN(Core,
                 "Guerrilla body: resistance '{}' playerClassWarrior '{}' not in the loaded data package - keeping "
                 "the authored player class",
                 (const char*)picked->GetName(), (const char*)warrior);
        return RString();
    }
    return warrior;
}

RString ResolvePlayerBodyClass(const ParamEntry* zonesCfg, const ParamEntry* factionsCfg, const char* selPlayerClass,
                               const char* selOutfit, const char* selResistance, const ClassProbe& probe)
{
    if (selPlayerClass && *selPlayerClass)
    {
        if (probe.Exists("CfgVehicles", selPlayerClass))
        {
            return RString(selPlayerClass);
        }
        // The pick replaced the outfit resolution for the body, so its
        // failure keeps the AUTHORED class - never a fall-through to the
        // token path, never a substitute body (see the header). issue #46
        // seam 3: when the player ALSO expressed a civilian token, two
        // selections are being dropped here, not one - name the second, so the
        // log never leaves the fall-through rule to be inferred.
        LOG_WARN(Core,
                 "Guerrilla body: picked player class '{}' not in the loaded data package - keeping the authored "
                 "player class{}",
                 selPlayerClass,
                 (selOutfit && stricmp(selOutfit, "civilian") == 0)
                     ? " (gmSelOutfit=civilian is deliberately NOT used as a fallback body)"
                     : "");
        return RString();
    }
    if (selOutfit && stricmp(selOutfit, "civilian") == 0)
    {
        return ResolveCivilianPlayerClass(zonesCfg, factionsCfg, selOutfit, selResistance, probe);
    }
    // WARRIOR (or no outfit token): the body follows the resistance PICK
    // (issue #54 A3) - a non-default roster substitutes its own
    // playerClassWarrior, the template's default keeps the authored class.
    return ResolveWarriorPlayerClass(zonesCfg, factionsCfg, selResistance, probe);
}

void CollectPlayerBodyAddons(const ParamEntry* vehiclesCfg, const ParamEntry* weaponsCfg,
                             const ParamEntry* magazinesCfg, RString className, FindArrayRStringCI& addons)
{
    if (!vehiclesCfg || className.GetLength() == 0)
    {
        return;
    }
    const ParamEntry* body = vehiclesCfg->FindEntry(className);
    if (!body)
    {
        return; // unknown class - nothing to activate, and nothing will spawn
    }
    AddOwnerOf(body, addons);

    // The body's own magazines[]: a mod body commonly carries mod ammo whose
    // CfgMagazines class lives in a DIFFERENT pbo than the body itself.
    if (const ParamEntry* mags = FindClassArray(body, "magazines"))
    {
        for (int i = 0; i < mags->GetSize(); i++)
        {
            AddOwnerOf(magazinesCfg ? magazinesCfg->FindEntry((RStringB)(*mags)[i]) : nullptr, addons);
        }
    }
    // The body's weapons[], plus each weapon's own magazines[]: the weapon
    // config names the ammo it accepts, and the engine touches those magazine
    // classes when it kits the unit out, so their owners have to be visible too.
    if (const ParamEntry* weapons = FindClassArray(body, "weapons"))
    {
        for (int i = 0; i < weapons->GetSize(); i++)
        {
            const ParamEntry* weapon = weaponsCfg ? weaponsCfg->FindEntry((RStringB)(*weapons)[i]) : nullptr;
            AddOwnerOf(weapon, addons);
            if (const ParamEntry* wMags = FindClassArray(weapon, "magazines"))
            {
                for (int j = 0; j < wMags->GetSize(); j++)
                {
                    AddOwnerOf(magazinesCfg ? magazinesCfg->FindEntry((RStringB)(*wMags)[j]) : nullptr, addons);
                }
            }
        }
    }
}

RString PlayerBodyModelIssue(const ParamEntry* vehiclesCfg, RString className,
                             const std::function<bool(RString)>& shapeFileExists)
{
    if (!vehiclesCfg || className.GetLength() == 0)
    {
        return RString();
    }
    const ParamEntry* body = vehiclesCfg->FindEntry(className);
    if (!body)
    {
        return RString(); // not this check's business - the ClassProbe rejects it first
    }
    RString model = body->ReadValue("model", RString());
    if (model.GetLength() == 0)
    {
        return RString("the class authors no model");
    }
    RString shape = GetShapeName(model);
    if (shapeFileExists && !shapeFileExists(shape))
    {
        return RString("shape file '") + shape + RString("' is not in the loaded data package");
    }
    return RString();
}

void ApplyPlayerOutfitSelection(ArcadeTemplate& t)
{
    // issue #46 seam 1: the published selections are read FIRST, and the
    // descriptor blocks below are no longer a gate. A BODY pick needs neither
    // CfgGuerrillaZones nor CfgGuerrillaFactions (ResolvePlayerBodyClass probes
    // CfgVehicles and nothing else), and neither does the issue-#45 addon
    // activation - so requiring both used to make a template that authors one
    // block but not the other discard every body pick, silently, with the
    // mission author's typo as the only cause. The "is this a Guerrilla
    // template" signal is the pending selection itself: the untouched-screen
    // early-out below keeps every non-Guerrilla launch byte-identical, and
    // every ordinary launch path (single mission, campaign, editor preview,
    // reference mission) runs GStats.ClearAll() before it publishes, so the
    // campaign variable bank cannot leak a stale pick into one.
    RString selPlayerClass = ReadMenuSelection("gmselplayerclass");
    RString selOutfit = ReadMenuSelection("gmseloutfit");
    // the resistance pick is the third channel (issue #54 A3): a non-default
    // roster brings its own warrior body
    RString selResistance = ReadMenuSelection("gmselresistance");
    if (selPlayerClass.GetLength() == 0 && selOutfit.GetLength() == 0 && selResistance.GetLength() == 0)
    {
        return; // untouched screen (or non-Guerrilla flow): authored class stands
    }
    ArcadeUnitInfo* uInfo = t.FindPlayer();
    if (!uInfo)
    {
        LOG_WARN(Core,
                 "Guerrilla body: a selection is pending (gmSelPlayerClass '{}', gmSelOutfit '{}') but the template "
                 "has no player unit - nothing to substitute",
                 (const char*)selPlayerClass, (const char*)selOutfit);
        return;
    }
    // the zones block: ExtParsMission then Pars, as ZoneRegistry::LoadFromConfig;
    // the factions table: the same global-U-island union the registry loads
    // (issue #54 A1), so a resistance picked out of an addon faction pack
    // resolves here too. Either may stay null: only the outfit-token path
    // reads them, and it degrades with a WARN of its own
    // (ResolveCivilianPlayerClass).
    const ParamEntry* zones = ExtParsMission.FindEntry("CfgGuerrillaZones");
    if (!zones)
    {
        zones = Pars.FindEntry("CfgGuerrillaZones");
    }
    FactionSources factionSources;
    BuildFactionSourcesFromEngine(factionSources);
    const ParamEntry* factions = factionSources.Factions();
    ParsClassProbe probe;
    RString newClass = ResolvePlayerBodyClass(zones, factions, selPlayerClass, selOutfit, selResistance, probe);
    if (newClass.GetLength() == 0)
    {
        // Every path that DROPS something logs its reason (issue #46). The one
        // silent return is gmSelOutfit="warrior"/nil with no pick, which drops
        // nothing: that token is defined as the authored class.
        return;
    }
    // issue #46 seam 4: the ClassProbe above is strictly weaker than the menu
    // roster's gate, which also requires a model whose .p3d the package ships
    // (GuerrillaListPlayerBodies). A class that passes one and fails the other
    // is unreachable through the CHARACTER browser but wide open on the
    // descriptor's playerClassCiv, which is never shape-probed anywhere.
    //
    // Measured consequence (not theory - reproduced by pointing Abel's
    // playerClassCiv at the shapeless CfgVehicles base class "Man"): the
    // process ACCESS-VIOLATES at mission start, in Man::Init -> ReactToDammage
    // -> EntityAI::GetHit, inside CreateVehicle. VehicleTypes' own shapeless
    // guard does not catch it, because the guard tests for an empty model
    // STRING while this class has a model naming a .p3d the package does not
    // ship, and ShapeBank answers that with an empty LODShape rather than a
    // null. So the failure this closes is a hard crash with no log line
    // naming the class, not a cosmetic one.
    //
    // Declining is the conservative branch, not the adventurous one: keeping
    // the authored class on a body that fails an existence test is this file's
    // standing contract (see the header), and the CHARACTER browser cannot
    // reach this code with a class that fails, because its roster is built
    // behind the same GetShapeName + FileExist gate. What it can reject is a
    // descriptor key, or a pick replayed out of the campaign bank into a
    // session whose addon set no longer ships the model - both of which crash
    // today. Recruits and garrisons spawned from the same bad descriptor key
    // are NOT covered here; that is ZoneRegistry's plan-15 pass.
    RString modelIssue = PlayerBodyModelIssue(Pars.FindEntry("CfgVehicles"), newClass,
                                              [](RString path) { return QIFStreamB::FileExist(path); });
    if (modelIssue.GetLength() > 0)
    {
        LOG_WARN(Core,
                 "Guerrilla body: player class '{}' resolved, but {} - the engine cannot build that body, so the "
                 "authored player class is kept",
                 (const char*)newClass, (const char*)modelIssue);
        return;
    }
    if (stricmp(newClass, uInfo->vehicle) != 0)
    {
        // Note: the body keeps its CONFIG side for distant identification (the
        // vanilla side-resolve ladder, stolen-uniform band at 1.35 / real ID at
        // 1.5) - accepted emergent gameplay, see the header. Instance side is
        // welded to the mission side field, so any body fights as GUER.
        LOG_INFO(Core, "Guerrilla outfit: substituting player class '{}' -> '{}' ({})", (const char*)uInfo->vehicle,
                 (const char*)newClass,
                 selPlayerClass.GetLength() > 0        ? "gmSelPlayerClass pick"
                 : stricmp(selOutfit, "civilian") == 0 ? "gmSelOutfit=civilian"
                                                       : "gmSelResistance playerClassWarrior");
        uInfo->vehicle = newClass;
    }

    // The activation below runs even when the class was ALREADY substituted, so
    // it deliberately sits outside the rewrite branch above. RetryMission and the
    // debriefing RESTART button (UI/DisplayUIMenus.cpp, UI/Map/UIMapDialogs.cpp)
    // re-run ActivateAddons(CurrentTemplate.addOns) - which CLEARS the active
    // list - and then InitVehicles, WITHOUT re-parsing the mission. CurrentTemplate
    // is the same global object this seam already mutated in place, so on that
    // second pass newClass == uInfo->vehicle: an early-out here would leave the
    // freshly cleared list without the mod owners and the denials would come
    // back on every retry. ActivateAddon is idempotent (ParamOwnerList::Add does
    // AddUnique), so re-running it costs nothing.
    //
    // Activate the picked body's addon closure (issue #45). The mission's
    // addOns[] was authored against the AUTHORED player class, and the character
    // roster is package-wide BY DESIGN (issue #43's uncapped mod-aware roster),
    // so a picked body is by construction absent from that array - launch an
    // @LoBo body on vanilla Guerrilla.Abel and World::CheckAddon denies its
    // owner plus the owners of its weapons[]/magazines[], each denial putting an
    // IDS_MSG_ADDON_MISSING warning on the player's screen (the unit still
    // spawns fully kitted, because CheckAccessCreate returns true
    // unconditionally - the defect is warning spam, not a spawn failure).
    //
    // Ordering: GWorld->ActivateAddons(CurrentTemplate.addOns) has already run
    // (e.g. UI/DisplayUIMenus.cpp before World::InitVehicles) and CLEARS the
    // active list, while this seam runs from inside InitVehicles
    // (World/WorldInit.cpp, before any CreateCenter/NewVehicle). So the additive
    // ActivateAddon lands after the reset and before every access check.
    //
    // Deliberately NOT written into t.addOns: that array is what a template SAVE
    // would serialize, and putting mod owners into a vanilla template's manifest
    // is what would turn a stock island into a hard @LoBo dependency
    // (ArcadeTemplate::Serialize returns LSNoAddOn once CheckPatch populates
    // missingAddOns). The activation is a runtime visibility grant only.
    if (GWorld)
    {
        FindArrayRStringCI bodyAddons;
        CollectPlayerBodyAddons(Pars.FindEntry("CfgVehicles"), Pars.FindEntry("CfgWeapons"),
                                Pars.FindEntry("CfgMagazines"), newClass, bodyAddons);
        if (bodyAddons.Size() > 0)
        {
            RString activated;
            for (int i = 0; i < bodyAddons.Size(); i++)
            {
                GWorld->ActivateAddon(bodyAddons[i]);
                activated = activated + (i > 0 ? RString(", ") : RString()) + bodyAddons[i];
            }
            LOG_INFO(Core, "Guerrilla outfit: activating addons for picked body '{}': {}", (const char*)newClass,
                     (const char*)activated);
        }
    }
}

} // namespace Poseidon::Guerrilla
