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

// ---------------------------------------------------------------------------
// The @LoBo war factions - the GLOBAL half of the Guerrilla faction table
// (issue #54 A4).
//
// These blocks used to be copied into Guerrilla.Sinai and Guerrilla.Lebanon80
// description.ext, one copy each, which is why IDF existed twice and Hizballah
// existed twice with the two copies already drifting. A roster is not an
// island fact - the Golani brigade is the same brigade on Sinai as in south
// Lebanon - so they live once, here, and the engine unions this table with
// each island template's own block (Game/Guerrilla/FactionSources.*), the
// island winning on a class-name collision.
//
// The CIV descriptor stays island-owned by rule and must never appear here:
// it names the population models and ambient-traffic hulls a given island's
// data set ships. Both @LoBo templates keep their own.
//
// This is a MOD config (bin\config.cpp of a mod folder), so it reaches the
// global config the same way an addon pbo config or the data dir's
// bin\config-extra.cpp does: deferred-merged into Pars. Mount it after @LoBo,
// as the tracer fixup above already requires. The key schema every descriptor
// follows is documented in guerrilla-mode/config/guerrilla-factions.hpp.
class CfgGuerrillaFactions
{
    class IDF // side WEST - occupier by default, resistance-capable since plan 16
    {
        side = "WEST";
        // No re-sided IDF INFANTRY roster exists in @LoBo: every LoBoIs man
        // hangs off LoBoShayetetWB : SoldierWB and inherits side=1. (LoBoIs is
        // not wholly side=1 - LoBo_paraE is side=0, LoBo_paraG/_haloparaG are
        // side=2, LoBo_paraC/_haloparaC are side=3 - but those are PARACHUTES,
        // vehicleClass="Air" with crew="SoldierGB", not a roster anything could
        // be twinned onto.) So IDF has no twin and can never auto-rebase off a
        // side collision; ResolveSideCollisions rebases the other pick instead.
        sideTwin = "";
        // WL<3 Golani regulars -> WL3-4 Paratroopers -> WL5+ Shayetet 13
        tiers[] = {"LoBoGolaniWB", "LoBoParaWB", "LoBoShayetetWB"};
        tierThresholds[] = {3, 5};
        // per-tier role variants (plan 15) - each tier family carries its
        // own MG/AT/medic/sniper siblings (verified in lost.pbo; see
        // tmp/class-survey/lobo-infantry.md for the full role map)
        tiersMG[]     = {"LoBoGolaniWMG", "LoBoParaWMG", "LoBoShayetetWMG"};
        tiersAT[]     = {"LoBoGolaniWLAW", "LoBoParaWLAW", "LoBoShayetetWLAW"};
        tiersMedic[]  = {"LoBoGolaniWMedic", "LoBoParaWMedic", "LoBoShayetetWMedic"};
        tiersSniper[] = {"LoBoGolaniWBSN", "LoBoParaWBSN", "LoBoShayetetSN"};
        // QRF escort ladder (full vehicleThresholds[] form): HMMWV patrol ->
        // M113 Zelda -> M60A1 Magach 6 -> Shot-Kal Mk.D (LoBo_Centurion2,
        // the period-correct '67/'73 Sinai Centurion - the Blazer-ERA
        // LoBo_Centurion is 1978+). Armed picks need the @lobofixup patched
        // pbos in strict/autotest lanes (malformed LoBoammo tracerColor).
        //
        // The Blazer-ERA hull would arguably fit 1980s Lebanon better than
        // the Mk.D, but one roster is one roster: an island that really wants
        // a different tank overrides the whole IDF class in its own
        // description.ext rather than forking this ladder.
        vehicles[] = {"LoBoHMWV_Mag_Patrol", "LoBo_Zelda", "LoBoM60A1", "LoBo_Centurion2"};
        vehicleThresholds[] = {3, 6, 9};
        officer = "LoBoGolaniWBo";

        // Resistance-capable keys (plan 16, purely additive - every key above
        // is byte-identical to the shipped/tested occupier descriptor). An IDF
        // insurgency reads as a Sinai stay-behind cell: it recruits from the
        // Golani tier and loots its own kit. Weapon->magazine pairs read out
        // of the LoBoWeapons / JAM_Magazines config bodies, not inferred.
        holdClass = "LoBoGolaniWB";
        holdCount = 3;
        recruitFighter = "LoBoGolaniWB";
        recruitSpecialist = "LoBoGolaniWMG";   // FN Minimi (LoBoM249)
        companionClass = "LoBoGolaniWB";

