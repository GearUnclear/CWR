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

## Restart

Read this log, inspect all worktrees and agent messages, fetch origin, compare upstream SHA and merge new upstream commits explicitly. Preserve worker changes. Finish baseline build/tests before staging tests-only worker diffs for red cases. Review each completed patch independently, merge accepted worker commits into delivery branch, run combined CI unit and synthetic lanes, update roadmap and this log, fetch again and integrate upstream if changed, then push only `HEAD:reviewable_forces`. No PR/issues without approval.
