# Cross-group shared enemy intel via AICenter (radio-net awareness)

*When one squad calls out a contact, nearby friendly squads get a fuzzy, delayed heads-up instead of staying blind until they personally see the enemy.*

## Summary

Today every AI group senses the world in near-isolation: a group only "knows" about an
enemy that one of its own units has personally detected. The side-wide `AICenter` keeps a
strategic target database, but that database is **only** used for the exposure/danger map and
strategic planning — it is never pushed back down into a sibling group's tactical
`TargetList`. So if squad A is in a firefight 80 m from squad B, squad B can be completely
oblivious until it independently spots the same enemy.

This mod makes `AICenter::ReceiveReport` (the function called whenever a group radios in a
fresh contact) **seed nearby friendly groups' own target lists with a degraded copy** of the
contact: later `delay` (radio propagation lag), larger `posError` (you got a grid callout,
not a laser fix), and lower `accuracy` / `sideAccuracy` (you were told "armor, roughly there"
not "T-72 at this exact tree"). The receiving group then reacts through all its normal
combat/behaviour code as if it had a weak sensor hit.

Observable result: order a single recon team to spot an enemy column, and watch neighbouring
squads orient, go to combat mode, and start maneuvering *before* they have line of sight —
with a believable delay and some positional slop, rather than the current all-or-nothing
per-group sensing.

## Why it's interesting

- It turns the side into something that *feels* like it has a radio net, which is exactly the
  fantasy of a Cold War combined-arms battlefield. This is a high-leverage AI behaviour change
  for a relatively small, well-localized code edit.
- The data plumbing already exists: `AITargetInfo` already stores fading side/type/position
  accuracy plus timestamps, and `AIGroup::AddTarget` already supports an explicit position and
  a `delay` parameter. The mod is mostly *wiring*, not new subsystems.
- It is fully tunable (range, degradation factors) and produces a clearly testable,
  observable change with no new art.

## Difficulty & prerequisites

**Advanced.** You need to understand the detection/target pipeline (`Target`, `TargetList`,
`AICenter`, `AIGroup`) and be comfortable editing the AI core C++ and rebuilding. No SQF is
strictly required, though optional config exposure (below) is a nice extension.

**No art assets are required.** This is pure engine C++ plus optional config/SQF tuning. No
models, textures, sounds, or missions need to be created. (Re-using the existing radio
chatter and known-target rendering is free.)

## Key files & entry points

- `engine/Poseidon/AI/AICenterImplPreview.cpp`
  - `AICenter::ReceiveReport(AIGroup* from, ReportSubject subject, Target& target)` — **line 886**.
    Currently the entire body is `UpdateTarget(target);`. This is the injection point.
  - `AICenter::UpdateTarget(Target&)` — the function ending at **line 884** that maintains the
    side-wide `_targets` list; useful to read for how accuracy/time/posError are folded in.
- `engine/Poseidon/AI/AICenter.hpp`
  - `class AITargetInfo` — **lines 17-49**: fields `_side`, `_type`, `_realPos`, `_pos`,
    `_precisionPos`, `_accuracySide`/`_timeSide`, `_accuracyType`/`_timeType`, `_time`, and the
    fading accessors `FadingSideAccuracy()` / `FadingTypeAccuracy()` / `FadingPositionAccuracy()`.
  - `class AICenter` — group access we will iterate over: `NGroups()` / `GetGroup(int)`
    (**lines 249-250**), side tests `IsEnemy`/`IsFriendly` (**lines 269-271**), `GetSide()`.
- `engine/Poseidon/AI/AIGroup.hpp`
  - `AIGroup::AddTarget(EntityAI* object, float accuracy, float sideAccuracy, float delay, const Vector3* pos, AIUnit* sensor, float sensorDelay)`
    — declared **lines 513-517**. This is the API we call on each nearby friendly group.
  - `AIGroup::GetCurrentPosition()` — **line 453** (`Leader() ? Leader()->Position() : VZero`),
    used for the proximity test between the reporter and candidate receivers.
  - `enum ReportSubject { ReportNew, ReportDestroy }` — **lines 239-243**.