        // LoBo_M4A1 : LoBoCAR15 -> magazines[]={LoBoCAR15Mag, JAM_W556_30mag,...}
        //
        // The plain LoBo_M4A1 is the family's BASE carbine, not the exact rifle
        // any tier rifleman carries: LoBoGolaniWB has LoBo_M4A1_LD_G_Falcon,
        // LoBoParaWB has LoBo_M4A1_Mepor, LoBoShayetetWB has LoBo_M4A1_LD_ACOG
        // (optics/laser variants). The loot schema is single-valued and the
        // ladder is not, so it names the common denominator - every one of those
        // variants feeds from LoBoCAR15Mag, so the drop is always usable. The
        // medic entry below is the one that IS class-exact.
        baseWeapon = "LoBo_M4A1";
        baseMagazine = "LoBoCAR15Mag";
        lootRiflemanWeapon = "LoBo_M4A1";
        lootRiflemanMag = "LoBoCAR15Mag";
        lootMedicWeapon = "LoBo_M4A1";        // LoBoGolaniWMedic carries LoBo_M4A1
        lootMedicMag = "LoBoCAR15Mag";
        lootMGWeapon = "LoBoM249";            // LoBoGolaniWMG carries LoBoM249
        lootMGMag = "LoBoM249Mag";
        lootATWeapon = "JAM_M72LAWLauncher";  // LoBoGolaniWLAW carries the JAM M72
        lootATMag = "JAM_M72ALLRocket";
        lootSniperWeapon = "LoBo_SR25";       // LoBoGolaniWBSN carries LoBo_SR25
        lootSniperMag = "LoBo_SR25Mag";

        // town flagpole texture (native TownFlags): without this key the
        // engine's side-WEST default is the US flag - wrong faction here
        flag = "\flags\israel.jpg";
    };

    class EgyptFrontier // resistance descriptor - side EAST
    {
        side = "EAST";
        // Same story as EgyptArmy below: side=0 all the way up from
        // LoBo_Egypt_General1 : SoldierEB, no re-sided Egyptian roster exists,
        // so no twin. Authored explicitly rather than left absent - every other
        // war faction in this file states it, and TwinOnSide/TwinOffSide read
        // the same "" either way.
        sideTwin = "";
        tiers[] = {"LoBo_Egypt_FrtCrp"};
        tierThresholds[] = {};
        // role variants: MG/AT are native Frontier Corps classes; the corps
        // fields no medic or marksman of its own, so those borrow same-side
        // Egyptian army kin (LoBoEgypt.pbo) - an army medic and an SVD
        // marksman attached to a frontier cell is Sinai-plausible
        tiersMG[]     = {"LoBo_Egypt_FrtCrpMG"};
        tiersAT[]     = {"LoBo_Egypt_FrtCrpAT"};
        tiersMedic[]  = {"LoBo_Egypt_infantryMedic"};
        tiersSniper[] = {"LoBo_Egypt_Grdsnip"};
        // resistance motor pool (phase-2; consumed by hold/QRF only if this
        // faction ever occupies): DShK Toyota technical (crewed by Frontier
        // Corps men in the config) -> BTR-60PB (carries a full 12-man QRF)
        // -> BMP-1 ('73-correct) -> T-55A
        //
        // ALL FOUR rungs are LoBoammo-armed (loBo_DShK, loBo_KPVT, LoBo_PKCoax,
        // LoBo_73gun, LoBo_D100a), so this ladder needs the @lobofixup patched
        // pbos in strict/autotest lanes - the same malformed-tracerColor abort
        // EgyptArmy and IDF call out. This descriptor is the WORST case for it,
        // not the best: EgyptArmy's WL1 Ural-375 is unarmed and boots clean,
        // and EgyptFrontier is Guerrilla.Sinai's defaultResistance, so it
        // loads on any direct launch of that template.
        vehicles[] = {"LoBo_EToymg", "LoBo_BTR60", "LoBo_BMP_EGY", "LoBo_T55_EGY_1"};
        vehicleThresholds[] = {3, 6, 8};
        officer = "LoBo_Egypt_FrtCrpo";

        holdClass = "LoBo_Egypt_FrtCrp";
        holdCount = 3;
        recruitFighter = "LoBo_Egypt_FrtCrp";
        recruitSpecialist = "LoBo_Egypt_FrtCrpMG";
        companionClass = "LoBo_Egypt_FrtCrp";

