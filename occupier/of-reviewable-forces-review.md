# Review: origin/reviewable_forces (ba2aa5c) against occupier-forces (fb9dde1)

Independent review, 2026-10-15. Main tree untouched; builds and the trial merge ran in a
detached scratch worktree (`tmp/wt-reviewable`, removed, no branch). No tri, nothing committed.

## Verdict in one paragraph

The five engine fixes are real bugs on occupier-forces HEAD (all reproduced red against HEAD
production code that already includes acf2311), each fix is correct and small, none breaks a
D-decision, and the merged tree (reviewable_forces + occupier-forces) builds and passes every
forces, guerrilla and controls tag. Take them. Do not take the branch wholesale: about half
of its diff is branch-process documentation (a root `REVIEWABLE_FORCES.md` with 9 em dashes,
three campaign logs under `roadmap/` with another machine's paths, a branch pointer in the
root README) and a policy change (the Remaster-only runtime guard plus rewritten
`CLAUDE.md` target sections) that is outside the forces scope and needs the owner's say.
Recommended: bring the code and test commits onto occupier-forces by a 3-way apply (section 5),
leave the docs behind, and put the Remaster runtime change in its own PR after #95.

## Findings, most severe first

1. **[HIGH, merge hygiene] Branch-only material would land on main.** A plain merge carries
   `REVIEWABLE_FORCES.md` (repo root, 203 lines, an em dash in each of its 9 dated
   headings), a root `README.md` line
   "For this review branch, see [the branch guide ...](REVIEWABLE_FORCES.md)", and
   `roadmap/forces-improvement-progress.md`, `roadmap/forces-independent-review.txt`,
   `roadmap/regression-test-audit.md`. `roadmap/README.md` defines the folder as
   `roadmap.yaml` + `generate.py` + generated outputs; these three are campaign logs that cite
   `/root/mobile-dev/CWR`, `/tmp/cwr-forces-*`, Codex task ids, "Astra" agents and a "friend
   archive" zip. They do not belong in the repo. The roadmap.yaml log also gains a dangling
   line "started F062/F078 follow-up on reviewable_forces at 9cc010d" for work that is not on
   the branch. The three roadmap files themselves contain no em dashes.
2. **[HIGH, operational] A merge in D:\Arma_CWA will be refused as things stand.** The branch
   edits `roadmap/roadmap.yaml` and `arma_CWR/guerrilla-mode/README.md`, both of which carry
   another session's uncommitted edits in the main tree (`git status`: ` M` on both). Any
   `git merge` touching them aborts with "local changes would be overwritten". The apply plan
   in section 5 avoids both files.
