# LoBo issue #1 runtime acceptance probes

These original mission and Trident sources exercise the installed repaired UD
profile. They do not contain or modify game PBOs. Read the repository guides and
`../CONTENT-REPAIRS.md`, `../AMMUNITION-POLICY.md`, and `../ARMOR-MODELS.md` first.
Keep proposed, implemented, applied, offline verified, and runtime verified
results separate. A runner exit code of zero verifies its assertions, not every
acceptance claim, screenshot, or unsupported case. The later repair 2026-10-11.3
and 2026-10-11.4 probes, which ran on the Remaster data, are described in the
last two sections.

## Reproduce locally

The 2026-10-11 acceptance task explicitly uses `D:\Arma_CWA\@LoBo`, the existing
`D:\Arma_CWA\ARMA Cold War Assault [Classic]` data, and binaries under
`D:\Arma_CWA\arma_CWR\dist\x64-win-rwdi`. This documents that task's environment,
not a new general Classic compatibility lane. Do not use Steam copies, a real
server, or the `@lobofixup` fixture overlay. Mount identical repaired content on
all peers and save phases; retain the content hashes with the run evidence.

Build Trident from the checkout containing these fixtures into an isolated
output directory. The older installed runner does not support all hosted
fixture staging and ephemeral-port behavior used here.

```powershell
Set-Location -LiteralPath 'C:\Users\Dane\.codex\worktrees\lobo-runtime-acceptance\Arma_CWA\arma_CWR'
$taskBuild = 'D:\Arma_CWA\tmp\lobo-runtime-acceptance-20261011\trident-build'
cargo build --manifest-path engine/Trident/Cargo.toml --target-dir $taskBuild
if ($LASTEXITCODE -ne 0) { throw 'Trident build failed' }
$taskTri = Join-Path $taskBuild 'debug\tri.exe'
powershell -NoProfile -File tools/lobo/runtime/run-local.ps1 `
  -Tri $taskTri `
  -TestPath tools/lobo/runtime/damage.test.sqf `
  -OutputDir 'D:\Arma_CWA\tmp\lobo-runtime-acceptance-20261011\damage-repro' `
  -TimeoutSeconds 540
```

Use a fresh output directory for each run. Pass `-GameDir` and `-DataDir` to the
wrapper when reproducing on another installation, and review the explicit mod
paths in the TOML files. The wrapper defaults to GL33, one worker and one
attempt, captures runner stdout/stderr, records `exit.txt`, and closes game
children belonging to its runner in `finally`. Preserve game stdout/stderr and
screenshots as local evidence. Do not leave game windows open after a run.

For a save roundtrip, select the directory `tools/lobo/runtime/tanks_save.seq`,
not an individual phase. For hosted multiplayer, select
`tools/lobo/runtime/wreck_fire.test`. The hosted case uses three instances of
`PoseidonGame.exe`, an isolated user directory per peer, a staged mission,
LAN hosting, and direct client joins to `127.0.0.1`. Allow at least 540 seconds
for the wrapper; the current scenario deadline is 420 seconds.

Before and after runtime work, use the report-only checks. The accepted repair
baseline for this task is `2026-10-11.1`; do not modify PBOs by hand.

```powershell
python tools/lobo/repair_lobo.py --mod 'D:\Arma_CWA\@LoBo' --compatibility ud --check
& 'D:\Arma_CWA\arma_CWR\dist\x64-win-rwdi\PoseidonTools.exe' mod doctor 'D:\Arma_CWA\@LoBo'
```

The repair check must exit 0 and the doctor must report `Nothing to do`.
Any actual repair follows the source-tooling, version, backup, reapply and
reverification policy in `CONTENT-REPAIRS.md` and the task instructions.

The historical `wreck_fire.test/test-dedicated.toml` and
`server-dedicated.sqf` are reference files, not the active hosted configuration.
The attempted dedicated run was rejected before acceptance because the installed
server build tag `f8b867c-dev` differed from client tag `9f3bc99-dev`, despite
matching protocol and mod names. This is not evidence of a LoBo script defect.

## Probe scope