        // ---- character outfit family (issue #25): the LoBoTer Terror
        // irregulars - PLO-style fighters in civilian dress. The *E twins
        // (side=0) match this faction's EAST side, so a kill on one books as
        // an EAST kill in the engine's stat line (AICenterStats reads the
        // config class side), and the mod's own R/E/W twin mechanism is used
        // as designed. NOT the *HD variants (hidden high-dispersion) and NOT
        // LoBo_Civ_* (truly unarmed; ambience-only). Verified in
        // tmp/class-survey/lobo-cfg/LoBoTer.cpp. Terror classes carry
        // Init/fired EventHandlers exec'ing \LoBoTer\scripts\* - covered by
        // the lobo-lane spawn smoke test (issue #25 M2.4).
        playerClassWarrior = "LoBo_Egypt_FrtCrp";
        playerClassCiv = "LoBo_Terror_01E";
        recruitFighterCiv = "LoBo_Terror_02E";
        recruitSpecialistCiv = "LoBo_Terror_MG2E";
        companionClassCiv = "LoBo_Terror_01E";
        holdClassCiv = "LoBo_Terror_02E";
        // single-rung civilian-outfit ladder (tiers[] is single-rung too)
        civTier[] = {"LoBo_Terror_01E"};

        // Frontier Corps kit (verified from LoBoEgypt config: LoBoAK47 rifle
        // firing JAM 7.62x39 mags; RPD MG; RPG-7; SVD for the marksman role).
        baseWeapon = "LoBoAK47";
        baseMagazine = "JAM_E762_30mag";
        lootRiflemanWeapon = "LoBoAK47";
        lootRiflemanMag = "JAM_E762_30mag";
        lootMedicWeapon = "LoBoAK47";
        lootMedicMag = "JAM_E762_30mag";
        lootMGWeapon = "JAM_RPD";
        lootMGMag = "JAM_E762M_100mag";
        lootATWeapon = "JAM_RPG7Launcher";
        lootATMag = "JAM_RPG7ALLRocket";
        lootSniperWeapon = "LoBo_SVD";
        lootSniperMag = "JAM_E762_10mag";

        // town flagpole texture: side-EAST default would be the USSR flag
        flag = "\flags\egypt.jpg";
    };

    class EgyptArmy // side EAST - the Egyptian regular army (LoBoEgypt.pbo)
    {
        side = "EAST";
        // No re-sided Egyptian roster exists: every LoBoEgypt class inherits
        // side=0 from LoBo_Egypt_General1 : SoldierEB. No twin -> no rebase.
        sideTwin = "";

        // WL<3 line infantry -> WL3-4 Paratroopers -> WL5+ Al-Quwaat
        // Al-Khaasat commandos. Unlike the Frontier Corps, each family here
        // carries every role natively, so nothing is borrowed except the
        // tier-1 marksman (see tiersSniper).
        tiers[] = {"LoBo_Egypt_infantry", "LoBo_Egypt_Para", "LoBo_Egypt_SF"};
        tierThresholds[] = {3, 5};
        // MG escalates RPD -> PK -> PKM. AT stays RPG-7 across the whole
        // ladder deliberately: the AT-3 (*AT) classes exist and their
        // Lobo_At3StaticWpn resolves (LoBo_statwp.pbo), but a wire-guided
        // Sagger is a crew-served ATGM and a poor fit for a rifle-squad slot.
        // Tier-1 sniper borrows the Republican Guard's SVD marksman - the
        // army line infantry fields no sniper class of its own.
        tiersMG[]     = {"LoBo_Egypt_infantryMG2", "LoBo_Egypt_ParaMG3", "LoBo_Egypt_SFMG"};
        tiersAT[]     = {"LoBo_Egypt_infantryRPG", "LoBo_Egypt_ParaRPG", "LoBo_Egypt_SFRPG"};
        tiersMedic[]  = {"LoBo_Egypt_infantryMedic", "LoBo_Egypt_ParaMedic", "LoBo_Egypt_SFMedic"};
        tiersSniper[] = {"LoBo_Egypt_Grdsnip", "LoBo_Egypt_Parasnip", "LoBo_Egypt_SFsnip2"};
        // Motor pool: the army moves by truck where the Frontier Corps moves
        // by technical. Ural-375 (period-correct, unarmed, zero LoBoammo
        // dependence) -> BTR-60PB (12 cargo, carries a whole QRF) -> BMP-1
        // ('73-correct) -> T-55A. The last three need the @lobofixup patched
        // pbos in strict/autotest lanes (their guns live in LoBoammo.pbo).
        vehicles[] = {"LoBo_Ural_egy", "LoBo_BTR60", "LoBo_BMP_EGY", "LoBo_T55_EGY_1"};
        vehicleThresholds[] = {3, 6, 8};
        officer = "LoBo_Egypt_infantryO";   // Mulazim Awwal (Lt)

