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
#include <Poseidon/IO/ParamFile/ParamFile.hpp> // ParamFile (the per-island descriptor this display owns)

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
// Character outfit family (issue #25). Published values are the cycler
// tokens "WARRIOR" / "CIVILIAN"; consumers compare case-insensitively and
// only "civilian" ever acts, so publishing "WARRIOR" and publishing nothing
// are behaviorally identical (the cycler's default must resolve exactly like
// a no-UI launch).
constexpr const char* kGuerrillaVarOutfit = "gmSelOutfit";

// Pure list builders (unit-testable with an injected ParamFile):
// islands = subclass names of CfgWorldList whose world file exists (the same
// enumeration DisplaySelectIsland uses for the editor's island list).
std::vector<RString> GuerrillaListIslands(const ParamEntry* worldList, const std::function<bool(RString)>& worldExists);
// factions = every subclass of factionsCfg (CfgGuerrillaFactions) whose
// resolved side (its `side` key, defaulting to the class name) is not "CIV",
// case-insensitively. Both cyclers are fed this same list: any faction may
// occupy or resist, and ZoneRegistry::ResolveSideCollisions rebases whatever
// the pick collides with. When there are no subclasses at all (or cfg is
// null), EMPTY — no built-in entry is invented, so the launch path can tell
// "no real faction config" apart from a real choice and publish nothing.
std::vector<RString> GuerrillaListFactions(const ParamEntry* factionsCfg);

// The template mission base the launch path resolves for an island:
// "missions\Guerrilla.<island>" (append ".pbo" for the banked form or
// "\mission.sqm" for the unbanked directory).
RString GuerrillaTemplateMissionBase(RString island);
// True when the island's template exists in either form. Existence checks
// are injected (FilePathExists for the .pbo, QIFStreamB::FileExist for the
// unbanked mission.sqm) so the resolution logic is unit-testable — the same
// pattern as GuerrillaListIslands' worldExists.
bool GuerrillaTemplateExists(RString island, const std::function<bool(RString)>& pboExists,
                             const std::function<bool(RString)>& missionFileExists);
// Resolve a faction selection (a CfgGuerrillaFactions subclass name) to its
// side string (`side` key, defaulting to the class name). Empty when the
// selection is empty or names no subclass — the caller cannot validate what
// the mission's own config will resolve.
RString GuerrillaFactionSide(const ParamEntry* factionsCfg, RString faction);

// Index into `list` of the faction `selection` names, or -1. `selection` is
// what ZoneRegistry::ResolveSides accepts: either a descriptor class name or a
// side string, matched SIDE FIRST then class name — the same order
// ZoneRegistry::FindFaction scans, so an index found here names the record the
// registry would match for the same string.
int GuerrillaIndexOfSelection(const ParamEntry* factionsCfg, const std::vector<RString>& list, RString selection);

// The indices the two cyclers must OPEN on, so that pressing OK without
// touching them launches exactly what a direct, no-UI launch of the same
// template resolves. Walks the same precedence chain
// ZoneRegistry::LoadFromParams does, minus the gmSel* rung (which is what
// these selections become): the zones config's defaultOccupier /
// defaultResistance keys, then the registry's built-in EAST occupier / GUER
// resistance, then two DISTINCT indices as the floor. Both -1 when `list` is
// empty (no real choice — the selections stay EMPTY and nothing is published).
// A template that deliberately aims both default* keys at one descriptor is
// honoured; ZoneRegistry::DivergeAliasedFactions handles that pair.
void GuerrillaDefaultSelections(const ParamEntry* factionsCfg, const ParamEntry* zonesCfg,
                                const std::vector<RString>& list, int& outOccupier, int& outResistance);

