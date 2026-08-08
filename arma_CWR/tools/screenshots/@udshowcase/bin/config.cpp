// Screenshot-shoot support mod.  Mount LAST, after @LoBo and @lobofixup:
//   --mod "@LoBo;arma_CWR/tests/fixtures/mods-lobo/@lobofixup;arma_CWR/tools/screenshots/@udshowcase"
//
// WHY THIS EXISTS.  The engine only builds a vehicle type when its class - and
// every weapon/magazine class it references - belongs to an ACTIVE addon
// (World::CheckAddon / ParamOwnerList).  A mission activates what its addOns[]
// lists; anything else spawned at runtime resolves partially and logs
//   Access denied: CONFIG.cpp/CfgVehicles/<class>/ - owner addon '<x>' is not
//   activated (missing from the mission's addOns[]?)
// and the shoot then died outright the first time it spawned a Syrian rifleman
// (exit -1, no stderr) rather than skipping the class.
//
// @lobofixup already preloads the handful of addons the Guerrilla.Sinai and
// Guerrilla.Lebanon80 templates spawn from.  The showcase spawns most of
// @LoBo's roster instead, so it needs a much longer list - and it must not
// widen a shared test fixture to get it, because guerrilla_lobo_body_save_reload
// asserts against the exact owner set @lobofixup activates.  Hence a separate
// mod that only this shoot mounts.
//
// HISTORY (2026-08-08).  This file was written before the mechanism it relies on
// actually worked, so its list looked inert for a while and the "Access denied"
// warnings kept coming.  Two engine bugs, both silent:
//   * ConfigParsers held ONE deferred mod bin/config in a single SRef, so with
//     @lobofixup also mounted only one of the two survived - and since mod
//     enumeration runs in reverse mount order, the survivor was the earlier
//     mod, i.e. this file was always the one discarded.
//   * The stock CWA config.bin locks CfgAddons at PAReadOnly and
//     ParamClass::Update refuses a read-only destination through
//     AccessDenied -> RptF, which is compiled out.  No mod's PreloadAddons
//     roster had ever reached World::ActivateAddons.
// Both are fixed (AddonSystem::MergeConfigInto); the list below now genuinely
// activates every name it holds.  World::ActivateAddons logs the resulting
// count at INFO, which is the quickest way to confirm it on a run.
// SCOPE FIXUP - MOVED (2026-08-08).  This file used to carry a CfgVehicles
// overlay forcing scope 2 onto the LoBoWreck classes, which the engine refused
// as abstract.  The cause was never a broken model: LoBoWreck.pbo and
// LoBoPalObj.pbo are the only two @LoBo addons that omit the
//     #define private 0 / #define protected 1 / #define public 2
// header every other @LoBo config carries, so their `scope = public;` resolved
// to nothing and read back as 0.  @LoBo is ours now, so that is repaired at
// source instead - tools/lobo/fix-lobo-scope.ps1 rewrites the value in place
// inside the pbo.  Run it once per @LoBo install; without it those classes log
// "Cannot create '<class>': type is abstract" and yield objNull again.
class CfgPatches
{
    class UDShowcase
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
        class UDShowcaseRoster
        {
            list[] =
            {
                // worlds and their terrain objects
                "sinai", "LoBo_Leb", "LoBolebObject",
                // shared weapons / ammo / effects
                "LoBoWeapons", "LoBoWeapNad", "LoBoWeapAT", "LoBoammo", "JAM_Magazines",
                "JAM_Vehicles", "ICPrpg7", "LoBo_Sounds", "LoBo_Mines", "LoBoRadios",
                "BN_Tracers", "BN_Tracers122", "BN_Tracers123",
                // Israel
                "LoBoIs", "LoBoMerkava2", "LoBo_Merkava3", "LoBoM60", "LoBoCen",
                "LoBoachzrit", "LoBo_Zelda", "LoBoHMWV", "LoBosufa", "LoBo_Trucks",
                "LoBo_IDF_Helo", "LoBoF15", "LoBoF16", "LoBo_Kfir", "LoBo_A4",
                "LoBo_FAB", "LoBo_Pilot",
                // Egypt
                "LoBoEgypt", "LoBo_Cars_Egy", "LoBoEghelo", "LoBo_T55", "LoBo_M1",
                "LoBo_IL28", "LoBo_MiG", "LoBo_AA", "LoBo_SA6",
                // Syria
                "LoBoSyria", "LoBo_T72", "LoBo_BTR60",
                // Jordan
                "LoBoJordan", "LoBoJordanAPC", "LoBo_Jordan_cars", "LoBoJordanhelo",
                "LoBo_Challenger", "LoBO_110",
                // irregulars and civilians
                "LoBoTerror", "LoBoHizballah", "LoBoHizballCar", "Lobo_Toy",
                "LoBo_Cars", "LoBo_staticweaps", "Lobo_Fighting_pos",
                "LoBo_Art", "LoBoWreck",
                // set dressing for the narrative shoot: wrecks, ruins,
                // sandbags, barbed wire, barrels, rubble, burning oil wells
                "LoBo_Objects", "LoBolebObject", "LoBoPalObj", "LoBobloc",
                "MAP_Editorupgrade", "MAP_MilObj_Pack", "MAP_OilAddon",
                "MAP_OilAddon_3D", "MAP_OilAddon_Tex", "MAP_Heaps", "MAP_Misc",
                "MAP_Shed", "GWbuild1", "JOF_Objects1", "TMYK_Bridges",
                "chd_walls", "chd_environment", "chd_houses",
                "f3wx_o1_version1", "f3wx_o1_version1_1",
                // Everything else declaring a CfgPatches class anywhere under
                // @LoBo/addons, harvested from the pbos themselves rather than
                // guessed from filenames (several disagree - the pbo is
                // MAP_MilObj-Pack, the addon is MAP_MilObj_Pack). An entry that
                // names no real addon is inert, so over-listing costs nothing
                // and beats another 150-second boot to discover one more owner.
                "AGS_build_r2", "AGS_Harbour", "AGS_inds_ver_2_1",
                "AGS_industrial_pack", "AGS_oil_refinery", "AGS_other",
                "ags_port_example", "AGS_port_update", "AWOL_Bridge1",
                "Baracken", "BDE_Mir3", "Boot3", "brg_n4", "bruecke",
                "CBT_m113a", "CBT_m577", "coc_diver", "CoC_M101",
                "CoC_UnifiedArtillery", "CoCMLRS", "DKMM_MOD", "ds_radiotower",
                "erba", "F3WX_industrial_pack", "gaza_strip", "GW_scorpion",
                "GWV_scimitar", "Iran", "ITA_SHELTR2", "kkb_gate",
                "LoBo_Camel", "LoBo_F4", "LoBo_F5", "LoBo_IraqiAK47",
                "LoBo_an12", "LoBo_airammo", "LoBo_airline", "LoBoC130",
                "LoBoCrop", "LoBoMusic", "LoBorollers", "LoBoUSEmbassy",
                "LoBoTer", "LoBo_Ob", "LoBoLandtex", "LoBofightpos",
                "LoBoEgAPC", "LoBoEgCar", "LoBoSA6", "LoBoChallen",
                "LoBoJorAPC", "LoBoJorCar", "LoBoJorHelo", "LoBoIDFhelo",
                "lobolebtex", "lobo_gaza", "lobo_iran", "lost",
                "M109A6", "MineS", "MLV_OERLIKON35", "MLV_ROLAND",
                "MLV_SKYGUARD", "MX_Mirage", "OPGWC_EE11", "pcwmost", "sa9",
                "SoldierEMirage", "sttdesertzsu", "upmortar", "vbsshed",
                "vbswatowr", "Vit_Iraq_Shilka", "whatever", "zpu4",
                "zwa_cott", "zwa_hri", "zwa_hup", "zwa_telbu", "zwa_wc",
                "chd_bloc", "chd_ind", "chd_oriental", "RKSL_HASN2",
                "RKSL_Hangers", "TYMK_BM", "W_CombatStatics", "W_DeadAnims",
                "W_RestStatics", "awmcams", "barackenworld", "bwm_boot3",
                "bwm_bruecke", "wbe_tracerfx", "xobridge", "SWI_UI",
                "jordanian_army_2002", "Locke_Anims", "DMA_Lean",
                "CSJairlines", "CSJcitation", "CSJils", "CoC_Arty",
                "CoC_TTV", "CoC_Torpedoes", "JAM_Sounds", "BRG_N4",
                "non_border_jor", "non_inf_jor", "non_para_jor",
                "non_m113_jor", "non_m60_jor", "non_cobra_jor",
                "non_hummer_jor", "non_jeepmg_jor", "non_crew_jor",
                "non_pilot_jor", "non_sniper_jor", "non_spoter_jor"
            };
        };
    };
};
