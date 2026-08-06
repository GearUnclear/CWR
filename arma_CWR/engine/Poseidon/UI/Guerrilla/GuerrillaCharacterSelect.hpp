#pragma once

// Guerrilla Mode character-select screen (issue #43): a full child display of
// GuerrillaNewGame that replaces the old flat BODY cycler. One CListBox lists
// every body the loaded package + mods offer (uncapped roster, human-readable
// GuerrillaBodyRowLabel rows grouped under per-side header rows), a large
// GuerrillaBodyPreview mannequin turntables the highlighted body on the
// right, and an info line names side/class/source addon. Opened from
// GuerrillaNewGame::OnButtonClicked (idc 155); the pick travels back through
// GuerrillaNewGame::OnChildDestroyed reading SelectedRosterIndex() on
// IDC_OK - the same child-display result protocol DisplaySelectIsland's
// wizard uses (DisplayUI.cpp).
//
// Resource strategy (the GuerrillaNewGame pattern): the engine repo ships no
// UI resource data, so when the game data provides a
// "RscDisplayGuerrillaCharacterSelect" class it is loaded and OnCreateCtrl /
// OnCreateObject substitute the custom controls by idc; otherwise every
// control is synthesized programmatically, gated on the styled main-menu
// Quit class being available. With no menu resources at all (headless runs)
// the display owns no controls and Esc still exits via the ControlsContainer
// key path - and GuerrillaNewGame does not open it at all then
// (GuerrillaCharacterSelect::CanBuild).

#include <Poseidon/UI/Guerrilla/GuerrillaNewGame.hpp> // GuerrillaBodyChoice + Display base

#include <vector>

namespace Poseidon
{
// Display id reported to GuerrillaNewGame::OnChildDestroyed. Vanilla dialog
// ids stop at IDD_JOIN_REQUIREMENTS (75) / IDD_UNITINFO (100); 76 is
// IDD_GUERRILLA_NEW_GAME - this is the next Guerrilla slot.
constexpr int IDD_GUERRILLA_CHARACTER_SELECT = 77;

class GuerrillaBodyPreview;

class GuerrillaCharacterSelect : public Display
{
  public:
    // roster: the parent's uncapped GuerrillaListPlayerBodies list (copied,
    // so the child never dangles into the parent). currentSel: the parent's
    // _bodySel; -1 is the "(match outfit)" default and opens on row 0.
    GuerrillaCharacterSelect(ControlsContainer* parent, const std::vector<GuerrillaBodyChoice>& roster, int currentSel);

    // True when the display would own controls: a dedicated resource class
    // or the styled main-menu Quit class to synthesize from. The parent
    // checks this before CreateChild so a headless run never opens an empty
    // display (Esc would exit it, but not opening beats opening blind).
    static bool CanBuild();

    Control* OnCreateCtrl(int type, int idc, const ParamEntry& cls) override;
    ControlObject* OnCreateObject(int type, int idc, const ParamEntry& cls) override;
    // Turntable rotation for the mannequin, the DisplayNewUser::OnSimulate ->
    // CHead::Simulate pattern (same as the parent's idc-154 preview).
    void OnSimulate(EntityAI* vehicle) override;
    void OnLBSelChanged(int idc, int curSel) override;
    void OnLBDblClick(int idc, int curSel) override;

    // The result read by GuerrillaNewGame::OnChildDestroyed on IDC_OK: an
    // index into the constructor's roster vector, or -1 for the
    // "(match outfit)" row (publish nothing; the outfit token keeps deciding
    // the player body). Meaningful only on an IDC_OK exit.
    int SelectedRosterIndex() const { return _sel; }

  protected:
    // Synthesize every control (backdrop, title, roster list, info line,
    // CONFIRM/BACK, all but the lazily injected mannequin). No-op without
    // the styled Quit class - the headless guard.
    void BuildControls();
    // Land the selection on `row`: header rows were already resolved away by
    // OnLBSelChanged, so this records the row's roster value, refreshes the
    // info line and re-resolves the mannequin.
    void ApplySelection(int row);
    // Re-resolve the mannequin for the current _sel: inject the CT_OBJECT on
    // the first model that resolves, swap its shape on later changes, hide
    // it for "(match outfit)" (nothing to preview without the parent's
    // outfit context) and for any class/model the package does not ship
    // (plan-15 shaped: degrade, never substitute).
    void UpdatePreview();

    // Owned copy of the parent's roster; row `value` fields index into it.
    std::vector<GuerrillaBodyChoice> _roster;
    // GuerrillaBodyRowLabels(_roster) - the same shared builder the parent's
    // CHARACTER label uses, so the two can never disagree on a row's name.
    std::vector<RString> _rowLabels;
    // Current pick: roster index, or -1 for "(match outfit)".
    int _sel = -1;
    // Last landed list row, used to infer the movement direction when the
    // selection lands on a header row (skip forward when moving down,
    // backward when moving up).
    int _lastListRow = 0;
};

} // namespace Poseidon
