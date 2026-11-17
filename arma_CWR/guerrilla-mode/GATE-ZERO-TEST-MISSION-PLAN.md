# Malden interactive blocker test mission

Date: 2026-10-12. Status: implementation plan; mission changes are not yet built.
Target: full official Remaster, Malden, initially without mods.
Tracking: [Gate-Zero #79](https://github.com/GearUnclear/CWR/issues/79),
with #31, #72, #76 and the recruiting proposal in #81.

Build **UD Test Lab: Malden**: a playable campaign with an action-menu test
guide, a teleport action for each station, selectable rule experiments, and
an external results bundle. The player should be able to test the blockers
and record preferences without entering console commands or editing files.
The lab can reproduce a blocker before its fix exists; selecting a preferred
rule does not itself fix or certify that rule.

## Reuse the existing implementation

The starting point is `mission/Showcase.Abel/human/`: `cases.sqs`, `boot.sqs`,
`action.sqs` and `run.sqs`. It already supplies 43 guided cases, step
predicates, explicit human verdicts, an attempt history and save/load
rearming. `HUMAN-TESTS.md` documents the player flow. Reuse this runner and
case catalogue rather than creating a second competing acceptance suite.

Its current constraints matter:

- `tests/contracts/test_human_suite.py` keeps observers from changing game
  state, including teleporting. Put setup operations in separately audited
  fixture helpers; preserve the observer boundary.
- Reports currently use `sideChat`, not a durable external test report.
  Use the production `logInfo` script command, implemented in
  `GameStateExtWorld.cpp`, for disk evidence through the engine logger.
- Native events have one handler slot each. The overlay must not replace
  campaign, capture, traffic or garrison handlers or drain their queues.
- The existing overlay detects load reconciliation through `GM_pSaveAct`.
  Reuse this for menu rearming; independently require a new process and a
  matching save fingerprint to prove cross-session resume.
- Running SQS text is saved inside a savegame. Freeze the executable and
  installed scripts for each run; after code changes, start a fresh run.

## What the player sees

Spawn at a safe Camp test desk. The opening card explains the controls,
current build, test mode and results location. A permanent **TEST LAB** action
opens a short paged menu. Normal recruitment, commerce and Save Game actions
remain available and retain their own handlers.

| Menu | Actions |
|---|---|
| Browse | Next test, Previous test, Next unfinished, Read instructions |
| Prepare | Teleport to selected test, Begin test, Return to Camp |
| During a test | Repeat instructions, I observed it: check step, Report problem, Stop |
| Report problem | Failed, Blocked, choose a short reason, Back |
| Compare rules | Current rule, Candidate A, Candidate B, preference page |
| Preference | Prefer A, Prefer B, Prefer current, Neither, Need more testing |
| Results | Show progress, Show decisions, Write summary, Finish session |

Use no more than six lab actions on a page and always provide Back. Choices
are dispatched by stored action IDs, with a single pending request and a
case/attempt generation check to reject stale or double clicks. A selection
records immediately; changing a preference appends a revision to history.
No free-text typing is required. Optional written notes can be added to the
report later, keyed to the case and attempt.

Each instruction card gives the purpose, current prerequisites, what to do,
what to observe, the measured state and an approximate wait. State checks
cannot confirm visual quality. Human confirmation cannot overrule a failed
state predicate. Timeouts offer Retry, Blocked or Stop; never automatic PASS.

## Mission and evidence modes

Add `mission/GateZero.Abel`, labelled **UD Test Lab: Malden**. Keep its
`init.sqs` as the standard shared-core bootstrap. Start its overlay through
the mission player initialization, following Showcase's existing seam.
Include the installed `Guerrilla.Abel/description.ext` so the lab uses the
real Malden zones, factions and market. Any experimental configuration must
be an explicit, versioned lab extension; do not silently fork campaign data.

Extract the reusable human runner into an installed shared test-support
directory, for example `gmtest`. Adapt Showcase and GateZero to supply their
own station context. Replace `SC_*` dependencies in shared human cases with
explicit context values. Preserve Showcase's staged-demo exclusion and
existing acceptance behavior. Test-support scripts are loaded only by test
missions or an explicit test launch, not by ordinary campaigns.

Every attempt records one evidence mode:

| Mode | Meaning |
|---|---|
| Campaign observation | Real rules, normal gameplay, no setup writes during the measured step |
| Travel assisted | Teleport used before the step; does not establish travel, discovery or natural arrival behavior |
| Controlled fixture | Bodies, funds, ownership or other prerequisites deliberately staged and logged |
| Rule experiment | A named candidate policy is active; records usability and policy behavior |

These classifications persist in the report. A staged convoy proves its
behavior after spawning, not ordinary admission. A prepared final zone
proves ending logic, not completion of a whole island campaign. The lab's
results supplement the full playthrough and two-hour soak in
`FULL-SCALE-SESSION.md`; they cannot silently replace them.

## Teleport and reset contract

Every station has a surveyed arrival point, facing, target zone and return
point. Initial survey anchors in script coordinate order `[east,north,up]`:

| Station | Survey anchor | Placement rule |
|---|---|---|
| Camp and recruitment | `[7515,5790,20]` | Safe briefing and logistics area |
| Houdan town | `[7117,6069,35]` | Outside the support/capture radius until Begin |
| Existing Outpost | `[7600,6300,25]` | Covered approach outside the zone |
| Airfield | Near `[7922,9590]` | Survey dry ground and road connection |
| Le Port harbour | Near `[8476,3754]` | On land, away from pier edges |
| Southeast compound | Near `[9690,3990]` | Outside fence, reachable approach |
| Road, cache-distance and remote observation stops | Derived from surveyed zones and roads | Separate near/far points based on actual configured radii |

These are source/issue anchors, not validated teleport coordinates. Survey
with `PoseidonTools` and a rendered Remaster visit before enabling each stop.
Resolve zones by stable names/IDs, never by assumed seeded-city indices.

Teleport only between attempts, before baseline sampling. Refuse while the
player is dead, in a vehicle, saving/loading or in an active measured step;
show the required action such as dismounting. Stop movement and use a dry,
collision-free arrival point. Default to player only; offer **Bring squad**
where the test needs it and **Leave squad here** for remote-capture tests.
Log exactly which units moved and their before/after positions. Do not alter
captivity, exposure, health or damage permission as a hidden teleport aid.

After arrival, wait for required managers and nearby cache populations to
settle with a bounded readiness check, then take the baseline. No teleport
during cache, driving, pursuit or off-screen measurement: walk/drive across
the boundary under test. Teleport to a different station ends the current
attempt as INCOMPLETE first.

Do not implement Reset by reversing a few globals: deaths, observer memory,
random director state and serialized scripts cannot be unwound reliably.
Incompatible experiments use a fresh disposable campaign or a verified
same-build checkpoint. Record checkpoint identity, branch of the experiment,
run seed when available, weather, time and loadout. If exact replay cannot be
guaranteed, record that limitation instead of claiming identical conditions.

## Guided blocker route

| ID | Playable test | Measurements and result |
|---|---|---|
| GZ00 | Boot, read card, open journal and map, teleport out and back | Exact build/content identity; seeded city inventory, manager readiness, action availability and log receipt |
| GZ01 B1 | Visit Outpost, compound, harbour and airfield; observe roads and inspect commander locations | Authored vs live garrisons, military endpoints, `gmTrafficDiag`, actual natural patrol/convoy admission and movement, Legend distribution |
| GZ02 B2 | Enter Houdan alone, with one fighter, and with occupiers present; later clear the defenders | Support vs elapsed simulation time, ownership, occupier bodies/reserves and capture eligibility; distinguish neutral towns from occupier-administered towns |
| GZ03 D1 | Leave all allies outside the radius; repeat a clear town and military post while disguised, locally exposed and explicitly organizing where supported | Actual presence census, undercover status/witnesses, support/capture progress, stalled or viable solo recovery |
| GZ04 B3 | Play the final assault from an explicitly prepared near-victory state; compare all victory predicates | Remaining military objectives, town objective state, journal entry, one ending transition, outro and post-load behavior |
| GZ05 B4 | Save with the real action, alter state, load via UI; quit fully, relaunch to menu and resume; attempt new campaign then cancel | Actual files, timestamps/hashes, save generation and restored fingerprint; cancellations preserve saves; no duplicate managers/actions/units |
| GZ06 | Combat, detection, QRF, casualty/cache, retreat and return | Damage, per-observer exposure, alert transitions, QRF travel/cleanup, casualties conserved across cache transitions |
| GZ07 | Recruit/train, buy, loot, move HQ, deposit/retrieve, garage, companions and civilian interaction | Production transactions, funds/manpower deltas, assets, action reachability, dialogue and journal; link existing human case IDs |
| GZ08 | Leave a squad, drive far away, observe, save near/far and revisit | OFF-a to OFF-d evidence, remote progress and no duplicate/healed garrisons |
| GZ09 | Populated fight with QRF and a large player squad; drive between hubs | Frame and simulation percentiles, live units/groups, retained corpses/wrecks, memory and save size; offer separate two-hour soak |

Begin GZ01 with a recorded current-map inventory. Add the Malden #76 slice
as real mission data: keep Outpost 6, provisionally compound 6, harbour 8,
airfield 12, subject to survey and play feedback. Verify the C1–C6 contract,
including Camp-time route admissibility. B1 stays open if the extra endpoints
exist but natural admission or reachable placement fails. A bounded natural
observation window may be inconclusive; use eligibility/counters and a
separate forced fixture to diagnose, never force a traffic spawn and call it
a natural pass.

For GZ02, record the current undefended-town behavior first. A candidate
defended town must use the production garrison service, including caching,
casualty accounting, hostility and save/load. Hand-spawned guards can provide
an early feel test only. Exposing the CITY eligibility policy and testing
support while hostile reserves exist is engine work under #72, not merely a
mission script change. Compare a small guard presence with the unguarded
baseline; record Too easy / About right / Too hard independently of correctness.

## Decisions selected in game

**Victory rule:** offer Military zones / Towns / Both / Undecided. Evaluate
all three predicates as read-only observations first. Define military scope
from eligible authored enemy military objectives at campaign start, excluding
Camp and CITY; zero objectives cannot win. Show the remaining count. For the
town predicate, require every scoped CITY to be resistance-owned, rather than
equating a momentary support value with ownership. A qualified win latches
once; subsequent support changes cannot undo it or repeat the journal/outro.
Provide a separate real-ending attempt because an outro may exit the mission.
Default proposal for feedback is military zones; the user's recorded choice
is the decision evidence, not an automatic production-default change.

**Player capture:** compare the current AI-only rule with a narrowly scoped
player-presence candidate and the explicit town-organizing candidate in
`REALISTIC-RECRUITING.md` (#81). Do not equate `gmUndercoverStatus` with a
globally exposed player or set `gmUndercover = false`. Exposure is local to
observers. Before enabling the exposed-player experiment, specify which
zone-relevant witnesses make the player eligible and what happens once the
last witness disappears. Show that condition on the test card and log it.

Explicit organizing is a CITY-only option; it does not capture military
posts. Bind intent to the chosen clear town and clear it on exit, death or
invalid prerequisites. Exercise companion-loss recovery in both town and
military contexts so a successful opening cannot hide a later military
soft-lock. Candidate selection happens between attempts and is saved.
Unsupported candidates are labelled **Not implemented**, with preference
recording still available; never present a scripted imitation as working
engine behavior.

**Recruitment backstop:** separately compare No backstop / Traditional
manpower recovery / Explicit organizing and local recruitment. Show the
applicable recruiting mode. #81 proposes no passive manpower for Realistic,
so do not apply a universal manpower floor that contradicts it. Log whether
recovery remains possible with no companion, zero recruits, insufficient
funds and no eligible friendly town. A preference is not a correctness pass.

**Ending and resume experience:** record whether stock death/Retry is adequate
for v1 and whether the win outro is clear. Save correctness is not a vote:
normal Save/Load must agree, resume must work after process exit, and starting
over must either preserve the campaign or require explicit replacement.

## Save and resume must leave the mission

GZ05 has a second launch path into the real GUERRILLA menu using
`Guerrilla.Abel`, with the test overlay enabled explicitly for that disposable
run. A direct launch of GateZero alone cannot verify new-game routing or
Continue. No `triSaveGame`/`triLoadGame` substitute qualifies as B4 evidence.

Use a dedicated child-process `POSEIDON_USER_DIR` inside the run folder;
verify resolved save paths before running any replacement test. Do not inherit
the player's normal saves. The launcher keeps this directory across relaunches.
Record a fingerprint containing zone owners/support/reserves, resources,
manpower, player position/equipment, recruits/companions, stash/garage,
journal marker and overlay ledger. Compare discrete values exactly and use
documented tolerances for simulation time, positions and other advancing state.

Test manual save/load, action Save Game/Retry, menu resume, a full process
restart, missing-save handling, new-game cancellation, and explicitly confirmed
replacement of disposable saves. Cover all relevant deletion call sites in
`OptionsUIApp.cpp`, including the paths near 566–598, 1203–1213 and 1298–1308;
centralize replacement policy rather than fixing only the GUERRILLA OK path.
Check both cancel and confirm behavior and assert unrelated campaign saves
are untouched. Record the chosen save-slot contract in the implementation.

Until Continue exists, report **BLOCKED: no user resume route** with its
evidence and let the user continue unrelated stations in a fresh session.
The external ledger must survive even when the game's save path is broken.

## Durable results bundle

Add a launcher and collector, proposed as
`tools/playtest/start-gate-zero.ps1` and `tools/playtest/collect-gate-zero.py`.
Each run writes under `logs/gate-zero/<run-id>/`:

- `manifest.json`: commit plus dirty-source hashes, executable hash, gmcore,
  mission/test-policy hashes, full Remaster identity, mods, machine/render
  settings, seed availability, user directory and schema version.
- `engine-<segment>.log`, `stdout-<segment>.log`, `stderr-<segment>.log`:
  explicit capture for each process segment, retained across restarts.
- `events.jsonl`: structured, append-only evidence extracted from marked
  `logInfo` records, with truncation/parse warnings retained.
- `results.json` and `summary.md`: case/attempt outcomes, decisions, blockers,
  expected-versus-observed values and evidence line references.
- `saves/` and `screenshots/`: explicit checkpoint copies and any captured
  images, with a file inventory and hashes.

The engine already supports `--log-file`; use an explicit path and INFO
Script logging even where automatic per-run logging is available. Capture
stderr because crash diagnostics bypass the normal logger. Do not depend on
`diag_log`, the radio history or simulate-only `textLog` behavior.

Emit versioned, single-line `[UDTEST]` records for session start, attempt
start, setup, teleport, baseline, check/rejection, state sample, decision,
save intent, load, outcome and summary. Implement and test field escaping;
do not assume SQF array text is JSON. Decode into JSONL in the collector.
Include run, process segment, save generation, attempt, sequence, case/step,
policy, evidence mode and simulation time. The collector supplies wall time
from the host log. Persist scalar identities in the save, but do not rely on
raw object handles as durable cross-process identities.

Log every action immediately, snapshot on important transitions and sample
active tests at a bounded rate. Flush event evidence at verdict/save/exit
boundaries; if the logger lacks a suitable flush seam, add a narrow one.
Crash or truncated final records leave the affected attempt INCOMPLETE.
On rollback, retain later external events and create a new process/attempt
lineage, so replayed saved sequence numbers cannot overwrite history.

Result states: UNTESTED, IN_PROGRESS, PASS, FAIL, BLOCKED, INCOMPLETE.
Keep `machine_checks`, `human_observations`, `preference` and `setup_history`
as separate fields. **Write summary** emits the current ledger; the companion
collector updates the files without exiting the game. **Finish session**
flushes a final summary. Collection also works after a crash, without loading
the save. No automatic issue publication or upload is part of the launcher.

## Implementation order and handoff gates

1. **Preflight and baseline.** Build PoseidonGame/PoseidonTools with the
   documented Clang preset. Prepare a full-Remaster shadow, reinstall current
   core/templates and establish isolated user storage. Pin hashes and record
   baseline blocker observations. Leave the current Git branch unchanged.
2. **Reusable runner and logging.** Extract the existing menu/ledger with
   its contracts intact; add durable events, collector and load rearming.
   Verify a clicked verdict reaches disk before adding more scenarios.
3. **Playable lab.** Install GateZero, survey safe arrivals, add paged station
   navigation and audited setup helpers. Ship blocked-case explanations so
   the lab is useful even before the blocker fixes land.
4. **Occupation stations.** Add/verify B1's real Malden network; implement the
   minimum B2 native town-defense slice and D1 candidate presence/organizing
   seams. Keep trial settings opt-in and absent from normal campaigns.
5. **Ending comparisons.** Add read-only candidate objectives first, then a
   real journal/win-outro path with once-only serialized completion. Wire
   the preference menu and clear fixture labels.
6. **Actual persistence UX.** Fix B4's shared save selection and replacement
   policy; add Continue/Load and drive it through the real menus. Preserve
   pre-fix evidence and run all entry-point/cancel/restart cases.
7. **Remaster verification and handoff.** Run targeted runner, gameplay and
   actual-UI persistence tests; inspect the rendered menu/cards and arrival
   points. Deliver one start command, a resume command for the same run, the
   quick route, the long route and the absolute report directory.

At implementation time, track the harness and minimum B1/B2/D1/B3/B4 work as
pre-Gate-Zero items. Those items depend on existing groundwork, not on
`full-mode` or the completed human playtest. Gate-Zero completion depends on
their validated results; broad #72/#76 work stays separately scoped. This
avoids importing the roadmap's full-mode dependency loop.

## Verification required before handing it to the player

- Contracts: unique case IDs; all station references resolve; production
  shared core/config reused; only audited setup helpers mutate gameplay;
  no replacement of native event handlers; setup cannot write verdicts.
- Runner: actual action clicks, rejected premature checks, stale/double
  clicks, Back/Cancel, bounded waits, unimplemented options, decision edits,
  teleport refusal/readiness, death and load rearming without duplicate menus.
- Evidence: escape/parse round-trip, crash/truncated record, save rollback,
  multiple process segments, missing files, stale build rejection and report
  reconstruction without a readable save.
- Gameplay: natural traffic admission separately from forced routing;
  native town guard persistence and support gating; solo and companion-loss
  presence cases; zero/final/partial victory plus post-load once-only ending.
- Save UX: real Save action and pause Load/Retry, quit/relaunch/Continue,
  exact discrete-state comparison, all new-game replacement/cancel routes.
- Regressions: existing human-suite contracts and automated runner/roads/save
  tests on full Remaster, plus affected native tests and Remaster menu tests.
  Automated runner confirmations remain distinct from human acceptance.
- Human handoff: first run at 1920x1080; verify action labels fit, cards are
  readable, arrival points are safe and report files contain the clicked
  observations. Use the existing budget thresholds: typical live bodies
  <=100, hard cap 130, groups per side <=40, frame p95 <=25 ms, gm average
  <=0.5 ms and p95 <=1.5 ms. Measure ordinary play and the worst-case fight.

The launcher leaves the human's game open for the requested session.
Automated smoke tests close only their own spawned process. A normal finish,
crash or manual close all leave readable evidence on disk.

## First player session

Start with a roughly 45–60 minute blocker route: briefing and teleport check,
occupation tour, town-defense and solo-capture comparisons, final-assault
preview, then the actual save/quit/resume check. Natural traffic or unfinished
policies may extend the session or leave individual rows BLOCKED; the menu
must allow moving on without losing evidence. End by selecting preferences
and **Write summary**. The broader 43-case suite and two-hour populated soak
remain separately selectable follow-up sessions.

The handoff is complete when the user can launch, reach every test through
the action menu, make and revise the design choices, and leave a report that
can distinguish a game defect, missing implementation, preference, untested
condition and successful observation without reconstructing events from memory.
