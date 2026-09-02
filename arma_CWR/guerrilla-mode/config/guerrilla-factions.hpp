// UD's GLOBAL Guerrilla faction library - the vanilla (CWA 1.99 core) rosters.
//
// WHERE THIS RUNS: install-missions.ps1 copies this file to
// <GameDir>\bin\guerrilla-factions.hpp and makes sure <GameDir>\bin\
// config-extra.cpp carries `#include "guerrilla-factions.hpp"`. The engine
// merges bin\config-extra.cpp into the global config LAST (ParseConfig,
// Asset/Addon/ConfigParsers.cpp), and the text preprocessor resolves an
// #include against the INCLUDING file's directory, which is why the two files
// sit side by side in bin\.
//
// WHY IT IS GLOBAL AND NOT PER-ISLAND: the engine builds the faction table as
// the UNION of this global CfgGuerrillaFactions and the island template's own
// block (Game/Guerrilla/FactionSources.*, issue #54 A1), the island winning on
// a class-name collision. Blocks that describe a ROSTER rather than an ISLAND
// - which is every war faction here, since a US or Soviet order of battle is
// the same order of battle on Malden as on Nogova - therefore belong in one
// place. Before A1 each template carried its own copy, so a faction pack that
// shipped its classes in an addon config appeared on exactly zero islands.
//
// CIV IS ISLAND-OWNED AND MUST NEVER APPEAR HERE. The civilian descriptor
// names the population MODELS a given island's data set actually ships
// (Civilian/Civilian2/Civilian3 on the stock islands, LoBo_Civ_* on the @LoBo
// worlds) plus that island's ambient-traffic hulls. A global CIV would be
// wrong on every island whose package does not carry those classes, and the
// union rule cannot help: an island that wants a different population has to
// redeclare the WHOLE class anyway. So each template's description.ext keeps
// exactly one faction class, CIV.
//
// AN ISLAND OVERRIDE is the escape hatch: an island template that redeclares
// a class name from this file replaces it WHOLE (no per-key merge). Use it
// only with a comment saying what differs and why - see Guerrilla.Demo's GUER.
//
// A MOD SHIPS ITS OWN by putting a `class CfgGuerrillaFactions` in its
// addon pbo config, in its own <mod>\bin\config.cpp, or in another
// bin\config-extra.cpp include. All of those land in Pars, so all of them
// join this table.
//
// KEY NAMING SCHEME (script-facing, read via gmFactionValue /
// gmFactionTierClass / gmFactionVehicle - see guerrilla-mode/ARCHITECTURE.md
// "Faction keys"). Every descriptor anywhere - here, an island override, a
// mod's own pack - follows it:
//   side                         "WEST"/"EAST"/"GUER"/"CIV"; defaults to the
//       class name, which is why `class CIV` needs no side key to be filtered
//       off the new-game cyclers
//   sideTwin                     the same roster re-sided, for the registry's
//       occupier/resistance side-collision rebase ("" = no twin)
//   tiers[] + tierThresholds[]   garrison/QRF infantry per war level (native + qrf.sqs)
//   tiers{MG,AT,Medic,Sniper}[]  per-tier role variants, parallel to tiers[];
//       "" = the tier fields no such role
//   vehicles[] + vehicleThresholds[]  QRF escort per war level (qrf.sqs)
//   officer                      squad leader class (native garrison + qrf.sqs)
//   holdClass / holdCount        capture hold-garrison class and size (capture.sqs)
//   recruitFighter / recruitSpecialist   Camp recruit classes (recruit.sqs)
//   companionClass               named-companion body class (companions.sqs)
//   playerClass{Warrior,Civ}     character-select outfit bodies (issue #25;
//       the engine substitutes the player's mission.sqm class at load when
//       gmSelOutfit=CIVILIAN - WorldInit.cpp/OutfitSelect.cpp)
//   {recruitFighter,recruitSpecialist,companionClass,holdClass}Civ
//       civilian-outfit twins of the keys above (scripts pick them when
//       GM_OUTFIT_CIV; absent/unresolvable = warrior bodies)
//   civTier[]                    civilian-outfit AI ladder (gmFactionCivTier;
//       issue #16's guard/militia rung; same tierThresholds[] gates)
//   baseWeapon / baseMagazine    always-available fallback kit (loot.sqs)
//   loot<Role>Weapon / loot<Role>Mag     role loadouts, Role in
//       {Rifleman, Medic, MG, AT, Sniper} (loot.sqs unlock table)
//   flag                         town flagpole texture (native TownFlags)
//   civClassCount + civClass<N>  CIV only: the population bodies, numbered
//       keys and not an array because gmFactionValue skips array entries
//   civVehicles[]                CIV only: ambient road-traffic hulls
//
// Every classname below was verified against Bin\CONFIG.BIN (PoseidonTools
// config search/dump) of the full CWA 1.99 dataset. NOTE the OFP magazine
// convention: simple weapons ARE their own magazine class (CfgWeapons AK47
// has scopeMagazine=2), so baseMagazine/loot*Mag reuse the weapon classname -
// there is no "AK47Mag" class in this dataset.
class CfgGuerrillaFactions
{
    class WEST // US force - selectable as occupier OR resistance
    {
        side = "WEST";
        // Mirrors EAST's ladder rung-for-rung: WL<3 riflemen -> WL3-4
        // grenadiers -> WL5+ crack troops. Deliberately SYMMETRIC with EAST:
        // either side can now be the occupier, so an asymmetric elite step
        // would silently turn the occupier pick into a difficulty setting.
        // SoldierWCrew (M4 + NVGoggles) is the structural twin of EAST's
        // SoldierECrew (AK74SU + NVGoggles). classic-infantry.md suggests
        // SoldierWSaboteurDay as the elite step; rejected - tiers[i] is the
        // squad's RIFLEMAN class and its only rifle is the ~200 m HK/MP5, so
        // a WL5+ garrison would shoot worse than the WL1 M16 conscripts it
        // replaces. A true elite step must change BOTH sides together.
        tiers[] = {"SoldierWB", "SoldierWG", "SoldierWCrew"};
        tierThresholds[] = {3, 5};
        // per-tier role variants (plan 15; all verified in the 1.99 config):
        // the native squad composer fills MG/AT/medic/sniper slots from
        // these; "" = the tier fields no such role. The sniper appears only
        // with the elite tier - identical policy to EAST.
        tiersMG[]     = {"SoldierWMG", "SoldierWMG", "SoldierWMG"};
        tiersAT[]     = {"SoldierWLAW", "SoldierWLAW", "SoldierWLAW"};
        tiersMedic[]  = {"SoldierWMedic", "SoldierWMedic", "SoldierWMedic"};
        tiersSniper[] = {"", "", "SoldierWSniper"};
        // QRF escort ladder, mirroring EAST rung-for-rung: Jeep patrol (3
        // cargo, the twin of EAST's UAZ) -> 5t troop truck (12, a full
        // group) -> M113 APC (8) -> M60 escort (0 cargo, escort-only)
        vehicles[] = {"Jeep", "Truck5t", "M113", "M60"};
        vehicleThresholds[] = {3, 6, 8};
        officer = "OfficerW";

