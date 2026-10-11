#pragma once

#include <vector>

namespace Poseidon
{

struct MainMenuControlRect
{
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;
    bool visible = true;
};

struct MainMenuModsPlacement
{
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;
    bool rightAlign = false;
};

MainMenuModsPlacement CalculateMainMenuModsPlacement(const MainMenuControlRect& quit,
                                                     const std::vector<MainMenuControlRect>& foreground);

class ParamClass;

/// The UD front-door menu class, defined by bin/ud-mainmenu.hpp (guerrilla-mode/runtime/common).
inline constexpr const char* kMainMenuClassUD = "RscDisplayMainUD";
/// The official Remaster package's menu class (Bohemia's bin/mainMenu.hpp).
inline constexpr const char* kMainMenuClassRemaster = "RscDisplayMainRemaster";
/// The vanilla menu class every resource ships.
inline constexpr const char* kMainMenuClassVanilla = "RscDisplayMain";

/// The main-menu resource class to load from `res`: the UD menu when it is merged, else the
/// Remaster menu, else vanilla. The UD menu carries its own class name so that setup's
/// Remaster mode can install it BESIDE Bohemia's menu: both are included from the same
/// bin/resource-extra.cpp parse, where a second RscDisplayMainRemaster would be a
/// "Member already defined" error (issue #67).
const char* SelectMainMenuResourceClass(const ParamClass& res);

} // namespace Poseidon