        holdClass = "LoBo_Egypt_infantry";
        holdCount = 3;
        recruitFighter = "LoBo_Egypt_infantry";
        recruitSpecialist = "LoBo_Egypt_infantryMG2";   // RPD
        companionClass = "LoBo_Egypt_infantry";

        // Army kit: LoBoAK47 firing JAM 7.62x39; RPD MG; RPG-7; SVD marksman.
        // JAM_RPD feeds JAM_E762M_100mag (JAM_Magazines.cpp) - the _200mag is
        // the PKM belt, not this one.
        baseWeapon = "LoBoAK47";
        baseMagazine = "JAM_E762_30mag";
        lootRiflemanWeapon = "LoBoAK47";
        lootRiflemanMag = "JAM_E762_30mag";
        lootMedicWeapon = "LoBoAK47";
        lootMedicMag = "JAM_E762_30mag";
        lootMGWeapon = "JAM_RPD";
        lootMGMag = "JAM_E762M_100mag";
        lootATWeapon = "JAM_RPG7Launcher";
        lootATMag = "JAM_RPG7ALLRocket";
        lootSniperWeapon = "LoBo_SVD";
        lootSniperMag = "JAM_E762_10mag";

        // town flagpole texture: side-EAST default would be the USSR flag
        flag = "\flags\egypt.jpg";
    };

    class Syria // side EAST - small roster, two camo families (LoBoSyria.pbo)
    {
        side = "EAST";
        // No re-sided Syrian roster: LoBo_WL_Infantry_SyriaBase pins side=0
        // and every descendant inherits it. No twin -> no rebase.
        sideTwin = "";

        // NEVER spawn LoBo_WL_Infantry_SyriaBase - it is scope=1 (protected)
        // and abstract (: vanilla Soldier). Both families below are scope=2.
        //
        // The Syria roster is two camo families with IDENTICAL role sets and
        // no elite branch, so this ladder is COSMETIC: Lizard reads as the
        // Sinai-plausible baseline, Woodland as later mainland reinforcement.
        // Flagged rather than faked - the real escalation is the role arrays.
        tiers[] = {"LoBo_Liz_Infantry_Syria", "LoBo_WL_Infantry_Syria"};
        tierThresholds[] = {5};
        // MG escalates RPD (MG2) -> PK (MG1) - that is the real escalation.
        // AT stays RPG-7 (the *AT3 Saggers exist but are crew-served; see
        // EgyptArmy's rationale).
        tiersMG[]     = {"LoBo_Liz_Infantry_SyriaMG2", "LoBo_WL_Infantry_SyriaMG1"};
        tiersAT[]     = {"LoBo_Liz_Infantry_SyriaRPG", "LoBo_WL_Infantry_SyriaRPG"};
        tiersMedic[]  = {"LoBo_Liz_Infantry_SyriaMedic", "LoBo_WL_Infantry_SyriaMedic"};
        // NO Syrian sniper class exists in EITHER camo family, and there is no
        // Syrian kin to borrow from. "" is the schema's role-absent marker
        // (ZoneRegistry gates on GetLength()>0) and the squad composer
        // substitutes the tier rifleman. Deliberately NOT an Egyptian
        // Republican Guard sniper - a Syrian cell fielding one is not plausible.
        tiersSniper[] = {"", ""};
        // Motor pool (LoBoSyrAPC / LoBo_BTR60 / LoBo_T72): no Syrian soft-skin
        // exists, so the BTR-60PB is the WL1 ride. All four are LoBoammo-armed
        // -> @lobofixup patched pbos needed in strict/autotest lanes.
        vehicles[] = {"LoBo_BTR60_Syria", "LoBo_bmp_syr", "LoBo_bmp2_syr", "LoBo_T72M_Syria"};
        vehicleThresholds[] = {3, 6, 8};
        officer = "LoBo_Liz_Infantry_SyriaO2";   // Mulazim Awwal (Lt); O1 = Sgt

        holdClass = "LoBo_Liz_Infantry_Syria";
        holdCount = 3;
        recruitFighter = "LoBo_Liz_Infantry_Syria";
        recruitSpecialist = "LoBo_Liz_Infantry_SyriaMG2";   // RPD
        companionClass = "LoBo_Liz_Infantry_Syria";

        baseWeapon = "LoBoAK47";
        baseMagazine = "JAM_E762_30mag";
        lootRiflemanWeapon = "LoBoAK47";
        lootRiflemanMag = "JAM_E762_30mag";
        lootMedicWeapon = "LoBoAK47";
        lootMedicMag = "JAM_E762_30mag";
        lootMGWeapon = "JAM_RPD";
        lootMGMag = "JAM_E762M_100mag";
        lootATWeapon = "JAM_RPG7Launcher";
        lootATMag = "JAM_RPG7ALLRocket";
        // No Syrian sniper CLASS exists, so no garrison body ever drops this -
        // it is here only so the loot/unlock table has a marksman rifle.
        lootSniperWeapon = "LoBo_SVD";
        lootSniperMag = "JAM_E762_10mag";