        // ---- resistance-capable key set (any faction may now be the
        // resistance roster; on a GUER-player island it spawns onto the GUER
        // side): post-capture hold squads, recruitment, the player's loot ----
        holdClass = "SoldierWB";
        holdCount = 3;
        recruitFighter = "SoldierWB";
        recruitSpecialist = "SoldierWMG";
        companionClass = "SoldierWB";

        baseWeapon = "M16";
        baseMagazine = "M16";
        lootRiflemanWeapon = "M16";
        lootRiflemanMag = "M16";
        lootMedicWeapon = "M16";
        lootMedicMag = "M16";
        // NOTE: "M60" is used with TWO meanings in this file, deliberately.
        // Here it is CfgWeapons M60 (the 7.62 MG, scopeWeapon=2/scopeMagazine=2);
        // in vehicles[] above it is CfgVehicles M60 (the tank, side=1 scope=2).
        // Safe: ResolveFactionClasses probes bank-scoped - it resolves
        // vehicles[] against kVeh="CfgVehicles" and loot*/base* against
        // kWpn="CfgWeapons", so neither reading can pick up the other's class.
        lootMGWeapon = "M60";
        lootMGMag = "M60";
        lootATWeapon = "LAWLauncher";
        lootATMag = "LAWLauncher";
        lootSniperWeapon = "M21";
        lootSniperMag = "M21";