| Entry point | Intended observation | Main limit |
| --- | --- | --- |
| `baseline.test.sqf` | Installed classes, one M833 firing path and basic object operations | Harness smoke check only |
| `aircraft.test.sqf` | Five affected loadouts, real Fired events and magazine depletion, helper counts and pylon screenshots | Each screenshot needs visual review; firing does not prove guidance or combat effectiveness |
| `lcm.test.sqf` | Native number initialization and both sides of IDF/Egyptian LCMs | Number visuals only, not every auxiliary initialization branch |
| `damage.test.sqf` | Jordanian M60A1 M833 gunshot, six controlled impacts, 108 representative blast observations | Synthetic downward projectiles and one observation per case are not penetration or lethal-radius measurements |
| `ai_ranges.test.sqf` | Commanded AI firing at 100/300/500 m using installed ammunition | Terrain and line of sight are not established; targets are captive and invulnerable |
| `ai_clear_lane.test.sqf` | Five native 30-second firing windows on a sampled clear terrain lane, with the player nearby | Terrain samples cannot exclude object occlusion or prove perception; zero shots remain open |
| `ai_controls.test.sqf` | Same-lane native LoBo M240 and stock M60 firing controls at 100/300 m | A diagnostic for the lane helper, not a LoBo range acceptance pass |
| `ordinary_mg.test.sqf` | Direct collision damage with an adjacent infantry splash check for nine ordinary projectiles | One geometry and separation, not all ammunition or target shapes |
| `blast_variants.test.sqf` | Additional Carbomb and Mk82 LDGB observations using the same blast matrix layout | Same placement, cover and displacement limits as the main matrix |
| `tanks_save.seq` | Eight model-bearing tank classes, component impacts, named track seeds and a native fresh-process save reload | Named getters do not expose separate duplicate blank slots; SP persistence does not prove networking |
| `wreck_fire.test` | Hosted ownership, unrelated local/remote victims, native killed handler, deliberate remote fallback, glow and natural cleanup; tank replication diagnostics | Client tank values are observational, so passing fire assertions does not establish replication |
| `wreck_diagnostic.test` | Short hosted before/5-second/15-second wreck, clock, locality and nearby-object observations | Diagnostic only, with no fire ownership or natural cleanup acceptance |

The aircraft classes are `LoBo_E_mig21bis_IC1`, `LoBo_E_mig21bis_IC2`,
`LoBo_Mig25PD_Syria`, `LoBo_F5_mir_Jor`, and `LoBo_F5_AT_Jor`. The probe fires
30 missiles and two Mk20 bombs. Airborne placement avoids ground-blast rollovers.

The LCM fixture sets `Server = objNull` before native initialization. This
selects the non-server auxiliary branch while the number scripts still run
through the installed config's Init handler. No texture is assigned directly.
The ordinary native auxiliary branch's `Server` global precondition remains a
coverage limit. A statically observed `_driver` use before assignment in an
auxiliary script is an unconfirmed candidate, not a reproduced defect.

Tank classes are `LoBo_M1`, `LoBo_M1G`, `LoBo_M1MG_2`, `LoBo_M1GMG_2`,
`LoBoMerkava1`, `LoBoMerkava2`, `LoBoMerkava3`, and `LoBoMerkava4DT`.
The first four cover M1A1E/M1A1E_R/M1A2E/M1A2E_R models. Actual downward
`LoBo_105Sabot2` collisions precede explicit `setSelectionDamage` track seeds
of 0.12/0.23. The reload compares the full ordered name list, values obtained
by name, and total damage within 0.0001. Engine, hull, turet, gun, pasl, and
pasp are mandatory; additional blank or light selections vary by model.

## Reviewed runtime evidence, 2026-10-11

Evidence root: `D:\Arma_CWA\tmp\lobo-runtime-acceptance-20261011`.
This records completed probes and their limits. Issue #1 still has open AI,
multiplayer and animation acceptance. No PBO repair is established by these
observations, and no issue closure follows from a passing diagnostic.

