# Forces improvement campaign

For a concise explanation of the delivery branch and its accepted changes, see
[REVIEWABLE_FORCES.md](../REVIEWABLE_FORCES.md). This file retains the detailed execution history.

**Status: batch complete; reviewed changes integrated and final headless checks passed.** Earlier sections retain the investigation history; final results and restart instructions are at the end.

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

- Full original-upstream build passed. Normal-access CI baseline: Foundation 496/496, Core 826/826, Poseidon3231 passed/12 skipped, all eight synthetic mod-intake checks passed. Sandbox-only run had read-only user-directory and socket failures and hung during audio shutdown; stopped only verified own runner/test PIDs379697/379827. Normal-access rerun passed, so these are environmental. Logs: `tmp/forces-evidence/baseline-unsandboxed-*`.
- Tests-only regression build on unchanged production passed compilation. `[birthbudget],[poolfloor],[town-capture-batch],[ContextControlsConfig]`:26 cases,17 passed/9 failed,206 failed assertions. Failures reproduce capture overcount, oversubscribed births, exact 30% boundary, and cleared-binding migration; log `red-regressions.log`.
- Pre-integration fetch found upstream advanced to `197a7d33a1f4c9b704bbeef38ba27dc6e2a5e28c` (arrival diagnostics, group-ID reuse, atomic air-replan band accounting). Explicit merge pending; independent Astra revalidation requested. Preserve upstream and worker EOF test additions deliberately.

## Integration at new upstream

- Upstream 197a7d3 explicitly merged in `ee9699a`. Astra revalidation found no semantic conflict: group-ID reuse is independent of pointer-based birth accounting; air-replan band hold ends before later admission; diagnostic wording does not alter eligibility.
- Town merged `82a57cf`, budget merged `155e2c9`, controls merged `d09ce3b`. Budget merge conflicted only at EOF in the two existing regression files. Resolution preserves each entire upstream file plus the exact worker-added suffix; FakeOps retains the worker change allowing BudgetOps inheritance. No test side discarded.
- Combined build:221 affected actions. Final independent integration review and execution pending. Market/Portrait patch applied but not committed; independent Astra review accepted it and verified three old-pass/new-fail mutations.

## Combined and mutation evidence

- Combined build at 197a7d3 plus accepted worker merges passed. Focused `[counterattack],[occupierforces],[ContextControlsConfig],[town-capture-batch],[town-presence-order],[market],[portrait]`:244 cases,278813 assertions passed (`combined-focused.log`). Python contracts:9/9 passed.
- Presence behavioral red used the new callable signature but restored ONLY the original cached `!r.suspended` predicate; this is explicitly a compatibility mutation, not an unchanged-old-header build. All3 presence cases failed with10 assertions, including premature town retake and stale base meter (`presence-old-predicate.log`). Accepted source restored byte-for-byte; final rebuild/full gate pending. Production GatherInputs wiring is source-reviewed; these unit cases inject current samples rather than booting the game.
- Final independent source/conflict review preserved in `roadmap/forces-independent-review.txt`. Market/Portrait tests/evidence landed separately as `6c3dcde`.

## Batch complete: final verification and delivery

- Final accepted build passed. Unit results: Foundation 496 passed, Core 826 passed, Poseidon 3250 passed/12 skipped, total 4572 passed. All 12 skips report unavailable OpenAL device. Eight synthetic mod-intake checks and nine Python contracts passed. Full logs remain in `tmp/forces-evidence/final-*`; local scratch is preserved for restart.
- Final fetch confirms upstream unchanged at 197a7d3. All accepted source changes have independent Astra review, including final merge conflict resolution. Separate Market/Portrait task model was unspecified; its contribution was independently Astra-reviewed and mutation-validated.
- Finding → fix → evidence: capture overcount →384b283→original-production red + focused/full green; stale suspension →93cd5c9→old-predicate 3-case/10-assert red + restored focused/full green; shared budgets and 30% boundary →b6901d6→original-production red + focused/full green; cleared controls →bad1530/44a8254→original-production red + authentic known-v3/v4 fixture/focused/full green; weak Market/Portrait assertions →6c3dcde→three actual-source mutation comparisons,20-case isolated green and combined green.
- Publishing only `HEAD:reviewable_forces` is authorized; upstream and main were never pushed. No PR/issues published. Worker branches/worktrees and the original checkout branch remain preserved.

### Manual/runtime checks still needed