        // town flagpole texture (native TownFlags); matches the engine's
        // side-WEST default - stated for the data-file contract
        flag = "\flags\usa.jpg";
    };
    class EAST // occupier descriptor; also selectable as the RESISTANCE roster
    {
        side = "EAST";
        // WL<3 conscripts -> WL3-4 grenadiers -> WL5+ crack troops
        tiers[] = {"SoldierEB", "SoldierEG", "SoldierECrew"};
        tierThresholds[] = {3, 5};
        // per-tier role variants (plan 15; all verified in the 1.99 config):
        // the native squad composer fills MG/AT/medic/sniper slots from
        // these; "" = the tier fields no such role. The sniper appears only
        // with the elite tier - regulars do not embed marksmen in a
        // checkpoint garrison.
        tiersMG[]     = {"SoldierEMG", "SoldierEMG", "SoldierEMG"};
        tiersAT[]     = {"SoldierELAW", "SoldierELAW", "SoldierELAW"};
        tiersMedic[]  = {"SoldierEMedic", "SoldierEMedic", "SoldierEMedic"};
        tiersSniper[] = {"", "", "SoldierESniper"};
        // QRF escort ladder (full vehicleThresholds[] form): UAZ patrol ->
        // Ural troop truck -> BMP IFV -> T72 escort
        vehicles[] = {"UAZ", "Ural", "BMP", "T72"};
        vehicleThresholds[] = {3, 6, 8};
        officer = "OfficerE";

        // ---- resistance-capable key set (EAST may now be the resistance
        // roster; on a GUER-player island it spawns onto the GUER side) ----
        holdClass = "SoldierEB";
        holdCount = 3;
        recruitFighter = "SoldierEB";
        recruitSpecialist = "SoldierEMG";
        companionClass = "SoldierEB";

        // SoldierECrew's AK74SU is deliberately NOT a loot weapon:
        // scopeMagazine=0, it feeds from AK74 mags (see the convention note
        // above the class).
        baseWeapon = "AK74";
        baseMagazine = "AK74";
        lootRiflemanWeapon = "AK74";
        lootRiflemanMag = "AK74";
        lootMedicWeapon = "AK74";
        lootMedicMag = "AK74";
        lootMGWeapon = "PK";
        lootMGMag = "PK";
        lootATWeapon = "RPGLauncher";
        lootATMag = "RPGLauncher";
        lootSniperWeapon = "SVDDragunov";
        lootSniperMag = "SVDDragunov";