| Run | Reviewed result |
| --- | --- |
| `aircraft-04` | Runner passed in 41.8 s. All 30 missiles and two Mk20 bombs produced Fired observations, ammunition depletion and expected helper counts. Close visual review showed missile visuals disappearing through all five sequences and empty missile pylons after, with expected tanks and pylons retained. |
| `lcm-01` | Visual review found IDF 042 and three Egyptian Arabic digits on the inspected sides/stern. Native numbering was exercised with the auxiliary-server limitation above. |
| `damage-02` | Passed in 88.7 s, exit 0; 108 blast rows and six positive M833 impacts. |
| `blast-variants-01` | Runner passed in 25.8 s; 36 additional Carbomb and Mk82 LDGB blast rows. |
| `ordinary-mg-02` | Passed in 9.2 s, exit 0; all nine projectiles caused positive direct infantry damage and zero nearby damage, with measured pre-impact separation 2.00349 m. |
| `ai-ranges-02` | Passed in 248.2 s, exit 0; commanded firing at about 100 m verified, longer-range cases fired zero. |
| `ai-clear-lane-01` | Runner passed setup assertions in 159.0 s. All five firing windows recorded zero shots despite a flat sampled terrain lane, so extended MG engagement remains unverified. |
| `ai-controls-01` | Passed diagnostic collection in 97.1 s, exit 0. LoBo M240 at 100 m and stock M60 at 100/300 m all fired zero, so the sampled lane has no positive firing control. |
| `tanks-save-01` | Passed in 37.0 s, exit 0; eight matching reload snapshots, empty mismatch list, and fresh-process baseline. No observable turret turn in the two aim screenshots. |
| `wreck-fire-04` | Timed out in 421.7 s. Both clients waited for wreck destruction; host waited for one glow object. Fire ownership, burn damage and natural cleanup were not reached. |
| `wreck-diagnostic-01` | Passed diagnostic collection in 39.0 s, exit 0. All clocks advanced, host wreck was destroyed, but both clients still saw it alive and undamaged after 15 seconds. Expected glow count remained zero. |

The aircraft setup used invulnerable aircraft at 250 m with orientation resets.
This verifies the controlled loadout and pylon sequences, not self-blast survival,
guidance, or general combat effectiveness. Aircraft screenshot evidence is under
`aircraft-04/aircraft`; LCM screenshots are under `lcm-01`.

Some early final-batch `exit.txt` files contain a blank `exit_code=` due to the
wrapper's process-handle lifetime bug. Their `runner.stdout.log` contains the
explicit passed summaries cited above. The wrapper was corrected before the
short MP diagnostic and `ordinary-mg-02`, which both recorded numeric exit 0.
Do not invent a numeric exit code for a blank record.

### Ammunition and damage

In `damage-02/game_stdout.log`, the Jordanian gun fired `LoBo_105Sabot2`
(line 368). Separate controlled impacts produced total damage 0.178045 on stock
Abrams, 0.0312112 on LoBo M1, 0.0333987 on Merkava 2 and 3, 0.022625 on
Merkava 4DT, and 1 on UAZ (lines 371-427). UAZ does not have the tank selection
names, so its zero hull/turet getter values do not mean its total damage failed.

The 60 mm blast had much less effect at nominal 15 m and zero at 30 m in the
matrix. The intentionally unchanged 120/160 mm mortar observations closely
matched. Exposed Abrams damage at nominal 5/15/30 m was
0.227164/0.169522/0.013320 for Mk82 and
0.228932/0.170764/0.013088 for GBU-12. M113 cover reduced vehicle damage to zero
at 15/30 m but often gave little protection at 5 m. These are observed layouts,
not a promise about every cover geometry. `actualDistance` is measured after
the blast and can include displacement. Matrix completion is at line 861.

`blast-variants-01/blast_variants/game_stdout.log` contains 36 rows and the
completion marker at line 527. Mk82 LDGB Abrams damage at nominal 5/15/30 m
was 0.229815/0.152141/0.0133214; Carbomb was
0.196822/0.115087/0.002627. The bomb family has similar qualitative results,
while Carbomb retains its lower authored strength than C4. The same cover and
postblast-distance limitations apply. These observations do not calibrate
physical penetration, explosive mass, or lethal radius.

`ordinary-mg-02/ordinary_mg/game_stdout.log`, lines 226-273, records all nine
ordinary projectile observations. Direct damage ranges from 0.371924 to 1;
nearby damage is zero in every case. Exact pre-impact placement was checked
against 2 m with 0.1 m tolerance and measured 2.00349 m. Final separation varies
from 1.98688 to 2.03086 m. The earlier `ordinary-mg-01` placed its nearby unit
about 4.6 m away, so use run 02 for the 2 m acceptance claim. One tested layout
does not prove all possible splash or collision geometries.

### AI firing

In `ai-ranges-02/ai_ranges/game_stdout.log`, M240 fired 100 rounds, M249 200,
and suppressed Negev 150 at about 100 m. All tested 300 m cases and M240 at
500 m fired zero. The eight summary lines are 695, 1052, 1409, 1966, 2323,
2681, 3188, and 3545. `reveal`, `doTarget`, and `doFire` were explicitly
commanded, so spontaneous target acquisition and accuracy are outside the test.

