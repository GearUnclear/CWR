#include <Poseidon/Game/Guerrilla/OutfitSelect.hpp>

#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp> // ClassProbe / ParsClassProbe

#include <Poseidon/AI/ArcadeTemplate.hpp>
#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/IO/ParamFileExt.hpp> // Pars / ExtParsMission

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
        return RString();
    }
    RString civClass = faction->ReadValue("playerClassCiv", RString());
    if (civClass.GetLength() == 0)
    {
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
        // token path, never a substitute body (see the header).
        LOG_WARN(Core,
                 "Guerrilla body: picked player class '{}' not in the loaded data package - keeping the authored "
                 "player class",
                 selPlayerClass);
        return RString();
    }
    return ResolveCivilianPlayerClass(zonesCfg, factionsCfg, selOutfit, selResistance, probe);
}

void ApplyPlayerOutfitSelection(ArcadeTemplate& t)
{
    ArcadeUnitInfo* uInfo = t.FindPlayer();
    if (!uInfo)
    {
        return;
    }
    // same ExtParsMission-then-Pars lookup as ZoneRegistry::LoadFromConfig;
    // both configs present == this is a Guerrilla template
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
    if (!zones || !factions)
    {
        return;
    }
    RString selPlayerClass = ReadMenuSelection("gmselplayerclass");
    RString selOutfit = ReadMenuSelection("gmseloutfit");
    if (selPlayerClass.GetLength() == 0 && selOutfit.GetLength() == 0)
    {
        return; // untouched screen (or non-Guerrilla flow): authored class stands
    }
    RString selResistance = ReadMenuSelection("gmselresistance");
    ParsClassProbe probe;
    RString newClass = ResolvePlayerBodyClass(zones, factions, selPlayerClass, selOutfit, selResistance, probe);
    if (newClass.GetLength() == 0 || stricmp(newClass, uInfo->vehicle) == 0)
    {
        return;
    }
    // Note: the body keeps its CONFIG side for distant identification (the
    // vanilla side-resolve ladder, stolen-uniform band at 1.35 / real ID at
    // 1.5) - accepted emergent gameplay, see the header. Instance side is
    // welded to the mission side field, so any body fights as GUER.
    LOG_INFO(Core, "Guerrilla outfit: substituting player class '{}' -> '{}' ({})", (const char*)uInfo->vehicle,
             (const char*)newClass, selPlayerClass.GetLength() > 0 ? "gmSelPlayerClass pick" : "gmSelOutfit=civilian");
    uInfo->vehicle = newClass;
}

} // namespace Poseidon::Guerrilla