        // town flagpole texture (native TownFlags); matches the engine's
        // side-EAST default - stated for the data-file contract
        flag = "\flags\ussr.jpg";
    };
    class GUER // resistance descriptor; also selectable as the OCCUPIER
    {
        side = "GUER";
        // WL<4 riflemen -> WL4+ grenadiers. Honestly TWO tiers, not three: the
        // 1.99 core config ships no GUER special-forces/saboteur class, so the
        // ladder stops here rather than promoting SoldierGMG out of its role
        // slot. The vehicle ladder (which does reach T55G) carries the
        // high-war-level escalation instead. Tier 0 stays SoldierGB, so the
        // pre-existing resistance behaviour at low war level is unchanged.
        tiers[] = {"SoldierGB", "SoldierGG"};
        tierThresholds[] = {4};
        // role variants for the post-capture hold squads AND for the
        // GUER-as-occupier swap; parallel to tiers[], so two entries each.
        tiersMG[]    = {"SoldierGMG", "SoldierGMG"};
        tiersAT[]    = {"SoldierGLAW", "SoldierGLAW"};
        tiersMedic[] = {"SoldierGMedic", "SoldierGMedic"};
        // no GUER sniper exists in the 1.99 core config (SoldierGSniper is a
        // Resistance-era addon class, absent from the merged CONFIG.BIN).
        // "" = role-absent. For the SNIPER specifically that means the slot is
        // SUPPRESSED, not filled by a stand-in: ComposeSquad's `tierHasSniper`
        // test reads this entry's length and forces the sniper count to 0, so
        // the role() rifleman-substitution the other three roles get never runs
        // for it and the squad simply fills to size with riflemen. NOT
        // substituted with a cross-side SVD carrier. The SVDDragunov
        // lootSniper* entries below stay: the player still arms marksmen from
        // captured Soviet stock, which is the historically plausible FIA
        // answer anyway.
        tiersSniper[] = {"", ""};
        // Captured/irregular motor pool: guerrilla jeep -> PV3S truck ->
        // captured BMP -> T55.
        //
        // "BMP" is CfgVehicles BMP, which is side=0 (EAST) and crew=SoldierECrew
        // - and that is deliberate, not an oversight. 1.99 ships NO GUER-sided
        // APC or IFV at all (the complete side=2 scope=2 vehicle set is GJeep,
        // UAZG, SGUAZG, TruckV3SG +Reammo/Refuel/Repair, T55G, ParachuteG), so
        // the alternative is no APC rung. A vehicle's SIDE comes from the
        // AICenter of the group it spawns into, never from its config class
        // (GameStateExtWorld.cpp does veh->SetTargetSide(center->GetSide())),
        // so a BMP spawned into the resistance's center IS resistance armour.
        // That is exactly the captured-hull fiction this rung wants: the FIA
        // does not manufacture IFVs, it takes them. Same reasoning as T55G's
        // real history, minus a re-sided class to take it from.
        vehicles[] = {"GJeep", "TruckV3SG", "BMP", "T55G"};
        vehicleThresholds[] = {4, 7, 9};
        // CHANGED from SoldierGB: now that GUER can be picked as the
        // OCCUPIER, `officer` seeds garrison/QRF leader slots, and a leader
        // identical to his own riflemen is a real downgrade. OfficerG is a
        // verified side=2 scope=2 class (: SoldierGB, AK47 + Binocular).
        officer = "OfficerG";

        holdClass = "SoldierGB";
        holdCount = 3;
        recruitFighter = "SoldierGB";
        recruitSpecialist = "SoldierGMG";
        companionClass = "SoldierGB";

        // ---- character outfit family (issue #25): the CIVILIAN column of
        // the warrior/civilian axis. SoldierGFakeC/SoldierGFakeC2 are BIS's
        // own resistance-in-plainclothes classes (": Civilian"/": Civilian2",
        // side=2, armed, accuracy=2, scope=1 - hidden from the editor but
        // spawnable at runtime; verified in the 1.99 CONFIG.BIN). Honest gap:
        // 1.99 ships no civilian-model MG/AT/medic variants, so every role
        // wears the FakeC pair and role weapons come from loot
        // (GM_fnEquipFromUnlocked) - which is what an outfit axis wants.
        // playerClassWarrior documents the authored mission.sqm class; the
        // engine seam substitutes ONLY playerClassCiv (WorldInit.cpp /
        // Game/Guerrilla/OutfitSelect.cpp) and keeps the authored class when
        // the choice is warrior, unset, or unresolvable.
        playerClassWarrior = "SoldierGB";
        playerClassCiv = "SoldierGFakeC";
        recruitFighterCiv = "SoldierGFakeC";
        recruitSpecialistCiv = "SoldierGFakeC2";
        companionClassCiv = "SoldierGFakeC";
        holdClassCiv = "SoldierGFakeC";
        // civilian-outfit ladder for AI garrison/guard/militia rungs (native
        // gmFactionCivTier; also issue #16's guardClass surface). Same
        // tierThresholds[] gates as tiers[], clamped to its own length.
        civTier[] = {"SoldierGFakeC", "SoldierGFakeC2"};

        baseWeapon = "AK47";
        baseMagazine = "AK47";
        lootRiflemanWeapon = "AK47";
        lootRiflemanMag = "AK47";
        lootMedicWeapon = "AK47";
        lootMedicMag = "AK47";
        lootMGWeapon = "PK";
        lootMGMag = "PK";
        lootATWeapon = "RPGLauncher";
        lootATMag = "RPGLauncher";
        lootSniperWeapon = "SVDDragunov";
        lootSniperMag = "SVDDragunov";

        // town flagpole texture; matches the engine's side-GUER default
        flag = "\flags\fia.jpg";
    };
};
