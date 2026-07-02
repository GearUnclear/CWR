#pragma once

namespace Poseidon
{
// Main-menu button IDC for Guerrilla Mode. Slot 120 is unused by the vanilla
// menu resources (IDC_MAIN_MODS = 119, IDC_MAIN_LOAD = 121 in Core/resincl.hpp);
// it lives here rather than in resincl.hpp so the Guerrilla module stays
// UI-local. No shipped RscDisplayMain provides a control with this idc —
// DisplayMain injects one at runtime (same clone-the-Quit-class mechanism it
// already uses to add the MODS entry to community menus).
constexpr int IDC_MAIN_GUERRILLA = 120;

namespace GuerrillaModule
{
void Register();
}
} // namespace Poseidon