`ai-clear-lane-01/ai_clear_lane/game_stdout.log` records the chosen airfield
lane at line 372: start [8000,9500], heading 0, endpoints [8000,9800] and
[8000,10000]. All 21 terrain samples at 25 m spacing are 32.175 m ASL.
Both approximate 1.7 m eye rays have minimum sampled terrain clearance 1.7 m.
The player was moved near the lane to keep the area active; infantry remained
in native simulation on real terrain, and only the target was invulnerable.

Five independent 30-second windows still recorded zero shots and full
magazines: M240 at 300/500 m, M249 at 300 m, Negev at 300 m, and suppressed
Negev at 300 m. Result lines are 538, 690, 842, 994 and 1146; completion is
1148. The 500 m M240 and 300 m suppressed observations are compatible with
the authored maximum envelopes, but do not independently validate the boundary.
The unsuppressed 300 m firing acceptance remains open. Object occlusion, aiming
and native perception were not independently measured. Original-lane terrain
alone cannot explain the airfield result.

The same-lane control run, `ai-controls-01/ai_controls/game_stdout.log`, then
recorded zero shots and full magazines in all three native 30-second windows:
LoBo M240 at 100 m, stock M60 at 100 m, and stock M60 at 300 m. Result lines
are 539, 690 and 842. Diagnostic collection passed in 97.1 seconds with exit 0,
but the helper/lane setup has no positive firing control, including for the
stock weapon. Its zero-shot observations therefore cannot isolate a LoBo range
or repair defect. The separate original-lane 100 m firing result remains valid
within its own setup; extended engagement remains unverified.

Current command registration provides `knowsAbout`, `knownTargets`,
`getPosASL`, and camera creation, but no generic `lineIntersects`,
`terrainIntersect`, `checkVisibility`, or `getTerrainHeightASL` query was found.
`knowsAbout` reports contact knowledge, not current visibility. The lane sampler
uses `"camera" camCreate [x,y,0]`, whose position conversion adds
`SurfaceYAboveWater`, reads the ASL height immediately, then calls `camDestroy`.
It measures terrain, not tree or building occlusion.

### Tanks and hosted multiplayer

In `tanks-save-01/tanks_save/game_stdout.log`, lines 367-368 show the fresh
process and native load, lines 994-1066 show all eight comparisons and no
mismatches, and line 1084 records completion. Phase-one text did not survive:
the runner opens the shared `game_stdout.log` with `File::create` for each
process. Both phases' distinct screenshots survived. The retained reload
snapshots support persistence, but do not recover the earlier raw impact log.
The east/west turret screenshots show no observable turn, so animation remains
unverified. A command or a static image alone is not animation evidence.

The pre-change `wreck-fire-03` role scripts are preserved locally under
`wreck-fire-03/source-before-change`. Current fire source retains host actual
impact assertions but uses a three-second client wait and unconditional tank
class/locality/damage logs. Client replication is observational. In
`wreck-fire-04`, host M1/Merkava2 total damage was 0.0312112/0.0333987, while
both clients logged zero total/hull/turet damage. Own-player and wreck ownership
were correct. A separate propagation result is needed for item 4.

`wreck-fire-04/runner.stdout.log` identifies the exact timeout: clients 1 and 2
were waiting on `alive lwWreck == false`, while the host waited for glow count 1.
The native burn windows, remote fallback and natural cleanup were not reached.

The separate short diagnostic uses the same hosted mission and binary on all
three peers, with no damage or glow assertions. Under
`wreck-diagnostic-01/wreck_diagnostic`, server `game_stdout.log` lines 819,
837 and 840 record before/5-second/15-second observations; each client records
its corresponding observations at lines 790, 793 and 796. All clocks advance
by about 15.2 seconds. Host wreck damage becomes 1 and alive becomes false;
both clients remain at damage 0 and alive true. Glow count for `M1Smoker` is 0
on every peer. The host sees a generic `SmokeSource`, crater and tracks at
5 seconds, with that smoke absent at 15 seconds; clients see only the unchanged
Toyota. Generic smoke is not proof of the installed wreck-fire script.

Source inspection confirms that `camCreate` is local-only, but damaged entity
state has a separate network serialization path. Local projectile creation
therefore does not establish the cause of absent target damage propagation.
The installed Toyota killed handler still references the repaired fire script,
and native local destruction dispatches killed handlers. No script error in
these logs identifies the blocker. Record the observed propagation/glow failure
and leave ownership, remote damage, cleanup and tank replication acceptance
open rather than attributing the cause to LoBo content without reproduction.

## Final installed-state handoff

