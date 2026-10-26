# Occupier forces: readiness for the Malden human playtest

Audit of branch `occupier-forces` at 36475a3 (= origin/occupier-forces; main and origin/main still at the base 16259cc), 2026-10-14. Read-only: nothing built, launched, edited or staged. Paths are relative to `D:\Arma_CWA` unless absolute.

**Verdict.** The binary and the shadow are current; no rebuild and no reinstall is needed. One launch blocker on this machine (PowerShell 7, item M1). Nothing in #97 or #76 blocks the session. Most of what the 12 steps exercise is covered only through test levers at 30x to 100x speed with a shielded player, so this session is the first natural, full-speed run of nearly every director decision.

## Findings by question

**1. Install currency: current, nothing to run.**
- Last commits touching the engine and the missions: f9d0444 at 2026-10-14 18:35:29 -0700 (`git log -5 -- arma_CWR/engine`, `-- arma_CWR/guerrilla-mode/mission`). After it only 36475a3 (roadmap). Last `guerrilla-mode/core` commit 5c7a771 (08:10); last `guerrilla-mode/runtime` commit 0785c10 (2026-10-12); `setup-guerrilla.ps1` f9e7c99 (2026-10-11).
- `arma_CWR/dist/x64-win-rwdi/PoseidonGame.exe` mtime 18:36:00, PoseidonTools.exe 18:36:01. Gate 14 started at 18:35:43, after the f9d0444 commit, with build exit 0, then "All tests passed (... 707 test cases)", then setup + install-missions on shadow-67 at 18:36:03 / 18:36:14, tri "passed 83/83", human suite OK (`tmp/of-gate-phase14.log`).
- Shadow content diffed against the sources: `Missions/Guerrilla.{Abel,Eden,Sinai,Lebanon80}` identical (`diff -rq`); `gmcore` identical except the excluded `portraits` (by design, `guerrilla-mode/install-missions.ps1:293-295`); `BIN/guerrilla-factions.hpp`, `BIN/guerrilla-hq.hpp`, `BIN/ud-mainmenu.hpp` byte-identical. `BIN/config-extra.cpp` differs on purpose: it is the package's own file with the two include lines appended (`install-missions.ps1:266-285`; the shadow's tail carries both includes). `@LoBo/Bin/config.cpp` rewritten 18:36 by the gate's setup.
- Working-tree engine sources equal HEAD (see 2), so the exe matches HEAD. If anything lands before the session, the redo is: from `arma_CWR`, `cmake --build build/win-x64-clang-rwdi --target PoseidonGame PoseidonTools -- -j 3`, then pwsh 7 `-File ./setup-guerrilla.ps1 -GameDir D:/Arma_CWA/tmp/remaster-shadow-67 -LoBoDir D:/Arma_CWA/tmp/remaster-shadow-67/@LoBo -Tools ./dist/x64-win-rwdi/PoseidonTools.exe` (it runs install-missions itself, `setup-guerrilla.ps1:327`).

**2. TargetFire.cpp: noise, not this branch's change, nothing to revert.**
- `git diff` is empty for it; `git hash-object` of the working file = `HEAD:` blob 44bcff5; `cmp` against HEAD says byte-identical. The " M" is a stale index stat entry (mtime 2026-10-14 09:40:45), not a content change.
- No branch commit touches it (`git log 16259cc..HEAD -- .../TargetFire.cpp` empty; last touched 0eeb38f, 2026-09-06). Progress log `tmp/occupier-forces-progress.md:91` records the fire_rogue agent editing it "outside its set: review"; `:92` and commit 77afbb1 (09:48:25, test-only, "test stance") show the fix landed in the test and the probe edit was reverted 7 minutes earlier.
- The exe (18:36) postdates the mtime, and the content is HEAD's anyway. A `git status` with index write access (or `git update-index --refresh`) clears the flag.

