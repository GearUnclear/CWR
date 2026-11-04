# XP-cputhreads: CPU render preparation on the existing worker pool

Lane xp-cputhreads, base 243ef1e0. Worktree `D:\Arma_CWA_worktrees\sq-xp-cputhreads` (detached, nothing committed). Patch: `items\XP-xp-cputhreads\patch.diff` (the game code is identical to `patch-r3.diff`; patch.diff also carries the last bench-test tweak). Frozen build for the queued jobs: `D:\Arma_CWA_worktrees\sq-xp-cputhreads\arma_CWR\dist\sq-xp-cputhreads-r3-rwdi` (exe sha256 bdf107c603a2...).

## Status in one paragraph

Built, unit-proven exact, and timed offline in a micro-benchmark. The in-game proof (same-exe stills with a built-in serial cross-check) and the in-game timing are queued but had not run when this report was written: at 08:26 the stills job was 44th in a queue that drains about 3 to 5 jobs an hour (16 older EXP-A timed jobs sit ahead of it at the same priority), so they will finish some time tomorrow. No pixels and no in-game numbers yet. Nothing here is proven pixel-identical in the game yet; it is designed and unit-tested to be.

## What it does

The engine already has a worker pool: `Core/TaskPool` (enkiTS, one process-wide instance, 8 threads including the main thread, `ParallelFor` and `Submit`). Until now only the terrain segment fill and audio decode used it. The experiment moves five pieces of the main thread's render preparation onto it, each built so the frame cannot change:

| part | on the pool | why the result is the same |
|---|---|---|
| sort | the four draw-list sorts per frame (occluders by distance, Pass1 by shape, Pass2 by surface order and by alpha depth): key reads, quicksort, final gather | the quicksort is split exactly where the serial quicksort splits (its own partition step, line for line); each side's sort depends only on that side, so sorting the sides on different threads gives the same order, ties and NaN keys included |
| hill | the hill cull's horizon tests for every static object | the same test code; a horizon wedge is built once, by whichever thread needs it first (atomic claim), with the same contents; flags, texture hints and counts stay in list order on the main thread |
| shadow | the shadow-map pass's per-frame copies of cached static casters (the camera-relative subtract) | space is reserved in the order the serial copies grow the buffers; every float is the same subtraction |
| lod | `Scene::AdjustComplexity`: each object's covered area (the serial loops computed it up to 4 times per object) and the final LOD searches | the same pure functions on the same inputs; the float sums stay serial and in order |
| prep | the per-object visibility decision of the object queue (`ObjectForDrawing`) | split into a read-only decision (pool) and the list update (main thread, in the original call order) |

Not done, on purpose: the object occlusion pass (each test depends on the occluders already drawn, front to back), the opaque draw loop and anything that issues GL calls, `Object::PassNum` (virtual, overridden by men and vehicles) and the texture mip hints (they mutate texture state). Land fit is already about 0 ms at steady state since W1-9 (landfit=0 in the bench counters), so there was nothing left to thread there.

## Switches (all default off; off runs the tip's code)

- `POSEIDON_XP_CPUTHREADS=1` turns on every part; or a list such as `sort,hill` (names: sort, hill, shadow, lod, prep).
- `POSEIDON_XP_CPUTHREADS_VERIFY=1` also runs the serial code beside each part and counts any difference (proof runs only, slower).
- `POSEIDON_XP_CPUTHREADS_LOG=1` logs `XPCT stages ...` every 256 frames: mean and max ms of each threaded stage and of the rest of the main thread's object work (`p1loop` Pass1 draw loop, `p2surf`, `p2alpha`, `smrender`), on either side, so one exe compares both; with VERIFY the line ends with `| verify` and the difference count per stage (all must be 0).

## Proof so far

- Build green through squad-heavy, no new warnings.
- Unit gate: PoseidonTests 3529 passed, 6 failed (the known data six: localization csv/html, pbo inspect, wrp), 4 skipped; PoseidonCoreTests 823 passed, 4 skipped; PoseidonFoundationTests 475 passed.
- New exactness tests, tag `[xp-cputhreads]` in `test_draw_list_sort.cpp` and `test_terrain_occlusion.cpp`: the parallel sorts against the original QSort comparators on random lists full of ties, duplicates, NaN and infinities (sizes 0 to 9,000, forced parallel even on tiny lists), sorted, reversed and all-equal input, element by element and with reference counts unchanged; the parallel quicksort against the serial one on ints that carry their original position (any tie reorder fails); the concurrent horizon tests against the serial ones on hilly terrain from 6 eyes with 3,000 objects each, including the number of wedges built. All pass (2,541,703 assertions with the existing drawsort and occlusion cases).

## Offline timing (micro-benchmark, not the game)

