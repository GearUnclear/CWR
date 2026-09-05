#include <Poseidon/UI/Guerrilla/GuerrillaNewGame.hpp>
#include <Poseidon/UI/Guerrilla/GuerrillaBodyPreview.hpp>     // the shared turntable mannequin (issue #43)
#include <Poseidon/UI/Guerrilla/GuerrillaCharacterSelect.hpp> // the idc-155 child display (issue #43)
#include <Poseidon/UI/Guerrilla/GuerrillaModule.hpp>
#include <Poseidon/UI/GameModule.hpp>
#include <Poseidon/UI/OptionsUICommon.hpp>            // CreateSingleMissionBank (per-island description.ext peek)
#include <Poseidon/Game/Guerrilla/FactionSources.hpp> // global U island faction table (issue #54 A1)
#include <Poseidon/Game/Guerrilla/FactionTwins.hpp>   // shared with ZoneRegistry::ResolveSideCollisions
#include <Poseidon/Game/Guerrilla/OutfitSelect.hpp>   // FindGuerrillaFactionEntry (outfit cycler, issue #25)
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>   // CollectTownNames (START TOWN cycler, issue #16)
#include <Poseidon/Core/resincl.hpp>
#include <Poseidon/Core/Global.hpp>
#include <Poseidon/Input/InputSubsystem.hpp>  // wheel drain for the island list (see OnSimulate)
#include <Poseidon/IO/Filesystem/FileOps.hpp> // FilePathExists (template .pbo check)
#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/IO/ParamFileExt.hpp> // GetShapeName (preview model probe)
#include <Poseidon/IO/Streams/QBStream.hpp>
#include <Poseidon/Foundation/platform.hpp>
#include <Poseidon/Foundation/Framework/DebugLog.hpp> // LOG_WARN (body-pick revalidation)
#include <Poseidon/Foundation/Strings/RString.hpp>

#include <ctype.h> // tolower (body-roster dedupe + label-ambiguity keys)
#include <map>     // per-side displayName counts (GuerrillaBodyRowLabels)
#include <stdio.h>
#include <string> // body-roster dedupe keys

