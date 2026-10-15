# Forces improvement campaign

Started 2026-10-15. Delivery branch: `reviewable_forces`. Integration writer: coordinator in `/root/mobile-dev/CWR`.

## Source and safeguards

- Fetched origin; remote only has `occupier-forces`, not `forces`; no prior delivery branch. Verified upstream SHA: `887ce31d43c723e9dbb7ca433c1a674f70838367`.
- Original checkout `fix/ci-context-controls-fstream` at `8041799` was clean and remains preserved as a branch. No upstream/main pushes; only explicit merges and delivery-branch push are authorized.
- `gpt-6-astra` exists in installed model catalog and collaboration model list. Two Astra subagents, an independent Astra review agent, and a separate Codex task explicitly configured as Astra were launched.
- Server: 4 CPUs, 7.6 GiB RAM, 25 GiB free disk initially. Central builds limited to two jobs.

## Queue and ownership

| Track | Worker/worktree | Current state |
| --- | --- | --- |
| Town capture history and support suspension | town, `/tmp/cwr-forces-town`, `worker/forces-town` | Both call-path defects confirmed; owns ZoneRegistry, OnTownFlipped and AbstractPresence; implementing regressions |
| Counterattack admission census and 30% floor | budget, `/tmp/cwr-forces-budget`, `worker/forces-budget` | Both confirmed; owns Step/GoLive and OccupierForces; implementing regressions |
| Cleared binding migration | separate Codex task `01a14158-cac9-7bc0-a720-4597cdf70928`, `/tmp/cwr-forces-controls`, `worker/forces-controls` | Save omits cleared rows; lean fallback also restores explicit clears; investigating minimal fixes |
| Independent review and friend archive triage | review subagent, read-only | Reading latest notes from `/root/mobile-dev/dropbox/CWR Review.zip`; no archive code executed |

Workers never write the integration checkout/index/build. CounterAttack overlap is divided by function, with deliberate merge review required.

## Baseline and evidence

- CI workflow uses Clang/Ninja preset `linux-x64-clang-rwdi`, direct Foundation/Core/Poseidon test binaries with `~[GameData] ~[gamescan] ~[external-data]`, plus synthetic mod-intake lanes.
- Existing configured build regenerated successfully; 996 build actions required against upstream. Running with `-j2`; evidence in `tmp/forces-evidence/baseline-build.log`.
- CTest discovery fails on missing Studio test include, matching the documented CI limitation. Use direct binaries; this is not a patch regression.
- Python contract baseline: 9 tests passed (`baseline-contracts.log`).
- Full Remaster data absent from configured package path; no graphical, Windows, real-save or performance verification claimed.

## Review checkpoints

- Controls first pass requested changes: distinguish explicit-empty preservation from historical omitted rows; count valid bindings, not padded slots, when deciding whether displaced Q requires PageUp. Worker is revising and adding actual legacy-format cases.
- Tests-only commits queued for baseline red run: town `39d67cf`, budget `d779fd6`, controls `0690e95` (more controls cases expected). No production change integrated yet. Budget correction/additional tests: `eb54da9`; exact float trigger is 15/50 (or 30/100), not 18/60.

- Source review accepted town capture `384b283`, budget/floor `b6901d6`, and current-view presence `93cd5c9`, pending execution. Presence regression uses a new API; old-predicate mutation will supply a labeled behavioral red, while GatherInputs wiring is source-reviewed.
- Controls revision: tests `886c52e`, production `44a8254` after `bad1530`; independent re-review pending. Known v3/v4 missing rows preserve clears; older/unknown-version recovery semantics remain unchanged.

## Candidate decisions

- Friend archive latest checkpoint: 2026-10-15 1832 UTC, source review of the same 887ce31 tip. Current tracks correspond to F109/F110/F111/F113 and F093/F106. Archive claims are leads, not runtime evidence.
- Rejected F107 reserve-input normalization and F112 obsolete prose: corrected already upstream.
- Rejected reapplying original checkout commit 8041799: independent review found equivalent include fix in upstream 6163d400; current controls syntax-check passes.
- Deferred next candidates: F008 immediate-save companion resurrection, F062 unconsumed traffic event queues, F078 captured-zone traffic alarm stimulus ownership. Reviewer source notes in `/tmp/cwr-forces-review.txt`; not yet implementation-approved or runtime-reproduced.

## Additional task and deferred runtime work

- A separate user-requested Codex task (`01a1415d-fde2-7061-ab6a-05e8e41435f6`) is strengthening Market/Portrait tests using plain `/tmp` scratch copies. It must send patches to the sole integration writer; no shared build/index writes. Its model was not verified as Astra; final review will use Astra. An unused `worker/forces-regressions` worktree was created during coordination and is preserved.
- Astra source triage confirms F008 mechanism, but implementation is deferred: meaningful validation needs RemasterUD Trident kill/immediate-save/reload sequences, plus living and missing-body controls. This machine lacks the package and default runner. Saved running SQS instances embed parsed script text, so a script-only fix would not automatically repair old campaigns. No fake script emulator or text-only test will be claimed as race validation.

## Restart

Read this log, inspect all worktrees and agent messages, fetch origin, compare upstream SHA and merge new upstream commits explicitly. Preserve worker changes. Finish baseline build/tests before staging tests-only worker diffs for red cases. Review each completed patch independently, merge accepted worker commits into delivery branch, run combined CI unit and synthetic lanes, update roadmap and this log, fetch again and integrate upstream if changed, then push only `HEAD:reviewable_forces`. No PR/issues without approval.

## Executed baseline and red regressions

- Full original-upstream build passed. Normal-access CI baseline: Foundation496/496, Core826/826, Poseidon3231 passed/12 skipped, all eight synthetic mod-intake checks passed. Sandbox-only run had read-only user-directory and socket failures and hung during audio shutdown; stopped only verified own runner/test PIDs379697/379827. Normal-access rerun passed, so these are environmental. Logs: `tmp/forces-evidence/baseline-unsandboxed-*`.
- Tests-only regression build on unchanged production passed compilation. `[birthbudget],[poolfloor],[town-capture-batch],[ContextControlsConfig]`:26 cases,17 passed/9 failed,206 failed assertions. Failures reproduce capture overcount, oversubscribed births, exact30% boundary, and cleared-binding migration; log `red-regressions.log`.
- Pre-integration fetch found upstream advanced to `197a7d33a1f4c9b704bbeef38ba27dc6e2a5e28c` (arrival diagnostics, group-ID reuse, atomic air-replan band accounting). Explicit merge pending; independent Astra revalidation requested. Preserve upstream and worker EOF test additions deliberately.