        flag = "\flags\syria.jpg";
    };

    class Jordan // side GUER - the mod's only regular army on the resistance side
    {
        side = "GUER";
        // No EAST/WEST Jordanian twin exists, so Jordan is the faction that
        // CANNOT rebase on a GUER/GUER collision - rebase the other one.
        sideTwin = "";

        // Every spawnable LoBoJordan class overrides side=2 even though the
        // root is LoBo_Jordan_General1 : SoldierEB - Jordan really is GUER.
        // NEVER spawn LoBo_Jordan_General1 itself: scope=0 (private). Its
        // children are the public anchors.
        //
        // WL<3 DESERT infantry (the Sinai-appropriate family, not the
        // woodland LoBo_inf_jor*) -> WL3-4 Paratroopers -> WL5+ Unit 71 SOF.
        //
        // "Desert vs woodland" is a TEXTURE distinction, not a separate
        // lineage: LoBo_inf_jor_D inherits straight from LoBo_inf_jor and
        // declares no side of its own, so this whole faction's GUER resolution
        // rests on the WOODLAND rifleman's side=2. Re-siding LoBo_inf_jor
        // would silently take the desert tier with it.
        tiers[] = {"LoBo_inf_jor_D", "LoBo_para_jor", "LoBo_commando_jor"};
        tierThresholds[] = {3, 5};
        // Unit 71 fields no MG and no sniper (its roster is Radio/Medic/
        // grenadier/demo/law/CT only), so tier 3 borrows the para M240 team
        // and the corps-level SR25 sniper - both side GUER, same army, which
        // is exactly how a SOF attachment works.
        tiersMG[]     = {"LoBo_inf_jor_D_mg", "LoBo_para_jor_mg", "LoBo_para_jor_mg"};
        // LAW-80 at every tier. LoBo_para_jor_AT exists but carries
        // LoBo_Drag_Launcher - a wire-guided Dragon, i.e. a crew-served ATGM in
        // a rifle-squad slot, which is exactly what EgyptArmy and Syria reject
        // the AT-3 for below. LoBo_para_jor_law is the para that matches its
        // neighbours (tier 1 and tier 3 are both LAW-80) and the declared
        // lootATWeapon/lootATMag pair.
        tiersAT[]     = {"LoBo_inf_jor_D_LAW", "LoBo_para_jor_law", "LoBo_commando_jor_law"};
        tiersMedic[]  = {"LoBo_inf_jor_D_Medic", "LoBo_para_jor_Medic", "LoBo_commando_jor_Medic"};
        tiersSniper[] = {"LoBo_inf_jor_D_snp", "LoBo_para_jor_snp", "LoBo_sniper_jor"};
        // Motor pool (LoBoJorAPC / LoBoJorCar): M113A2 -> Ratel 20 IFV
        // (1970s-ok) -> BMP-2 (1980s) -> M60A3. LoBo_Challenger1_Jor_W skipped
        // (1999+, out of period); LoBo_Rakon skipped (motorcycle, 1 seat,
        // AI-fragile). All four are LoBoammo-armed -> @lobofixup needed in
        // strict/autotest lanes.
        vehicles[] = {"LoBo_m113_jor", "LoBo_Ratel20_jor", "LoBo_BMP2_Jor", "LoBo_M60A3_jor"};
        vehicleThresholds[] = {3, 6, 8};
        officer = "LoBo_inf_jor_D_off";

        holdClass = "LoBo_inf_jor_D";
        holdCount = 3;
        recruitFighter = "LoBo_inf_jor_D";
        recruitSpecialist = "LoBo_inf_jor_D_mg";   // M240
        companionClass = "LoBo_inf_jor_D";