**3. Coverage per playtest step:** see "Should watch for" below, which names the test for each step and what it skips. Common patterns, all in `arma_CWR/tests/integration/scripting/`:
- Forced launches: purchases go through `gmCounterAttackLaunchFunded` (air_op, armoured_column, motorized, hq_assault, watched_route), `gmForcesForce ["strike", A]` (town_strike:11,69), `gmAirInsert` / `gmAirSortie` with the director off (`gmForcesTune ["enabled", 0]`: heli_insertion:35, jet_sortie:30, air_op:37).
- Time compression: `gmCounterAttackTune ["speed", 100]` (alarm_answer:43, base_retake:45, wave_base:47) or 30 (hq_assault:105); `directorInterval 1`; `refillHours` 0.01 to 0.05 (pool_on:36, refill_pushout:40); `waveLead 5`, `waveCluster 300` (wave_base:44-45).
- Shielded player: 16 of the 18 forces tests set `allowDammage false` or captive, so the player never fights. `guerrilla_forces_both_towns`, `_pool_on`, `_pool_off` and the forces save seq are headless.
- Natural director choices (armour, heli, strike, HQ, waves 2 and final) are proved in Catch2 only: `[occupierforces]` (68 cases in test_occupier_forces.cpp), `[packages]` (28, test_force_packages.cpp), `[air]` (8, test_occupier_air.cpp), `[counterattack]` (70, test_counter_attack.cpp).