- `engine/Poseidon/AI/AIGroupImpl.cpp`
  - `AIGroup::AddTarget(...)` implementation — **line 704**. Note it already (a) finds-or-creates
    a `Target` in `_targetList`, (b) when `accuracy < 1` and no explicit `pos` is given it injects
    a ±100 m random XZ error, (c) sets `lastSeen = Glob.time - 40 + delay` and `delay = Glob.time + delay`
    for the no-sensor path, and (d) only upgrades `accuracy`/`side` when the new fading value beats
    the current one. This is exactly the "merge a weaker hit" behaviour we want.
- `engine/Poseidon/AI/AIGroupCmd.cpp`
  - `AIGroup::SendReport(ReportSubject, Target&)` — **line 1401**; calls
    `_center->ReceiveReport(this, subject, target)` at **line 1409**, but only when the leader is
    alive and `target.IsKnownBy(leader)` is true. This confirms `ReceiveReport`'s `from` is the
    *originating* group and the contact is genuinely known.
- `engine/Poseidon/World/Detection/Target.cpp` + the `struct Target` definition in
  `engine/Poseidon/AI/VehicleAI.hpp` (**lines 256-322**)
  - Fields we read from the incoming report: `idExact`, `side`, `type`, `position`, `posError`,
    `accuracy`/`accuracyTime`, `sideAccuracy`/`sideAccuracyTime`, `lastSeen`, `delay`.
  - `Target::FadingAccuracy()` (Target.cpp **line 59**) and `Target::FadingSideAccuracy()`
    (**line 66**) — the time-decayed accuracy values; `ReceiveReport`/`UpdateTarget` already use
    these, so we use the same accessors to derive the degraded values we pass on.

## How it works today

1. A unit detects something; its group's `TargetList` gets a `Target` via the detection code in
   `Target.cpp` (`AddTarget` / `Manage`).
2. When the group decides the contact is worth radioing, `AIGroup::SendReport`
   (AIGroupCmd.cpp:1401) checks the leader is alive and `target.IsKnownBy(leader)`, then calls
   `AICenter::ReceiveReport(this, subject, target)`.
3. `AICenter::ReceiveReport` (AICenterImplPreview.cpp:886) does exactly one thing:
   `UpdateTarget(target)`. `UpdateTarget` folds the contact into the **side-wide** `_targets`
   list (`AITargetList` of `AITargetInfo`), updating fading accuracy/side/type/position and
   feeding the strategic exposure map (`_map->AddChange(...)`).
4. That side-wide list drives strategic planning and the danger/exposure map — **but it is
   never written back into any other group's tactical `_targetList`.** Sibling groups keep
   reacting only to what they personally sensed. The `from` parameter of `ReceiveReport` is
   currently ignored entirely.

So the "report" today is one-way and strategic-only. There is no tactical situational-awareness
sharing between peer groups.

## Implementation approach

All edits are in `AICenter::ReceiveReport` (AICenterImplPreview.cpp:886) plus a couple of small
helpers/constants. Keep the existing `UpdateTarget(target)` call — strategic behaviour is
unchanged; we *add* the peer-sharing on top.

1. **Guard the cases we care about.** Only share on `subject == ReportNew` (not `ReportDestroy`),
   only when `target.idExact` is non-null, `target.isKnown` is true, and the contact is an enemy
   of this center: `IsEnemy(target.side)` (or `target.side == TSideUnknown` if you want unknown
   contacts shared too — recommend enemy + unknown, skip friendly/civilian). Bail early
   otherwise.

2. **Find the reporter's position.** `Vector3 origin = from ? from->GetCurrentPosition() : target.AimingPosition();`
   Prefer the *target* position as the broadcast origin for range, or the reporter — pick the
   reporter so that only squads near the *caller* (i.e. on the same part of the net / same
   battle) get the intel. Document the choice in a comment.

3. **Iterate sibling groups.** Loop `for (int i = 0; i < NGroups(); i++)` over `GetGroup(i)`.
   Skip the originating group (`grp == from`), skip groups with no living leader
   (`!grp->Leader()` or leader not `LSAlive`), and skip the reporter itself. Because we are
   inside `AICenter`, every group here is already on the same side — no cross-side leak.

4. **Range gate.** Compute `float d2 = grp->GetCurrentPosition().Distance2(origin);` and skip if
   `d2 > Square(shareRange)`. Start with `shareRange = 500` m (radio/line-of-battle scale). This
   is the single most important tunable.