        // Jordanian kit (LoBoWeapons.cpp / LoBoWeapat.cpp): M16 + LoBoM16Mag;
        // M240 + LoBoM240Mag; LAW-80 launcher + LoBo_LAW80 rocket; SR25 +
        // LoBo_SR25Mag.
        //
        // The loot schema is single-valued while the tier ladder is not, so
        // these match the WL<3 desert tier and not the two above it: the desert
        // rifleman and medic carry LoBoM16, but LoBo_para_jor / _Medic and
        // LoBo_commando_jor / _Medic all carry LoBo_M4A1. Harmless at runtime
        // (LoBoM16 is a real, public, scope=2 weapon and a valid drop at any
        // tier) but do not read these as "the class spawned above".
        baseWeapon = "LoBoM16";
        baseMagazine = "LoBoM16Mag";
        lootRiflemanWeapon = "LoBoM16";
        lootRiflemanMag = "LoBoM16Mag";
        lootMedicWeapon = "LoBoM16";
        lootMedicMag = "LoBoM16Mag";
        lootMGWeapon = "LoBoM240";
        lootMGMag = "LoBoM240Mag";
        lootATWeapon = "LoBo_LAW80Launcher";
        lootATMag = "LoBo_LAW80";
        lootSniperWeapon = "LoBo_SR25";
        lootSniperMag = "LoBo_SR25Mag";

        flag = "\flags\jordan.jpg";
    };

    class Hizballah // side EAST - the richest irregular roster in the mod
    {
        side = "EAST";
        // No re-sided Hizballah roster: LoBo_Hizballah_base is side=0 and
        // every descendant inherits it. No twin -> no rebase.
        sideTwin = "";

        // NEVER spawn LoBo_Hizballah_base - scope=0 (private).
        // LoBo_HizballahRifle1 is the public anchor of the family. Never emit
        // any *HD class either: their displayName starts with "-" (hidden
        // high-dispersion variants).
        //
        // WL<3 AK-armed cell -> WL3-5 M16-armed line fighters -> WL6+ the
        // recon branch (LoBoHizballah.pbo ships it as its own role mirror).
        tiers[] = {"LoBo_HizballahRifle4", "LoBo_HizballahRifle1", "LoBo_HizballahRecon1"};
        tierThresholds[] = {3, 6};
        // MG: the ubiquitous PKM first, captured/imported Minimis later; the
        // recon branch has no PKM gunner, so tier 3 keeps the Minimi and stays
        // visually recon-consistent.
        //
        // AT escalates RPG-7 (ICPRPG7) -> RPG-7VR tandem (ICPRPG7v) -> RPG-29
        // (ICPRPG29, the recon branch's launcher). ICP launchers, ICPrpg7.pbo -
        // present in @LoBo\addons. The RPG-29 is 1989: at the edge of period
        // for a late-80s Lebanon, in period on the same line Jordan's 1980s
        // BMP-2 sits on (only 1999+ is out; see the Challenger note there),
        // and a real step up from the VR either way.
        //
        // MIND THE SUFFIXES - they do NOT mean the same thing on the two
        // families, and reading across them is how you ship the wrong launcher:
        //   LoBo_Hizballah_RPG  / _RPG2 / _RPG3  = RPG-7 / RPG-29  / RPG-7VR
        //   LoBo_HizballahReconRPG / RPG2 / RPG3 = RPG-7 / RPG-29  / RPG-7VR
        // so tier 2's _RPG3 is the VR and tier 3's ReconRPG2 is the RPG-29.
        // ReconRPG3 (recon RPG-7VR) exists and is NOT what this ladder wants.
        // Only ONE medic class exists faction-wide and the recon branch has no
        // twin, so it repeats at every tier - a repeat, not a substitution.
        // No recon sniper exists either; tier 3 reuses the line M24.
        tiersMG[]     = {"LoBo_HizballahMG1", "LoBo_HizballahMG2", "LoBo_HizballahReconLMG"};
        tiersAT[]     = {"LoBo_Hizballah_RPG", "LoBo_Hizballah_RPG3", "LoBo_HizballahReconRPG2"};
        tiersMedic[]  = {"LoBo_HizballahMedic", "LoBo_HizballahMedic", "LoBo_HizballahMedic"};
        tiersSniper[] = {"LoBo_HizballahSniper2", "LoBo_HizballahSniper1", "LoBo_HizballahSniper1"};
        // Hizballah owns NO APC and NO tank anywhere in the mod, so this
        // ladder escalates GUN TRUCKS, not armour: Ural-375 troop truck
        // (unarmed, zero LoBoammo dependence) -> M2 jeep -> ZU-23-2 Ural ->
        // M40A1 106 mm recoilless jeep (the faction's only real AT escort).
        // Flagged rather than borrowing an Egyptian/Syrian vehicle whose
        // displayName literally says "Egyptian"/"Syrian". The last three are
        // LoBoammo-armed -> @lobofixup needed in strict/autotest lanes.
        vehicles[] = {"LoBo_Ural_Hizballah", "LoBo_JeepMG_Hizballah", "LoBo_ural_Hizbal_zsu23", "LoBo_JeepAT_Hizballah"};
        vehicleThresholds[] = {3, 6, 8};
        officer = "LoBo_HizballahLeader";