**4. What the player sees, and how to turn the trail on.**
- The `-Dev` flag is enough. `play-guerrilla.ps1:56` adds `--dev`. `DirectorLogLevel()` returns INFO when `AppConfig::DevMode()` or `forcesLog` is on (`engine/Poseidon/Game/Guerrilla/OccupierForces.cpp:62-66`, `DirectorLog.hpp:7-12`). Guerrilla.Abel's description.ext authors no `forcesLog` (grep finds none; `occupierForces = 1` at :75). Run-time alternative: `gmForcesTune ["log", 1]`. The lines go to `arma_CWR\logs\game-<ts>.log` (`play-guerrilla.ps1:68-72`), not to the screen.
- On screen, a normal player sees: hints + journal notes for "We lost our hold on X", "X is back under occupation", "X is back in occupier hands" (`guerrilla-mode/core/scripts/capture.sqs:113-137`); civilian panic and half income after a strike (`civilians.sqs:32-37`, `economy.sqs:56`), with no hint; the win hint (`campaign.sqs:147`); the lose debriefing. The HQ loss shows **no hint**: the marker is deleted and the journal objective "Set up a headquarters" is re-armed (`market.sqs:197-206`). Waves, operations, alarm answers and air insertions have no player-facing message (D12, OCCUPIER-FORCES.md:64; #98).
- Every console name in `tmp/of-owner-summary.md:26` is registered: `gmOccupierForces` / `gmOccupierOps` / `gmOccupierIntel` (OccupierForcesCommands.cpp:552-554), `gmCounterAttacks` (CounterAttackCommands.cpp:484), `gmHqState` (GuerrillaBaseCommands.cpp:267), `gmZoneSim` and `gmZoneStruck` (ZoneRegistryCommands.cpp:651-652), `gmLandmass` and `gmRoadRoute` (RoadRoute.cpp:891,893). Also useful: `gmForcesTune` / `gmForcesForce` / `gmLegacyQrf` (OccupierForcesCommands.cpp:556-558), `gmOccupierAir` (OccupierAirCommands.cpp:426), `gmSimDiag`, `gmSeededBases`, `gmPlayerPresence` (ZoneRegistryCommands.cpp:653-657).
- Argument forms. `gmZoneSim` and `gmZoneStruck` take a zone INDEX: `gmZoneSim (gmZoneIndex "Village")`. Step 8's bare `(gmZoneStruck)` will error. gmZoneSim slots: 14 occCapture, 16 standing, 17 struckUntil (ZoneRegistryCommands.cpp:288-320). `gmHqState` slot 3 is hqCapture (OCCUPIER-FORCES.md:979). `gmCounterAttacks` slot 24 is airId, 25 the LZ, 26 airOrigin, 27 dismounted, 28 struck (CounterAttackCommands.cpp:161-176). The plan doc still says "24 slots" (OCCUPIER-FORCES.md:976); the code wins.

**5. Long-session risks.** No TODO / FIXME / "not run" / "unverified" markers in OCCUPIER-FORCES.md section 17 (:1701-1764) or OFFSCREEN-CONTRACT.md. The open items are design or content: Showcase capture contest, #94, D12/#98, #76 remainder, the Compound parked.
- **Ops ledger.** MaxOpRows 32 (`OccupierForces.hpp:410`). Only settled rows are pruned. Past 128 rows the oldest finished row is force-settled to the pool (`OccupierForces.cpp:1253-1300`). Covered by unit only (test_occupier_forces.cpp:3350, 3399, 3445).
- **Leftovers.** Retried every 30 s (`CounterAttack.hpp:537`). A group with corpses leaves only when the player is beyond cacheRadius + hysteresis AND every body is unseen, and dead units keep their group slot (`CounterAttack.cpp:3084-3117`). A player who lingers at a battle site holds those slots, so watch the `[gmsim]` maxSide (budget 40). An intact abandoned hull goes only when unoccupied and unseen; wrecks stay (`:3068-3082`); a stolen truck stays a Leftover.
- **Pending deposits.** A retaken town's refill stays pending while the zone is in the bubble, unless the 3 x 3 grid and the next spawn points all rate hidden (`GarrisonCache.hpp:194-205`, `ZoneRegistry.hpp:233-242`). Expected look: a town retaken in front of you is empty (OCCUPIER-FORCES.md:1819).
- **Slow knobs** (section 16):
  - Pool refill 4 h (:1638); tokens max 2 at 1800 / 2700 / 3600 s by band, none in SPENT (:1646); opsMax 2 (:1647); town cooldown 1800 s (:1651).
  - Foot speed 1.5 m/s (:1659); birth give-up 600 s (:1691). An alarm answer to a bubble zone took 1822 s on foot (progress:75); born at its base by owner decision (OCCUPIER-FORCES.md 17, "Closed 2026-10-14").
  - Waves at 0.75 / 0.50 with a 600 s lead (:1683, :1685). The director sleeps until the second distinct town.
- **Hull packages need stored spare at the source** (progress:23; review M1 in the status row, OCCUPIER-FORCES.md:18). Armour may simply not appear.
- **Multi-hour only:** pool exhaustion and the single 10 % SPENT exit (proved in unit at :556, :632); ledger and Leftover growth; repeated save/load; wave 2 and the final stand.

**6. Save/load.**
- Tested, on direct-launch missions with `triSaveGame`:
  - `guerrilla_forces_save_reload.seq` (01_wake / 02_reload, headless, Abel): pool, posture, tokens, ops.
  - `guerrilla_forces_air_save_reload.seq` (01_inflight / 02_resume): saved mid-flight only.
  - `guerrilla_bases_detect_eden_save.seq`: Eden, not Abel.
  - `guerrilla_save_reload.seq`: Demo-world mission `tests/integration/missions/guerrilla_persist.Demo`, not run locally (`tmp/of-fork-issue.md:27`).
  - The menu path: `tests/integration/ui/remaster/remaster_ud_guerrilla_continue.seq` (5 phases: Save action, Continue, Abort snapshot), with no operations in flight.
  - Unit: `[save][load]` cases (test_occupier_forces.cpp:1017, 1112, 2171, 2787, 3262).
- Not tested:
  - an armoured column mid-route (skipped, `tmp/of-fork-issue.md:26`);
  - a save during an insertion's landing or unload;
  - a wave inbound across a save;
  - the campaign "Save Game" action or the Continue snapshot with live forces;
  - a real pre-branch save (legacySave is driven by a lever, `gmForcesForce ["legacySave", 1]`, alarm_answer:22,91, plus the unit case :3262);
  - several reload cycles across hours.
- **Modless body rebuild** (5c7a771 campaign.sqs rebind, e8ef485 engine `playerOnVars` repoint) runs only when a save made WITH @LoBo is loaded WITHOUT it. The planned command has no `-LoBoDir`, so Malden saves stay modless end to end and never take that path. A Malden save made and loaded with @LoBo is also not a rebuild. Mixing the two launches is what triggers it (guarded by `guerrilla_lobo_body_save_reload_modless.seq`, tag full_cwa).

**7. #97 / #76 / #98 / #94.**
- Nothing is blocking or needs the owner first. The #97 body says "none blocks the PR" (`tmp/of-fork-issue.md:1`). Its body is stale: the K probe and the air re-plan ledger items closed in comments 2 and 3. Comment 4 (2026-10-14T23:25) leaves only "the human session (plan 12.3), `editor_wizard_finish` (needs packages/Game), `objNull distance <pos>` raising (upstream), dealer respawn drift".
- #76's remainder (Sinai playRegion, outpost ladders, two names, old Eden/Sinai saves, plan-13 yields, Sinai soak) does not touch Malden: "Abel's authored Airfield, Le Port Harbour and Compound cover the detections" (#76 comment 2026-10-15T02:02).
- #98 (journal) and #94 (HQ respawn) are deferred design. Death still opens the stock retry screen (#94 body).

## Must do before the session (ranked)

1. **Launch with PowerShell 7.** `play-guerrilla.ps1:15` is `#requires -Version 7.0`. No `pwsh` is on PATH and `C:\Program Files\PowerShell` does not exist; pwsh 7 is at `C:\dev\vcpkg\downloads\tools\powershell-core-7.6.2-windows\pwsh.exe` (`tmp/of-agent-common-rules.md:13`). From `D:\Arma_CWA\arma_CWR`:
   `& 'C:\dev\vcpkg\downloads\tools\powershell-core-7.6.2-windows\pwsh.exe' -File .\play-guerrilla.ps1 -GameDir D:\Arma_CWA\tmp\remaster-shadow-67 -GameExe .\dist\x64-win-rwdi\PoseidonGame.exe -Dev`
   Note the printed PID and log paths.
2. **Freeze the target for the whole session.** No rebuild or reinstall is needed now. But no agent may run a gate meanwhile: the gate builds into `dist/x64-win-rwdi` and reinstalls shadow-67 (`tmp/of-gate-phase14.log` "--- setup" / "--- install-missions"), which would change gmcore and the missions under a running game. No game, tri or build process is running now.
3. **Keep the mod flags identical for every launch** (as planned, no `-LoBoDir`: vanilla EAST occupier, `Guerrilla.Abel/description.ext:100`, so Mi17 / Mi24 / Su25). Pick EAST at new game if you want the airframes in steps 5 and 8 to match `tmp/of-owner-summary.md`.
4. **Use the 12 steps as the record sheet, not FULL-SCALE-SESSION.md A3.** A3 predates the forces work (last change 3612f3c, 2026-10-12). Its step 4 "nothing else changes owner" and step 5 "the QRF stood down" (`guerrilla-mode/FULL-SCALE-SESSION.md:67-68`) are superseded by D5 drains and native alarm answers. HUMAN-TEST-RUN.md has no forces rows. Record the commit (36475a3), data dir, mods, view distance and resolution (FULL-SCALE-SESSION.md:54).
5. **Default view distance, 1920x1080** (FULL-SCALE-SESSION.md:47). A raised view distance re-opens 800 m garrison pop-in (OCCUPIER-FORCES.md:1819-1821).
6. **For step 9, locate a pre-branch Abel save in the game profile.** Without one, only the lever reproduces legacySave. A save from before bcca80e also derives legacySave (`OccupierForcesCommands.cpp:39-44`).
7. **Decide up front whether steps 10 (wave 2, final stand) and 11 may use levers.** At the real knobs they may not be reached in one sitting. Levers: `gmForcesForce ["wave", n]`, `gmHqForceCapture 100`, `gmForcesForce ["strike", gmZoneIndex "<town>"]`. They bypass the director's natural choice, so log any lever use as such.
8. Fix the argument forms on your crib sheet (finding 4): `gmZoneSim (gmZoneIndex "<zone>")`, `gmZoneStruck (gmZoneIndex "<town>")`, the LZ is `gmCounterAttacks` slot 25.

## Should watch for during the session (per step)

1. **First town.**
   - Covered by: `guerrilla_counter_attack_*` (live, offscreen, unseen_spawn, watched_base); unit W.
   - Gaps: tests set `firstTownChance 0`; the 0.4 roll is unit-only.
   - Watch: a LEGACY row with id 0 in `gmOccupierOps` (OccupierForcesCommands.cpp:51-60); an `INFO CounterAttack:` line in the log.
2. **Director wakes, drain.**
   - Covered by: `guerrilla_forces_both_towns` (headless, directorInterval 1), `_passerby_revert`, `guerrilla_forces_save_reload.seq` 01_wake.
   - Watch: slot 4 posture AWAKE and slot 3 tokens in `gmOccupierForces`; the "is back under occupation" hint when a patrol drains an empty town.
3. **Watched arrival.**
   - Covered by: `_no_popin`, `_wiped_release`, `_motorized` (lever launch, 2.7 km), `_watched_route`, which asserts the re-source and the birth, not the arrival (`tmp/of-fork-issue.md:25`).
   - Watch: anything appearing within 250 m or beside you; a record stuck in HOLDING past about 600 s (give-up, then re-source, then cancel unseen); a truck that does not brake before dismounting (DoGetOut at speed once killed 5 of 6 riders, progress:22).
4. **Armour.**
   - Covered by: `_armoured_column` (lever: Ural + T72 from the Harbour); `[packages]`.
   - Gaps: the director never bought a column in any tri test.
   - Watch: `gmOccupierOps` [11]/[12] hull and escort classes; whether a column ever appears when STRONG (stored-spare refusal, M1); the escort waiting 150 m / 60 m.
5. **Helicopter insertion.**
   - Covered by: `_air_op` (lever, director off), `_heli_insertion` (gmAirInsert), `_air_save_reload.seq`; unit re-plan :1596.
   - Spike result: 44 of 45 landed within 100 m, 18 of 18 on the final build (AIR-SPIKE.md:13-17).
   - Gaps: a natural re-plan in play (every base route watched) is untested.
   - Watch: landings over 100 m from slot 25; one airframe and no escort by design (OCCUPIER-FORCES.md:1786); airframes that never leave (departTimeout 900 s backstop).
6. **Base retake.**
   - Covered by: `_base_retake` (speed 100, invulnerable hold squad on hold fire), `_wave_base`.
   - Watch: occCapture (gmZoneSim slot 14) rising 6 x min(crew, 3) per 3 s tick, frozen while both sides are present, decaying 10 with resistance alone.
7. **HQ.**
   - Covered by: `_hq_assault` (lever HQ_ASSAULT, speed 30, gmHqForceCapture), unit :2345.
   - Gaps: the director's own HQ choice (STRONG, in a wave, or the final push only) in play.
   - Watch: there is no hint when the HQ falls (finding 4), so judge whether the loss reads as a setback (12.3); the garage jeep wrecked; the cache emptied; re-establish anywhere.
8. **Air strike.**
   - Covered by: `_town_strike` (forced, bought-when rule bypassed, player at the Camp more than 1.5 km away, so probably abstract), `_jet_sortie` (lever, director off); unit :2919, :2958, :3105.
   - Gaps: a live strike only flies in your bubble.
   - Watch: support/standing -40 there and -10 within 2 km, panic, `gmZoneStruck` true for 1200 s.
9. **Alarm answer.**
   - Covered by: `_alarm_answer` (DORMANT, speed 100, legacySave by lever); unit :3143, :3195.
   - Watch: at real speed the answer may take about 30 min on foot (progress:75); no scripted QRF; a slow answer arriving late can contest a later capture (section 17).
10. **Waves.**
    - Covered by: `_wave_base` (wave 1 only, lead 5 s, cluster 300 m, tokens 0, script flips); waves 2 and final stand unit-only (:2518, :2592, :2646).
    - Watch: slots 6 / 15-22 in `gmOccupierForces`; the hub landing; no win while any enemy military zone is held.
11. **Lose.**
    - Covered by: `ui/remaster/remaster_ud_guerrilla_forces_lose` (owners flipped by script, `gmHqForceCapture 100`, campaignEndDelay 6).
    - Gaps: never reached through a real assault.
    - Watch: the EMLoser debriefing; a save made in that state reloads into the defeat (OCCUPIER-FORCES.md section 19 item 4).
12. **Frame time and landings.**
    - Covered by: `ui/remaster/remaster_ud_offscreen_budget_abel` (records frame time, asserts only the 130 bodies / 40 groups budget).
    - Gaps: no test measures frames during a column or an insertion.
    - Watch: the dev panel Profile tab and `gmSimDiag`; `[gmsim] budget:` WARN lines; stalls.

## Not covered by any test, first exercised by this session

- Real-speed travel across the island for every package (tests run 30x to 100x or launch beside the target), and arrivals on long routes.
- Multi-hour pool economics: the 4 h refill, band transitions, and SPENT starvation in play (tri uses refillHours 0.01 to 0.05).
- The director choosing armour, a helicopter, a strike or the HQ by itself. All of these are lever-launched in tri.
- Waves 2 and the final stand in-game; real wave lead (600 s) and real cluster radius (2000 m).
- Lose state through a natural HQ assault; a strike on a town the player is standing in.
- Save/load with live forces through the campaign Save action and Continue; an armoured column mid-route; an insertion during landing; a wave in flight; a real legacy save; repeated cycles.
- A player who fights and can die (16 of 18 forces tests shield him), and death mid-operation (stock retry, #94).
- Interaction with undercover (disguised HOLD/CAPTURE during a live retake), recruits holding a zone, the market and garage after an HQ loss, journal pages, and traffic convoys alongside columns (traffic runs in tests but nothing is asserted about it).
- Ledger growth past 32 rows and Leftover accumulation over hours (unit only).

## Noise to ignore

- `git status`: TargetFire.cpp " M" (byte-identical to HEAD, finding 2). Foreign uncommitted files: `arma_CWR/guerrilla-mode/README.md`, `arma_CWR/voice-lines/README.md`, `arma_CWR/tools/lobo/lobo-modfolder-assembly.txt`, `roadmap/roadmap.yaml` (plus its CRLF warning), untracked `GATE-ZERO-TEST-MISSION-PLAN.md`, `REALISTIC-RECRUITING.md`, `voice-lines/samples/compare_20261012/` (`tmp/of-agent-common-rules.md:8`).
- Known pre-existing reds (`tmp/of-agent-common-rules.md:17`):
  - 5 unit failures in test_map_legend_localized / test_equipment_localization (missing packages/Remaster);
  - tri `guerrilla_outfit_scope_label_e2e` and three stock-menu wizard tests;
  - `remaster_ud_guerrilla_sinai_lobo` flakes once sometimes.
- Plus `editor_wizard_finish`, which needs packages/Game (#97 comment 1).
- In play: `objNull distance <pos>` raising a dimension error is upstream command behaviour (#97 comment 4); dealer respawn drift of a few metres per death (#97 comment 4); DEBUG-level director lines appear only with `--log-level debug`.
