#include <Poseidon/UI/Guerrilla/GuerrillaNewGame.hpp>
#include <Poseidon/UI/Guerrilla/GuerrillaModule.hpp>
#include <Poseidon/UI/GameModule.hpp>
#include <Poseidon/Core/resincl.hpp>
#include <Poseidon/Core/Global.hpp>
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

// Normalized-screen slots for the injected 2D cyclers — bottom-left column,
// clear of the island-list notebook that fills the screen centre.
constexpr float kCyclerX = 0.02f;
constexpr float kCyclerW = 0.32f;
constexpr float kCyclerH = 0.05f;
constexpr float kCyclerOccupierY = 0.80f;
constexpr float kCyclerResistanceY = 0.87f;
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

std::vector<RString> GuerrillaListFactions(const ParamEntry* factionsCfg, const char* side)
{
    std::vector<RString> all;
    std::vector<RString> matching;
    if (factionsCfg)
    {
        for (int i = 0; i < factionsCfg->GetEntryCount(); i++)
        {
            const ParamEntry& e = factionsCfg->GetEntry(i);
            if (!e.IsClass())
            {
                continue;
            }
            RString name = e.GetName();
            all.push_back(name);
            RString entrySide = e.ReadValue("side", name);
            if (stricmp(entrySide, side) == 0)
            {
                matching.push_back(name);
            }
        }
    }
    if (!matching.empty())
    {
        return matching;
    }
    if (!all.empty())
    {
        return all;
    }
    return {RString(side)};
}

GuerrillaNewGame::GuerrillaNewGame(ControlsContainer* parent) : Display(parent)
{
    _exitWhenClose = -1;

    const ParamEntry* factions = Pars.FindEntry("CfgGuerrillaFactions");
    _occupiers = GuerrillaListFactions(factions, kGuerrillaDefaultOccupier);
    _resistances = GuerrillaListFactions(factions, kGuerrillaDefaultResistance);

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

void GuerrillaNewGame::UpdateFactionLabel(int idc)
{
    IControl* ctrl = GetCtrl(idc);
    if (!ctrl)
    {
        return;
    }
    const bool occupier = idc == kIdcOccupier;
    const std::vector<RString>& list = occupier ? _occupiers : _resistances;
    const int sel = occupier ? _occupierSel : _resistanceSel;
    const char* selected = (sel >= 0 && sel < (int)list.size()) ? (const char*)list[sel] : "?";
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "%s: %s", occupier ? "OCCUPIER" : "RESISTANCE", selected);
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
        default:
            // IDC_OK falls through to the base Exit(IDC_OK); the launch runs
            // in DisplayMain::OnChildDestroyed (IDD_GUERRILLA_NEW_GAME).
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
    return kGuerrillaDefaultOccupier;
}

RString GuerrillaNewGame::SelectedResistance() const
{
    if (_resistanceSel >= 0 && _resistanceSel < (int)_resistances.size())
    {
        return _resistances[_resistanceSel];
    }
    return kGuerrillaDefaultResistance;
}

void __cdecl CreateDisplayGuerrilla(ControlsContainer* parent)
{
    parent->CreateChild(new GuerrillaNewGame(parent));
}

} // namespace Poseidon
