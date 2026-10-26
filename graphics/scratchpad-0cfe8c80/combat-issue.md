Combat light (step B4 of the `graphics-remaster` integration) was shelved on 2026-10-18 and must be finished. In the research it was the strongest "different game" result: muzzle flashes, shell bursts, burning wrecks, fires and flares light the men and ground around them and cast long shadows at night.

## Where the work is

- Shelved patch: `tmp/integration/B4/SHELVED.patch` in the `graphics-remaster` worktree (48 files, 4125 insertions). It applies cleanly on 706b9a3 but conflicts in 23 files on the current tip, because six later steps landed on top of it.
- Notes and the review findings: `tmp/integration/B4/SHELVED.md` (same folder).
- Scratch copy with the work: `D:\Arma_CWA_worktrees\gr-B4` (detached at 706b9a3).
- Research background: `arma_CWR/engine/Poseidon/Graphics/Remaster/research/combat.md`. Plan: `research/INTEGRATION-PLAN.md`, section 5 B4.

These paths are on the development machine; the patch is not in the repository yet.

## Blocker found in review

`CombatLightSet::Tick` (`CombatLights.cpp`) skips any step longer than `kCombatMaxTickStep = 0.25 s`. `World::Simulate` multiplies the step by the time acceleration before calling it, so under `setAccTime 4` (the Showcase uses it, `escalation.sqs:34`), any frame slower than 1/16 s means no burst ever ages. Flashes then freeze fully lit, fires and flares stop flickering, and the burst list grows without bound, because bursts are only removed in `Tick` and at scene teardown.

## To do

- [ ] Rebase the shelved work onto the current `graphics-remaster` tip and resolve the 23 conflicts.
- [ ] Fix the time-acceleration blocker: age bursts by real elapsed time, or clamp and split long steps; never skip aging. Add a unit test that fails today.
- [ ] Fold in the earlier critic findings: the shadow flipping between two fires, the centre-column seam, and a flash frame that is darker than the original look's.
- [ ] Gate per plan section 8, with combat views at dusk and night, feature off vs on.
- [ ] Land it on `graphics-remaster` and add it to the side-by-side comparison video.