        holdClass = "LoBo_HizballahRifle4";
        holdCount = 3;
        recruitFighter = "LoBo_HizballahRifle4";
        recruitSpecialist = "LoBo_HizballahMG1";   // PKM
        companionClass = "LoBo_HizballahRifle4";

        // ---- character outfit family (issue #25): the LoBoTer Terror
        // irregulars in civilian dress, *E twins (side=0) matching this
        // faction's EAST side so kill stats book correctly (AICenterStats
        // reads the config class side). Not the *HD hidden variants, not the
        // unarmed LoBo_Civ_* ambience classes. Same picks as the shipped
        // Guerrilla.Sinai EgyptFrontier block - the Terror roster is
        // island-independent (LoBoTer.pbo, requiredAddons[]={}).
        playerClassWarrior = "LoBo_HizballahRifle4";
        playerClassCiv = "LoBo_Terror_01E";
        recruitFighterCiv = "LoBo_Terror_02E";
        recruitSpecialistCiv = "LoBo_Terror_MG2E";
        companionClassCiv = "LoBo_Terror_01E";
        holdClassCiv = "LoBo_Terror_02E";
        // civilian-outfit ladder: riflemen at every war level (the Terror
        // family has no tier escalation; three rungs would repeat bodies)
        civTier[] = {"LoBo_Terror_01E"};

        // LoBo_PKM : LoBo_PK -> magazines[]={JAM_E762M_200mag,...} (NOT the
        // _100mag, which is the RPD belt). ICPRPG7 : LAWLauncher ->
        // magazines[]={ICPRPG7Mag, RPGLauncher}.
        baseWeapon = "LoBoAK47";
        baseMagazine = "JAM_E762_30mag";
        lootRiflemanWeapon = "LoBoAK47";
        lootRiflemanMag = "JAM_E762_30mag";
        lootMedicWeapon = "LoBoAK47";          // LoBo_HizballahMedic carries LoBoAK47
        lootMedicMag = "JAM_E762_30mag";
        lootMGWeapon = "LoBo_PKM";             // LoBo_HizballahMG1 carries LoBo_PKM
        lootMGMag = "JAM_E762M_200mag";
        lootATWeapon = "ICPRPG7";
        lootATMag = "ICPRPG7Mag";
        lootSniperWeapon = "LoBo_SVD";
        lootSniperMag = "JAM_E762_10mag";

        // Hizballah is Lebanese; there is no "hizballah" texture in Classic's
        // Flags.pbo, so lebanon.jpg is the correct stand-in.
        flag = "\flags\lebanon.jpg";
    };

    class PLO // side GUER - Palestinian irregulars (LoBoTer.pbo, the *R twins)
    {
        side = "GUER";
        // LoBoTer is the mod's OWN faction-swap mechanism: nearly every unit
        // ships as *R (side 2) / *E (side 0) / *W (side 1). PLO_East below is
        // the EAST twin of this exact roster, and this pair is the ONLY twin
        // pair in the dataset - the only reason a GUER PLO can be picked as
        // the resistance on an EAST-player island. On an occupier/resistance
        // side collision, the pending rebase pass reads this key and swaps to
        // PLO_East (config-clean, no re-sided classes invented).
        sideTwin = "PLO_East";

        // WL<3 SKS-armed fedayeen -> WL3-5 AK-47 -> WL6+ AKM.
        tiers[] = {"LoBo_Terror_SKSR", "LoBo_Terror_01R", "LoBo_Terror_02R"};
        tierThresholds[] = {3, 6};
        // MG escalates RPD -> PKM. Sniper escalates Mosin -> SVD. AT is RPG-7
        // at EVERY tier: LoBo_Terror_RPGR is the ONLY AT class in the roster,
        // so AT never escalates - that is the roster, not an oversight.
        // There is NO medic class anywhere in LoBoTer, on any side - "" is the
        // schema's role-absent marker and the composer falls back to the tier
        // rifleman. Not substituted: there is no plausible PLO medic to borrow.
        tiersMG[]     = {"LoBo_Terror_MG2R", "LoBo_Terror_MGR", "LoBo_Terror_MGR"};
        tiersAT[]     = {"LoBo_Terror_RPGR", "LoBo_Terror_RPGR", "LoBo_Terror_RPGR"};
        tiersMedic[]  = {"", "", ""};
        tiersSniper[] = {"LoBo_Terror_mosinR", "LoBo_Terror_SVDR", "LoBo_Terror_SVDR"};
        // Only TWO usable QRF vehicles exist per side in LoBoTer (the towed
        // LoBo_ZU23_terror / LoBo_ZPU4_terror are maxSpeed=1 - excluded). Both
        // are LoBoTer-internal: the Hilux is unarmed and the MG Hilux's
        // loBo_DShK_Ter weapon+ammo are self-contained with a well-formed
        // tracerColor, so this is the ONLY vehicle ladder in the file with
        // ZERO @lobofixup dependence. Flagged rather than borrowing a vehicle
        // whose displayName says "Egyptian".
        vehicles[] = {"LoBo_Ter_Toy_R", "LoBo_Ter_ToyMG_R"};
        vehicleThresholds[] = {4};
        // No officer class exists in LoBoTer: 03R is a cosmetic AKM-CIA
        // rifleman variant standing in as squad leader, so the QRF leader has
        // no leadership kit.
        officer = "LoBo_Terror_03R";

