#include <Poseidon/UI/Guerrilla/GuerrillaNewGame.hpp>
#include <Poseidon/UI/Guerrilla/GuerrillaModule.hpp>
#include <Poseidon/UI/GameModule.hpp>
#include <Poseidon/UI/OptionsUICommon.hpp>          // CreateSingleMissionBank (per-island description.ext peek)
#include <Poseidon/Game/Guerrilla/FactionTwins.hpp> // shared with ZoneRegistry::ResolveSideCollisions
#include <Poseidon/Game/Guerrilla/OutfitSelect.hpp> // FindGuerrillaFactionEntry (outfit cycler, issue #25)
#include <Poseidon/Core/resincl.hpp>
#include <Poseidon/Core/Global.hpp>
#include <Poseidon/IO/Filesystem/FileOps.hpp> // FilePathExists (template .pbo check)
#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/IO/ParamFileExt.hpp>
#include <Poseidon/IO/Streams/QBStream.hpp>
#include <Poseidon/Foundation/platform.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>

#include <stdio.h>

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
// 152 is RESERVED for issue #16's gmSelStartTown cycler; the character
// outfit cycler (issue #25) takes 153.
constexpr int kIdcOutfit = 153;

// Normalized-screen slots for the injected 2D cyclers — bottom-left column,
// clear of the island-list notebook that fills the screen centre. The
// outfit cycler stacks ABOVE the faction pair so the shipped 0.80/0.87
// slots (and the e2e assertions on them) stay put.
constexpr float kCyclerX = 0.02f;
constexpr float kCyclerW = 0.32f;
constexpr float kCyclerH = 0.05f;
constexpr float kCyclerOutfitY = 0.73f;
constexpr float kCyclerOccupierY = 0.80f;
constexpr float kCyclerResistanceY = 0.87f;

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
    // side first, then class name — the exact order ZoneRegistry::FindFaction
    // scans in, so an index found here names the record the registry would
    // have matched for the same string.
    for (int i = 0; i < (int)list.size(); i++)
    {
        RString side = GuerrillaFactionSide(factionsCfg, list[i]);
        if (side.GetLength() > 0 && stricmp(side, selection) == 0)
        {
            return i;
        }
    }
    for (int i = 0; i < (int)list.size(); i++)
    {
        if (stricmp(list[i], selection) == 0)
        {
            return i;
        }
    }
    return -1;
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
        {"GuerrillaOutfit", kIdcOutfit, kCyclerOutfitY},
        {"GuerrillaOccupier", kIdcOccupier, kCyclerOccupierY},
        {"GuerrillaResistance", kIdcResistance, kCyclerResistanceY},
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
        UpdateFactionLabel(slot.idc);
    }
}