1. On a prepared full Remaster shadow, capture two towns in one tick; verify history/first-town response and save/reload state.
2. Enter and leave a holding force's town/base and an offset HQ near the retake threshold; observe immediate suspension/resumption and no premature retake. Unit sampling is covered; actual World/GatherInputs execution is not run here.
3. Admit multiple simultaneous foot/mounted/air operations under tight body/group budgets; verify actual spawned crews, partial births and later census recovery. This fix is not universal cap enforcement; the pre-existing strike body-cap omission is separate.
4. Load authentic user v3/v4 controls files with cleared rows; verify UI mappings and save/restart on Windows. Earlier/unknown layouts retain previous recovery, and already resurrected/saved preferences cannot be recovered from absent history.
5. Run real campaign save compatibility and companion immediate-save/death sequence once RemasterUD and Trident are available. Saved running SQS text requires a deliberate old-save strategy.

No graphical gameplay, real save compatibility, Windows behavior or performance verification is claimed. F008/F062/F078 remain reviewed leads, not landed fixes.

### Resume after this batch

Inspect branch and worktrees; fetch origin and explicitly merge any new occupier-forces tip before another batch. Read final evidence and deferred leads above. Do not redo the accepted five fixes, reapply rejected old findings, or rerun mutations against the delivery source without restoring and verifying it. Default final gate is the CI direct test binaries and synthetic tools; normal filesystem/loopback access was required here.

## Remaster setup/launch batch, 2026-10-15

- Delivered by separate task `01a14171-8b36-7783-985c-8f87eaf89bdb`: implementation and independent reviewer explicitly configured as `gpt-6-astra`. Root coordinator alone applied/committed the scratch patches; no shared worker writes or C++ build.
- Baseline delivery `9cc010d` (documentation-only changes since `a6fe940`); origin/occupier-forces refreshed and unchanged at `197a7d3`. Runtime implementation/tests committed as `d773dd5`.
- Shared Remaster layout check now covers defaults, environment and explicit paths before setup/install/build/launch. Classic fallback and Classic setup path retired. Existing Classic data/profiles/saves are preserved; no schema or profile conversion. Full-package guard is a heuristic, not authentication/version compatibility.
- Worker before/after: existing setup baseline passes; new tests on old production fail 22/28, including actual disposable Classic writes and intercepted launch. Candidate and independent reviewer pass all 28 guard checks. Root re-ran the old setup baseline (exit 0), applied content exactly matching the reviewed scratch candidate, then ran the full updated setup suite (exit 0, 28 helper/entrypoint checks and four preservation summary groups).
- Existing trusted PowerShell 7.6.6 used, no software installed. Exact coordinator invocation: `XDG_CACHE_HOME=/tmp/cwr-remaster-integration/cache XDG_CONFIG_HOME=/tmp/cwr-remaster-integration/config XDG_DATA_HOME=/tmp/cwr-remaster-integration/data /tmp/casenoter-powershell/runtime/pwsh -NonInteractive -NoProfile -File arma_CWR/tests/runtime/test-setup.ps1`. Logs: `tmp/remaster-evidence/baseline-setup.log` and `combined-setup.log`; worker red and independent logs: `/tmp/cwr-remaster-batch/evidence/`.
- The branch guide's pending Remaster row is now a landed change plus remaining runtime acceptance. README, setup docs, both CLAUDE guides and Trident example consistently target a prepared full Remaster shadow. Existing local env files and per-test pins remain unchanged.
- No C++ source changes: earlier 4,572-unit result remains tied to `6c3dcde`; it was not rerun or used to claim runtime compatibility for this scripts/docs batch. No Windows/PowerShell 5.1, actual engine startup, Steam build identity or authentic save compatibility verified. FRESH-INSTALL carries the detailed matrix and checklist.
- Coordinator independently checked the official Bohemia release note cited by the docs. Its statement about separate official Remaster/Classic profiles and incompatible Classic saves is not a claim about this fork's older UD save schemas.

### Integration ownership handoff after Remaster publication

The separate user-requested F062/F078 task `01a14176-7c0c-7ea2-a360-45a08e6ae7ca`
owns the uncommitted AlertMachine/traffic regression work. It paused shared writes,
commits and builds for this publication; those files are preserved and excluded
from the Remaster batch. Its roadmap start note is retained as in-progress.
After this docs commit/push, integration/build ownership transfers to that task.
It must keep the branch guide current with actual landed commits and test evidence;
no traffic/alarm changes are covered by the Remaster runtime test result.