On 2026-10-11 the final UD repair check returned exit 0 with version
`2026-10-11.1` and zero changed members. Report-only mod doctor returned exit 0:
`Nothing to do: no known defect found in 124 pbo(s) (15 entry/entries not readable).`
All 124 installed archive hashes, the Bin roster config, and both runtime binary
hashes matched the pre-run inventory. No installed content or repair rules were
changed by this acceptance task. All game processes opened by the task were closed.

The earlier applied/baseline rule fingerprint is
`54a34c7b1144259b89d4c5522795f8b65f8ffc72b43a4dfddecb386e00f2c1fb`.
The final check from both the shared and isolated merged source checkouts reports
`e025a5228caebc861cc39a981e8b84322ffbc180de6625980ba3e0e3c43e38f3`.
These are byte-based source fingerprints, recorded separately; this task did not
identify the cause of the difference or apply a new repair batch. Both final
checks propose no changes, and the installed archive hashes are unchanged.
Evidence: `repair-check-final*.json`, `doctor-final.log`, and
`final-state-comparison.json` under the local evidence root.

The user-reported jet self-destruction at the original location and menu flicker
are not claimed fixed. The aircraft visual probe deliberately uses invulnerable
aircraft reset at 250 m altitude and does not test normal launch survival.

## Repair 2026-10-11.3 probes