5. **Derive a degraded copy and seed the peer group.** Translate the report's current fading
   values into weaker inputs for `AIGroup::AddTarget`:
   - `float accuracy = target.FadingAccuracy() * accuracyFalloff;` (e.g. `accuracyFalloff = 0.5`).
     Keeping it `< 1` is important: in `AddTarget` (AIGroupImpl.cpp:738) an accuracy `< 1` is what
     triggers the built-in ±100 m positional randomization, which is exactly the "fuzzy callout"
     we want.
   - `float sideAccuracy = target.FadingSideAccuracy() * sideFalloff;` (e.g. `sideFalloff = 0.6`).
   - `float delay = baseDelay + d2Scaled;` — a propagation/processing lag. Start with a flat
     `baseDelay = 8` seconds (recall `AddTarget` adds this to `Glob.time` and back-dates
     `lastSeen = Glob.time - 40 + delay`, so a larger delay means the peer treats it as an older,
     less certain contact). Optionally scale with distance.
   - Position: either pass `nullptr` and let `AddTarget` inject its ±100 m random error (simplest,
     and ties the slop to accuracy), **or** pass an explicit `Vector3 pos = target.AimingPosition()`
     plus your own larger jitter if you want bigger, range-dependent error. Recommend `nullptr`
     for the first slice.
   - Call: `grp->AddTarget(target.idExact, accuracy, sideAccuracy, delay /*, nullptr, nullptr, 1e10f */);`
     (`sensor` left null so it is treated as a group-wide hint, not a specific unit's sensor lock).
   Because `AddTarget` only *upgrades* a target's accuracy when the new fading value beats the
   stored one (AIGroupImpl.cpp:782, :789), repeatedly sharing the same contact will not
   overwrite a peer group's *better* first-hand information — the degraded copy loses to a real
   sighting automatically. This is the key correctness property that makes the feature safe.

6. **Factor the tunables into named constants** at the top of the file (`SHARE_RANGE`,
   `SHARE_ACCURACY_FALLOFF`, `SHARE_SIDE_FALLOFF`, `SHARE_BASE_DELAY`). Optionally promote them to
   config (below) so missions can tune without recompiling.

7. **(Optional) Throttle.** `ReportSent`/the radio path already de-dupes how often a group
   *sends*, so `ReceiveReport` is not called every frame for the same contact. If profiling shows
   churn, add a cheap "already shared recently" guard keyed on `idExact` + time, but do **not**
   over-engineer the first slice.

## Config / data / SQF touchpoints

Optional but recommended for tuning without rebuilds:

- **Config-driven tunables.** Read `SHARE_RANGE` / falloff / delay from a `ParamEntry` under
  `CfgAI` (or an existing AI tuning class) at center init, mirroring how other AI constants are
  pulled from config. Falls back to the hard-coded defaults if the entry is absent.
- **SQF observability hook (no new commands needed).** Verification can lean on existing
  scripting: `nearTargets` / `knowsAbout` already report what a unit/group knows about an enemy.
  After this mod, `playerSide knowsAbout enemy` (or a peer leader's `knowsAbout`) should rise for
  groups that never sensed the target directly. No new SQF surface is required; this is purely a
  read-side observation point.
- If you want an explicit debugging toggle, gate the sharing behind an existing global/diag flag
  rather than inventing a new persisted variable.

## Risks & gotchas

- **Don't "fix" the heavy intentional warning suppression.** The root `CMakeLists.txt` disables
  ~50 clang warning classes on purpose. If your new loop trips a tautological-compare or
  unused-variable pattern, that is consistent with the codebase style — don't add casts/pragmas
  to silence things the project already suppresses globally.
- **`Target` is not a trivially-copyable POD you can `memcpy`.** It is `struct Target : public
  RemoveLLinks` with `OLink<>`/`Ref<>`-style members (VehicleAI.hpp:256). Do **not** apply the
  `ClassIsMovableZeroed` memcpy/memmove movable pattern to it. Always go through `AddTarget`,
  which constructs/looks up a real `Target` and sets fields individually.
- **Self-report / friendly leakage.** Skip `from` and rely on the fact that all `GetGroup(i)`
  here share this center's side; never call `IsEnemy` on your own side. Verify a freshly
  reported friendly/civilian contact is filtered out before the loop.
- **Stale `idExact`.** The reported `Target::idExact` is an entity link; the peer `AddTarget`
  takes `EntityAI*`. Confirm the entity is still valid (the `SendReport` precondition
  `target.IsKnownBy(leader)` at AIGroupCmd.cpp:1407 implies it is, but guard `!= nullptr`).
- **Performance.** `NGroups()` is small in practice, and `ReceiveReport` is event-driven (per
  radioed contact), so O(groups) per report is fine. Avoid adding any per-frame scan.
- **clang-cl vs GNU-driver `__FILE__`** only matters if you add asserts/logs using
  path-relative helpers — keep that asymmetry in mind if you log from the new code.
- **PCH / FileSize.** PCH is per-target PRIVATE; you are only editing existing `.cpp`/`.hpp` in
  the `Poseidon` target, so no PCH wiring changes. `AICenterImplPreview.cpp` should stay well
  under the FileSize target's 3000-line warn / 5000-line error thresholds — this change adds only
  a few dozen lines.

## Testing

1. **Build:** `cmake --preset win-x64-clang-rwdi` then
   `cmake --build build/win-x64-clang-rwdi`. Run format/tidy:
   `cmake --build build/win-x64-clang-rwdi --target Format` and `... --target Tidy`.

2. **Catch2 unit tests (`tests/unit/`):** the AI suites are `PoseidonTests` / `PoseidonCoreTests`.
   Add a focused test that:
   - constructs an `AICenter` with two groups placed within `SHARE_RANGE`,
   - feeds a `Target` (enemy side, known) into `ReceiveReport(groupA, ReportNew, target)`,
   - asserts groupB's `GetTargetList()` now contains a `Target` with matching `idExact`, a
     `delay` later than groupA's, and a `posError`/fading accuracy strictly worse than the source.
   Also add the negative cases: a friendly/civilian contact is **not** shared, and a group
   outside `SHARE_RANGE` is **not** seeded. Run with
   `ctest --test-dir build/win-x64-clang-rwdi -R PoseidonTests --output-on-failure`.

3. **Trident integration (`tests/integration/`):** author an SQF scenario with a recon team and a
   separate rifle squad ~300 m apart, only the recon team in LOS of an enemy. After the recon
   team detects + reports, assert via `knowsAbout` that the rifle squad's knowledge of the enemy
   rises above zero within a few seconds (and that it lags the recon team). Build the runner
   (`cargo build --manifest-path engine/Trident/Cargo.toml`) and run
   `tri test -j6 --retries 2 tests/integration` with `OFPR_GAME_DIR`/`OFPR_DATA_DIR` set.

4. **In-game smoke test (win-x64-clang-rwdi):** place a player-led group plus an AI squad nearby,
   and an enemy patrol visible only to a third spotter group. Confirm the previously-blind squad
   reorients / goes to combat before LOS, with a visible delay and some positional error
   (it moves toward an approximate, not pixel-perfect, location). Toggle `SHARE_RANGE` to 0 to
   confirm the behaviour reverts to today's isolated sensing.

## Scope estimate

- **Core C++ change:** ~0.5-1 day. The edit is localized to `AICenter::ReceiveReport` plus four
  named constants; everything it needs (`NGroups`/`GetGroup`, `AddTarget`, `GetCurrentPosition`,
  the fading accessors) already exists.
- **Unit + integration tests:** ~0.5-1 day.
- **Optional config exposure:** ~0.5 day.

**Suggested minimal first slice (fast visible win):** in `AICenter::ReceiveReport`, after the
existing `UpdateTarget(target)`, add the guarded loop with **hard-coded** constants
(`SHARE_RANGE = 500`, `accuracy = target.FadingAccuracy() * 0.5`,
`sideAccuracy = target.FadingSideAccuracy() * 0.6`, `delay = 8`, `pos = nullptr`), skipping
`from` and non-enemy contacts. Build win-x64-clang-rwdi, run the in-game smoke test, and watch a
blind squad react to a spotter's call. Tunables → config and the negative-case tests come after
the behaviour is confirmed.
