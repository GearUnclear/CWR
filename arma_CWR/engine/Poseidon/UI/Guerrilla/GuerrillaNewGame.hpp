#pragma once

// Guerrilla Mode new-game selection — the Phase-1.5 stopgap flow from
// mod-plans/13-guerrilla-mode.md: pick an island, an occupier faction and a
// resistance faction at new-game, then launch the "Guerrilla.<World>" template
// mission through the same path the single-mission browser uses (the launch
// itself happens in DisplayMain::OnChildDestroyed, OptionsUIApp.cpp, keyed on
// IDD_GUERRILLA_NEW_GAME). This is NOT the Phase-3 GuerrillaDisplay strategy
// screen.
//
// Resource strategy: the engine repo ships no UI resource data — every Rsc*
// class lives in the game data's bin/resource-extra.cpp (see
// MergeBaseResourceExtra in Asset/Addon/ConfigParsers.cpp). The display
// therefore loads "RscDisplayGuerrillaNewGame" when the data provides one and
// otherwise reuses the vanilla "RscDisplaySelectIsland" layout (island list +
// OK/Cancel notebook), adding two occupier/resistance cycler buttons cloned
// from the main menu's Quit class — the same clone-and-SetBase technique
// DisplayMain uses to inject the MODS entry into community menus. With no
// menu resources at all (headless --test-mission runs, PoseidonUITest) the
// display simply has no controls, matching DisplayMain's guard philosophy;
// Esc still exits via the ControlsContainer key path.

#include <Poseidon/UI/Controls/UIControls.hpp>

#include <functional>
#include <vector>

namespace Poseidon
{
// Display id GuerrillaNewGame reports to DisplayMain::OnChildDestroyed.
// Forced after Load() so the reused RscDisplaySelectIsland resource does not
// dispatch as the editor's IDD_SELECT_ISLAND. Vanilla dialog ids stop at
// IDD_JOIN_REQUIREMENTS (75) / IDD_UNITINFO (100) in Core/resincl.hpp; the
// constant lives here to keep the module UI-local.
constexpr int IDD_GUERRILLA_NEW_GAME = 76;

// Script-visible globals the launch path VarSets (and mirrors into
// GStats._campaign._variables so the engine's load-variables pass re-applies
// them after the mission parses). Read by the template mission's init.sqs.
constexpr const char* kGuerrillaVarIsland = "gmSelIsland";
constexpr const char* kGuerrillaVarOccupier = "gmSelOccupier";
constexpr const char* kGuerrillaVarResistance = "gmSelResistance";

// Built-in fallback faction names when the global config (Pars) has no
// CfgGuerrillaFactions — mission-side config is not loaded yet at the main
// menu, so these sides mirror the ZoneRegistry faction schema defaults.
constexpr const char* kGuerrillaDefaultOccupier = "EAST";
constexpr const char* kGuerrillaDefaultResistance = "GUER";

// Pure list builders (unit-testable with an injected ParamFile):
// islands = subclass names of CfgWorldList whose world file exists (the same
// enumeration DisplaySelectIsland uses for the editor's island list).
std::vector<RString> GuerrillaListIslands(const ParamEntry* worldList, const std::function<bool(RString)>& worldExists);
// factions = subclass names of factionsCfg (CfgGuerrillaFactions) whose
// `side` (default: the class name) matches `side` case-insensitively; when
// none match, every subclass; when there are no subclasses at all (or cfg is
// null), the single built-in default { side }.
std::vector<RString> GuerrillaListFactions(const ParamEntry* factionsCfg, const char* side);

class GuerrillaNewGame : public Display
{
  public:
    GuerrillaNewGame(ControlsContainer* parent);

    Control* OnCreateCtrl(int type, int idc, const ParamEntry& cls) override;
    void OnButtonClicked(int idc) override;
    void OnLBDblClick(int idc, int curSel) override;
    void OnCtrlClosed(int idc) override;

    // Selections read by DisplayMain::OnChildDestroyed on IDC_OK. The island
    // falls back to the current (menu cutscene) world when no island list was
    // shown, so OK still resolves to a concrete "Guerrilla.<World>" template.
    RString SelectedIsland();
    RString SelectedOccupier() const;
    RString SelectedResistance() const;

  protected:
    void InjectFactionCyclers();
    void UpdateFactionLabel(int idc);

    int _exitWhenClose;
    std::vector<RString> _occupiers;
    std::vector<RString> _resistances;
    int _occupierSel = 0;
    int _resistanceSel = 0;
};

} // namespace Poseidon
