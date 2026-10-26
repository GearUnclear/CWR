# Issue comment drafts, repair 2026-10-11.3

Drafts only, not posted. Replace `SHA_PLACEHOLDER` with the CWR commit on main
before posting. Verification levels follow the README vocabulary: implemented,
applied, offline verified, runtime verified.

## #2 Six vehicle scripts loop every frame

Fixed in repair 2026-10-11.3 (CWR commit SHA_PLACEHOLDER on main), rule module `loops`, applied to the installed @LoBo in backup batch `20261012T034354Z-0532e837`.

- Sea King `float.sqs` waits `~0.1`. When the helicopter dies or is deleted, the float plate is deleted and the script exits, so a destroyed Sea King is no longer held on the water.
- Dolphin arm waits `~1` and exits with its vehicle. MiG-25 gear, Syrian BMP-1 doors and F-5 canopy wait `~0.5`. The A-4 canopy waits `~0.5` instead of `~0.0001` and exits with its vehicle. The BMP-1 script gains its missing `#Exit` label.
- The same module also slows the LoBo_FAB flag loops, the CSJ airliner lights and the CSJ ILS helper, and fixes the CoC artillery MaxWind guard (audit items AG-1, AG-3, AG-4, AG-10).

Status: implemented, applied and offline verified. Synthetic tests with literal rule pins pass, the installed repeat check exits 0 with zero changes, and every changed member hash was checked. Not runtime verified: no game run exercised these vehicles. Still unchecked in game: the Sea King resting on water with the plate following, plate removal on death, the door, canopy, gear and arm animations, and a flat script count over time.

Recommendation: close. The source defect is fixed; the in-game confirmation above is optional and can be done in a later play test.

## #3 Placed mines poll nearestObject every frame

Fixed in repair 2026-10-11.3 (CWR commit SHA_PLACEHOLDER on main), rule module `mines`, applied in batch `20261012T034354Z-0532e837`.

- All 17 mine scripts in LoBo_Mines and JOF_Objects1 poll every 0.1 s instead of every frame. By the spec's estimate that still catches about 99% of top-speed AP crossings and keeps AT detonation lag under 2 m at 70 km/h.
- The flare tripwire is only about 1 m wide, so it polls every 0.03 s (about the frame rate its author tuned for). The suggested 0.25 s would make it unreliable.
- Each loop exits when its mine is deleted (`isNull`), so a deleted mine can no longer fire a ghost explosion.
- The flare mine deletes its `_Mess` helper on trigger, pickup and deletion.

Status: implemented, applied and offline verified, with the same offline checks as #2. Not runtime verified: no game run placed or triggered mines. Still unchecked in game: AP mine walk, run and sprint; AT mine drive-over at speed; naval mine; flare tripwire on both sides; claymore; no ghost explosion after deletion; flare helper gone. Not part of this fix: in multiplayer every client still runs every mine (a locality redesign, separate from the polling cost).

Recommendation: close.

## #4 Mk-20 Rockeye never opens on the A-4, F-4 and IL-28

Fixed in repair 2026-10-11.3 (CWR commit SHA_PLACEHOLDER on main), rule module `clusters`, applied in batch `20261012T034354Z-0532e837`.

- The A-4R and A-4RN (`firemissiles_A2.sqs`, `firemissiles_A3.sqs`), F-4 WW (`firemissiles_WW.sqs`) and IL-28 AS (`fire_mk20.sqs`) now call the consolidated `\LoBo_airammo\Scripts\cbu\rockeye.sqs`. No script still calls the old LoBoF16 or LoBoF15 path. The IL-28 fired script also gains the .1 projectile owner guard.
- The F-5 and IL-28 rockeye copies now create the defined `LoBo_mk20Deployed`. No alias class was added.

Status: runtime verified. Run `mk20-03` (UD engine, official Remaster data, installed @LoBo): the A-4R, F-4 WW and IL-28 AS each opened one `LoBo_mk20Deployed` and released at least 194, 194 and 187 of 197 bomblets (the probe samples about every 10 frames, so these are lower bounds). Limits: only the script path was tested, with the aircraft held still and invulnerable at 150 m, so ballistics, spread and damage are unchecked. The A-4RN and F-5 paths were not flown; the F-5 class rename is offline verified only.

Recommendation: close.

## #5 Syrian MiG RBK-500 vanishes at 50 m

