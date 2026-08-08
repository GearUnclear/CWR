// Fixture mod for scripting/prop_spawn_safety. Mount LAST, after @LoBo and
// @lobofixup:
//   --mod "@LoBo;arma_CWR/tests/fixtures/mods-lobo/@lobofixup;arma_CWR/tests/fixtures/mods-propsafety/@propsafety"
//
// It ships nothing but a preload roster, and it exists to pin two engine
// behaviours the prop-spawn regression depends on:
//
//  1. EVERY mounted mod's bin/config is merged, not just one. The deferred-merge
//     slot in ConfigParsers used to be a single SRef that each config-bearing mod
//     overwrote, and because the mod enumeration runs in reverse mount order the
//     survivor was the EARLIEST-listed mod. With this fixture mounted last, its
//     roster is exactly the one that used to be thrown away, while @lobofixup's is
//     the one that used to win - so the test asserting BOTH took effect fails if
//     the accumulation regresses in either direction.
//
//  2. A CfgAddons/PreloadAddons roster from a mod reaches World::ActivateAddons at
//     all. The stock CWA config.bin locks CfgAddons at PAReadOnly, and
//     ParamClass::Update refuses a read-only destination silently (AccessDenied
//     reports through RptF, which is compiled out), so this whole mechanism was
//     dead. See AddonSystem::MergeConfigInto.
//
// The two names are chosen for what they unlock in the test:
//   LoBoWreck        - owns the ten wreck props. They used to read back scope 0
//                      because LoBoWreck.pbo omits the #define public 2 header
//                      its sibling @LoBo configs carry; tools/lobo/
//                      fix-lobo-scope.ps1 repairs that in the pbo, and CASE 2b
//                      of the test asserts they are createable again.
//                      Activating the owner also isolates the ABSTRACT-type path
//                      from the denied-owner path, so each is proved separately.
//   LoBoPalObj       - the second pbo with the same missing header; owns
//                      LoBo_Poster_01 and the seven posters that inherit it.
//   MAP_Editorupgrade - owns MAP_Barrel1, a static prop that must actually spawn
//                      and draw.
// Deliberately NOT listed: GWbuild1 (owner of GWRubble), the control for a class
// whose owner nobody activates.
class CfgPatches
{
    class PropSafety
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 1.30;
        requiredAddons[] = {};
    };
};

class CfgAddons
{
    class PreloadAddons
    {
        class PropSafetyRoster
        {
            list[] = {"LoBoWreck", "LoBoPalObj", "MAP_Editorupgrade"};
        };
    };
};