void GuerrillaNewGame::RefreshFactionsForIsland(RString island)
{
    // CfgGuerrillaFactions is authored per-island, inside that island's own
    // Guerrilla.<island> template mission's description.ext (see
    // guerrilla-mode/mission/*/description.ext) — it is never an addon's
    // global config, so Pars does not carry it. Peek the template's own
    // description.ext the same way the single-mission browser previews a
    // banked mission's overview (CreateSingleMissionBank), without running
    // the full SetMission()/launch path.
    //
    // Lifetime: _islandCfg.Clear() invalidates every ParamEntry* previously
    // handed out of it, and OnLBSelChanged re-enters here on every island
    // change — so the two pointers must be nulled BEFORE the Clear and
    // re-derived after the Parse. _occupiers/_resistances hold owning RString
    // copies and are unaffected.
    // Keep the player's picks across a refresh by NAME, not by index: the two
    // lists are rebuilt from scratch below and an index means nothing against
    // the new one.
    RString keepOccupier = SelectedOccupier();
    RString keepResistance = SelectedResistance();

    _islandFactions = nullptr;
    _islandZones = nullptr;
    _islandCfg.Clear();
    _islandForFactions = island;

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
            _islandFactions = _islandCfg.FindEntry("CfgGuerrillaFactions");
            _islandZones = _islandCfg.FindEntry("CfgGuerrillaZones");
        }
    }
    // No per-island descriptor (missing template, or a template with no
    // CfgGuerrilla* of its own) — fall back to a global config an addon might
    // publish, then to an empty list (cyclers show "(mission default)"; OK
    // still works via the mission's own default* keys). Pars lives for the
    // process, so storing a pointer into it is safe. Both entries fall back
    // INDEPENDENTLY: sourcing the factions from Pars while leaving the zones
    // null left the OK guard reading playerSide off nothing, so it took its
    // legacy branch and blocked pairs the registry rebases happily — and left
    // the cyclers with no default* keys to seed from.
    if (!_islandFactions)
    {
        _islandFactions = Pars.FindEntry("CfgGuerrillaFactions");
    }
    if (!_islandZones)
    {
        _islandZones = Pars.FindEntry("CfgGuerrillaZones");
    }
    // One list, both cyclers: any faction may occupy or resist, and the
    // registry rebases whatever the pick collides with.
    _occupiers = GuerrillaListFactions(_islandFactions);
    _resistances = GuerrillaListFactions(_islandFactions);
    // Seed from the template's own defaultOccupier/defaultResistance so that
    // opening this screen and pressing OK launches exactly what a direct,
    // no-UI launch of the same template would. The lists used to be empty
    // (the descriptor was looked up in Pars, which never carries it), so the
    // selections resolved to EMPTY, nothing was published and the default*
    // keys always won — that is the contract this restores now that the lists
    // are real. Seeding 0/0 instead would both override those keys silently
    // and open on occupier == resistance.
    GuerrillaDefaultSelections(_islandFactions, _islandZones, _occupiers, _occupierSel, _resistanceSel);
    // A pick the new roster still carries survives the refresh.
    int keptOccupier = GuerrillaIndexOfSelection(_islandFactions, _occupiers, keepOccupier);
    if (keptOccupier >= 0)
    {
        _occupierSel = keptOccupier;
    }
    int keptResistance = GuerrillaIndexOfSelection(_islandFactions, _resistances, keepResistance);
    if (keptResistance >= 0)
    {
        _resistanceSel = keptResistance;
    }
    // Cyclers don't exist yet on the first call (constructor, before
    // InjectFactionCyclers) — UpdateFactionLabel no-ops safely via its own
    // GetCtrl null check.
    UpdateFactionLabel(kIdcOccupier);
    UpdateFactionLabel(kIdcResistance);
    // The outfit pair is read off the (possibly re-seeded) resistance block.
    RefreshOutfitChoices();
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
    UpdateFactionLabel(kIdcOutfit);
}

void GuerrillaNewGame::UpdateFactionLabel(int idc)
{
    IControl* ctrl = GetCtrl(idc);
    if (!ctrl)
    {
        return;
    }
    const char* prefix = idc == kIdcOccupier ? "OCCUPIER" : (idc == kIdcResistance ? "RESISTANCE" : "OUTFIT");
    const std::vector<RString>& list = idc == kIdcOccupier ? _occupiers
                                       : idc == kIdcResistance ? _resistances
                                                               : _outfits;
    const int sel = idc == kIdcOccupier ? _occupierSel : (idc == kIdcResistance ? _resistanceSel : _outfitSel);
    // An empty list means no config offered a real choice — nothing will be
    // published and the mission's own defaults decide (defaultOccupier /
    // defaultResistance keys, or the authored mission.sqm class for the
    // outfit), so say so instead of showing a value the mission may override.
    const char* selected = (sel >= 0 && sel < (int)list.size()) ? (const char*)list[sel] : "(mission default)";
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "%s: %s", prefix, selected);
    if (CActiveText* text = dynamic_cast<CActiveText*>(ctrl))
    {
        text->SetText(buffer);
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
            }
            UpdateFactionLabel(idc);
            break;
        case kIdcResistance:
            if (!_resistances.empty())
            {
                _resistanceSel = (_resistanceSel + 1) % (int)_resistances.size();
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

void __cdecl CreateDisplayGuerrilla(ControlsContainer* parent)
{
    parent->CreateChild(new GuerrillaNewGame(parent));
}

} // namespace Poseidon