Fixed in repair 2026-10-11.3 (CWR commit SHA_PLACEHOLDER on main), rule module `clusters`, applied in batch `20261012T034354Z-0532e837`. `LoBo_airammo\Scripts\cbu\rbk500.sqs` now creates the defined `LoBoRBK500Bomblet`; no alias class was added, and no script still names `LoBoRBKBomblet`.

Status: runtime verified. Run `rbk500-01` (UD engine, official Remaster data, installed @LoBo): the Syrian MiG-21 CAS1 fired the RBK-500 through its installed fired handler, the dispenser opened at about 47.6 m (the script releases at 50 m), and at least 100 of the script's 105 bomblets were seen. A bomblet created directly first proved the counter could see them. Limits: script path only, aircraft held still and invulnerable at 150 m; ballistics, spread and damage are unchecked.

Recommendation: close.

## #8 Tail numbers never appear on F-15, F-16, IL-28 and F-4

Fixed in repair 2026-10-11.3 (CWR commit SHA_PLACEHOLDER on main), rule module `numbers`, applied in batch `20261012T034354Z-0532e837`. The F-15, F-16 IAF (`number.sqs`), F-16 EAF (`number2.sqs`), IL-28 and F-4 scripts now use the form #1 gave the landing craft: the vehicle variable is set, every label resolves, and the digit slots follow each model's `hiddenSelections[]` (num1 to num3). The F-4 afterburner flame (index 4) is no longer painted. The same-length patch in the issue text would have left the IL-28 and F-4 slots wrong, so the members were repacked instead.

Status: runtime verified. Run `numbers-01` (UD engine, official Remaster data, installed @LoBo): two copies each of F-15C, F-16 IAF, F-16 EAF, F-4 and IL-28 show visible numbers that differ within each pair, checked by eye on the screenshots. No script error and no `Unknown label` in a 1.2 million-line debug log. Offline, all 30 digit textures exist and no copy uses `_unit` any more. Known limits, not part of this fix: numbers are random per client, so multiplayer peers will disagree, and the engine does not save `setObjectTexture` results, so numbers vanish after loading a save.

Recommendation: close.

## #9 Wreck fire scripts spawn undefined M1Smoker/M60Smoker/Smoker classes

Implemented in repair 2026-10-11.3 (CWR commit SHA_PLACEHOLDER on main), rule module `effects`, applied in batch `20261012T034354Z-0532e837`, with follow-up guards in batch `20261012T042613Z-b68ba5de`.

- The question is answered: `M1Smoker`, `M60Smoker`, `Smoker` and the helicopter scripts' `KioGlowFire` were defined nowhere (all @LoBo configs, @LoBo_UD, @LoBo_Deps, the Classic and Remaster base configs).
- LoBo_airammo's config now defines `LoBo_WreckGlow`, an invisible vanilla `Fire` (model `empty`), with all four classes under it.
- Consequence: defining `KioGlowFire` makes the BAS helicopter wreck burn live for the first time. As authored it would hit any player within 10 m, seated or not, with +0.04 every 0.25 s. The second batch gives the five helicopter `fire.sqs` copies (LoBoEghelo `Engine`, `Engine2`, `Engine3`; LoboIDFhelo `CH53\Engine`, `UH1\Engine`) the ground scripts' policy: the wreck owner runs the burn, only a local, living player on foot is hit, at most 0.04 per 10 s.
- Known side effect: the glow is a vanilla Fireplace, so a soldier on foot within 5 m who faces it is offered "Put out fire". No config key suppresses it.

Status: partly runtime verified. Run `glow-01` (UD engine, official Remaster data, installed @LoBo): all four classes create objects, and the killed handlers of a Toyota (`M1Smoker`), a Merkava2 (`M60Smoker`) and a Mi-8 (`KioGlowFire`) each made exactly one burning glow 1 to 3 m from the wreck. Run `glow-unlisted-01`: in a mission that does not activate `lobo_airammo`, each glow logs one `Access denied` warning and is still created.

Remaining runtime checks before closing:
1. A player seated in a vehicle next to a burning helicopter wreck takes no burn.
2. A player on foot within 10 m of it gets one MEDIC line and at most 0.04 damage per 10 s.
3. The "Put out fire" action at a wreck glow: confirm it appears, and decide whether it is acceptable.
4. The glow is removed by the 150 s cleanup.
5. The `say "fire"` sound and the light are present, and nothing visible (no campfire mesh) sits at the glow. The wreck screenshots so far show only smoke.
6. Optional: decide whether UD should preload `lobo_airammo` to silence the `Access denied` warning.

Recommendation: keep open for the checks above. The `question` label can go, since the classes are confirmed missing and now defined.