Hidden Catch2 case `[.xpbench]` (`tests\unit\engine\Poseidon\World\Scene\test_xp_cputhreads_bench.cpp`), run through squad-heavy while other lanes were building, so the numbers are indicative only. 8,000 entries spread over the heap like the valley's draw list, the workers asleep before each repetition as between two stages of a frame. Medians, quietest run (items\XP-xp-cputhreads\xpbench4.log):

| stage | serial | on the pool |
|---|---|---|
| empty pool dispatch | | 0.002 to 0.005 ms |
| Pass1 shape sort | 0.58 ms | 0.35 ms |
| occluder distance sort | 0.30 ms | 0.19 ms |
| Pass2-style re-sort of a shape-sorted list | 0.29 ms | 0.22 ms |
| hill tests, still camera (8,000 objects in a 90 degree cone) | 0.43 ms | 0.12 ms |
| hill tests, camera moving (table rebuilt every frame, 140 wedges) | 0.83 ms | 0.19 ms |

A first cut split the quicksort a fixed four levels deep and left one side holding 3,408 of 8,000 entries, so the shape sort hardly moved (0.72 to 0.68 ms). Splitting adaptively (any side above max(256, n/32), up to 10 levels) and starting the largest sides first fixed it.

Estimate for the valley at VD 4,000 (not measured): about 0.6 to 0.8 ms off the sorts, 0.25 ms off the hill cull with a still camera and about 0.6 ms with a moving one, 0.1 to 0.3 ms each from lod and prep, unknown from shadow: roughly 1 to 1.5 ms of the about 9.3 ms of CPU work on the W3-11 batch build, more at sunrise (hill cull 0.83 ms there). That is CPU margin, not a frame-rate change at these views: the valley and Eden already sit under 15 ms p95 on both sides. It matters more on a slower CPU and in busy scenes.

## Finding beyond this lane

The hill cull's horizon table is rebuilt whenever the eye moves at all (`HorizonTable::Matches` compares the eye exactly), so in play, with the camera always moving, the hill cull costs about twice what the static bench views show (hillcpu 0.37 ms valley, 0.83 ms sunrise, wedges=0 there). The bench cannot see this because its cameras stand still. The concurrent wedge build is this lane's biggest win in motion.

## Queued jobs and how to read them

Both same exe (r3 dist), each dry-run alone with exit 0:
- `queue\pending\60-20261103-065000-XP-xp-cputhreads.json` (stills, B = on with VERIFY and LOG, A = off with LOG, POSEIDON_DETERMINISTIC_WORLD=1, settle 300, 8 views: valley_0515, broken_high, sunrise_ridge, noon_fields, town_night_rain, combat_fires, bay_offshore, sinai_desert_noon). Out: `items\XP-xp-cputhreads\results\20261103-065000-stills-sameexe-r3`. Pass = every view IDENTICAL in vgate's class column (its claim column refuses same-exe pairs) and every `XPCT stages ... | verify` line in the B boots' game_stdout.log showing 0 for every stage.
- `queue\pending\61-20261103-065001-XP-xp-cputhreads.json` (quick, timed, B = on, A = off, both LOG, views valley_0515, broken_high, sunrise_ridge, 4 rounds). Out: `...\results\20261103-065000-quick-sameexe-r3`. Read cpu_work median and p95 per view in bench.md, and compare the `XPCT stages` lines of the A and B boots stage by stage (sortshape, hill, prep and so on). GPU must not move.

If the stills fail on one view, rerun with a part list (for example `POSEIDON_XP_CPUTHREADS=sort,hill`) to find the part; VERIFY names the stage directly.

## Risks

- prep and lod read engine objects from worker threads. Everything they call was read for purity (camera clip tests, `GetShapeOnPos` including RandomShape's position hash, `IntersectWithGround`, the LOD tables, `FindNearestWithoutProperty`). A diagnostic draw mode (cheat DiagMode) falls back to serial. A future lazy cache inside any of those functions would break the assumption; VERIFY catches it.
- enkiTS `WaitforTask` lets the main thread run any pending task of the same priority while it waits, including an audio decode submitted earlier: a possible rare stall. Worth a priority split if a hitch shows.
- The pool is capped at 8 threads (`--max-threads`); on a CPU with fewer fast cores the gain shrinks, but no stage gets slower than its dispatch cost (a few microseconds).
- Motion: nothing temporal changes by design (same draw lists), so no owner clip is needed unless the stills show a difference.

## Recommendation

Once the stills come back IDENTICAL with zero verify differences and the timing job shows the stages faster: on by default for every preset (a pure performance change that keeps the frame), sort and hill first since they are the most clearly exact and the largest; prep and lod after one more look at VERIFY counts in a moving-camera run. Until then: off.
