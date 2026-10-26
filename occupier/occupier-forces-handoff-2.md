# Occupier forces: handoff 2 (2026-10-17)

You are taking over the orchestration of the occupier forces work on branch
`occupier-forces` (PR #95 to main on GearUnclear/CWR). Read this file fully,
then the four files it points at, before doing anything. The owner directs
by chat; you orchestrate Opus subagents, commit by path, gate, push.

## 1. Read these first, in this order

1. `D:\Arma_CWA\CLAUDE.md` and `D:\Arma_CWA\arma_CWR\CLAUDE.md` (repo rules;
   the Remaster is the target; no new branches without the owner's word).
2. `D:\Arma_CWA\arma_CWR\guerrilla-mode\OCCUPIER-FORCES-NEXT.md` (TRACKED: the
   owner-approved plan, the round table with status, the ownership map, the
   owner decisions made and pending). Keep its status table current.
3. `D:\Arma_CWA\tmp\of-agent-common-rules.md` (binding rules every agent gets;
   paste its path into every brief).
4. `D:\Arma_CWA\tmp\occupier-forces-progress.md` (the running log; append a
   line for every commit, gate, decision and lesson).
5. The design: `arma_CWR/guerrilla-mode/OCCUPIER-FORCES.md` (0.2 D1 to D24
   binding; 16 the knobs; 5.7 generated bases) and `OFFSCREEN-CONTRACT.md`.

## 2. State at handoff

- origin/occupier-forces at 5fbe95c (pushed 2026-10-17; 1ed1415 is the last code commit, 5fbe95c the roadmap and plan) (gate 21 green: 820 unit cases, human suite OK, tri 80 of 83 with the three reds resolved afterwards (two test premises on the new poolStart 48, and the LoBo helicopter drop error fixed in e371a68 and verified 3 of 3 on a shadow); the strike band sequence is held out).
- PR #95 body refreshed through round 3.5; refresh it again from
  `tmp/of-pr-body.md` after the next push (`gh -R GearUnclear/CWR pr edit 95
  --body-file tmp/of-pr-body.md`).
- PR #213 (branch `remaster-only-runtime`, base occupier-forces): the
  Remaster-only runtime gate and CLAUDE.md wording, stacked on #95; retarget
  to main after #95 merges. The owner reviews both PRs.
- Issues: #97 follow-ups, #98 journal integration, #99 vehicle dealer, #78
  (note posted: the InCargo root cause, 973a36d), #76 occupation network.
  An issue draft for the upstream radio-channel defect is at
  `tmp/of-radio-check-issue.md` (not filed; file it with
  `gh -R GearUnclear/CWR issue create` if the owner wants it tracked).
- Working tree: clean except the foreign uncommitted files, if any reappear
  (other sessions use this checkout; never stage what you did not cause).
- No agent running. No game running (`tools/stop-owned-games.ps1 -DryRun`).

## 3. What landed, by round (all on the branch, all gated)

- Rounds 1 to 3.5 (2026-10-14 to 16): see the progress log and the plan doc.
  Highlights: the playable-rate retune (acf2311), the five reviewable_forces
  fixes (6eefc87), the group-id allocator leak (2921bcf), the InCargo seat-in
  (973a36d), the road quote (3f4972b, 98cba8a), the pool ceiling and banking
  (3c9947b, df9457e), traffic and civilians under the body budget (797173a),
  re-arm after a rest (1ae2dc9), arrival births with armour (06414a4),
  empty groups out of the group budget (0c43877).
- Round 4a phase 1 (2026-10-16/17): the repel ladder, massing, value-weighted
  candidates, jitter, wave strikes under the arrival rule, conservation for
  generated bases (d5fa57b); break-off and withdraw with hulls leaving after
  the infantry and one road factor (16acfd5); the chooser takes the cheapest
  package meeting the ETA, air only across water, past forcesAirEta or with
  fighters reported (d90a942); spent hold groups released and the shakedown
  pair under the budget (64d447a); generated bases step 1 (811c5c3,
  2943711); the side centre radio-channel fix (4d0cc45).
- Round 4a phase 2: the outpost layout props (0d5e73a); the wave surge, the
  band on strength (pool plus committed airframes minus unrepaid losses in a
  one-hour window), stored spare hulls instead of banking hull units
  (7765904).

## 4. What is next (the owner approved decisions 1 to 15; 3 is the one left)

0. **Run gate 22 first.** Two engine commits landed after gate 21's build (e371a68 the drop fix, verified only by its own tags and three runs of the Sinai lane on an agent shadow) and the suite has the strike band sequence held out. The held sequence `tmp/of-held-tests/guerrilla_forces_strike_band.seq` (plus its .seq.toml) carries the poolStart and staging fixes; on the gate 21 build its sighting step (observers 50 m from the hold squad, `reveal` every tick, `gmOccupierIntel` for the Outpost) reported zero fighters within 300 s in four runs after passing twice earlier the same day; `guerrilla_forces_opener_reported` uses the same recipe and passes. Find why (a timing change from the re-arm or the massing ladder? the generated bases altering which base's garrison is "the Outpost"'s observers? the sighting table decaying before the assert?), fix, move it back, and re-add it to the extra-tests list.


1. **Phase 3: the lever consolidation** (decision 2). Brief from
   `tmp/of-director-levers-audit.md` section "Proposed eight levers": fold the
   75 keys into forcesCadence, forcesStrength, forcesRecovery, forcesReach,
   forcesAggression, forcesWaves, forcesMinimums, forcesConcurrency, each
   with four preset values; every old key keeps reading as a warned override;
   gmForcesTune keeps every field name (so no tri test changes); the new
   constants from phase 1 A, B, C, G (escalate*, approachRepels,
   containRepels, massingMax, restJitter, adjacentTown, BreakOffDivisor,
   WithdrawHullWait, SlowCostShare, AirFightersMin, surgeDecayHours,
   surgeShare, lossWindowHours, bankHulls, StoredHullsPerBase) fold under
   the matching lever or stay constants; OCCUPIER-FORCES.md 16 and the
   contract's section 4 rewritten (four rows are wrong today: see the audit);
   the contract's 10.1 band table still shows the old 1800/2700/3600 token
   intervals. One Opus agent owning OccupierForces.*, CounterAttack.cpp (the
   derived caps only), ZoneRegistry.cpp (heatCapSpike), the Commands files,
   the default-pinning unit cases, docs. Then gate and push.
2. **Video 6** on the consolidated tree: a defended-town scenario (the video
   5 pipeline in `tmp/of-director-video-5` plus the video 4 renderer), the
   protagonist holding two towns with fighters so the repel ladder, massing,
   break-off and the surge show, measured against the audit's eight playtest
   numbers (time outside STRONG vs kills, launch spacing, urgent share, slow
   legs, lethality vs reported, three in flight, wave timing, peaks). Its
   findings start the next round; the stop rule is a video with no finding
   against the docs.
3. **Round 4 leftovers:** vehicle dealer discoverability (#99), journal lines
   for director actions (#98), generated bases step 3 (per-kind layouts, the
   scorer preferring object-free footprints so Vigny keeps its gate, minor
   kinds placed), Eden's Regina (no site passes the terrain rules within
   2.5 km), the direct Refund call sites in CounterAttack / CounterAttackAir
   that bypass RefundHull (hunks in G's report in the progress log), the
   stale header of `guerrilla_forces_bank_full` and of
   `guerrilla_traffic_budget`.
4. **The owner's second Malden playtest** from `tmp/of-owner-summary.md`
   (refresh the crib sheet first: the generated outposts, the ladder lines,
   the new commands gmOccupierRepels, gmOccupierMassing, gmOccupierLedger,
   gmGeneratedBases, gmBaseProps, gmCounterAttackEnds).

## 5. How to run a round

- One Opus agent per disjoint file set; each brief names its build dir and
  shadow (build/agent-rwdi + of1, agent71 + of3, agent67 + of6, ud66 + of4,
  agent2 + of2, of5 spare), the common-rules path, the plan doc, what done
  looks like, the tri tests to run, and "no commits, no branches, no em
  dashes, report hunks for files you do not own".
- Commit by path from the report, checking em dashes on changed lines
  (`git diff -- <files> | grep "^+" | grep -c $'\xe2\x80\x94'`), commit
  messages with the trailer `Co-Authored-By: Claude Fable 5.1
  <noreply@anthropic.com>`. When two agents share a doc or test file,
  stage one agent's hunks with a filtered patch (`git apply --cached`, see
  `tmp/of-c-contract.patch` and the python snippets in the transcript) or
  commit the file with the last of them.
- Gate: `powershell -NoProfile -File D:\Arma_CWA\tmp\of-gate.ps1 -Phase N
  -ExtraTests <list>` where the list is every
  `tests/integration/scripting/guerrilla_forces_*.test.sqf`,
  `guerrilla_bases_detect_*.test.sqf`, `guerrilla_bases_generate_*.test.sqf`,
  `guerrilla_bases_props_abel.test.sqf`, the `.seq` dirs of those names,
  plus `ai_group_id_churn`, `ai_incargo_traffic_crewdead`,
  `guerrilla_traffic_budget`, `guerrilla_census_empty_groups`,
  `guerrilla_script_holdgroups`, `guerrilla_shakedown_budget` (the
  PowerShell that builds this list is in the progress log and in the
  transcript; copy it). Read the log with PowerShell (UTF-16). Only a green
  gate gets pushed. Never run a gate while the owner is in-game (it reinstalls
  shadow-67). The base list alone is 25 tests and misses the forces work.
- After a push: refresh the PR body, update the plan doc's status table, the
  roadmap item (`roadmap/roadmap.yaml`, partial-stage script pattern in
  `tmp/of-stage-roadmap-close9.py`: copy, bump the commit list and add a log
  line), the progress log, and the memory note at
  `C:\Users\Dane\.claude\projects\D--Arma-CWA\memory\occupier-forces-branch-and-orchestration.md`.

## 6. Gotchas learned (do not relearn them)

- A gmOccupierForces row-length change breaks pool_off and the save seq
  asserts (31 slots now, slot 30 = re-armed zones).
- The generated bases changed test premises: the nearest source to a western
  town may be Vigny Outpost or La Riviere Outpost; assert rules, not base
  names; drain or pin generated bases in tests that assume a source
  (replan_band, pool_on and eta_quote were fixed this way).
- Committing a shared test file by path sweeps another agent's appended
  case (fe228a0); committing a shared doc by path sweeps another agent's
  hunks (harmless but confusing).
- Sibling sessions in the shared checkout can misattribute uncommitted
  edits; `search_session_transcripts` on a distinctive line settles it.
- A usage-limit hit kills every running agent; resume each with
  SendMessage "re-read your own diff first" and they continue.
- Agents hit the 5000-line FileSize threshold: OccupierForces.cpp was split
  (OccupierForcesLedger.cpp); CounterAttack.cpp (5700) and the two big test
  files are over and should be split next time they are touched.
- tri eval timeouts under a long triSimUntil are usually a strict-mode exit
  underneath: read the game stdout tail for `check failed` first.
- `assert_timeout` makes a failing assert take 120 to 600 s; a 28 s green
  seq is real.
- Two agents' reports once crossed when I mixed their ids; keep a table of
  agent id to role in your notes.
- The owner forbids em dashes anywhere; the owner's own instructions override
  agent-local refusals (an agent would not commit a line about a named
  person's politics; the owner committed it himself).
- The app refuses `start_session` from a turn that began with an agent event;
  use the Agent tool, or cut an owner-authorized branch in a detached
  worktree yourself and hand it to an agent (PR #213 was done that way).

## 7. Reference material

- Assessments: `tmp/of-abstract-director-assessment.md`,
  `tmp/of-generated-bases-design.md`, `-engine.md`,
  `tmp/of-reviewable-forces-review.md`, `tmp/of-director-levers-audit.md`,
  `tmp/of-realism-feel-assessment.md`, `tmp/of-pretest-readiness.md`.
- Videos and pipelines: `tmp/of-director-video` (1), `-2` (partial),
  `-3` (branch comparison), `-4`, `-5`; each has scripts, traces, cues,
  run logs, and the mp4.
- The owner's crib sheet: `tmp/of-owner-summary.md`; the PR body:
  `tmp/of-pr-body.md`; the gate script: `tmp/of-gate.ps1`; gate logs:
  `tmp/of-gate-phaseN.log` (UTF-16).
