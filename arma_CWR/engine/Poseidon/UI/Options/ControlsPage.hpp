#pragma once

// Controls section index , landing page for input settings.
//
// Rows: Keyboard & Mouse (binding rebinder), Mouse (sensitivity /
// Y-invert / button swap), Gamepad (binding rebinder), Gamepad Tuning
// (deadzones / sensitivity), Reset all to defaults, Arma 3 controls, Close. Mirrors
// IndexPage's FlatMenuPage shape; see optionsShell.hpp's
// "RscOptionsPageControls" for the resource bundle.
//
// The Arma 3 controls row (idc 1407) is UD's, not Bohemia's: the official
// Remaster's optionsShell.hpp defines a six-row RscOptionsPageControls without
// it, and setup keeps that file byte-identical. The row comes from
// bin/ud-options.hpp (guerrilla-mode/runtime/common), which setup appends as an
// #include to Bohemia's resource-extra.cpp; it defines RscOptionsPageControlsUD,
// inheriting Bohemia's page and adding the row. Redefining
// RscOptionsPageControls itself in that parse would be a "Member already
// defined" error, so the page prefers the UD class when it is merged and falls
// back to the stock one (the Classic shim's stock page already carries 1407).

#include <Poseidon/UI/Options/FlatMenuPage.hpp>

namespace Poseidon
{
class ParamClass;

/// The UD Controls page class, defined by bin/ud-options.hpp (guerrilla-mode/runtime/common).
inline constexpr const char* kControlsPageClassUD = "RscOptionsPageControlsUD";
/// The Controls page class every Options shell ships (Bohemia's or the Classic shim's).
inline constexpr const char* kControlsPageClassStock = "RscOptionsPageControls";

/// The Controls page resource class to mount from `res`: the UD page when it is merged,
/// else the stock page.
const char* SelectControlsPageResourceClass(const ParamClass& res);

class ControlsPage : public FlatMenuPage
{
  public:
    ControlsPage() : FlatMenuPage(kCycle, kCycleSize) {}

    const char* TitleText() const override;               // STR_DISP_OPT_CONTROLS
    int DefaultFocusIdc() const override { return 1401; } // Keyboard & Mouse, top row
    const char* ResourceClassName() const override;       // SelectControlsPageResourceClass(Res)

  protected:
    bool OnNav(OptionsShell& shell, int idc) override;

  private:
    // Cycle order matches the visible row layout in RscOptionsPageControls.
    // 1401 KB&M, 1402 Mouse, 1405 Gamepad, 1406 Gamepad Tuning,
    // 1403 Reset all, 1407 Arma 3 controls, 1404 Close. Stay in sync with the resource bundle
    // when adding rows.
    static constexpr int kCycle[] = {1401, 1402, 1405, 1406, 1403, 1407, 1404};
    static constexpr int kCycleSize = sizeof(kCycle) / sizeof(kCycle[0]);
};

} // namespace Poseidon
