// Test fixture for the @LoBo mod (a THIRD-PARTY 2005-era community addon set
// used as the guerrilla-mode swappability test bed - see the guerrilla_sinai
// tests). Mount AFTER @LoBo: --mod "@LoBo;.../@lobofixup".
//
// WHY IT IS STILL MOUNTED: the CfgAddons preload roster below and the
// #include of guerrilla-mode/config/lobo-factions.hpp at the end. The lobo
// lane tests depend on both (prop_spawn_safety, for one, needs LoBolebObject
// preloaded), so this file stays load-bearing on a fully repaired @LoBo.
//
// HISTORY: LoBoammo.pbo and LoBo_airammo.pbo ship a malformed float literal in
// eleven CfgAmmo classes: tracerColor[]={0.8,0.5,0.0.1,...} - note "0.0.1".
// The original 1.96 engine silently coerced that token. This engine's config
// reader used to evaluate it as a script expression instead, which logged
// "Script error at '0.0|#|.1': Unknown operator" on every read and, under
// --autotest, killed the boot with exit code 2. That no longer happens: since
// issue #54 D1 the reader coerces the token to its strtod prefix at load
// (ClassifyToleratedLiteral, IO/ParamFile/ParamFile.cpp; only --strict-config
// keeps it raw), and `PoseidonTools mod doctor <@LoBo> --fix` repairs the
// bytes inside @LoBo itself.
//
// So the CfgAmmo block below, which overwrites just those arrays with the
// obviously intended values (blue channel 0.0, alpha kept), and the optional
// addons/ shadow pbos from gen-patched-pbos.ps1 are no longer required. Both
// are harmless and stay as belt and braces. No gameplay change beyond the
// tracer tint of a few tank shells.
class CfgPatches
{
    class LoBoFixup
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 1.30;
        requiredAddons[] = {"LoBoammo"};
    };
};
// Pre-activate the LoBo addons the Guerrilla.Sinai and Guerrilla.Lebanon80
// templates spawn from. The engine only builds a vehicle type when its
// class - and every weapon/magazine class it references - belongs to an
// ACTIVE addon (World::CheckAddon / ParamOwnerList; failures log "Access
// denied" at debug level and abort the mission boot). Missions activate what
// their addOns[] lists, but runtime spawns (the native guerrilla garrison,
// QRF) are safest with the whole set preloaded at mod level. The Lebanon80
// additions: LoBo_Leb (world) + LoBolebObject (its terrain objects),
// LoBoHizballah/LoBoHizballCar (the resistance roster) and ICPrpg7 (its
// RPG-7/7VR/29 launchers + magazines).
class CfgAddons
{
    class PreloadAddons
    {
        class LoBoGuerrilla
        {
            list[] = {"sinai", "LoBoTerror", "LoBoEgypt", "LoBoIs", "LoBoWeapons", "LoBoWeapNad", "LoBoWeapAT", "LoBoammo", "JAM_Magazines", "LoBoHMWV", "LoBo_Zelda", "LoBo_Leb", "LoBolebObject", "LoBoHizballah", "LoBoHizballCar", "ICPrpg7"};
        };
    };
};
class CfgAmmo
{
    // LoBoammo.pbo
    class LoBo_60HVMSAP    { tracerColor[] = {0.8, 0.5, 0.0, 0.25}; };
    class LoBo_105L7Sabot  { tracerColor[] = {0.8, 0.5, 0.0, 1}; };
    class LoBo_105L7Sabot2 { tracerColor[] = {0.8, 0.5, 0.0, 1}; };
    class LoBo_120mk4Sabot { tracerColor[] = {0.8, 0.5, 0.0, 0.25}; };
    class LoBo_125Sabot1   { tracerColor[] = {0.8, 0.5, 0.0, 0.25}; };
    class LoBo_125Sabot2   { tracerColor[] = {0.8, 0.5, 0.0, 0.25}; };
    class LoBo_M240Coax    { tracerColor[] = {0.8, 0.5, 0.0, 1}; };
    class LoBo_50cal       { tracerColor[] = {0.8, 0.5, 0.0, 1}; };
    // LoBo_airammo.pbo
    class LoBo_M240Air     { tracerColor[] = {0.8, 0.5, 0.0, 0.25}; };
    class LoBo_20          { tracerColor[] = {0.8, 0.5, 0.0, 0.25}; };
    class LoBo_Apach30     { tracerColor[] = {0.8, 0.5, 0.0, 0.25}; };
};

// ---------------------------------------------------------------------------
// The @LoBo war factions (IDF, EgyptFrontier, EgyptArmy, Syria, Jordan,
// Hizballah, PLO, PLO_East) live ONCE in the tracked
// guerrilla-mode/config/lobo-factions.hpp and are included here so the lobo
// test lanes carry them whether or not the player-facing install has run
// (tools/lobo/install-lobo-factions.ps1 writes the same file into
// <@LoBo>\bin\config.cpp). The config reader resolves #include against the
// including file's directory, hence the relative climb to the repo.
#include "..\..\..\..\..\guerrilla-mode\config\lobo-factions.hpp"