3. **[MEDIUM, policy] The Remaster runtime change rewrites owner policy text.** `ba2aa5c` edits
   the root `CLAUDE.md` ("Target platform: the full Remaster only", "Classic 1.99 is
   unsupported. Setup and the UD launch/install helpers reject Classic data") and
   `arma_CWR/CLAUDE.md`. The direction matches the 2026-10-11 policy, but the wording moves
   from "being dropped" to "refused", and CLAUDE.md is the owner's file. Not forces scope;
   own PR, owner sign-off. Details in section 2.
4. **[MEDIUM, contract and plan doc drift] Two fixes change documented rules without the docs.**
   OFFSCREEN-CONTRACT.md row 1b says abstract presence counts forces "holding the post, not
   suspended (`CounterAttack::AbstractPresence`)"; after 93cd5c9 it is "not inside the
   player's current bubble at the zone census" (the stored flag lags one zone tick).
   OCCUPIER-FORCES.md lines 121 to 122 say k is the larger of `_townsTaken` and "the non-exempt
   towns held now"; after 384b283 it is "held before this event in the tick's batch". The
   common rules require the contract to stay true in the same change: add one sentence each.
5. **[MEDIUM, correctness, fixed by the branch] Births in one Step overran the body and group
   budgets** (b6901d6). Most consequential of the five, and more so after acf2311 (600 s
   STRONG cadence, three STRONG operations). Section 1.3.
6. **[LOW to MEDIUM, fixed] Abstract forces drained a town for one zone tick after the player
   entered its bubble** (93cd5c9), the "invisible retake in front of the player" class the
   contract forbids. Section 1.2.
7. **[LOW, fixed] Simultaneous captures inflated `_townsTaken` / k** (384b283). Section 1.1.
8. **[LOW, fixed] A configured floor like 0.3 rejected its exact boundary** (in b6901d6, the
   OccupierForces.cpp hunk). Section 1.4.
9. **[LOW, fixed, not forces scope] Cleared controls were resurrected on v3/v4 -> v5
   migration** (bad1530, 44a8254). Section 1.5. Belongs with the controls work, fine to ride.
10. **[LOW, tests] The presence tests re-implement the GatherInputs wiring** in their fixture
    (`TickZone` calls `AbstractPresence(..., &view)` itself), so the one production line in
    `ZoneRegistry::GatherInputs` that passes `&currentView` is not pinned. A future revert of
    that line stays green. Section 3.
11. **[NIT] `UDDataDir.psm1` assigns to `$matches`** (a PowerShell automatic variable) inside
    `Resolve-UDPackageChild`; harmless under StrictMode, rename to `$hits`. The Steam folder
    itself passes `Assert-UDRemasterDataDir`, so the new guard does not enforce the "never run
    setup against the Steam folder" rule (pre-existing gap, not a regression).

## 1. Engine fixes

### 1.1 384b283 "exclude pending town flips from capture history floor"
- Bug claimed: `ZoneRegistry::Simulate` runs `EvaluateTick(in, fired)` (applies every flip of
  the tick) and only then `CounterAttack::OnZoneEvents(fired)`. HEAD `OnTownFlipped`
  (CounterAttack.cpp:1153 at HEAD) computes `held = HeldTowns(registry) - 1`, which for the
  first of two same-tick flips already includes the second town. With no prior towns: first
  event before = max(0, 2-1) = 1 (should be 0), `_townsTaken` 2; second before = 2 (should be
  1), `_townsTaken` 3 (should be 2).
- Real on HEAD: yes. `[town-capture-batch]` against HEAD production: "f.counts[0] == before
  for: 1 == 0", "f.counts[1] == before + 1 for: 2 == 1", "f.counter.TownsTaken() == before + 2
  for: 3 == 2"; 1 case, 9 of 33 assertions failed.
- Fix: `OnZoneEvents` pre-counts eligible captures in the batch (`countedTownFlip` mirrors
  `HeldTowns` eligibility: CITY, not `garrisonExempt`, owner resistance) and passes the count
  of later ones; `held = HeldTowns - 1 - laterTownFlips`. Correct and minimal; the default
  argument keeps every other caller unchanged. Impact is mostly the legacy answer path and the
  saved `_townsTaken` (the director ignores k), so LOW.
- D-decisions: none touched. Plan doc wording drift (finding 4). acf2311: no overlap.

### 1.2 93cd5c9 "sample current player bubble for abstract town presence"
- Bug claimed: `GatherInputs` (ZoneRegistry.cpp ~3405) reads `AbstractPresence`, which at HEAD
  (CounterAttack.cpp:1089) filters on `!r.suspended`, a flag only `Advance`'s CAHolding case
  refreshes (`r.suspended = InBubbleAt(registry, view, dest)`, CounterAttack.cpp:2573), and
  `Step` runs after the zone tick. On the tick the player enters, the abstract force still
  drains support / raises occCapture with nothing visible.
- Real on HEAD: yes. With the new signature shimmed but the HEAD predicate kept: 3 of 3
  `[town-presence-order]` cases red, e.g. "AbstractPresence(f.registry, 0, &near) == 0 for:
  5 == 0", "f.Target().support == Approx(60.0f) for: 55.0f == Approx( 60.0 )".
- Fix: an optional `const CounterAttackView*`; when given, the filter is
  `!InBubbleAt(registry, *currentView, DestOf(registry, r))`, the exact predicate `Advance`
  uses, so the census and the force tick agree. `GatherInputs` builds the view from the same
  `GetRealPlayer()` sample `BuildView` uses. Correct and minimal; the leaving side resumes one
  zone tick earlier, which is harmless. Contract wording drift (finding 4). acf2311: none.

### 1.3 b6901d6 "account counterattack births within each budget census" (CounterAttack.cpp hunk)
- Bug claimed: `Step` passed one `view` to every `Advance`; `GoLive` checks
  `view.liveBodies + bodies > budgetLiveBodies` and `view.occGroups + groups >
  budgetSideGroups` (CounterAttack.cpp:1940 to 1941 at HEAD) against the pre-tick census,
  while `LiveSquads()`/`BornOps()` are recomputed from records. Two births in one tick both
  passed. The contract's live-entity budget was breachable.
- Real on HEAD: yes. `[birthbudget]` against HEAD: 3 of 5 cases red (20 assertions), e.g.
  "f.ca.Get(1)->phase == CATravel for: 3 == 1", "(column ? ops.mounts : ops.squads) == 1 for:
  2 == 1". The two green ones (partial births, failed births) guard against over-charging.
- Fix: a local `tickView` copy charged on each CALive transition: groups for
  group/crewGroup/escortCrewGroup, bodies for strength (except `CASpawnGarrison`, whose bodies
  are already counted) plus actual crews. Caller's view untouched; `FinishHull` keeps the
  original. Correct and minimal. Leftover/home-hull births are not charged (they run after the
  loop) and the strike body cap omission is pre-existing; the branch says so itself.
- acf2311: no textual overlap (acf2311 only changed `OpsCap`); semantically complementary.

### 1.4 b6901d6, OccupierForces.cpp hunk: the pool floor boundary
- HEAD `CanOperateIn` (OccupierForces.cpp ~435) tests `(float)pool < floor * poolStart`.
  0.3f x 50 rounds to 15.00000095f, so pool 15 is rejected; 0.3f x 100 rounds above 30.
  By my arithmetic the default 0.2 floor rounds to exact integers at poolStart 25, 35 and 50,
  so the defect shows mainly with authored floors.
- Real on HEAD: yes. `[poolfloor]`: "f.of.CanOperate() for: false", "f.of.Spend(1, "at
  floor") for: false"; 4 of 16 assertions.
- Fix: compare `pool / poolStart < floor` when poolStart > 0. Correctly rounded division of
  the exact ratio lands on the same float as the parsed decimal, so equality is exact. Correct.
  D-decisions: the table at OCCUPIER-FORCES.md:809 says "none below forcesPoolFloor", which the
  fix now honours exactly. acf2311 also edits OccupierForces.cpp; it auto-merged cleanly.

### 1.5 bad1530 + 44a8254: cleared controls across migration
- Bug: HEAD `Save` skips actions with no bindings; `Load` of any version < 5 seeds defaults,
  so a cleared action in a v3/v4 file came back bound on the v5 migration. Also the lean
  fallback bound PageUp to MoveUp whenever `BindingCount(UAMoveUp) == 0`, overriding an
  explicit clear, and counted positional empty slots.
- Real on HEAD: yes. `[ContextControlsConfig]`: 4 of 19 cases red (173 assertions), e.g.
  "BindingCount(UAMoveUp) == 0 for: 2 == 0 with 'version := 4'".
- Fix: `Save` writes an empty row for cleared actions; `Load` clears a missing row for
  3 <= version < 5 unless the action postdates the file (`ActionAddedAfterV3`: ZoomTemp in v4,
  LeanLeft/Right in v5). I checked the history: v3 arrived with 3ab0930 (push-to-talk, map
  zoom), v4 with 04e977c (ZoomTemp), v5 with ca9bd9d (lean); no other `UserAction` was added
  since v3, so the list is complete. The PageUp fallback now needs a Q actually removed from
  MoveUp and no valid binding left. Correct. Side effect: every saved file now carries empty
  rows for every unbound action in every context (bigger file, cosmetic).

### Trial merge (reviewable_forces + occupier-forces)
`git merge --no-commit --no-ff occupier-forces` in the detached worktree:
CounterAttack.cpp, OccupierForces.cpp, test_counter_attack.cpp auto-merged;
"CONFLICT (content)" in `test_occupier_forces.cpp` and `roadmap/roadmap.yaml` only.
- `test_occupier_forces.cpp`: EOF append on both sides (the branch's `[poolfloor]` case vs
  acf2311's cadence / ops cap / spare cap / arrival rule block). Resolve by keeping both,
  branch case first, closing brace added. No semantic conflict.
- `roadmap.yaml`: the occupier-forces item's `commits:` list (keep the union: add
  "384b283", "93cd5c9", "b6901d6" after "acf2311") and its `log:` head (keep both lines,
  newest first). The merge was aborted and the worktree removed afterwards.

### Build and unit results (merged tree, conflicts resolved as above)
`cmake --preset win-x64-clang-rwdi -B build/review-rwdi` then
`cmake --build build/review-rwdi --target PoseidonTests -- -j 3`: exit 0.
```
Filters: [counterattack],[occupierforces],[packages],[air],[market],[portrait],[ContextControlsConfig],[town-capture-batch],[town-presence-order],[poolfloor],[birthbudget]
All tests passed (279920 assertions in 254 test cases)
Filters: [guerrilla],[campaignend],[save],[journal]
All tests passed (2261330 assertions in 731 test cases)
[controls]: All tests passed (28 assertions in 7 test cases)
```
Then the five production files restored to occupier-forces HEAD (signature shim only for
`AbstractPresence`), incremental rebuild, branch tests:
`[town-capture-batch]` 1 of 1 failed; `[town-presence-order]` 3 of 3 failed;
`[birthbudget]` 3 of 5 failed; `[poolfloor]` 1 of 1 failed; `[ContextControlsConfig]` 4 of
19 failed; `[market],[portrait]` "All tests passed (262494 assertions in 32 test cases)".
Market/portrait are tests-only strengthening (production unchanged), so green on HEAD is
expected.

## 2. Runtime change d773dd5 (require full Remaster data)

- What it refuses now: `Assert-UDRemasterDataDir` (UDDataDir.psm1) requires an existing
  folder with at least one official marker (root `dtaExt`, `bin/remaster.bin`,
  `bin/mainMenu.hpp`, `bin/splashLogo.hpp`), no Demo sign (`PoseidonGameDemo(.exe)`, `demo`),
  plus `worlds/abel.wrp`, `bin/resource-extra.cpp`, `bin/config.bin`, `bin/resource.bin`,
  `dta/data3d.pbo`, all case-insensitive. `bin/remaster.cpp` and copied engine binaries are
  deliberately not markers. Classic is refused (no marker). `Get-UDDefaultDataDir` drops the
  warned Classic fallback and throws; an invalid `UD_DATA_DIR` throws without fall-through.
  Called by setup-guerrilla.ps1 (Classic mode deleted), install-missions.ps1, run-game.ps1,
  run-game-with-mods.ps1, play-guerrilla.ps1.
- Shadows: I ran the branch module read-only against every folder, in pwsh 7.6.2 and Windows
  PowerShell 5.1, identical results: OK for remaster-shadow-67, -b1 to -b4, -of1 to -of6 and
  the Steam Remastered folder; FAIL for `ARMA Cold War Assault [Classic]` ("No official
  Remaster marker") and the Demo ("Remaster Demo package is not supported
  (PoseidonGameDemo.exe ...)"). Default resolves to `D:\Arma_CWA\tmp\remaster-shadow-67`.
- Callers: `tmp/of-gate.ps1` calls setup-guerrilla.ps1 and install-missions.ps1 on
  shadow-67, which passes, so the gate keeps working. run-game / play-guerrilla /
  run-game-with-mods keep working on any shadow. Tri does not use these scripts; `--data-dir`
  and toml pins are untouched, so every tri lane runs as before (an unpinned lane pointed at
  Classic by a local `.trident.env` still runs; only `.trident.env.example` changed to
  `packages/RemasterUD`).
- `tests/runtime/test-setup.ps1` (which now calls `test-data-dir.ps1`) on Windows pwsh 7.6.2
  from the worktree: exit 0, last line "PASS: Demo packages and incomplete Remaster folders
  refused before any write (dtaExt, remaster.bin, mainMenu/splashLogo, any case); Remaster
  reruns pass." (the branch had only run it on Linux).
- Policy fit: in line with "Remaster is the target" (no new Classic shims; this retires the
  Classic install path). Left behind: manifest `skip` entries, `guerrilla-mode/runtime/classic`,
  `thirdparty/classic-ui` are now dead inputs; fine as a follow-up. The CLAUDE.md rewrites are
  the part that needs the owner (finding 3).

## 3. Tests

- Behaviour, not implementation: capture batch drives `EvaluateTick` + `OnZoneEvents` and
  checks the k each listener call receives and `TownsTaken()`; birthbudget drives
  `LaunchFunded` + `Step` with fake ops and checks phases and spawn counts, including the next
  census not double charging, partial births charging only survivors, failed births leaving
  room, strikes, and garrison detachments charging groups only. Controls use real v3/v4
  layouts. Market/portrait catch three real mutations (stale `out.Clear()`, wrong cancel,
  partial teardown) per `roadmap/regression-test-audit.md`; I did not rerun the mutations.
- Weak spots: the presence tests replicate the `GatherInputs` wiring (finding 10); the exact
  `support == 55` and `occCapture == 18` pin current drain tuning (fine but brittle);
  `FakeOps` lost `final` so `BudgetOps` can derive (harmless).
- Red after acf2311? None. Every branch case passed in the merged build, and acf2311's own
  cadence, ops cap, spare cap and arrival cases passed beside them. The branch tests do not
  depend on token intervals or `forcesOpsMaxStrong`: birthbudget runs CounterAttack with
  OccupierForces inactive, so `OpsCap` uses the local `forcesOpsMax`; poolfloor drives
  `SetPool`/`Spend` directly.

## 4. Docs

- `roadmap/forces-improvement-progress.md`, `forces-independent-review.txt`,
  `regression-test-audit.md`: wrong folder (finding 1). No em dashes. They do not contradict
  OCCUPIER-FORCES.md on rules, but they describe a different machine's evidence (Linux,
  4572 unit cases, no game data) and say "Upstream ... `887ce31`", which reads oddly against
  PR #95's history. Fold the useful facts into the commit messages; drop the files.
- `REVIEWABLE_FORCES.md`: 9 em dashes; says "Classic 1.99 compatibility is being dropped"
  while the same branch's later commit makes it refused (internal inconsistency). Drop.
- `roadmap.yaml`: the three code commits on the occupier-forces item are fine to keep (use
  the new SHAs if cherry-picked). The controls item gains bad1530/44a8254 (fine). The #67 setup
  item rewrite and the `.trident.env` item belong with the runtime PR. The F062/F078
  "started" line refers to absent work: drop.
- PR #95 body: nothing contradicted; it would gain a close-out bullet for the five fixes.

## 5. Merge plan

Recommended: code and tests onto occupier-forces now (PR #95 carries them), runtime and
policy as a separate PR after #95, docs not merged.

Order of operations (orchestrator, main tree on occupier-forces):
1. Do not `git merge`. Apply the code and test delta 3-way, touching neither roadmap.yaml nor
   guerrilla-mode/README.md:
   ```
   git diff occupier-forces...origin/reviewable_forces -- \
     arma_CWR/engine/Poseidon/Game/Guerrilla/CounterAttack.cpp \
     arma_CWR/engine/Poseidon/Game/Guerrilla/CounterAttack.hpp \
     arma_CWR/engine/Poseidon/Game/Guerrilla/OccupierForces.cpp \
     arma_CWR/engine/Poseidon/Game/Guerrilla/ZoneRegistry.cpp \
     arma_CWR/engine/Poseidon/UI/Settings/ContextControlsConfig.cpp \
     arma_CWR/tests/unit/engine/Poseidon/CMakeLists.txt \
     arma_CWR/tests/unit/engine/Poseidon/Game/Guerrilla/test_counter_attack.cpp \
     arma_CWR/tests/unit/engine/Poseidon/Game/Guerrilla/test_occupier_forces.cpp \
     arma_CWR/tests/unit/engine/Poseidon/Game/Guerrilla/test_town_capture_batch.cpp \
     arma_CWR/tests/unit/engine/Poseidon/Game/Guerrilla/test_town_presence_order.cpp \
     arma_CWR/tests/unit/engine/Poseidon/Game/Guerrilla/test_market.cpp \
     arma_CWR/tests/unit/engine/Poseidon/Game/Guerrilla/test_portrait_service.cpp \
     arma_CWR/tests/unit/engine/Poseidon/UI/Settings/test_contextControlsConfig.cpp \
     > patch && git apply -3 patch
   ```
   Expected conflict: only the EOF of `test_occupier_forces.cpp` (keep both blocks).
   Alternative with history: cherry-pick d779fd6 0690e95 39d67cf bad1530 384b283 eb54da9
   855658f 93cd5c9 886c52e b6901d6 44a8254 758e308 5f3cacd, then 6c3dcde's two test files
   (same EOF conflict at d779fd6 and eb54da9).
2. Same commit (or the next): one sentence in OFFSCREEN-CONTRACT.md row 1b (current-bubble
   census) and in OCCUPIER-FORCES.md lines 121 to 122 (k counts towns held before the event in
   the batch). Optionally add one assertion that drives `ZoneRegistry::GatherInputs` (or a
   comment) to pin the wiring.
3. Roadmap: add the new SHAs to the occupier-forces item and a log line, once the other
   session's uncommitted roadmap.yaml edits are committed (coordinate; do not commit them by
   accident).
4. Re-gate (CounterAttack and OccupierForces changed, so the whole forces set is mandatory):
   `powershell -NoProfile -File tmp/of-gate.ps1 -Phase 18 -ExtraTests <forces set>` with the
   forces set = guerrilla_forces_* (air_op, air_save_reload.seq, alarm_answer,
   armoured_column, arrival_empty_base, arrival_rule, base_retake, both_towns, cadence,
   heli_insertion, hq_assault, jet_sortie, motorized, no_popin, passerby_revert, pool_off,
   pool_on, refill_pushout, replan_band, save_reload.seq, spare_cap, town_strike,
   watched_route, wave_base, wiped_release) plus ai_group_id_churn. The base list already
   covers counter_attack_live / offscreen / unseen_spawn / watched_base and the capture tests,
   which exercise the birth census and the presence change live. The gate's unit filter does
   not include the controls cases: also run
   `PoseidonTests.exe "[ContextControlsConfig],[controls],[market],[portrait],[town-capture-batch],[town-presence-order]"`.
5. Separate PR against main after #95: d773dd5 plus the non-process parts of ba2aa5c
   (FRESH-INSTALL.md, guerrilla-mode/README.md, arma_CWR/README.md, tests/README.md,
   `.trident.env.example`, root README without the REVIEWABLE_FORCES pointer), with the two
   CLAUDE.md edits only if the owner approves the wording. Its gate: `test-setup.ps1` on
   Windows pwsh 7, setup-guerrilla + install-missions on shadow-67 (already verified to pass
   the guard), a run-game.ps1 `-SkipBuild` launch to the menu. This one would need a branch;
   ask the owner, or let reviewable_forces itself carry it after a cleanup commit.
6. Never merge: `REVIEWABLE_FORCES.md`, `roadmap/forces-improvement-progress.md`,
   `roadmap/forces-independent-review.txt`, `roadmap/regression-test-audit.md`, the README
   pointer line, the F062/F078 roadmap log line.

Rejected: a whole merge (brings finding 1 into #95, blocked by finding 2); reviewable_forces
as its own PR as is (still carries the process docs; the fixes belong with the retune).
