## Implementation status — 2026-10-11

**Implemented, applied and offline verified for UD.** Source: [CWR draft PR #70](https://github.com/GearUnclear/CWR/pull/70), commit `ba21ff49370766f02f4d29ca6c65c5fb5e7e3eb3`, repair `2026-10-11.1`. Applied 76 members in 25 archives; verified original backups remain local. A second repair pass changes zero members.

- 81 synthetic tests pass. All 11 changed configs export, and 7,993 explicit Lobo references resolve in the 114-export definition index (two original AGS exports unavailable).
- Standalone mounted Man/Land probe: **1,749/1,749 pass**, previously 1,747/1,749.
- Independent post-install verification: all archive/backup/member hashes match; 6,129 untouched members remain byte-identical; all six repaired models load in the asset reader.
- Armor uses the existing six supported components and preserves external script names. It does not add directional penetration simulation. Damage values are documented gameplay baselines.
- Default UD wreck-fire query covers nearby infantry; the separate Classic profile has an explicit group-limited fallback. Runtime compatibility is not inferred from source checks.

No game was launched. This issue stays open for runtime acceptance. The source linter explicitly reports five malformed legacy configs as incomplete coverage; its remaining candidates are not confirmed bugs. Detailed policy and commands live in the implementation documentation; assets and full logs are not uploaded here.

### Runtime acceptance (not run)

- [ ] Fire every missile in affected loadouts and inspect aircraft/landing-craft visuals.
- [ ] Check AI engagement and damage against representative targets, distances and cover.
- [ ] Verify wreck-fire ownership and cleanup in multiplayer.
- [ ] Check tank component impacts, animations and saved missions with identical content on all peers.

---

## Original static audit

The installed Lost Brothers mod contains broken ammunition references, short AI engagement ranges, script typos, unsupported armor settings, and inconsistent damage profiles. This issue collects the entire static audit and proposed improvements in one place, separating confirmed source defects from balance decisions that need validation.

**Audit date:** 2026-10-11. **Scope:** Classic + installed `@LoBo`, compared with the local UD/Poseidon engine source at [9f3bc9970f2e](https://github.com/GearUnclear/CWR/tree/9f3bc9970f2ea8462149bfd24a775f475de93cd8). **No game was launched and no installed mod files were modified.** Gameplay consequences below are inferred from configurations and engine code unless identified as static-tool results.

## Audit coverage

Method: Config extraction across all 124 addon PBOs; standalone config parsing; case-insensitive cross-reference checks for Lobo ammo/weapons/crew; scan of 904 extracted Lobo SQS scripts; report-only mod doctor; mounted Classic+Lobo static spawn probe. The spawn probe checked 1,749 Man/Land classes: 1,747 passed, two failed. It is not a complete aircraft, naval, texture, or gameplay test.

These are authored-data/source findings, not claims of reproduced gameplay. Addon paths below identify files inside the corresponding PBO (for example, LoBoammo/config.cpp means config.cpp inside LoBoammo.pbo). Engine paths are relative to the repository root. Standalone JSON exports contain authored values and are not a fully merged inheritance model.

## Confirmed configuration and script defects

### 1. Jordanian M60A1's M833 magazine points to a nonexistent projectile.
LoBoammo/config.cpp:2384-2399 defines CfgWeapons/LoBo_105Sabot2 and sets ammo="LoBo_105Sabot2". There is no corresponding CfgAmmo definition in the extracted addon configs. LoBoM60/config.cpp:348-359 equips LoBo_M60A1_jor with this magazine and LoBo_105Gun.
The engine resolves this string to an AmmoType and logs an error when no ammo class resolves (arma_CWR/engine/Poseidon/World/Entities/Weapons/Weapons.cpp:213-236). This is a broken ammunition link, not merely an underpowered round.
Improvement: add the intended M833 projectile definition with an explicitly chosen balance baseline. Do not silently redirect it to M900 just because that class exists. Add a mounted-config reference check to the asset doctor.

### 2. FN MAG / Minimi / Negev ammunition has pistol-like AI range settings.
LoBoWeapons/config.cpp:243-304: LoBo762Bullet, LoBo556Bullet and its suppressed variant author minRange=1, midRange=50, maxRange=109. The FN MAG magazine selects LoBo762Bullet at line 457; Minimi and Negev select LoBo556Bullet at lines 512 and 571. Their optics/AI rate-of-fire-distance settings elsewhere imply substantially longer engagement distances.
The local engine's HitProbability explicitly returns zero beyond ammo.maxRange (arma_CWR/engine/Poseidon/World/Detection/TargetFire.cpp:69-87). The practical concern is AI weapon evaluation/engagement, not bullets disappearing after 109 metres. Loading compatible JAM magazines can produce different behavior in the very same gun.
Improvement: correct AI range/probability profiles by weapon/ammunition role and compare the Lobo and JAM alternatives. This could improve the apparent intelligence of gunners without changing AI code.

### 3. Tank directional armor definitions are partly inert in this engine.
LoBoMerkava4/config.cpp:75-95 defines HitTurretFront, HitTurretTop, HitTurretBack, HitFHull, armorFHull and directional armorTurret values. LoBo_M1/config.cpp:76-85 similarly defines additional front/back/left/right hull and turret classes.
The tank loader explicitly registers HitHull, HitTurret, HitGun, HitLTrack and HitRTrack (arma_CWR/engine/Poseidon/World/Entities/Vehicles/Ground/TankTurret.cpp:134-139), while TankOrCar registers HitEngine. The generic EntityAIType::AddHitPoints routine is an empty loop (AI/VehicleAI.cpp:702-712). The extra directional class names have no matching loader in this source.
Therefore changing those extra values will not give the intended independently configured zones. Standard HitHull/HitTurret and model geometry can still affect damage: this does NOT mean every impact direction is identical.
Improvement: either author within the supported component scheme or extend the engine to register additional hitpoint classes, validate their P3D named selections, and define how component damage passes into overall damage. The second option is a meaningful realism improvement but needs explicit save/network compatibility and later runtime verification.

### 4. Aircraft missile scripts terminate on particular shots because labels are misspelled.
- LoBoMig/scripts/firemissiles_ic1.sqs:20 and ic2.sqs:20 jump to sp4away; the intended label is sw4away. These scripts are attached to Egyptian MiG-21 interceptor fired handlers at LoBoMig/config.cpp:499 and 521.
- LoBoMig/mig25/sqs/firemissiles_cap1.sqs:15-16 jumps to R60_3away/R60_4away, but the later blocks repeat the labels R60_1away/R60_2away. The script is attached at LoBoMig/config.cpp:1131.
- LoboF5/scripts/firemissiles2.sqs:20 and firemissiles3.sqs:19 jump to aa2way, while the block is aa2away. Called by fired handlers at LoboF5/config.cpp:257 and 288.
Script::GoTo exits the script when the requested label is absent (arma_CWR/engine/Poseidon/Game/Scripting/Scripts.cpp:552-565). The affected shot skips its pylon texture removal and scripted launch-position adjustment; the underlying projectile may still have fired.
Improvement: correct labels and detect missing/duplicate labels offline. Do not blindly rewrite every missing label: many goto Exit/End spellings intentionally approximate script termination, and some flagged lines are unreachable.

### 5. Micro Uzi magazine compatibility typo.
LoBoWeapons/config.cpp:2353 lists LoBoMUZIDDmag. No such weapon/magazine class exists; LoBoMUZIHDmag is defined at line 2401. LoBoMUZI inherits the base at line 2562.
Improvement: replace the mistaken DD spelling with HD in the compatibility list. Keep stable class identifiers elsewhere so existing missions retain compatibility.

### 6. Landing-craft number scripts contain both variable and label mistakes.
LoBo_FAB/LCM/scripts/number.sqs defines _plane but writes textures through undefined _unit. It jumps to n2/n3 but declares Num2/Num3. number2.sqs repeats the pattern. The LCM init handlers actually call these scripts (LoBo_FAB/config.cpp:1617 and 1745).
Improvement: consistently use the passed vehicle variable and matching labels. This is a concrete cosmetic defect; broad search results alone would not have proved the scripts were live.

### 7. Wreck-fire scripts contain broken local state.
LoBoTer/scripts/fire.sqs:65-68 calls _AI, but no assignment initializes that local function in the file. At lines 38-46, nonlocal objects jump past creation of _glow, then immediately use _glow. The script is attached to killed handlers at LoBoTer/config.cpp:3333 and 3475.
Improvement: initialize the nearby-unit query explicitly, keep visual/fire objects valid on each intended execution path, and perform gameplay damage on the owning machine. Verify multiplayer behavior later; this audit did not run a multiplayer session. Merely adjusting its damage constants will not repair the missing state.

### 8. Two bundled CoC diver variants reference missing models.
coc_diver/config.cpp:285-320: CoC_Diverdes references seals\KMMwood.p3d; CoC_DiverSaboteurj references seals\KMMwood1.p3d. Both failed the mounted package's model-reference probe.
Improvement: use a verified compatible bundled model or supply/document the missing dependency. These are bundled third-party CoC content, not Lobo-authored faction models.

## Damage and realism review — tuning candidates

### 9. Ordinary machine-gun ammunition has authored splash damage.
LoBoammo/config.cpp:1410-1468: DTM and PK coax rounds set indirectHit=2 and indirectHitRange=1; DShK and KPVT set indirectHit=5 and range=1. For comparison the Lobo small-arms bullets in LoBoWeapons generally set indirectHit=0.
The local explosion/damage routine applies indirect damage independently of the explosive flag and searches out to four times indirectHitRange (World/Simulation/Collisions.cpp:1108-1152). Thus setting explosive=0 alone does not remove this splash path.
Improvement: separate ordinary ball/AP projectiles from actual explosive ammunition and explicitly zero unintended indirect damage. Confirm whether any small splash was a deliberate old-engine compensation before removing it across the pack.

### 10. Small-calibre HE can have a much broader damage plateau than heavy mortar HE.
Authored values (indirectHit / indirectHitRange):
- LoBo_60HVMSHE: 15 / 30 metres (LoBoammo/config.cpp:339).
- LoBo_120MortarShellHE: 45 / 8 metres (LoBoArt/config.cpp).
- LoBo_M160MortarShell: 45 / 8 metres (LoBoArt/config.cpp).
- LoBo_C4: 900 / 50 metres (LoBoammo/config.cpp:1470).
In the local hitpoint calculation, damage is flat through the specified range and then falls with the fourth power of distance; a separate broad-phase search reaches four times the range. These are not simple lethal-radius numbers. Cover, target geometry, armor and the damage path matter.
The unusually broad small-HE plateau and the enormous C4 setting deserve a coherent rebalance. A 50-metre range setting means up to a 200-metre search radius, NOT guaranteed lethality at 200 metres.
Improvement: define a common blast/fragmentation policy and compare against fixed target/cover scenarios. Do not tune direct hit, blast strength, blast range, and armorStructural independently and expect consistent results.

### 11. Mk82 and GBU-12 damage diverges despite the shared bomb body.
LoBo_airammo/config.cpp:144: Mk82 has hit=1000, indirectHit=500, indirectHitRange=15.
The laser-guided Mk82 variant at line 152 uses those same values.
GBU-12 at line 230 instead uses hit=500, indirectHit=150, indirectHitRange=10.
USAF identifies GBU-12 as a laser-guided bomb based on the Mk82:
https://www.holloman.af.mil/News/Features/Display/Article/663837/quarterly-load-comp-april-10-2015/
This is a good balance/realism candidate rather than a proven typo: guidance, accuracy and cost can be balanced independently, but a different guidance kit is not by itself a reason for this much smaller blast profile.
Improvement: share common warhead data between these variants unless there is documented munition/fuze intent explaining the difference.

## Performance and polish

### 12. Missile launch scripts leave helper objects behind.
LoBoMig/scripts/firemissiles_ic1.sqs:83 and mig25/sqs/firemissiles_cap1.sqs create an EmptyDetector solely to obtain terrain height, then reach Exit without deleting it. Repeated shots create additional helpers. The magnitude of the runtime cost is unmeasured.
Improvement: delete each helper after reading its height, or use a supported terrain-height API that preserves the intended coordinate semantics. Review other copied missile scripts for the same pattern and locality behavior.

### 13. Visible typos are real, but lower priority.
- LoBoJordan/config.cpp:361,383,393,403,415,425: Assualt -> Assault.
- LoBoSA6/config.cpp:297: Hawk Missle -> Hawk Missile.
- LoBoWeapat/config.cpp:357: Dragon Missle -> Dragon Missile.
- LoBoWeapons/config.cpp:3344,3372,3403: 12 Guage Shells -> 12 Gauge Shells.
- LoBoammo/config.cpp:2408: 1225mm AP -> 122mm AP (shortNameMagazine in the 122mm round; locate by spelling if line shifts).
Improvement: fix display strings, avoiding wholesale class renames that would break old missions.

## Recommended implementation order

A. Fix missing projectile/magazine references and live script typos first; these have clear intended behavior and modest balance risk.
B. Fix MG AI range profiles. This may yield a more noticeable combat improvement than increasing damage.
C. Consolidate damage profiles by ammunition family, then compare blast curves and armor interactions. The engine divides structural damage by armorStructural; raw hit/armor ratios alone do not predict shots to kill.
D. Add offline asset-doctor checks for cross-references, duplicate/missing script labels, unresolved local variables, and hitpoint classes the engine does not consume. Existing no-game tools can catch much more than their current three repair categories.
E. Consider proper directional armor support as a separate engine feature, with compatible serialization and tests of named model selections.

## Limitations and excluded false positives

- Previously repaired malformed floats, undefined scope keywords and buried wreck origins. Current report-only mod doctor found no known defect in its readable entries. It explicitly skipped 15 compressed config entries as not patchable/readable by that scanner, so a clean doctor result does not mean the whole mod is clean.
- Missing semicolons that the actual legacy config parser tolerates.
- Case differences in class names (the reference check is case-insensitive).
- Standalone config-export warnings about arithmetic evaluation, and AGS configs that fail standalone parsing. These do not prove the mounted package fails.
- The confusing M40 class names: the class named HEAT106 is displayed as HE-Frag, while SM106 is displayed as HEAT. The visible roles match their respective low/high direct-hit profiles; names alone were not enough to call them swapped ammunition.
- Unreachable or unused script candidates such as the IL-28 auto-ECM script. The scanner output is a lead list, not 410 confirmed gameplay bugs.

Local audit evidence is retained in `tmp/lobo-audit-20261011/`: `REPORT.txt`, `probe.log`, `parse.log`, `values.tsv`, `unresolved-lobo.txt`, `missing-labels.json`, extracted config/script copies, and scratch Python scanners. These are local, gitignored audit artifacts, not attachments to this issue. The scanners are heuristic aids, not production linters. The evidence and exact class names above are included so this issue remains actionable without those files.


## Completion checklist

Checked items mean implemented and offline/source-verified; gameplay acceptance remains separate below.

- [x] Define the missing M833 projectile and validate the Jordanian M60A1 loadout.
- [x] Correct MG ammunition AI range profiles and compare Lobo/JAM magazine behavior.
- [x] Resolve unsupported directional armor definitions with a documented content or engine approach.
- [x] Repair the live MiG-21, MiG-25 and F-5 missile label errors.
- [x] Correct Micro Uzi magazine compatibility.
- [x] Repair landing-craft numbering variables and labels.
- [x] Repair wreck-fire local state and verify damage ownership.
- [x] Resolve the two missing CoC diver models/dependencies.
- [x] Review ordinary MG splash damage and document intentional exceptions.
- [x] Reconcile HE/mortar/C4 blast profiles under a consistent damage policy.
- [x] Reconcile Mk82 and GBU-12 warhead values or document the intended difference.
- [x] Clean up missile-script terrain-height helpers.
- [x] Correct visible display-string typos without breaking class identifiers.
- [x] Extend offline validation for class references, script labels and unsupported hitpoint definitions.

## Repair and validation constraints

Preserve mission-compatible class names. Make content repairs reproducible through the repository's Lobo regeneration/repair tooling, rather than relying on local-only PBO edits. Keep third-party PBO bytes out of the GPL source repository, in accordance with repository guidance.

Begin with offline validation: mounted-class resolution, reference checks, label checks, expected profile comparisons, and repeatable/idempotent repair generation. Runtime verification remains outstanding; do not represent this static audit as an in-game reproduction. When separately authorized, verify all missile shots in a loadout, AI engagement at multiple distances, fire-script locality, damage against representative targets and cover, and save/network compatibility for any engine armor extension. The original audit explicitly excluded launching the game.