namespace Poseidon
{
namespace
{
// Injected occupier/resistance cycler buttons. Safely clear of the idcs the
// reused RscDisplaySelectIsland resource occupies (IDC_OK/IDC_CANCEL = 1/2,
// IDC_SELECT_ISLAND* = 101..103) and of any future RscDisplayGuerrillaNewGame
// that keeps the same layout.
constexpr int kIdcOccupier = 150;
constexpr int kIdcResistance = 151;
// 152 is the START TOWN cycler (issue #16 M4, gmSelStartTown): the CITY zone
// the headquarters is elected in on the first tick; "(camp)" publishes
// nothing. The character outfit cycler (issue #25) takes 153.
constexpr int kIdcStartTown = 152;
constexpr int kIdcOutfit = 153;
// 154 is the outfit-preview mannequin (issue #25 M4): a CT_OBJECT rendering
// the resolved playerClassWarrior/playerClassCiv body for the current
// island/resistance/outfit, the CHead rotating-preview technique from the
// player identity screen. Injected only once a model actually resolves;
// hidden (never a substitute body) whenever one does not. Since issue #43
// it is a GuerrillaBodyPreview (UI/Guerrilla/GuerrillaBodyPreview.hpp),
// shared with the character-select screen.
constexpr int kIdcOutfitPreview = 154;
// 155 is the CHARACTER entry button (player-body pick). It used to be the
// flat BODY cycler; issue #43 absorbed that into the
// GuerrillaCharacterSelect child display (uncapped scrollable roster, big
// mannequin, readable names), so the button now shows the current pick
// ("CHARACTER: (match outfit)" / "CHARACTER: <row label>") and clicking it
// opens the child. The idc keeps its slot and injection so any resource
// that authored 155 keeps working. Next free idc: 156.
constexpr int kIdcBody = 155;

// Normalized-screen slots for the injected 2D cyclers — bottom-left column,
// clear of the island-list notebook that fills the screen centre. The
// body browser and outfit cycler stack ABOVE the faction pair so the
// shipped 0.80/0.87 slots (and the e2e assertions on them) stay put.
constexpr float kCyclerX = 0.02f;
constexpr float kCyclerW = 0.32f;
constexpr float kCyclerH = 0.05f;
constexpr float kCyclerBodyY = 0.66f;
constexpr float kCyclerOutfitY = 0.73f;
constexpr float kCyclerOccupierY = 0.80f;
constexpr float kCyclerResistanceY = 0.87f;
// The START TOWN row sits BELOW the faction pair (the shipped 0.80/0.87
// slots stay put, the mannequin owns the column above CHARACTER). The
// reused RscDisplaySelectIsland's OK/Cancel row is y 0.90..0.95 at x >=
// 0.60, so the 0.02..0.34 column is free there; the hidden Wizard button
// (x 0.05..0.40, same row) is ShowCtrl(false) by the constructor.
constexpr float kCyclerStartTownY = 0.94f;

// The preview mannequin's slot: the free stretch of the bottom-left column
// above the CHARACTER button. Authored as a normalized-screen box like the
// 2D cyclers and passed to GuerrillaBodyPreview::PlaceInSlot (each display
// authors its own slot; the character-select screen uses a bigger one); the
// 3D placement is derived from it at runtime (Convert2DTo3D) so it tracks
// the engine's aspect settings.
constexpr float kPreviewCX = 0.18f;      // column centre (the cyclers' midline)
constexpr float kPreviewTopY = 0.26f;    // below the screen-top header strip
constexpr float kPreviewBottomY = 0.61f; // above the CHARACTER button at 0.66

// The outfit-cycler tokens. WARRIOR must be index 0 (the default): it is
// defined as the authored mission.sqm class, so an untouched cycler
// publishes a value the substitution seam ignores — indistinguishable from
// publishing nothing (the same invariant GuerrillaDefaultSelections keeps
// for the faction pair).
constexpr const char* kOutfitWarrior = "WARRIOR";
constexpr const char* kOutfitCivilian = "CIVILIAN";

// The one player-facing wording for an unlaunchable pair, shared by the guard
// helper and the display so the message box and the tests cannot disagree.
RString SameSideMessage(RString side)
{
    char buffer[256];
    snprintf(buffer, sizeof(buffer),
             "OCCUPIER and RESISTANCE both resolve to side %s.\nPick factions on two different sides.",
             (const char*)side);
    return RString(buffer);
}

// The turntable mannequin class itself lives in
// UI/Guerrilla/GuerrillaBodyPreview.{hpp,cpp} since issue #43 (shared with
// GuerrillaCharacterSelect); this display keeps only its slot constants
// above and the injection/refresh logic in UpdateOutfitPreview.
} // namespace

std::vector<RString> GuerrillaListIslands(const ParamEntry* worldList, const std::function<bool(RString)>& worldExists)
{
    std::vector<RString> islands;
    if (!worldList)
    {
        return islands;
    }
    for (int i = 0; i < worldList->GetEntryCount(); i++)
    {
        const ParamEntry& entry = worldList->GetEntry(i);
        if (!entry.IsClass())
        {
            continue;
        }
        RString name = entry.GetName();
        if (worldExists && !worldExists(name))
        {
            continue;
        }
        islands.push_back(name);
    }
    return islands;
}

std::vector<RString> GuerrillaListFactions(const ParamEntry* factionsCfg)
{
    std::vector<RString> out;
    if (!factionsCfg)
    {
        return out;
    }
    for (int i = 0; i < factionsCfg->GetEntryCount(); i++)
    {
        const ParamEntry& e = factionsCfg->GetEntry(i);
        if (!e.IsClass())
        {
            continue;
        }
        RString name = e.GetName();
        // the class name is the default side, so a `class CIV` needs no
        // `side` key to be excluded here
        RString side = e.ReadValue("side", name);
        if (stricmp(side, "CIV") == 0)
        {
            continue; // the population is ambience, never a combatant choice
        }
        out.push_back(name);
    }
    return out;
}

std::vector<RString> GuerrillaListStartTowns(const ParamEntry* zonesCfg, const ParamEntry* namesCfg)
{
    // ONE rule, owned by the registry: the cycler lists exactly the CITY
    // zones the launched campaign will carry (authored + seeded), so a pick
    // always resolves through ZoneRegistry::FindZoneIndex on the first tick.
    AutoArray<RString> names;
    Guerrilla::ZoneRegistry::CollectTownNames(zonesCfg, namesCfg, names);
    std::vector<RString> out;
    for (int i = 0; i < names.Size(); i++)
    {
        out.push_back(names[i]);
    }
    return out;
}

std::vector<GuerrillaBodyChoice> GuerrillaListPlayerBodies(const ParamEntry* vehiclesCfg,
                                                           const std::function<bool(RString)>& shapeFileExists)
{
    std::vector<GuerrillaBodyChoice> out;
    if (!vehiclesCfg)
    {
        return out;
    }
    const ParamEntry* manEntry = vehiclesCfg->FindEntry("Man");
    if (!manEntry || !manEntry->IsClass())
    {
        return out; // no Man base class: nothing is a body
    }
    const ParamClass* manCls = static_cast<const ParamClass*>(manEntry);
    // TargetSide values 0..3 (World/Scene/Object.hpp) to the script-side
    // strings the rest of the Guerrilla surface speaks
    static constexpr const char* kSideNames[] = {"EAST", "WEST", "GUER", "CIV"};
    constexpr int kNSides = 4;
    // per-side buckets, WEST/EAST/GUER/CIV output order (index into these)
    static constexpr int kSideOrder[] = {1, 0, 2, 3};
    std::vector<GuerrillaBodyChoice> buckets[kNSides];
    std::vector<std::string> seen[kNSides]; // lowercased displayName|model dedupe keys
    for (int i = 0; i < vehiclesCfg->GetEntryCount(); i++)
    {
        const ParamEntry& e = vehiclesCfg->GetEntry(i);
        if (!e.IsClass())
        {
            continue;
        }
        const ParamClass* cls = static_cast<const ParamClass*>(&e);
        if (cls == manCls || !cls->IsDerivedFrom(*manCls))
        {
            continue;
        }
        // scope > 0: scope=1 script-only classes (SoldierGFakeC...) are
        // deliberate picks, scope=0 abstract bases are not creatable
        if (e.ReadValue("scope", 0) <= 0)
        {
            continue;
        }
        int side = e.ReadValue("side", (int)kNSides);
        if (side < 0 || side >= kNSides)
        {
            continue; // TLogic and friends are not playable bodies
        }
        // plan-15 shaped: a class whose p3d the package does not ship is
        // skipped, never crashes and never reaches the publish channel
        RString model = e.ReadValue("model", RString());
        if (model.GetLength() == 0 || !shapeFileExists || !shapeFileExists(GetShapeName(model)))
        {
            continue;
        }
        // dedupe by what the player can SEE: one look (displayName+model)
        // per side, first class in config scan order wins
        RString displayName = e.ReadValue("displayName", RString());
        std::string key = std::string((const char*)displayName) + "|" + std::string((const char*)model);
        for (char& c : key)
        {
            c = (char)tolower((unsigned char)c);
        }
        bool dup = false;
        for (const std::string& k : seen[side])
        {
            if (k == key)
            {
                dup = true;
                break;
            }
        }
        if (dup)
        {
            continue; // UNCAPPED since issue #43: only the dedupe holds rows back
        }
        seen[side].push_back(key);
        // GetOwner: the CfgPatches class AddonSystem::ParseAddonConfig
        // stamped on the class (lowercased there), EMPTY for base game -
        // the ArcadeUnitInfo::RequiredAddons attribution.
        buckets[side].push_back(
            GuerrillaBodyChoice{RString(e.GetName()), RString(kSideNames[side]), displayName, RString(e.GetOwner())});
    }
    for (int side : kSideOrder)
    {
        for (const GuerrillaBodyChoice& b : buckets[side])
        {
            out.push_back(b);
        }
    }
    return out;
}

RString GuerrillaSanitizeLabel(RString s)
{
    if (s.GetLength() == 0)
    {
        return s;
    }
    std::string buffer((const char*)s);
    bool changed = false;
    for (char& c : buffer)
    {
        if (c == '_')
        {
            c = '-'; // the menu fonts drop the '_' glyph; '-' keeps the join readable
            changed = true;
        }
    }
    return changed ? RString(buffer.c_str()) : s;
}

RString GuerrillaBodyRowLabel(const GuerrillaBodyChoice& b, bool displayNameAmbiguous)
{
    if (b.displayName.GetLength() == 0)
    {
        return GuerrillaSanitizeLabel(b.className);
    }
    if (!displayNameAmbiguous)
    {
        return GuerrillaSanitizeLabel(b.displayName);
    }
    // the classname suffix is what tells two same-named looks apart; both
    // halves sanitized so the label never carries an '_' from either
    return GuerrillaSanitizeLabel(b.displayName) + RString(" (") + GuerrillaSanitizeLabel(b.className) + RString(")");
}

namespace
{
// per-side, case-insensitive ambiguity key for a roster row's displayName
std::string BodyLabelKey(const GuerrillaBodyChoice& b)
{
    std::string key = std::string((const char*)b.side) + "|" + std::string((const char*)b.displayName);
    for (char& c : key)
    {
        c = (char)tolower((unsigned char)c);
    }
    return key;
}
} // namespace

std::vector<RString> GuerrillaBodyRowLabels(const std::vector<GuerrillaBodyChoice>& roster)
{
    // Ambiguity = the same displayName on 2+ rows of ONE side, over the
    // final (deduped) roster. Case-insensitive like the dedupe key.
    std::map<std::string, int> counts;
    for (const GuerrillaBodyChoice& b : roster)
    {
        if (b.displayName.GetLength() > 0)
        {
            counts[BodyLabelKey(b)]++;
        }
    }
    std::vector<RString> out;
    out.reserve(roster.size());
    for (const GuerrillaBodyChoice& b : roster)
    {
        bool ambiguous = b.displayName.GetLength() > 0 && counts[BodyLabelKey(b)] >= 2;
        out.push_back(GuerrillaBodyRowLabel(b, ambiguous));
    }
    return out;
}

bool GuerrillaClassIsCivilian(const ParamEntry* vehiclesCfg, RString className)
{
    if (!vehiclesCfg || className.GetLength() == 0)
    {
        return false;
    }
    const ParamEntry* cls = vehiclesCfg->FindEntry(className);
    if (!cls || !cls->IsClass())
    {
        return false; // unknown class: nothing says civilian, the warrior rule applies
    }
    // the same inherited-side read the roster builder performs; 3 = CIV
    // (TargetSide, World/Scene/Object.hpp)
    return cls->ReadValue("side", -1) == 3;
}

RString GuerrillaTemplateMissionBase(RString island)
{
    return RString("missions\\Guerrilla.") + island;
}

bool GuerrillaTemplateExists(RString island, const std::function<bool(RString)>& pboExists,
                             const std::function<bool(RString)>& missionFileExists)
{
    RString base = GuerrillaTemplateMissionBase(island);
    if (pboExists && pboExists(base + RString(".pbo")))
    {
        return true;
    }
    return missionFileExists && missionFileExists(base + RString("\\mission.sqm"));
}

RString GuerrillaFactionSide(const ParamEntry* factionsCfg, RString faction)
{
    if (!factionsCfg || faction.GetLength() == 0)
    {
        return RString();
    }
    const ParamEntry* cls = factionsCfg->FindEntry(faction);
    if (!cls || !cls->IsClass())
    {
        return RString();
    }
    return cls->ReadValue("side", faction);
}

int GuerrillaIndexOfSelection(const ParamEntry* factionsCfg, const std::vector<RString>& list, RString selection)
{
    if (selection.GetLength() == 0)
    {
        return -1;
    }
    // class name first, then side — the exact order ZoneRegistry::FindFaction
    // scans in, so an index found here names the record the registry would
    // have matched for the same string. (Both flipped to name-first in issue
    // #54: with the faction library global, a template's defaultOccupier =
    // "EAST" was matching the first EAST-SIDE faction of the merged table,
    // EgyptFrontier with @LoBo mounted, instead of the class named EAST.)
    for (int i = 0; i < (int)list.size(); i++)
    {
        if (stricmp(list[i], selection) == 0)
        {
            return i;
        }
    }
    for (int i = 0; i < (int)list.size(); i++)
    {
        RString side = GuerrillaFactionSide(factionsCfg, list[i]);
        if (side.GetLength() > 0 && stricmp(side, selection) == 0)
        {
            return i;
        }
    }
    return -1;
}

int GuerrillaIndexOfName(const std::vector<RString>& list, RString name)
{
    // Deliberately NOT GuerrillaIndexOfSelection: no side rung. See the header
    // for why the two must not be interchanged.
    if (name.GetLength() == 0)
    {
        return -1;
    }
    for (int i = 0; i < (int)list.size(); i++)
    {
        if (stricmp(list[i], name) == 0)
        {
            return i;
        }
    }
    return -1;
}

RString GuerrillaFactionIssue(const ParamEntry* factionsCfg, RString faction, const Guerrilla::ClassProbe& probe,
                              const ParamEntry* vehiclesCfg, const std::function<bool(RString)>& shapeExists)
{
    const ParamEntry* cls = factionsCfg && faction.GetLength() > 0 ? factionsCfg->FindEntry(faction) : nullptr;
    if (!cls || !cls->IsClass())
    {
        return RString("no such faction");
    }
    char buffer[256];
    const ParamEntry* tiers = cls->FindEntry("tiers");
    if (!tiers || !tiers->IsArray() || tiers->GetSize() < 1)
    {
        return RString("authors no tiers[]");
    }
    RString tier0 = (RStringB)(*tiers)[0];
    if (tier0.GetLength() == 0 || !probe.Exists("CfgVehicles", tier0))
    {
        snprintf(buffer, sizeof(buffer), "tiers[0] '%s' is not in the loaded data", (const char*)tier0);
        return RString(buffer);
    }
    RString warrior = cls->ReadValue("playerClassWarrior", RString());
    if (warrior.GetLength() > 0 && !probe.Exists("CfgVehicles", warrior))
    {
        snprintf(buffer, sizeof(buffer), "playerClassWarrior '%s' is not in the loaded data", (const char*)warrior);
        return RString(buffer);
    }
    if (warrior.GetLength() > 0 && vehiclesCfg && shapeExists)
    {
        // the same gate the launch runs on the substituted body
        RString modelIssue = Guerrilla::PlayerBodyModelIssue(vehiclesCfg, warrior, shapeExists);
        if (modelIssue.GetLength() > 0)
        {
            snprintf(buffer, sizeof(buffer), "playerClassWarrior '%s': %s", (const char*)warrior,
                     (const char*)modelIssue);
            return RString(buffer);
        }
    }
    return RString();
}

std::vector<RString> GuerrillaFactionIssues(const ParamEntry* factionsCfg, const std::vector<RString>& factions,
                                            const Guerrilla::ClassProbe& probe, const ParamEntry* vehiclesCfg,
                                            const std::function<bool(RString)>& shapeExists)
{
    std::vector<RString> out;
    out.reserve(factions.size());
    for (const RString& f : factions)
    {
        out.push_back(GuerrillaFactionIssue(factionsCfg, f, probe, vehiclesCfg, shapeExists));
    }
    return out;
}

RString GuerrillaUnavailableMessage(const char* role, RString faction, RString issue)
{
    char buffer[512];
    snprintf(buffer, sizeof(buffer),
             "%s faction '%s' is not in the loaded data (%s).\nMount the mod that ships it, or pick another faction.",
             role ? role : "", (const char*)faction, (const char*)issue);
    return RString(buffer);
}

void GuerrillaDefaultSelections(const ParamEntry* factionsCfg, const ParamEntry* zonesCfg,
                                const std::vector<RString>& list, int& outOccupier, int& outResistance)
{
    outOccupier = -1;
    outResistance = -1;
    if (list.empty())
    {
        return; // no real choice: SelectedOccupier/Resistance stay EMPTY
    }
    // Whatever the cyclers start on IS published on OK, so it has to be the
    // same pair a no-UI direct launch of this template resolves. Walk the same
    // precedence chain ZoneRegistry::LoadFromParams does, minus the gmSel*
    // rung (that rung is what these selections BECOME).
    RString defOccupier = zonesCfg ? zonesCfg->ReadValue("defaultOccupier", RString()) : RString();
    RString defResistance = zonesCfg ? zonesCfg->ReadValue("defaultResistance", RString()) : RString();
    outOccupier = GuerrillaIndexOfSelection(factionsCfg, list, defOccupier);
    outResistance = GuerrillaIndexOfSelection(factionsCfg, list, defResistance);
    // Then the registry's built-in EAST occupier / GUER resistance, for a
    // template that authors neither default* key.
    if (outOccupier < 0)
    {
        outOccupier = GuerrillaIndexOfSelection(factionsCfg, list, RString("EAST"));
    }
    if (outResistance < 0)
    {
        outResistance = GuerrillaIndexOfSelection(factionsCfg, list, RString("GUER"));
    }
    // Floor: two DISTINCT indices. Index 0 for both would open the screen on a
    // campaign fighting itself. (A template that deliberately points both
    // default* keys at one descriptor is honoured above and lands here only if
    // neither key resolved at all.)
    if (outOccupier < 0)
    {
        outOccupier = (outResistance == 0 && list.size() > 1) ? 1 : 0;
    }
    if (outResistance < 0)
    {
        outResistance = (outOccupier == 0 && list.size() > 1) ? 1 : 0;
    }
}

bool GuerrillaSelectionIsResolvable(const ParamEntry* factionsCfg, const ParamEntry* zonesCfg, RString occupier,
                                    RString resistance, RString& outMessage)
{
    outMessage = RString();
    RString occSide = GuerrillaFactionSide(factionsCfg, occupier);
    RString resSide = GuerrillaFactionSide(factionsCfg, resistance);
    if (occSide.GetLength() == 0 || resSide.GetLength() == 0)
    {
        // Empty selections (no real faction config) cannot be validated here:
        // the mission's own default* keys decide what these become.
        return true;
    }

    RString playerSide = zonesCfg ? zonesCfg->ReadValue("playerSide", RString()) : RString();
    if (playerSide.GetLength() == 0)
    {
        // Legacy template: ZoneRegistry::ResolveSideCollisions returns early,
        // so two picks on one side would spawn the campaign fighting itself.
        if (stricmp(occSide, resSide) == 0)
        {
            outMessage = SameSideMessage(occSide);
            return false;
        }
        return true;
    }

    // Step 1, the resistance pin: twin substitution and rebase both land on
    // playerSide, so the registry's outcome is playerSide either way.
    resSide = playerSide;
    if (stricmp(occSide, resSide) != 0)
    {
        return true;
    }
    // Step 2, the occupier stepping off: its sideTwin, else the first free war
    // side. With three war sides and one pinned there is always one free, so
    // this block is unreachable by construction on any template that authors a
    // playerSide — it stays live because the floor has to be correct anyway.
    if (Guerrilla::TwinOffSide(factionsCfg, occupier, resSide).GetLength() > 0)
    {
        return true;
    }
    if (Guerrilla::FirstFreeWarSide(resSide).GetLength() > 0)
    {
        return true;
    }
    outMessage = SameSideMessage(resSide);
    return false;
}

std::vector<RString> GuerrillaOutfitChoices(const ParamEntry* factionsCfg, const ParamEntry* zonesCfg,
                                            RString resistance)
{
    std::vector<RString> out;
    if (!factionsCfg)
    {
        return out;
    }
    // the same resistance-block precedence the substitution seam walks
    // (OutfitSelect): selection > defaultResistance > the built-in GUER side
    const ParamEntry* faction = Guerrilla::FindGuerrillaFactionEntry(factionsCfg, resistance);
    if (!faction && zonesCfg)
    {
        RString defResistance = zonesCfg->ReadValue("defaultResistance", RString());
        faction = Guerrilla::FindGuerrillaFactionEntry(factionsCfg, defResistance);
    }
    if (!faction)
    {
        faction = Guerrilla::FindGuerrillaFactionEntry(factionsCfg, "GUER");
    }
    if (!faction)
    {
        return out;
    }
    // the pair is offered iff the descriptor authors a civilian player body;
    // playerClassWarrior needs no probe here — WARRIOR always means "the
    // authored mission.sqm class", key or no key
    RString civ = faction->ReadValue("playerClassCiv", RString());
    if (civ.GetLength() == 0)
    {
        return out;
    }
    out.push_back(RString(kOutfitWarrior));
    out.push_back(RString(kOutfitCivilian));
    return out;
}

RString GuerrillaOutfitPreviewClass(const ParamEntry* factionsCfg, const ParamEntry* zonesCfg, RString resistance,
                                    RString outfit)
{
    // the same resistance-block precedence GuerrillaOutfitChoices (and the
    // OutfitSelect substitution seam) walks: selection > defaultResistance >
    // the built-in GUER side
    const ParamEntry* faction = Guerrilla::FindGuerrillaFactionEntry(factionsCfg, resistance);
    if (!faction && zonesCfg)
    {
        RString defResistance = zonesCfg->ReadValue("defaultResistance", RString());
        faction = Guerrilla::FindGuerrillaFactionEntry(factionsCfg, defResistance);
    }
    if (!faction)
    {
        faction = Guerrilla::FindGuerrillaFactionEntry(factionsCfg, "GUER");
    }
    if (!faction)
    {
        return RString();
    }
    if (outfit.GetLength() > 0 && stricmp(outfit, kOutfitCivilian) == 0)
    {
        return faction->ReadValue("playerClassCiv", RString());
    }
    // WARRIOR and "no pair offered" both preview the warrior body: every
    // shipped template's playerClassWarrior documents the authored
    // mission.sqm class, and a descriptor without the key just hides the
    // preview (EMPTY here).
    if (outfit.GetLength() == 0 || stricmp(outfit, kOutfitWarrior) == 0)
    {
        return faction->ReadValue("playerClassWarrior", RString());
    }
    return RString(); // unknown token: show nothing rather than guess
}

RString GuerrillaOutfitPreviewModel(const ParamEntry* vehiclesCfg, RString className,
                                    const std::function<bool(RString)>& shapeFileExists)
{
    if (!vehiclesCfg || className.GetLength() == 0)
    {
        return RString();
    }
    const ParamEntry* cls = vehiclesCfg->FindEntry(className);
    if (!cls || !cls->IsClass())
    {
        return RString(); // class not in the loaded package: hide
    }
    // inherited `model` keys resolve too, ParamClass::FindEntry follows the
    // base chain
    RString model = cls->ReadValue("model", RString());
    if (model.GetLength() == 0)
    {
        return RString();
    }
    if (!shapeFileExists || !shapeFileExists(GetShapeName(model)))
    {
        return RString(); // shape file missing: hide, never substitute
    }
    return model;
}

namespace
{
// case-insensitive substring scan shared by the proxy predicates
bool ProxyNameHasStem(const char* proxyName, const char* stem)
{
    const int stemLen = (int)strlen(stem);
    for (const char* p = proxyName; *p; p++)
    {
        if (strnicmp(p, stem, stemLen) == 0)
        {
            return true;
        }
    }
    return false;
}
} // namespace

bool GuerrillaPreviewIsFlagProxy(const char* proxyName)
{
    if (!proxyName)
    {
        return false;
    }
    // Case-insensitive substring match on the two naming stems: the vanilla
    // bodies all reference "flag_vojak", community bodies use either the
    // English word or the Czech "vlajka" the rest of the flag machinery uses
    // (the flagpole cloth selection House.cpp animates is "vlajka").
    static constexpr const char* kStems[] = {"flag", "vlajka"};
    for (const char* stem : kStems)
    {
        if (ProxyNameHasStem(proxyName, stem))
        {
            return true;
        }
    }
    return false;
}

bool GuerrillaPreviewHideWeaponProxy(const ParamEntry* nonAIVehiclesCfg, const char* proxyName, bool civilian)
{
    if (!proxyName || !*proxyName)
    {
        return false;
    }
    // Belt-and-braces name stems for gear no vanilla config classes at all
    // (Classic+AddOns author no Proxynvg_proxy / Proxydalekohled_proxy, so
    // those proxies never even get created - see the header note) but a mod
    // package might: binoculars ("dalekohled" is the OFP-era model stem,
    // "binoc" the English one) and night-vision goggles.
    static constexpr const char* kHideStems[] = {"dalekohled", "binoc", "nvg"};
    for (const char* stem : kHideStems)
    {
        if (ProxyNameHasStem(proxyName, stem))
        {
            return true;
        }
    }
    if (!nonAIVehiclesCfg)
    {
        return false; // no config to classify against: draw (never hide the rifle)
    }
    // The engine's own proxy identity: CfgNonAIVehicles class "Proxy<name>",
    // spaces underscored - the exact lookup ShapeLOD.cpp builds the proxy
    // object from. The inherited simulation key types the weapon slot
    // (ProxyRPG7_Proxy : ProxySecWeapon inherits "ProxySecWeapon").
    char clsName[256];
    snprintf(clsName, sizeof(clsName), "Proxy%s", proxyName);
    for (char* p = clsName; *p; p++)
    {
        if (*p == ' ')
        {
            *p = '_';
        }
    }
    const ParamEntry* cls = nonAIVehiclesCfg->FindEntry(clsName);
    if (!cls || !cls->IsClass())
    {
        return false;
    }
    RString sim = cls->ReadValue("simulation", RString());
    if (sim.GetLength() == 0)
    {
        return false;
    }
    // launchers and pistols always hide; proxyweapon (the primary rifle)
    // hides only for a civilian body (issue #43: civilians preview
    // unarmed), and every non-weapon proxy always draws
    if (stricmp(sim, "proxysecweapon") == 0 || stricmp(sim, "proxyhandgun") == 0)
    {
        return true;
    }
    return civilian && stricmp(sim, "proxyweapon") == 0;
}

RString GuerrillaFactionsDescriptionPath(RString island, RString bankPrefix)
{
    if (bankPrefix.GetLength() > 0)
    {
        return bankPrefix + RString("description.ext");
    }
    return GuerrillaTemplateMissionBase(island) + RString("\\description.ext");
}

GuerrillaNewGame::GuerrillaNewGame(ControlsContainer* parent) : Display(parent)
{
    _exitWhenClose = -1;

    // Prefer a dedicated resource when the game data ships one; fall back to
    // the vanilla island-selection layout; load nothing when no menu
    // resources exist (headless runs) — see the class comment.
    const char* rsc = nullptr;
    if (Res.FindEntry("RscDisplayGuerrillaNewGame"))
    {
        rsc = "RscDisplayGuerrillaNewGame";
    }
    else if (Res.FindEntry("RscDisplaySelectIsland"))
    {
        rsc = "RscDisplaySelectIsland";
    }
    if (rsc)
    {
        Load(rsc);
    }
    // Never keep the resource's idd — the reused RscDisplaySelectIsland
    // carries the editor's IDD_SELECT_ISLAND, and DisplayMain keys the
    // Guerrilla launch on IDD_GUERRILLA_NEW_GAME.
    _idd = IDD_GUERRILLA_NEW_GAME;

    // The island-select wizard button belongs to the editor flow.
    if (IControl* wizard = GetCtrl(IDC_SELECT_ISLAND_WIZARD))
    {
        wizard->ShowCtrl(false);
    }

    // The island list (when present) is already populated with a current
    // selection by Load() above, so SelectedIsland() resolves to a real
    // island here — pull that island's own faction roster before wiring up
    // the cyclers, instead of the never-populated global Pars lookup.
    RefreshFactionsForIsland(SelectedIsland());

    // The BODY browser roster comes from the loaded package (Pars), not from
    // any island's template, so one build here covers the display's life.
    _bodies = GuerrillaListPlayerBodies(Pars.FindEntry("CfgVehicles"),
                                        [](RString path) { return QIFStreamB::FileExist(path); });

    InjectFactionCyclers();
}

Control* GuerrillaNewGame::OnCreateCtrl(int type, int idc, const ParamEntry& cls)
{
    switch (idc)
    {
        case IDC_SELECT_ISLAND:
        {
            // Same enumeration as the editor's DisplaySelectIsland: every
            // CfgWorldList class whose .wrp actually exists on disk.
            C3DListBox* lbox = new C3DListBox(this, idc, cls);
            std::vector<RString> islands = GuerrillaListIslands(Pars.FindEntry("CfgWorldList"), [](RString name)
                                                                { return QIFStreamB::FileExist(GetWorldName(name)); });
            const ParamEntry* worlds = Pars.FindEntry("CfgWorlds");
            int sel = 0;
            for (const RString& name : islands)
            {
                RString description = name;
                if (worlds)
                {
                    if (const ParamEntry* world = worlds->FindEntry(name))
                    {
                        description = world->ReadValue("description", name);
                    }
                }
                // An installed world is only launchable when its
                // "Guerrilla.<World>" template is installed too — say so in
                // the list instead of letting OK fail with a message box.
                if (!GuerrillaTemplateExists(
                        name, [](RString p) { return FilePathExists(p); },
                        [](RString p) { return QIFStreamB::FileExist(p); }))
                {
                    description = description + RString(" (no Guerrilla template)");
                }
                int index = lbox->AddString(description);
                lbox->SetData(index, name);
                if (stricmp(name, Glob.header.worldname) == 0)
                {
                    sel = index;
                }
            }
            if (lbox->GetSize() > 0)
            {
                lbox->SetCurSel(sel);
            }
            return lbox;
        }
    }
    return Display::OnCreateCtrl(type, idc, cls);
}

ControlObject* GuerrillaNewGame::OnCreateObject(int type, int idc, const ParamEntry& cls)
{
    if (idc == kIdcOutfitPreview)
    {
        // the DisplayNewUser::OnCreateObject -> CHead hook, for the mannequin
        return new GuerrillaBodyPreview(this, idc, cls);
    }
    return Display::OnCreateObject(type, idc, cls);
}

void GuerrillaNewGame::OnSimulate(EntityAI* vehicle)
{
    // Turntable for the mannequin: the DisplayNewUser::OnSimulate ->
    // CHead::Simulate pattern (UI displays get no other per-frame tick).
    if (auto* preview = dynamic_cast<GuerrillaBodyPreview*>(GetCtrl(kIdcOutfitPreview)))
    {
        if (preview->IsVisible())
        {
            preview->SimulateTurntable();
        }
    }
    // Wheel -> island list.  The reused RscDisplaySelectIsland hosts the
    // island C3DListBox inside a 3D notebook prop (ControlObjectContainer),
    // and Display::OnSimulate forwards the wheel to that container, which
    // only relays it to a child while its hover bookkeeping (_indexMove)
    // points at one — ControlObject's own OnMouseZChanged is a no-op, so
    // when the hover latch misses the 3D quad the wheel is silently
    // swallowed and islands below the fold are unreachable.  Drain the
    // wheel before Display::OnSimulate and drive the list directly — the
    // OptionsScrollList::PollWheelScroll precedent (see the comment there
    // about the notebook otherwise eating the wheel).  The list is the only
    // scrollable control on this display, so screen-wide wheel capture
    // can't shadow anything; child displays (character select) are safe
    // because Display::SimulateHUD only simulates the topmost child.
    if (auto* lbox = dynamic_cast<C3DListBox*>(GetCtrl(IDC_SELECT_ISLAND)))
    {
        float dz = InputSubsystem::Instance().ConsumeCursorScroll();
        if (dz != 0)
        {
            lbox->OnMouseZChanged(dz);
        }
    }
    Display::OnSimulate(vehicle);
}

void GuerrillaNewGame::InjectFactionCyclers()
{
    // Clone the main menu's Quit button class so the cyclers pick up the
    // menu's font/colors/sounds — the exact technique DisplayMain uses to
    // inject the MODS entry (see the long comment there for why the local
    // idc/text/style entries must be added BEFORE SetBase).
    const char* mainRsc = Res.FindEntry("RscDisplayMainRemaster") ? "RscDisplayMainRemaster" : "RscDisplayMain";
    ParamEntry* quitCls = Res.FindEntry(mainRsc) ? (Res >> mainRsc).FindEntry("Quit") : nullptr;
    if (quitCls == nullptr || !quitCls->IsClass())
    {
        return; // no styled menu resource — selections stay at their defaults
    }

    // Outlives the injected controls — like DisplayMain's s_modsInjectCfg,
    // each control back-references its ParamClass for the display's lifetime.
    static ParamFile s_guerrillaCyclerCfg;
    s_guerrillaCyclerCfg.Clear();
    const struct
    {
        const char* cls;
        int idc;
        float y;
    } slots[] = {
        {"GuerrillaBody", kIdcBody, kCyclerBodyY},
        {"GuerrillaOutfit", kIdcOutfit, kCyclerOutfitY},
        {"GuerrillaOccupier", kIdcOccupier, kCyclerOccupierY},
        {"GuerrillaResistance", kIdcResistance, kCyclerResistanceY},
        {"GuerrillaStartTown", kIdcStartTown, kCyclerStartTownY},
    };
    for (const auto& slot : slots)
    {
        ParamClass* cls = s_guerrillaCyclerCfg.AddClass(slot.cls);
        cls->Add("idc", slot.idc);
        cls->Add("text", RString(""));
        cls->Add("style", ST_LEFT);
        cls->SetBase(quitCls->GetClassInterface());
        LoadControl(*cls);
        if (Control* ctrl = dynamic_cast<Control*>(GetCtrl(slot.idc)))
        {
            ctrl->SetPos(kCyclerX, slot.y, kCyclerW, kCyclerH);
        }
        // every cycler is the same cloned class, so one capture serves the
        // dim/restore of the faction rows (UpdateFactionLabel)
        if (CActiveText* text = dynamic_cast<CActiveText*>(GetCtrl(slot.idc)); text && !_cyclerColorKnown)
        {
            _cyclerColor = text->GetColor();
            _cyclerColorKnown = true;
        }
        UpdateFactionLabel(slot.idc);
    }

    // The cyclers exist now, so the preview gate is open; show the body the
    // opening selections resolve to.
    UpdateOutfitPreview();
}

void GuerrillaNewGame::UpdateOutfitPreview()
{
    // Gate on the styled outfit cycler being present: with no menu resources
    // (headless runs) this display owns no controls and must keep owning
    // none, and a mannequin without its cycler label would be unreadable.
    if (!GetCtrl(kIdcOutfit))
    {
        return;
    }
    // The BODY browser's explicit pick previews directly; on its "(match
    // outfit)" default the mannequin falls back to the outfit-resolution
    // chain, exactly what the launch-time substitution seam will do.
    RString previewClass = SelectedPlayerClass();
    if (previewClass.GetLength() == 0)
    {
        previewClass =
            GuerrillaOutfitPreviewClass(_islandFactions, _islandZones, SelectedResistance(), SelectedOutfit());
    }
    RString model = GuerrillaOutfitPreviewModel(Pars.FindEntry("CfgVehicles"), previewClass,
                                                [](RString path) { return QIFStreamB::FileExist(path); });
    GuerrillaBodyPreview* preview = dynamic_cast<GuerrillaBodyPreview*>(GetCtrl(kIdcOutfitPreview));
    if (model.GetLength() == 0)
    {
        // plan-15 shaped degrade: no descriptor key, unknown class or a
        // missing shape file just hides the preview.
        if (preview)
        {
            preview->ShowCtrl(false);
        }
        return;
    }
    if (!preview)
    {
        // First model that resolved: inject the CT_OBJECT now. Function-
        // static lifetime for the same reason as s_guerrillaCyclerCfg: the
        // control back-references its ParamClass for the display's lifetime.
        static ParamFile s_guerrillaPreviewCfg;
        s_guerrillaPreviewCfg.Clear();
        ParamClass* cls = s_guerrillaPreviewCfg.AddClass("GuerrillaOutfitPreview");
        cls->Add("type", CT_OBJECT);
        cls->Add("idc", kIdcOutfitPreview);
        cls->Add("model", model);
        ParamEntry* position = cls->AddArray("position");
        position->AddValue(0.0f);
        position->AddValue(0.0f);
        position->AddValue(kGuerrillaPreviewDepthCfg); // PlaceInSlot refines below
        ParamEntry* direction = cls->AddArray("direction");
        direction->AddValue(0.0f);
        direction->AddValue(0.0f);
        direction->AddValue(1.0f);
        ParamEntry* up = cls->AddArray("up");
        up->AddValue(0.0f);
        up->AddValue(1.0f);
        up->AddValue(0.0f);
        cls->Add("scale", 1.0f);
        LoadObject(*cls);
        preview = dynamic_cast<GuerrillaBodyPreview*>(GetCtrl(kIdcOutfitPreview));
        if (!preview)
        {
            return;
        }
    }
    // On the injection pass the constructor already loaded this model and
    // SetModel just records the name (the shape bank caches the reload); on
    // later passes it swaps the shape when the body actually changed.
    // Civilian bodies preview unarmed (issue #43): the rule follows the
    // PREVIEWED class's config side, whichever selection resolved it.
    preview->SetModel(model);
    preview->SetCivilian(GuerrillaClassIsCivilian(Pars.FindEntry("CfgVehicles"), previewClass));
    preview->PlaceInSlot(kPreviewCX, kPreviewTopY, kPreviewBottomY);
    preview->ShowCtrl(true);
}

void GuerrillaNewGame::RefreshFactionsForIsland(RString island)
{
    // The faction table is the UNION of two sources (issue #54 A1, executing
    // #26): the island's own Guerrilla.<island> template description.ext block
    // (see guerrilla-mode/mission/*/description.ext) and the global config's
    // CfgGuerrillaFactions (Pars - addon faction packs, mod bin/config.cpp,
    // the UD bin/config-extra.cpp), the island winning on a class-name
    // collision. Peek the template's own description.ext the same way the
    // single-mission browser previews a banked mission's overview
    // (CreateSingleMissionBank), without running the full SetMission()/launch
    // path, then merge through FactionSources - the same helper the mission
    // side (ZoneRegistry::LoadFromConfig) and the player-body seam use, so
    // what the cyclers list is exactly what the campaign resolves.
    //
    // Lifetime: _islandCfg.Clear() invalidates every ParamEntry* previously
    // handed out of it, and OnLBSelChanged re-enters here on every island
    // change — so the two pointers must be nulled BEFORE the Clear and
    // re-derived after the Parse. _islandFactions points into
    // _factionSources' owned copy (rebuilt below), _islandZones into
    // _islandCfg or Pars. _occupiers/_resistances hold owning RString copies
    // and are unaffected.
    // Keep the player's picks across a refresh by NAME, not by index: the two
    // lists are rebuilt from scratch below and an index means nothing against
    // the new one.
    RString keepOccupier = SelectedOccupier();
    RString keepResistance = SelectedResistance();

    _islandFactions = nullptr;
    _islandZones = nullptr;
    _factionSources.Clear();
    _islandCfg.Clear();
    _islandForFactions = island;
    const ParamEntry* islandFactions = nullptr;

    RString missionBase = GuerrillaTemplateMissionBase(island);
    RString bankPrefix;
    if (FilePathExists(missionBase + RString(".pbo")))
    {
        bankPrefix = CreateSingleMissionBank(missionBase);
    }
    bool unbanked = bankPrefix.GetLength() == 0 && QIFStreamB::FileExist(missionBase + RString("\\mission.sqm"));
    if (bankPrefix.GetLength() > 0 || unbanked)
    {
        RString descPath = GuerrillaFactionsDescriptionPath(island, bankPrefix);
        if (QIFStreamB::FileExist(descPath))
        {
            _islandCfg.Parse(descPath);
            islandFactions = _islandCfg.FindEntry("CfgGuerrillaFactions");
            _islandZones = _islandCfg.FindEntry("CfgGuerrillaZones");
        }
    }
    // Merge: island block U Pars block. Either may be missing (a template
    // with no block of its own, a package with no faction pack); with both
    // missing the list is empty (cyclers show "(mission default)"; OK still
    // works via the mission's own default* keys). The zones block falls back
    // to Pars INDEPENDENTLY: sourcing the factions globally while leaving the
    // zones null left the OK guard reading playerSide off nothing, so it took
    // its legacy branch and blocked pairs the registry rebases happily — and
    // left the cyclers with no default* keys to seed from.
    _factionSources.Build(Pars.FindEntry("CfgGuerrillaFactions"), islandFactions);
    _islandFactions = _factionSources.Factions();
    if (!_islandZones)
    {
        _islandZones = Pars.FindEntry("CfgGuerrillaZones");
    }
    // One list, both cyclers: any faction may occupy or resist, and the
    // registry rebases whatever the pick collides with.
    _occupiers = GuerrillaListFactions(_islandFactions);
    _resistances = GuerrillaListFactions(_islandFactions);
    // Availability on THIS data package (issue #54 A2): a faction whose
    // tiers[0] / playerClassWarrior the loaded addons do not carry is listed
    // greyed and refused on OK, instead of launching into fallback bodies.
    // One line per greyed faction so a "why can't I pick X" has an answer in
    // the log; INFO, not WARN, because a faction pack from an unmounted mod
    // is the expected shape of a multi-mod install, not a defect.
    {
        Guerrilla::ParsClassProbe probe;
        _factionIssues = GuerrillaFactionIssues(_islandFactions, _occupiers, probe, Pars.FindEntry("CfgVehicles"),
                                                [](RString path) { return QIFStreamB::FileExist(path); });
        for (size_t i = 0; i < _factionIssues.size(); i++)
        {
            if (_factionIssues[i].GetLength() > 0)
            {
                LOG_INFO(Core, "Guerrilla menu: faction '{}' greyed out on island '{}': {}", (const char*)_occupiers[i],
                         (const char*)island, (const char*)_factionIssues[i]);
            }
        }
    }
    // Seed from the template's own defaultOccupier/defaultResistance so that
    // opening this screen and pressing OK launches exactly what a direct,
    // no-UI launch of the same template would. The lists used to be empty
    // (the descriptor was looked up in Pars, which never carries it), so the
    // selections resolved to EMPTY, nothing was published and the default*
    // keys always won — that is the contract this restores now that the lists
    // are real. Seeding 0/0 instead would both override those keys silently
    // and open on occupier == resistance.
    GuerrillaDefaultSelections(_islandFactions, _islandZones, _occupiers, _occupierSel, _resistanceSel);
    // A pick the new roster still carries BY NAME survives the refresh.
    //
    // GuerrillaIndexOfName, not GuerrillaIndexOfSelection: the keep is an
    // IDENTITY question ("is the faction the player picked also on this
    // island?"), and the side rung in the resolution helper turns it into a
    // role question, which aliases onto a different faction. Measured on the
    // real menu before this changed: Abel opens on the class literally named
    // EAST, and hopping to Lebanon80 matched the side rung against Hizballah
    // (side = "EAST"), which is that template's defaultResistance - so the
    // screen opened on OCCUPIER: Hizballah / RESISTANCE: Hizballah, silently
    // discarding the authored defaultOccupier = "IDF". Abel -> Sinai was the
    // same shape: EgyptFrontier (Sinai's intended resistance) as occupier and
    // Jordan (Abel's GUER keeping onto the only GUER-side faction) as
    // resistance, which then emptied the outfit pair as a knock-on because
    // Jordan authors no playerClassCiv. A name that does not appear on the new
    // island now falls through to the template's own default* keys, which is
    // the documented island-scoped re-seed contract.
    // Only a name the player actually CYCLED to is a pick worth keeping; an
    // untouched row was the previous island's seed and must re-seed here
    // (see _occupierPicked in the header for why the union made this
    // necessary). An unpicked name is neither kept nor reported as dropped.
    int keptOccupier = _occupierPicked ? GuerrillaIndexOfName(_occupiers, keepOccupier) : -1;
    int keptResistance = _resistancePicked ? GuerrillaIndexOfName(_resistances, keepResistance) : -1;
    if (!_occupierPicked)
    {
        keepOccupier = RString();
    }
    if (!_resistancePicked)
    {
        keepResistance = RString();
    }
    // A kept pick that lands on the SAME faction the other cycler was just
    // re-seeded to (Sinai RESISTANCE: IDF, then Lebanon80 where IDF is the
    // authored occupier) would open the screen on occupier == resistance,
    // which is the issue-#50 failure by another route. The keep that
    // collides yields to the template's own default* pair.
    if (keptOccupier >= 0 && keptOccupier == _resistanceSel && keptResistance < 0)
    {
        LOG_WARN(Core,
                 "Guerrilla menu: dropping the kept occupier '{}' on island '{}' - it is that template's default "
                 "resistance",
                 (const char*)keepOccupier, (const char*)island);
        keptOccupier = -1;
    }
    if (keptResistance >= 0 && keptResistance == (keptOccupier >= 0 ? keptOccupier : _occupierSel))
    {
        LOG_WARN(Core,
                 "Guerrilla menu: dropping the kept resistance '{}' on island '{}' - it collides with the occupier "
                 "pick",
                 (const char*)keepResistance, (const char*)island);
        keptResistance = -1;
    }
    if (keptOccupier >= 0)
    {
        _occupierSel = keptOccupier;
    }
    else if (keepOccupier.GetLength() > 0)
    {
        // the two siblings (RefreshOutfitChoices, RevalidateBodySelection)
        // already say when they drop a pick; this one was the silent exception
        LOG_WARN(Core, "Guerrilla menu: dropping the occupier pick '{}' - island '{}' does not offer it",
                 (const char*)keepOccupier, (const char*)island);
    }
    if (keptResistance >= 0)
    {
        _resistanceSel = keptResistance;
    }
    else if (keepResistance.GetLength() > 0)
    {
        LOG_WARN(Core, "Guerrilla menu: dropping the resistance pick '{}' - island '{}' does not offer it",
                 (const char*)keepResistance, (const char*)island);
    }
    // Cyclers don't exist yet on the first call (constructor, before
    // InjectFactionCyclers) — UpdateFactionLabel no-ops safely via its own
    // GetCtrl null check.
    UpdateFactionLabel(kIdcOccupier);
    UpdateFactionLabel(kIdcResistance);
    // The outfit pair is read off the (possibly re-seeded) resistance block.
    RefreshOutfitChoices();
    // The START TOWN list is island-scoped too (the template's CITY zones +
    // the world's Names towns).
    RefreshStartTowns();
    // ...and the CHARACTER pick gets an explicit revalidation pass, so nothing
    // island-shaped can leave the button or the mannequin stale.
    RevalidateBodySelection();
}

void GuerrillaNewGame::RefreshStartTowns()
{
    // Keep the pick by NAME across the rebuild (the faction-cycler contract);
    // a town the new island does not carry drops back to "(camp)".
    RString keep = SelectedStartTown();
    const ParamEntry* names = nullptr;
    if (const ParamEntry* worlds = Pars.FindEntry("CfgWorlds"))
    {
        if (const ParamEntry* world = worlds->FindEntry(_islandForFactions))
        {
            names = world->FindEntry("Names");
        }
    }
    _startTowns = GuerrillaListStartTowns(_islandZones, names);
    _startTownSel = -1;
    for (int i = 0; i < (int)_startTowns.size(); i++)
    {
        if (keep.GetLength() > 0 && stricmp(_startTowns[i], keep) == 0)
        {
            _startTownSel = i;
            break;
        }
    }
    UpdateFactionLabel(kIdcStartTown);
}

void GuerrillaNewGame::RevalidateBodySelection()
{
    // Why the pick is KEPT here while occupier/resistance/outfit are re-seeded
    // just above: those three come from the island's OWN CfgGuerrillaFactions
    // (its template description.ext), so switching islands genuinely invalidates
    // them - the new island may not even offer the same factions. _bodies is
    // built from Pars, which is process-global and identical on every island, so
    // silently discarding a body the player deliberately picked just because
    // they browsed the island list would be the surprising behaviour.
    //
    // What this therefore guarantees: idc 155 and the preview mannequin are
    // re-rendered on every island change, and the button can never promise a
    // body the launch will not deliver - the launch-side existence test is
    // ParsClassProbe in Game/Guerrilla/OutfitSelect.cpp, the same
    // Pars >> CfgVehicles lookup performed here.
    //
    // The ADDON half of the picked-body problem (a mod body launched on an
    // island whose mission.sqm addOns[] never mentions the mod) is handled at
    // the substitution seam in OutfitSelect.cpp, not here, precisely because the
    // roster is package-wide by design and must stay that way.
    //
    // Ordering note: the constructor calls RefreshFactionsForIsland BEFORE it
    // builds _bodies, so this runs once against an empty roster with
    // _bodySel == -1. Harmless (it normalizes to -1, and the label update no-ops
    // because the cyclers do not exist yet) - not a bug to "fix".
    if (_bodySel >= 0 && _bodySel < (int)_bodies.size())
    {
        const ParamEntry* vehicles = Pars.FindEntry("CfgVehicles");
        if (!vehicles || !vehicles->FindEntry(_bodies[_bodySel].className))
        {
            LOG_WARN(Core,
                     "Guerrilla body: picked class '{}' no longer resolves in the loaded package - falling back to "
                     "(match outfit)",
                     (const char*)_bodies[_bodySel].className);
            _bodySel = -1;
        }
    }
    else
    {
        _bodySel = -1; // normalize any out-of-range index to the default
    }
    UpdateFactionLabel(kIdcBody);
    // The OUTFIT row's scope qualifier is a function of _bodySel (issue #47),
    // so it must re-render wherever _bodySel can change — here (a pick this
    // package no longer resolves drops back to "(match outfit)", which HANDS
    // the player body back to the token) and in OnChildDestroyed. Those are
    // the only two sites that write _bodySel; a third must land this call too.
    UpdateFactionLabel(kIdcOutfit);
    UpdateOutfitPreview();
}

void GuerrillaNewGame::RefreshOutfitChoices()
{
    // Keep the player's pick by NAME across the rebuild — same contract as
    // the faction cyclers (an index means nothing against a new list).
    RString keep = SelectedOutfit();
    _outfits = GuerrillaOutfitChoices(_islandFactions, _islandZones, SelectedResistance());
    _outfitSel = _outfits.empty() ? -1 : 0; // WARRIOR default (index 0)
    for (int i = 0; i < (int)_outfits.size(); i++)
    {
        if (keep.GetLength() > 0 && stricmp(_outfits[i], keep) == 0)
        {
            _outfitSel = i;
            break;
        }
    }
    // issue #46 seam 6: unlike the CHARACTER pick (package-wide, so
    // RevalidateBodySelection keeps it), the outfit token is scoped to the
    // resistance descriptor, and an island or resistance change may land on a
    // block that authors no playerClassCiv. Re-seeding is the same contract
    // the occupier/resistance cyclers follow and is deliberate - but it drops
    // a choice the player made, and the loss is one-way: `keep` is re-derived
    // from the list above, so once it empties the token is gone from this
    // object and a trip back to an island that DOES offer CIVILIAN reopens on
    // WARRIOR. The row does re-render ("(mission default)", and the mannequin
    // hides), which nobody is obliged to look at, so say it in the log too.
    if (keep.GetLength() > 0 && (_outfitSel < 0 || stricmp(_outfits[_outfitSel], keep) != 0))
    {
        LOG_WARN(Core,
                 "Guerrilla outfit: dropping the selected outfit '{}' - the resistance descriptor now in effect "
                 "offers {}",
                 (const char*)keep,
                 _outfits.empty() ? "no outfit pair at all (no playerClassCiv key)" : "a different pair");
    }
    UpdateFactionLabel(kIdcOutfit);
    // Island changes and resistance cycling both land here, so this one call
    // keeps the mannequin tracking every path that can change the body.
    UpdateOutfitPreview();
}

void GuerrillaNewGame::UpdateFactionLabel(int idc)
{
    IControl* ctrl = GetCtrl(idc);
    if (!ctrl)
    {
        return;
    }
    char buffer[256];
    if (idc == kIdcBody)
    {
        // "CHARACTER: <row label>" via the SAME GuerrillaBodyRowLabels
        // builder the child display's list uses, so button and list can
        // never disagree on a body's name; the default reads
        // "(match outfit)" because that is literally what it does: publish
        // nothing and let the OUTFIT token resolve the body.
        if (_bodySel >= 0 && _bodySel < (int)_bodies.size())
        {
            std::vector<RString> labels = GuerrillaBodyRowLabels(_bodies);
            snprintf(buffer, sizeof(buffer), "CHARACTER: %s", (const char*)labels[_bodySel]);
        }
        else
        {
            snprintf(buffer, sizeof(buffer), "CHARACTER: (match outfit)");
        }
        if (CActiveText* text = dynamic_cast<CActiveText*>(ctrl))
        {
            text->SetText(buffer);
        }
        else if (CStatic* text = dynamic_cast<CStatic*>(ctrl))
        {
            text->SetText(buffer);
        }
        return;
    }
    if (idc == kIdcStartTown)
    {
        // "START TOWN: <zone>" or the "(camp)" default, which publishes
        // nothing: the campaign opens at the authored mission.sqm start and
        // the HQ is established later through the action menu.
        if (_startTownSel >= 0 && _startTownSel < (int)_startTowns.size())
        {
            snprintf(buffer, sizeof(buffer), "START TOWN: %s", (const char*)_startTowns[_startTownSel]);
        }
        else
        {
            snprintf(buffer, sizeof(buffer), "START TOWN: (camp)");
        }
        if (CActiveText* text = dynamic_cast<CActiveText*>(ctrl))
        {
            text->SetText(buffer);
        }
        else if (CStatic* text = dynamic_cast<CStatic*>(ctrl))
        {
            text->SetText(buffer);
        }
        return;
    }
    // Scope qualifier for the OUTFIT row (issue #47). A CHARACTER pick beats
    // the outfit token for the PLAYER's body unconditionally —
    // ResolvePlayerBodyClass (Game/Guerrilla/OutfitSelect.cpp) returns on the
    // pick and never falls through to the token path, not even when the picked
    // class is missing from the package. That precedence is deliberate; what
    // was wrong is this label, which kept reading "OUTFIT: CIVILIAN" while the
    // player spawned as the picked body. The token still governs everyone
    // ELSE — recruits, companions and the captured-town hold garrison all
    // branch on GM_OUTFIT_CIV (scripts/recruit.sqs, companions.sqs,
    // capture.sqs) — so the cycler stays live and readable and only its
    // advertised scope narrows. Greying it would be the opposite lie: it would
    // claim the token does nothing while the squad still obeys it, and would
    // strand the player with no way to dress their fighters.
    // Keyed on SelectedPlayerClass(), the exact value the launch publishes as
    // gmSelPlayerClass, so the label and the precedence cannot drift apart.
    const bool outfitSquadOnly = idc == kIdcOutfit && SelectedPlayerClass().GetLength() > 0;
    const char* prefix = idc == kIdcOccupier     ? "OCCUPIER"
                         : idc == kIdcResistance ? "RESISTANCE"
                         : outfitSquadOnly       ? "OUTFIT (squad only)"
                                                 : "OUTFIT";
    const std::vector<RString>& list = idc == kIdcOccupier     ? _occupiers
                                       : idc == kIdcResistance ? _resistances
                                                               : _outfits;
    const int sel = idc == kIdcOccupier ? _occupierSel : (idc == kIdcResistance ? _resistanceSel : _outfitSel);
    // An empty list means no config offered a real choice — nothing will be
    // published and the mission's own defaults decide (defaultOccupier /
    // defaultResistance keys, or the authored mission.sqm class for the
    // outfit), so say so instead of showing a value the mission may override.
    const char* selected = (sel >= 0 && sel < (int)list.size()) ? (const char*)list[sel] : "(mission default)";
    // A faction the loaded package cannot field stays in the cycle (the
    // player must be able to SEE what a missing mod would offer) but reads
    // "(not in loaded data)" and dims; OK refuses it (issue #54 A2).
    const bool factionRow = idc == kIdcOccupier || idc == kIdcResistance;
    const bool unavailable =
        factionRow && sel >= 0 && sel < (int)_factionIssues.size() && _factionIssues[sel].GetLength() > 0;
    snprintf(buffer, sizeof(buffer), "%s: %s%s", prefix, selected,
             unavailable ? kGuerrillaFactionUnavailableSuffix : "");
    if (CActiveText* text = dynamic_cast<CActiveText*>(ctrl))
    {
        text->SetText(buffer);
        if (factionRow && _cyclerColorKnown)
        {
            text->SetColor(unavailable ? PackedColor(_cyclerColor.R8() / 2, _cyclerColor.G8() / 2,
                                                     _cyclerColor.B8() / 2, _cyclerColor.A8())
                                       : _cyclerColor);
        }
    }
    else if (CStatic* text = dynamic_cast<CStatic*>(ctrl))
    {
        text->SetText(buffer);
    }
}

void GuerrillaNewGame::OnButtonClicked(int idc)
{
    switch (idc)
    {
        case kIdcOccupier:
            if (!_occupiers.empty())
            {
                _occupierSel = (_occupierSel + 1) % (int)_occupiers.size();
                _occupierPicked = true;
            }
            UpdateFactionLabel(idc);
            break;
        case kIdcResistance:
            if (!_resistances.empty())
            {
                _resistanceSel = (_resistanceSel + 1) % (int)_resistances.size();
                _resistancePicked = true;
            }
            UpdateFactionLabel(idc);
            // The outfit pair is authored per resistance block — the new
            // roster may or may not offer a civilian body.
            RefreshOutfitChoices();
            break;
        case kIdcOutfit:
            if (!_outfits.empty())
            {
                _outfitSel = (_outfitSel + 1) % (int)_outfits.size();
            }
            UpdateFactionLabel(idc);
            UpdateOutfitPreview();
            break;
        case kIdcStartTown:
            // cycles "(camp)" -> town 1 -> ... -> town N -> "(camp)"; an
            // empty list (no CITY zones on this island) keeps the default
            if (!_startTowns.empty())
            {
                _startTownSel = _startTownSel + 1 >= (int)_startTowns.size() ? -1 : _startTownSel + 1;
            }
            UpdateFactionLabel(idc);
            break;
        case kIdcBody:
            // The old flat cycler is retired (issue #43): the button opens
            // the character-select child display; the pick comes back
            // through OnChildDestroyed. Gated on the child being able to
            // own controls (CanBuild) - it could only be reached headless
            // through synthetic events, and opening an empty display there
            // would just trap the flow behind an invisible Esc. An empty
            // roster keeps the click a no-op too: the only offer would be
            // the "(match outfit)" row the button already shows.
            if (!_bodies.empty() && GuerrillaCharacterSelect::CanBuild())
            {
                CreateChild(new GuerrillaCharacterSelect(this, _bodies, _bodySel));
            }
            break;
        case IDC_CANCEL:
        {
            // Close the notebook with its animation first (matching
            // DisplaySelectIsland); exit immediately when the resource has no
            // animated notebook (or no controls loaded at all).
            ControlObjectContainerAnim* ctrl =
                dynamic_cast<ControlObjectContainerAnim*>(GetCtrl(IDC_SELECT_ISLAND_NOTEBOOK));
            if (ctrl)
            {
                _exitWhenClose = idc;
                ctrl->Close();
            }
            else
            {
                Exit(idc);
            }
        }
        break;
        case IDC_OK:
        {
            // A pair the registry cannot resolve to two distinct sides would
            // spawn the campaign fighting itself — keep the player in the
            // dialog. The descriptor comes from the SELECTED ISLAND's own
            // template (Pars never carries CfgGuerrillaFactions, which is why
            // this guard used to be unreachable).
            RString message;
            // A greyed pick (issue #54 A2) is refused BEFORE the side check:
            // launching it would put fallback bodies on every spawn of that
            // side, which is exactly the silent-drop shape issue #46 forbids.
            // Refuse, never auto-skip - the player chose it on purpose.
            if (_occupierSel >= 0 && _occupierSel < (int)_factionIssues.size() &&
                _factionIssues[_occupierSel].GetLength() > 0)
            {
                CreateMsgBox(MB_BUTTON_OK,
                             GuerrillaUnavailableMessage("OCCUPIER", SelectedOccupier(), _factionIssues[_occupierSel]));
                break;
            }
            if (_resistanceSel >= 0 && _resistanceSel < (int)_factionIssues.size() &&
                _factionIssues[_resistanceSel].GetLength() > 0)
            {
                CreateMsgBox(MB_BUTTON_OK, GuerrillaUnavailableMessage("RESISTANCE", SelectedResistance(),
                                                                       _factionIssues[_resistanceSel]));
                break;
            }
            if (!GuerrillaSelectionIsResolvable(_islandFactions, _islandZones, SelectedOccupier(), SelectedResistance(),
                                                message))
            {
                CreateMsgBox(MB_BUTTON_OK, message);
                break;
            }
            // The launch itself runs in DisplayMain::OnChildDestroyed
            // (IDD_GUERRILLA_NEW_GAME) after the base Exit(IDC_OK).
            Display::OnButtonClicked(idc);
        }
        break;
        default:
            Display::OnButtonClicked(idc);
            break;
    }
}

void GuerrillaNewGame::OnLBDblClick(int idc, int curSel)
{
    if (idc == IDC_SELECT_ISLAND)
    {
        OnButtonClicked(IDC_OK);
    }
    else
    {
        Display::OnLBDblClick(idc, curSel);
    }
}

void GuerrillaNewGame::OnLBSelChanged(int idc, int curSel)
{
    if (idc == IDC_SELECT_ISLAND)
    {
        // Different islands ship different rosters (e.g. Guerrilla.Demo's
        // vanilla EAST/GUER vs Guerrilla.Sinai's @LoBo IDF/EgyptFrontier) —
        // re-peek the newly selected island's own template instead of
        // leaving the previous island's faction list on screen.
        //
        // Only when the island ACTUALLY changed. C3DListBox::OnLButtonUp calls
        // SetCurSel with sendUpdate=true, which skips SetCurSel's
        // same-index early-out — so this fires on every click, including a
        // click on the already-selected row. Refreshing there would re-seed
        // the cyclers from the template defaults and throw the player's picks
        // away, which double-click-to-launch (OnLBDblClick -> IDC_OK) does in
        // one gesture: LButtonUp lands first, then OK publishes the reset
        // pair. It would also unmount and remount the template PBO per
        // mouse-up.
        //
        // Audited for issue #46 seam 7 and kept as-is. Two corrections to the
        // paragraph above, now that the name-keep below exists: an unguarded
        // SAME-island refresh would no longer lose the occupier/resistance
        // pair (RefreshFactionsForIsland restores both by name), so what the
        // guard still genuinely buys is the PBO churn, the dangling
        // _islandFactions window across _islandCfg.Clear(), and the one
        // selection the name-keep cannot recover - the outfit token, whose
        // list can come back empty (seam 6, RefreshOutfitChoices). And the
        // case it protects most often is not the double click at all but the
        // ordinary repeat single click, which fires this same handler.
        RString island = SelectedIsland();
        if (island.GetLength() > 0 && stricmp(island, _islandForFactions) != 0)
        {
            RefreshFactionsForIsland(island);
        }
    }
    Display::OnLBSelChanged(idc, curSel);
}

void GuerrillaNewGame::OnCtrlClosed(int idc)
{
    if (idc == IDC_SELECT_ISLAND_NOTEBOOK)
    {
        Exit(_exitWhenClose);
    }
    else
    {
        Display::OnCtrlClosed(idc);
    }
}

void GuerrillaNewGame::OnChildDestroyed(int idd, int exit)
{
    if (idd == IDD_GUERRILLA_CHARACTER_SELECT)
    {
        if (exit == IDC_OK)
        {
            // Read the pick BEFORE the base call releases _child (the
            // DisplayMain::OnChildDestroyed IDD_GUERRILLA_NEW_GAME
            // precedent, OptionsUIApp.cpp).
            if (auto* select = dynamic_cast<GuerrillaCharacterSelect*>((ControlsContainer*)_child))
            {
                int pick = select->SelectedRosterIndex();
                _bodySel = (pick >= 0 && pick < (int)_bodies.size()) ? pick : -1;
            }
        }
        // Any other exit (BACK/Esc): the pick stays what it was.
        Display::OnChildDestroyed(idd, exit);
        UpdateFactionLabel(kIdcBody);
        // Both directions of the pick change the OUTFIT row's scope qualifier
        // (issue #47): picking a body narrows it to "(squad only)", and
        // choosing the "(match outfit)" row back out of the child restores the
        // plain "OUTFIT" — the token governs the player body again.
        UpdateFactionLabel(kIdcOutfit);
        UpdateOutfitPreview();
        return;
    }
    Display::OnChildDestroyed(idd, exit);
}

RString GuerrillaNewGame::SelectedIsland()
{
    if (C3DListBox* lbox = dynamic_cast<C3DListBox*>(GetCtrl(IDC_SELECT_ISLAND)))
    {
        int sel = lbox->GetCurSel();
        if (sel >= 0)
        {
            return lbox->GetData(sel);
        }
    }
    // No island list (no menu resources) — the current menu cutscene world is
    // always a valid installed world.
    return Glob.header.worldname;
}

RString GuerrillaNewGame::SelectedOccupier() const
{
    if (_occupierSel >= 0 && _occupierSel < (int)_occupiers.size())
    {
        return _occupiers[_occupierSel];
    }
    // No real faction config was offered — return an EMPTY selection so the
    // launch path publishes nothing and the mission's own
    // defaultOccupier/defaultResistance config keys keep precedence
    // (ZoneRegistry::LoadFromParams). Publishing the built-in "EAST" here
    // used to match a mission faction by side and silently override them.
    return RString();
}

RString GuerrillaNewGame::SelectedResistance() const
{
    if (_resistanceSel >= 0 && _resistanceSel < (int)_resistances.size())
    {
        return _resistances[_resistanceSel];
    }
    return RString(); // see SelectedOccupier
}

RString GuerrillaNewGame::SelectedPlayerClass() const
{
    if (_bodySel >= 0 && _bodySel < (int)_bodies.size())
    {
        return _bodies[_bodySel].className;
    }
    // "(match outfit)" default — publish nothing so the outfit token (or the
    // authored mission.sqm class) keeps deciding (see SelectedOccupier).
    return RString();
}

RString GuerrillaNewGame::SelectedOutfit() const
{
    if (_outfitSel >= 0 && _outfitSel < (int)_outfits.size())
    {
        return _outfits[_outfitSel];
    }
    // No descriptor offered an outfit pair — publish nothing so the authored
    // mission.sqm class keeps deciding (see SelectedOccupier).
    return RString();
}

RString GuerrillaNewGame::SelectedStartTown() const
{
    if (_startTownSel >= 0 && _startTownSel < (int)_startTowns.size())
    {
        return _startTowns[_startTownSel];
    }
    // "(camp)" default: publish nothing, the authored start stands (see
    // SelectedOccupier)
    return RString();
}

void __cdecl CreateDisplayGuerrilla(ControlsContainer* parent)
{
    parent->CreateChild(new GuerrillaNewGame(parent));
}

} // namespace Poseidon