// The IDC_OK pair check, factored out of the display so it is testable without
// the UI stack. It mirrors ZoneRegistry::ResolveSideCollisions step for step:
// the registry performs the substitution, this decides whether to let the
// player launch at all, and the two drifting apart is exactly what the old
// dead guard was. zonesCfg supplies the template's `playerSide` (null or
// absent = the legacy templates the registry rebases nothing for, where a
// same-side pair really would fight itself and must be blocked here).
// outMessage carries the player-facing reason, set only on a false return.
// Selections that resolve to no side are unvalidatable and pass through.
bool GuerrillaSelectionIsResolvable(const ParamEntry* factionsCfg, const ParamEntry* zonesCfg, RString occupier,
                                    RString resistance, RString& outMessage);

// The outfit-family choices the character cycler offers for the CURRENT
// resistance selection: {"WARRIOR", "CIVILIAN"} when the resolved resistance
// faction block authors a non-empty playerClassCiv, EMPTY otherwise (the
// cycler shows "(mission default)" and the launch publishes nothing). The
// resistance block resolves with the registry's precedence: `resistance`
// (side first, then class name - ZoneRegistry::FindFaction order) > the
// zones config's defaultResistance > the built-in GUER side. WARRIOR is
// always index 0: it is the authored mission.sqm class, so an untouched
// cycler publishes a value indistinguishable from publishing nothing.
std::vector<RString> GuerrillaOutfitChoices(const ParamEntry* factionsCfg, const ParamEntry* zonesCfg,
                                            RString resistance);

// The CfgVehicles class the character-preview mannequin (issue #25 M4)
// renders for the CURRENT resistance + outfit selection: playerClassCiv for
// "CIVILIAN", playerClassWarrior for "WARRIOR" or for an EMPTY outfit (the
// cycler may offer no pair while the descriptor still authors the warrior
// body, which documents the authored mission.sqm class in every shipped
// template). EMPTY when no faction block resolves, the block lacks the key,
// or the token is unknown. The resistance block resolves with the registry's
// precedence, exactly like GuerrillaOutfitChoices.
RString GuerrillaOutfitPreviewClass(const ParamEntry* factionsCfg, const ParamEntry* zonesCfg, RString resistance,
                                    RString outfit);

// Resolve that class to the raw CfgVehicles `model` value, gated plan-15
// style: EMPTY (the preview hides, never a substitute body) when the class
// is not in the loaded data package, authors no model (inherited `model`
// keys resolve, ParamClass::FindEntry follows the base chain), or the shape
// file the model resolves to (GetShapeName) fails the injected existence
// probe. The probe is injected (QIFStreamB::FileExist in the display) so the
// resolution logic is unit-testable, the GuerrillaTemplateExists pattern.
RString GuerrillaOutfitPreviewModel(const ParamEntry* vehiclesCfg, RString className,
                                    const std::function<bool(RString)>& shapeFileExists);

// True when a body proxy is the soldier's shoulder/back flag proxy
// ("flag_vojak" on every vanilla body; matched on the flag/vlajka name stems
// so third-party bodies that author their own flag proxy are covered too).
// The preview mannequin never draws these: the proxy model is hard-textured
// with a default US flag (data\usa_vlajka.pac) that only in-mission
// flag-carrier machinery may rebind, so NO flag is the only faction-safe
// rendering. Pure, for the same unit-test seam as the resolvers above.
bool GuerrillaPreviewIsFlagProxy(const char* proxyName);

// Where an island's Guerrilla template publishes its CfgGuerrillaFactions:
// under the CreateSingleMissionBank mount prefix for a banked (.pbo)
// template (bankPrefix non-empty, already ending in "\\"), otherwise
// directly under the unbanked mission directory. CfgGuerrillaFactions lives
// in each template's own description.ext (see guerrilla-mode/mission/*), NOT
// in any addon's global config — Pars never carries this class — so the
// new-game screen must read it per-island instead of from Pars.
RString GuerrillaFactionsDescriptionPath(RString island, RString bankPrefix);

class GuerrillaNewGame : public Display
{
  public:
    GuerrillaNewGame(ControlsContainer* parent);

