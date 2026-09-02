// Data fixup for the @LoBo mod (a THIRD-PARTY 2005-era community addon set
// used as the guerrilla-mode swappability test bed - see the guerrilla_sinai
// tests). Mount AFTER @LoBo: --mod "@LoBo;.../@lobofixup".
//
// WHY: LoBoammo.pbo and LoBo_airammo.pbo ship a malformed float literal in
// eleven CfgAmmo classes: tracerColor[]={0.8,0.5,0.0.1,...} - note "0.0.1".
// The original 1.96 engine silently coerced that token; this engine's config
// reader falls back to evaluating non-numeric tokens as script expressions,
// so every read of those entries logs "Script error at '0.0|#|.1': Unknown
// operator" - and under --autotest ANY script error is a hard abort
// (GameStateStringtableInfoFunctions::DisplayErrorMessage, Game/Scripting/
// ExpressExt.cpp), which kills the game with exit code 2 during boot.
//
// This deferred-merge config overwrites just those arrays with the obviously
// intended values (blue channel 0.0, alpha kept), letting integration tests
// mount the otherwise-unmodified @LoBo. No gameplay change beyond the tracer
// tint of a few tank shells.
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