        holdClass = "LoBo_Terror_01R";
        holdCount = 3;
        recruitFighter = "LoBo_Terror_01R";
        recruitSpecialist = "LoBo_Terror_MG2R";   // RPD
        companionClass = "LoBo_Terror_01R";

        // JAM_PKM feeds JAM_E762M_200mag (JAM_Magazines.cpp) - NOT the
        // _100mag, which belongs to JAM_RPD. LoBo_Terror_MGR carries JAM_PKM.
        baseWeapon = "JAM_AKM";
        baseMagazine = "JAM_E762_30mag";
        lootRiflemanWeapon = "JAM_AKM";
        lootRiflemanMag = "JAM_E762_30mag";
        lootMedicWeapon = "JAM_AKM";          // no medic class - rifleman kit
        lootMedicMag = "JAM_E762_30mag";
        lootMGWeapon = "JAM_PKM";
        lootMGMag = "JAM_E762M_200mag";
        lootATWeapon = "JAM_RPG7Launcher";
        lootATMag = "JAM_RPG7ALLRocket";
        lootSniperWeapon = "JAM_SVD";
        lootSniperMag = "JAM_E762_10mag";

        // no "plo" texture exists; palestine.jpg is in Classic's Flags.pbo
        flag = "\flags\palestine.jpg";
    };

    class PLO_East // side EAST twin of PLO - the *E LoBoTer roster
    {
        side = "EAST";
        // Rebase target for PLO on a GUER/GUER collision (e.g. Jordan
        // occupier). A *W (WEST) twin set also exists in LoBoTer but is
        // unreachable with the current factions - IDF is the only WEST
        // faction, and IDF-occupier vs PLO-resistance already resolves.
        sideTwin = "PLO";

        // Byte-for-byte the PLO ladder with *E classes substituted for *R;
        // every rationale above applies unchanged.
        tiers[] = {"LoBo_Terror_SKSE", "LoBo_Terror_01E", "LoBo_Terror_02E"};
        tierThresholds[] = {3, 6};
        tiersMG[]     = {"LoBo_Terror_MG2E", "LoBo_Terror_MGE", "LoBo_Terror_MGE"};
        tiersAT[]     = {"LoBo_Terror_RPGE", "LoBo_Terror_RPGE", "LoBo_Terror_RPGE"};
        tiersMedic[]  = {"", "", ""};   // no medic class in LoBoTer, any side
        tiersSniper[] = {"LoBo_Terror_mosinE", "LoBo_Terror_SVDE", "LoBo_Terror_SVDE"};
        // Zero @lobofixup dependence (see PLO).
        vehicles[] = {"LoBo_Ter_Toy_E", "LoBo_Ter_ToyMG_E"};
        vehicleThresholds[] = {4};
        officer = "LoBo_Terror_03E";   // no officer class - cosmetic stand-in

        holdClass = "LoBo_Terror_01E";
        holdCount = 3;
        recruitFighter = "LoBo_Terror_01E";
        recruitSpecialist = "LoBo_Terror_MG2E";   // RPD
        companionClass = "LoBo_Terror_01E";

        // Weapon layer is side-agnostic - identical to PLO.
        baseWeapon = "JAM_AKM";
        baseMagazine = "JAM_E762_30mag";
        lootRiflemanWeapon = "JAM_AKM";
        lootRiflemanMag = "JAM_E762_30mag";
        lootMedicWeapon = "JAM_AKM";
        lootMedicMag = "JAM_E762_30mag";
        lootMGWeapon = "JAM_PKM";
        lootMGMag = "JAM_E762M_200mag";
        lootATWeapon = "JAM_RPG7Launcher";
        lootATMag = "JAM_RPG7ALLRocket";
        lootSniperWeapon = "JAM_SVD";
        lootSniperMag = "JAM_E762_10mag";

        flag = "\flags\palestine.jpg";
    };
};