    Control* OnCreateCtrl(int type, int idc, const ParamEntry& cls) override;
    // Creates the outfit-preview mannequin (idc 154) as a GuerrillaOutfitPreview
    // (a ControlObject subclass, file-local in the .cpp) - the same hook
    // DisplayNewUser uses to substitute CHead for its head object.
    ControlObject* OnCreateObject(int type, int idc, const ParamEntry& cls) override;
    // Turntable rotation for the preview mannequin, the
    // DisplayNewUser::OnSimulate -> CHead::Simulate pattern.
    void OnSimulate(EntityAI* vehicle) override;
    void OnButtonClicked(int idc) override;
    void OnLBDblClick(int idc, int curSel) override;
    void OnLBSelChanged(int idc, int curSel) override;
    void OnCtrlClosed(int idc) override;

    // Selections read by DisplayMain::OnChildDestroyed on IDC_OK. The island
    // falls back to the current (menu cutscene) world when no island list was
    // shown, so OK still resolves to a concrete "Guerrilla.<World>" template.
    // The faction selections are EMPTY when no CfgGuerrillaFactions offered a
    // real choice — the launch path must publish nothing in that case, so the
    // template's own default* keys keep deciding. When there IS a real choice
    // the cyclers open on the pair those same default* keys resolve to
    // (GuerrillaDefaultSelections), so an untouched screen publishes a pick
    // that is indistinguishable from publishing nothing.
    RString SelectedIsland();
    RString SelectedOccupier() const;
    RString SelectedResistance() const;
    RString SelectedOutfit() const; // EMPTY when the descriptor offers no outfit pair

  protected:
    void InjectFactionCyclers();
    void UpdateFactionLabel(int idc);
    // Reparses _islandCfg from the given island's own Guerrilla template
    // description.ext (falling back to a global CfgGuerrillaFactions /
    // CfgGuerrillaZones, then to an empty/mission-default list when neither
    // publishes one), repopulates _occupiers/_resistances from it, and re-seeds
    // the cycler selections from the template's default* keys
    // (GuerrillaDefaultSelections) — preserving any current pick the new roster
    // still carries, by NAME. When the cycler controls already exist their
    // labels are refreshed too.
    //
    // Call ONLY when the island actually changed: this re-seeds the selections
    // and remounts the template PBO, so running it on a click that reselected
    // the same island would discard the player's picks mid-gesture (see
    // OnLBSelChanged).
    void RefreshFactionsForIsland(RString island);
    // Recompute _outfits for the CURRENT resistance selection (island change
    // or resistance cycling both change which block playerClassCiv is read
    // from), preserving the player's pick by NAME when the new roster still
    // offers it; refreshes the label when the control exists.
    void RefreshOutfitChoices();
    // Re-resolve the preview mannequin's body for the CURRENT island/
    // resistance/outfit selection: inject the CT_OBJECT on the first model
    // that resolves, swap its shape on later changes, and hide it whenever
    // the descriptor offers no class or the class/model is missing from the
    // loaded package (plan-15 shaped: degrade, never crash, never a
    // substitute body). Gated on the styled cyclers existing, so headless
    // no-resource runs keep owning no controls.
    void UpdateOutfitPreview();

    int _exitWhenClose;
    // OWNS the selected island's parsed description.ext: the two entry
    // pointers below point INTO it, so it must outlive them (the previous
    // function-local ParamFile is why the IDC_OK guard fell back to Pars,
    // which never carries CfgGuerrillaFactions — see the note above).
    ParamFile _islandCfg;
    const ParamEntry* _islandFactions = nullptr; // into _islandCfg, or into Pars, or null
    const ParamEntry* _islandZones = nullptr;    // ditto; carries playerSide + the default* keys
    // The island _islandCfg/_occupiers/_resistances currently describe. The
    // island list fires OnLBSelChanged on every click, not just on a real
    // change, so this is what makes the refresh idempotent.
    RString _islandForFactions;
    std::vector<RString> _occupiers;
    std::vector<RString> _resistances;
    int _occupierSel = 0;
    int _resistanceSel = 0;
    // Character outfit family (issue #25): {"WARRIOR", "CIVILIAN"} when the
    // selected resistance descriptor offers the pair, empty otherwise.
    std::vector<RString> _outfits;
    int _outfitSel = -1;
};

} // namespace Poseidon