The probes in `repair3/` check repair 2026-10-11.3 (LoBo #4, #5, #8 and #9) in
game. They were run on 2026-10-11 with the UD engine from
`D:\Arma_CWA\arma_CWR\dist\x64-win-rwdi` on the official Remaster data
(`D:\SteamLibrary\steamapps\common\ARMA Cold War Assault\Remastered`), not on
Classic. Results and limits per issue are in `../CONTENT-REPAIRS.md` under
"Runtime results"; local evidence is under
`D:\Arma_CWA\tmp\lobo-repair-3\runtime\<run>`.

| Entry point | Mission | Run | What it checks |
| --- | --- | --- | --- |
| `repair3/lobo_rbk500_bomblets.test.sqf` | `Repair3.Abel` | `rbk500-01` | #5: the Syrian MiG-21 CAS1 releases one RBK-500 through its fired handler; passes when the dispenser opens and `LoBoRBK500Bomblet` objects exist. A bomblet created directly first proves the counter can see them. |
| `repair3/lobo_mk20_rockeye.test.sqf` | `Repair3.Abel` | `mk20-03` | #4: one Mk-20 each from the A-4R, F-4 WW and IL-28 AS; passes when every drop opens a `LoBo_mk20Deployed` and bomblets exist. |
| `repair3/lobo_tail_numbers.test.sqf` | `Repair3.Abel` | `numbers-01` | #8: two empty copies each of F-15C, F-16 IAF, F-16 EAF, F-4 and IL-28 run their installed Init handlers; debug log level keeps any `Unknown label` line. The digits need visual review of the screenshots. |
| `repair3/lobo_wreck_glow.test.sqf` | `Repair3.Abel` | `glow-01` | #9: each glow class creates an object, and the killed handlers of a Toyota, a Merkava2 and a Mi-8 each create their glow. |
| `repair3/lobo_wreck_glow_unlisted.test.sqf` | `Repair3Toyota.Abel` | `glow-unlisted-01` | #9 activation case: the mission's `addOns[]` names only `LoBoTerror`, so `lobo_airammo` is not activated. |

`repair3/lobo_cluster_observe.sqf` is a shared helper that the two cluster
probes `#include`. It records each distinct bomblet about every ten frames, so
its counts are lower bounds. Both cluster probes hold the aircraft still and
invulnerable at 150 m: they check the script path, not ballistics, spread or
damage. The wreck glow probes do not cover burn damage, the "Put out fire"
action, the 150 s cleanup or the `say` sound. The missions are
`tools/lobo/runtime/Repair3.Abel` (its `addOns[]` activates `LoBo_airammo` and
the aircraft and vehicle addons) and `tools/lobo/runtime/Repair3Toyota.Abel`.

Like the issue #1 probes, these are machine-specific. Each TOML passes
`--dev --no-strict --mods-dir D:/Arma_CWA --mod @LoBo`, so it mounts the
repaired `D:\Arma_CWA\@LoBo` by absolute path. A relative mods dir would not
reach that folder from the Remaster data folder, and the Remaster folder holds
its own unrepaired `@LoBo` copy, which must not be the one tested. The TOMLs
carry the `full_cwa`, `lobo` and `exclusive` tags and are not part of a default
lane. Run them one at a time with retries, through the same wrapper, and pass
the Remaster data folder explicitly (the wrapper defaults to Classic):

```powershell
Set-Location -LiteralPath 'D:\Arma_CWA\arma_CWR'
$taskTri = 'D:\Arma_CWA\tmp\lobo-repair-3\runtime\trident-build\debug\tri.exe'
powershell -NoProfile -File tools/lobo/runtime/run-local.ps1 `
  -Tri $taskTri `
  -TestPath tools/lobo/runtime/repair3/lobo_rbk500_bomblets.test.sqf `
  -DataDir 'D:\SteamLibrary\steamapps\common\ARMA Cold War Assault\Remastered' `
  -Retries 2 `
  -OutputDir 'D:\Arma_CWA\tmp\lobo-repair-3\runtime\rbk500-02' `
  -TimeoutSeconds 540
```

Build `tri.exe` from the checkout that holds these files, as in "Reproduce
locally" above, and use a fresh output directory per run. Before and after,
`repair_lobo.py --mod 'D:\Arma_CWA\@LoBo' --compatibility ud --check` must exit
0 with zero changes and report `repair_version` `2026-10-11.3`.

## Repair 2026-10-11.4 probes

The probes in `repair4/` check repair 2026-10-11.4 (LoBo #13, #14, the open #9
items and the pilot wounds). R1 to R3 and R5 follow the runtime plan in the
repair .4 spec; R6 here is the wounded pilot probe, not the spec's optional R6
(the package's four modfolders on the UD engine), which has no probe. They run
like the .3 probes: UD engine, the Remaster data folder passed with
`-DataDir`, the installed `D:\Arma_CWA\@LoBo` mounted by absolute path, `-j 1`,
one run at a time, a fresh output directory per run. Start each new probe with
`-Retries 0`: a probe that does not boot shows the flashing main menu, and every
retry relaunches it (`agent_guidelines/flashing-main-menu.md`). Use
`-Retries 2` once it has booted. Results are in "Results, 2026-10-12" below
and, per issue, in `../CONTENT-REPAIRS.md` under "Runtime verification";
local evidence is under `D:\Arma_CWA\tmp\lobo-repair-4\runtime\<run>`.

```powershell
Set-Location -LiteralPath 'D:\Arma_CWA\arma_CWR'
$taskTri = 'D:\Arma_CWA\tmp\lobo-repair-3\runtime\trident-build\debug\tri.exe'
$out = 'D:\Arma_CWA\tmp\lobo-repair-4\runtime\tailgun-03'
powershell -NoProfile -File tools/lobo/runtime/run-local.ps1 `
  -Tri $taskTri `
  -TestPath tools/lobo/runtime/repair4/lobo_il28_tailgun.test.sqf `
  -DataDir 'D:\SteamLibrary\steamapps\common\ARMA Cold War Assault\Remastered' `
  -Retries 0 `
  -OutputDir $out `
  -TimeoutSeconds 540
python -B tools/lobo/runtime/repair4/check_logs.py r1 $out
```

The working directory matters: the test path and each TOML's `mission =` are
relative to it. The 2026-10-12 runs used the same commands from the repair .4
worktree, with `-GameDir` pointing at the main checkout's
`dist\x64-win-rwdi` (the wrapper's default).

| Entry point | Mission | Wrapper `-TimeoutSeconds` | What it checks |
| --- | --- | --- | --- |
| `repair4/lobo_il28_tailgun.test.sqf` | `Repair4.Abel` | 540 | R1, #13: IL-28 tail turret with no pilot (player gunner on the ground and at 400 m), an empty IL-28, a pilot-first IL-28 and the dead-pilot bailout. |
| `repair4/lobo_missing_refs.test.sqf` | `Repair4.Abel` | 540 | R2, #14: spawns both Hiluxes, the EAF, RJAF and 69th Sqn pilots, the MiG-21bis MR1 and both F-16 AA variants; side shots of each Hilux at 8, 35, 70 and 120 m, the EAF pilot from behind at 5 and 40 m, wounded backs, AIM-9 rails. Debug log. |
| `repair4/lobo_wreck_burn.test.sqf` | `Repair3.Abel` | 600 | R3, #9: isolated glow beside a vanilla Fire (noon and 23:00), "Put out fire" on it, then a Mi-8 wreck: no burn while seated, +0.04 steps on foot, "Put out fire" at the wreck, the 150 s cleanup, the say and scream sounds. Debug log; the TOML's `--log-categories` was meant to drop GRAPHICS, but `burn-01` still logged the GL33 draw lines (1.5 GB), so allow the disk space. |
| `repair4/lobo_pilot_wounds.test.sqf` | `Repair4.Abel` | 540 | R6, the `references` and `wounds` rules: each of the eight LoBo pilot classes healthy from behind, then wounded from the front, from behind and close behind. |
| `repair4/lobo_official_smoke.test.sqf` | `Repair4.Abel` | 540 | R5 S2 only: the official Remaster exe in a throwaway shadow with the rebuilt package (R1 case G, the R2 spawns, one M1Smoker from a killed Hilux). Never run it on the UD lane or against the Steam folder. |

`repair4/lobo_missing_refs_body.sqf` is the R2 body that both R2 and the R5 smoke
test `#include`; it is not a test on its own.

Two helpers, both read-only:

- `check_mission_addons.py` checks before a run that the mission's `addOns[]`
  activates the owner addon of every class the mission places and every class
  the probe spawns, with their `weapons[]`, `magazines[]` and the magazines each
  weapon accepts. It models the engine's addon merge and owner rules
  (`AddonSystem.cpp`, `World::ActivateAddons`, `AddonClosure.cpp`). A gap
  in a placed class rejects the boot; a gap in a spawned class is reported the
  same way, although at runtime the ZoneRegistry's later additive activation
  can hide it. On 2026-10-12 every repair .4 probe passed, and so did the .3
  `rbk500`, `tail_numbers` and `wreck_glow` probes. Two reports are expected:
  `lobo_wreck_glow_unlisted` (its mission leaves `LoBo_airammo` out on
  purpose) and `lobo_mk20_rockeye`, whose script-spawned EAF pilot carries
  `LoBo_throw2` from `LoBoWeapNad`, which `Repair3.Abel` does not list; its
  `mk20-03` run logged no `Access denied`, because the ZoneRegistry had
  activated `loboweapnad` by then. A copy of `Repair4.Abel` without
  `LoBoWeapNad` reproduces the boot rejection the guideline describes.

  ```powershell
  python tools/lobo/runtime/check_mission_addons.py --mod 'D:\Arma_CWA\@LoBo' `
    --base-config 'D:\SteamLibrary\steamapps\common\ARMA Cold War Assault\Remastered\BIN\config.bin' `
    --probe tools/lobo/runtime/repair4/lobo_pilot_wounds.test.sqf
  ```

- `repair4/check_logs.py r1|r2|r3|r6|r5 <output dir>` applies the spec's log
  criteria to `<output dir>\<test stem>\game_stdout.log` and prints one PASS,
  FAIL or INFO line each. Run on the .3 logs it reproduces the spec's baseline
  counts (327 `black_matte.pac`, 35 and 34 EAF `.pac`, 56 `rktf5\swtex.paa`,
  3 `soundss` lines).

Expected results, beyond runner exit 0:

- R1: `check_logs.py r1` passes (no `Zero divisor`, no error naming
  `tailgun.sqs`). Case A and case C3 must eject as the spec's 6.1 table says; a
  different bailout is a stop condition, not a probe to adjust.
- R2: `check_logs.py r2` passes: no `black_matte.pac`, `eaf\tex\back.pac`,
  `eaf\tex\legsbk.pac`, `rktf5\swtex.paa` or `soundss` line, and no load
  failure for the repointed targets. One `backfuse.pac` line per F-16 variant
  is expected. Visual: Hilux trim black at every distance, EAF pilot back
  textured at 5 and 40 m, AIM-9s as in the .3 `numbers-01` captures.
- R3: the assertions cover the seated phase (no damage to player or Jeep over
  28 s), the on-foot steps (each +0.04, at most floor(L/10)+2, steps after the
  second at least 9.5 s apart), "Put out fire" on both glows and the cleanup
  window. `check_logs.py r3` adds one `eng_lookout3` scream per step, at least
  100 `objects\fire` plays, plays continuing after the put-out and no load
  failure. A seated burn, a step other than 0.04 or more than one scream per
  step is a stop condition. The seated and on-foot phases start at t0+70 s,
  8 s later than the spec's outline, so the script's bullet window (60 s plus
  frame overshoot) has closed.
- R6: all eight classes spawn, stay alive at total damage 0.5 and reach a hit of
  at least 0.7 on body and legs. Wound textures need visual review: with
  `references` and `wounds` applied, every class shows wounds front and back,
  on body, arms and legs, in its own suit. EAF (reddish brown) and IAF grey
  (69th, 106th and 133rd Sqn) show red blood from the generated `_dr`
  textures; the olive classes (IAF 101st and 253rd Sqn, RJAF, SyAF) show the
  shipped olive `_d` art.

A wound texture only appears at a part hit of 0.7 or more (0.8 for the head):
`Man::WoundsAnimation` in `SoldierOldSimProxy.cpp`. `setDammage` sets every
part to the same value, so `setDammage 0.5` or `0.6` alone shows no wound. The
R2 and R6 probes keep total damage at 0.5 and raise the body, hands and legs
hits to 0.8 (head 0.85) with `setSelectionDamage`.

Before and after the runs, `repair_lobo.py --check` (pass `--tools` with the
main checkout's `PoseidonTools.exe`, since a worktree has no `dist`) must exit 0
with zero changes and report `repair_version` `2026-10-11.4`, and a recursive
listing of the Steam Remastered folder must not change.

### Results, 2026-10-12

UD engine `dist\x64-win-rwdi\PoseidonGame.exe` (SHA-256
`F0BCDA0852C3AAF84F00F23E23B0FF81275689DE0D937B9E9F46176384A85A0A`), Trident
`D:\Arma_CWA\tmp\lobo-repair-3\runtime\trident-build\debug\tri.exe` (SHA-256
`43D48C10D73AB8925880908FE4CBB4C704EF0B451236157D151E7DF5574A09CF`), official
Remaster data, the installed `@LoBo` after both repair .4 batches. Each final
run passed on its first attempt with `-Retries 0`, recorded `exit_code=0`,
passed its `check_logs.py` criteria with no script error, `Zero divisor` or
`Access denied` line, and closed every game it started; both executables had
the same hash after each run. The repair check gave 0 changed and 0 added
members before and after, and the Steam folder listing did not change.

| Run | Probe | Result | What it verifies |
| --- | --- | --- | --- |
| `tailgun-01` | R1 | Did not boot: `Repair4.Abel` lacked `LoBoWeapNad`, the owner of the placed EAF pilot's `LoBo_throw2` (`Access denied`, then `could not boot`). Kept as evidence; the mission was fixed. | Nothing. |
| `tailgun-02` | R1 | Passed in 18.3 s. | #13: no `Zero divisor` or `tailgun.sqs` error; C1 empty IL-28 and G (player gunner, no pilot, ground) keep an unmanned turret; C2 gives the pilot's group a living `LoBo_Egypt_Pilot` turret gunner; C3 (pilot killed at 400 m) puts that gunner in a `ParachuteEast`; A (player gunner, no pilot, 400 m) ejects the player (`ParachuteWest`) and leaves the turret unmanned. |
| `refs-01` | R2 | Passed in 47.6 s; 18 screenshots reviewed. | #14: zero lines for the five old names, no load failure for a repointed target, 2 expected `backfuse.pac` lines; Hilux and Hilux MG textured at 8, 35, 70 and 120 m (LOD bias 1), EAF pilot back and leg-backs at 5 and 40 m, AIM-9s, F-16s and MiG-21bis MR1 textured. Also logged: `lobo_airammo\lobo_mk81.p3d` not found (not a .4 target). |
| `wounds-01` | R6 | Passed in 27.3 s; 32 screenshots reviewed. | Pilot wounds: all eight classes alive at damage 0.5 with hits `[0.85, 0.8, 0.8, 0.8]` (head, body, hands, legs), wounded front and back in their own suit colour; no `lobo_pilot` texture failed to load. |
| `burn-01` | R3 | Passed in 197.1 s. | #9: S glow without mesh beside a vanilla Fire; D1 "Put out fire" then "Light fire" on the isolated glow; A no damage at 60 m; B seated 28.1 s at 6.86 m without burn; C five +0.04 steps in 33.3 s, at least 10.24 s apart from the second step, MEDIC line on screen; one `eng_lookout3` scream per step (6); D2 "Put out fire" offered and working at the wreck, particles and 29 `say` plays after it; 150 `objects\fire` plays; E glow deleted 163.7 s after t0, wreck kept. |
| none yet | R5 | Pending: the official exe smoke test runs separately. | Nothing is claimed for the official Remaster exe yet. |

In `burn-01` the D1 menu sampled 30 frames after the player was placed did
not list "Put out fire" yet; the runner retries a failing assert until its
timeout, and the retried `triAssertIncludes` found it.
