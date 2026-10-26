# capture-harness: the capture rig gave misleading before pictures

Scratch worktree: `D:\Arma_CWA_worktrees\fix-capture-harness` (detached at
`claude/youthful-lovelace-8imbdh` = 3c53141, nothing committed). Build:
`arma_CWR\build\fix-capture-harness-rwdi`, game in
`arma_CWR\dist\fix-capture-harness-rwdi`. Patch: `fix.patch` (applies cleanly to
the branch tip, checked with `git apply --check`).

## The four items from MENU.md section 4, Step 0

### 1. The first shot was taken before the view had finished loading

**Root cause.** It was not view distance streaming. Objects, terrain segments
and shapes are all in memory once the island loads, and the terrain cache fills
synchronously each frame (`LandCache::Fill`). What moves is the adaptive object
detail: `Scene::AdjustComplexity` (`World/Scene/SceneDraw.cpp:778`, the
`!DENSITY_LOD` variant, the one that is built) picks the LOD width
`_lodInvWidth` from the measured frame time, and its oscillation guard refuses
any step toward finer detail for 5 s of UI time after the last step toward
coarser detail (`SceneDraw.cpp:907`, `_lastScaleWorseTime > Glob.uiTime - 5`).
A camera jump makes a slow frame, the detail coarsens, and it then stays coarse
for up to 5 s. The rig's wait (150 + 90 frames) is 2 to 6 s depending on the
frame time, so whether the first (Classic) shot fell inside the hold depended on
the machine's load. That is why the perf track's replay saw 1,465 draw calls
for the Classic sunrise_ridge shot and other runs did not.

A smaller real load does exist: the per-frame texture upload budget
(`TextureBankGL33_Cache.cpp:149`, 8 allocations a frame unless a load boost is
active) spreads mip uploads after a jump over several frames, and the
remastered terrain horizon map is built on a worker thread
(`World/Terrain/TerrainLighting.cpp`).

**Measured** (`lod_trajectory/`, research test
`research_tests/lod_trajectory.test.sqf`, fix build, same view, same jump,
stats every 10 frames): with the stock adaptive detail, sunrise_ridge drew
1,063 objects at LOD width 0.1297 (6.2 times the finest, 0.0208) from frame 30
to frame 150 after the jump; the jump was at 21:53:22.48 and the detail refined
at 21:53:25.39, 5.0 s after the previous camera jump (21:53:20.35), then drew
7,680 objects. With the detail pinned it drew 7,680 objects from frame 30.
`lod_trajectory_first_shot.png`: the adaptive shot at frame 30 differs from the
pinned one in 1,794 pixels by more than 8 levels (tree silhouettes along the
ridge line); the adaptive shot at frame 430 differs from the pinned frame-30
shot in 0 pixels. So the pinned first shot is the fully loaded picture.

**Fix.**
- `triPinObjectDetail <factor>` (new tri verb, `GameStateExtTestRender.cpp`;
  `Scene::PinObjectDetail`, `Scene.hpp`/`Scene.cpp`): 0 = adaptive (stock),
  >= 1 holds `_lodInvWidth` at that multiple of `_minLodInvWidth` (1 = finest
  the quality setting allows, 16 = coarsest, the adaptive range). The pinned
  path is an early return at the top of `Scene::AdjustComplexity` that still
  runs the per-object LOD selection. This is item 3 as well.
- `triSettleScene [maxFrames, stableFrames, tolerancePct]` (new tri verb): runs
  frames (`AppIdle`, as `triSimFrames`) until the last `stableFrames` frames
  rendered, drew draw-call counts within `tolerancePct` of each other, at one
  LOD width, with no horizon map build in flight
  (`TerrainLightingPending()`, new in `TerrainLighting.cpp`). It lifts the
  texture upload budget for each of those frames through the existing
  `AbstractTextBank::BoostLoadBudget` (the mechanism mission load already
  uses, `WorldInit.cpp:835`). Answers `OK:<frames>` or `UNSETTLED:<frames>`
  (never `FAIL`), and logs the draw range. The decision is a small pure class,
  `Dev::SceneSettle` (`Dev/Diag/SceneSettle.hpp`), so it is unit tested.
- `capture_lib.sqf` `capMode` now pins the detail (`capDetail = 1`), switches
  the lighting model, waits with `triSettleScene capSettleArgs`
  (`[300, 30, 2]`), reads the scene stats, then restarts the timers.

### 2. `triSetLighting "remastered"` kept knobs from an earlier `key=value`

**Root cause.** `TriSetLighting` (tip `GameStateExtTestRender.cpp:517-526`)
read the current tuning, changed only `remastered`, and wrote it back, so every
knob set before survived a mode switch. Two more silent traps in the same
parser: the README documented `"key=value,..."` but the code read one key and
`std::atof` stopped at the comma, so `"bounce=2.5,warmth=1.5"` answered `OK`
and set only bounce; and `atof` turned a bad value into 0 with `OK`.

**Fix** (engine, in the verb, documented in its comment and both READMEs):
`"classic"` and `"remastered"` now reset the whole `LightingTuning` to its
default-constructed value (the shipped look) and keep only the mode, the same
reset the dev overlay's "Reset knobs" button does; resetting the struct, not a
list of knobs, also covers knobs added later. Comma lists apply in order, all
or none (a mode word in the list resets what came before it), values are parsed
with `strtof` and an end check, and a refused item answers `FAIL:<why>` with
nothing applied. The knob table is unchanged, so the research prototypes' extra
rows still apply on top.

### 3. Pin the object detail for A/B shots

`triPinObjectDetail` above; the capture rig pins at 1 for every shot.

### 4. Log draw counts next to each shot

The engine counter is `gPerfDrawCalls` (GL draws, `EngineGL33_Queue.cpp:132`,
`EngineGL33_VertexBuffer.cpp:345`), handed to the frame profiler each frame
(`World.cpp:1793`). `triPerfStats` already printed its window mean
(`calls=`); nothing logged the shot frame's own count. New nular verb
`triSceneStats` returns `draws=<last frame's GL draws>;objects=<objects with a
visible LOD>;lights=<light slots>;lod=<LOD width>;pin=<factor>;horizon=<off|
pending|ready>`, fields that splice into the UDCAP line. `objects` is a new
per-frame count kept in `Scene::DrawObjectsAndShadowsPass1` (`SceneDraw.cpp`)
because draw calls are not comparable across scenes with and without light
slots (below). `capRecord` logs the stats on every `event=shot` line, `capMode`
on every `event=mode` line. `make_pairs.py` puts draws, objects, lights,
`calls_avg`, lod, pin, horizon and the load wait into `timings.csv/.json/.md`,
prints draws and objects on each pair image, and lists pairs whose objects
differ by more than 5% and modes whose wait ran out.

### Found on the way: street lamps after `setDate`

`World::SetDate` (`WorldSetup.cpp:1359`, the only caller is the SQF `setDate`)
did not tell the landscape the time changed, unlike `skipTime`
(`GameStateExtUi.cpp:1882`). Auto street lamps only switch in their own
`Simulate`, which runs every 12.1 s (`ObjectClasses.cpp:211`). The rig calls
`setDate` per view, so lamps switched at a random moment after a view crossed
dusk or dawn. Measured without the sync (`lod_trajectory_nolampsync/`, fix build
with only this change reverted): `lights=0` for steps 0 to 21 after the jump to
05:08, then `lights=12`, and the draw count went from 2,797 to 11,172 at an
unchanged 7,680 objects, because any light slot turns instancing off for the
whole scene (`SceneDraw.cpp:1529`, `NLights() == 0`) and a slot stays once a
lamp has been lit (`Scene::AddLight` reuses empty slots, `Scene.cpp:300`).
**Fix:** `World::SetDate` now calls `GLandscape->OnTimeSkipped()`, as
`skipTime` does. With it, `lights=12` from frame 30 (`lod_trajectory/`).

## Tests

New `tests/unit/engine/Poseidon/Game/test_tri_capture_verbs.cpp` (registered in
the unit CMakeLists), tags `[tri][triSetLighting][remaster]` and
`[SceneSettle][tri]`. It calls `TriSetLighting` against a test engine
(`EngineDummy` that keeps the tuning) and drives `Dev::SceneSettle`.

- **Before the fix** (the two verb files restored to the tip, everything else
  the same, `unit_tests/new_tests_before_fix.txt`): 7 cases, 3 failed, 10 of 61
  assertions failed: knobs survive `remastered` and `classic`; `bounce=2.5,
  warmth=1.5` leaves warmth at 1.0; `bounce=2.0,nonsense=1`, `bounce=abc` and
  `bounce=2.0x` answer OK.
- **After** (`unit_tests/new_tests_after_fix.txt`): all 61 assertions in 7
  cases pass.
- Requested tags on the final build, run from D: (`unit_tests/tag_runs.txt` and
  the final rerun): `[remaster]` 16 cases, `[gpu]`, `[GL33]` 104, `[DebugGroup]`,
  `[ShutdownOrder]`, `[UBORestoreGuard]`, `[StateOwnership]`, `[StateCacheAudit]`,
  `[BufferUsageAudit]`, `[IboBindAudit]`, `[Conventions]`, `[tri]` 59,
  `[SceneSettle]`, `[triSetLighting]`: all pass. `[graphics]`: 351 of 352; the
  failure is `DDSConverter: generate DDS fixture from synthetic_dxt5 DXT5`
  (`test_dds_converter.cpp:347`, `32 == 64`), a hidden `[.fixture]` test the
  case-insensitive tag pulls in; it fails alone too, my diff touches nothing on
  that path, and the clear-sky-clouds agent reports it failing on the tip rig.
  I did not run the tip's binary myself.
- Full `PoseidonTests` (one build before the last comment-only edits):
  3,061 of 3,071 cases pass, 6 fail, all `[external-data]` localization tests
  that read `packages\Remaster\BIN\*.csv`, a junction this scratch worktree
  does not have (`unit_tests/full_suite_summary.txt`).
- `clang-format --dry-run --Werror` (22.1.8): exit 0 on every touched C++ file.
  `Scene.hpp` had one pre-existing violation (`}  // namespace` at its end);
  fixed, one character.
- In game: `smoke_abel` and `smoke_eden` (assert `triSetLighting` answers OK on
  every switch) pass on the fix build (`smoke/`).

## In-game evidence (all on hidden desktops, data junction `RemasterUD-fix-capture-harness`)

| Folder | Build | Rig | Result |
|---|---|---|---|
| `before/` | branch tip `lighting\arma_CWR\dist\lighting-rwdi` | tip tests (copies in `research_tests/before/`) | 3/3 tests passed, 30 shots |
| `after/` | fix build | this patch | 3/3 passed, 30 shots, every mode `OK:` settled |
| `after_run1/` | fix build | this patch, with the mode line logging stats after the timer reset (`draws=-1` there; fixed, then `after/` was rerun) | 3/3 passed |
| `lod_trajectory/`, `lod_trajectory_nolampsync/` | fix build, the second without the lamp sync | `research_tests/lod_trajectory.test.sqf` | above |
| `isolate/`, `before_sinai_repeat/` | fix build with the tip rig; tip build repeat | attribution runs, below | |

**The before pictures were wrong, the after ones match.** BEFORE logs only the
90-frame window mean of draw calls; AFTER logs the shot frame.

| View | BEFORE calls (Classic / Remastered) | AFTER draws (C / R) | AFTER objects (C / R) | AFTER wait (C / R) |
|---|---|---|---|---|
| lowsun_fp | 1,064 / 1,379 | 1,575 / 1,568 | 1,179 / 1,179 | OK:30 / OK:30 |
| sunrise_ridge | 8,397 / 8,130 | 11,172 / 11,172 | 7,680 / 7,680 | OK:30 / OK:30 |
| noon_fields | 5,572 / 5,567 | 5,572 / 5,567 | 3,920 / 3,920 | OK:30 / OK:30 |
| forest_edge_golden | 1,850 / 1,847 | 1,850 / 1,847 | 1,316 / 1,316 | OK:30 / OK:30 |
| coastline_sunset | 155 / 155 | 155 / 155 | 0 / 0 | OK:30 / OK:30 |
| town_night | 999 / 1,164 | 1,207 / 1,220 | 1,047 / 1,051 | OK:276 / OK:30 |
| tank_gunner_interior | 2,179 / 2,236 | 2,261 / 2,272 | 1,216 / 1,230 | OK:30 / OK:30 |
| tank_gunner_optics | 1,406 / 1,392 | 1,396 / 1,377 | 743 / 743 | OK:30 / OK:30 |
| overcast_broken | 6,837 / 6,835 | 6,837 / 6,835 | 5,083 / 5,083 | OK:30 / OK:30 |
| overcast_heavy | 6,567 / 6,588 | 6,593 / 6,591 | 5,083 / 5,083 | OK:30 / OK:30 |
| sinai_lowsun_fp | 369 / 488 | 573 / 570 | 314 / 314 | OK:30 / OK:30 |
| sinai_desert_noon | 2,233 / 2,230 | 2,231 / 2,228 | 1,100 / 1,100 | OK:30 / OK:30 |

(firefight 1-3 are not matched by design: AI fire and smoke differ; their
objects differ by up to 10%, which `make_pairs.py` lists.)

`lowsun_fp_pair_detail_before_after.png` shows what this meant on screen: in the
BEFORE pair the Classic shot has the tent at a coarser LOD (no guy ropes or
pegs) while the Remastered shot has them, so the comparison mixed lighting with
model detail. In the AFTER pair both have them.
`lowsun_fp_classic_before_after.png`: BEFORE vs AFTER Classic, 4,852 pixels
differ, all on model edges (tent ropes, soldier, M113). The first shot of every
AFTER view is fully loaded by the measures above (finest LOD, steady counts,
same objects as the second shot).

**Differences between runs that this patch does not cause** (checked so they
are not mistaken for its effect):
- `sinai_desert_noon` ground texture detail differs between runs of the same
  build: tip build 29.5 then 25.0 (Laplacian standard deviation of the ground
  band), fix build with the tip rig 26.3, fix build runs 27.1 and 26.0
  (`sinai_ground_run_to_run.png`). Within each run Classic and Remastered match
  (29.5 / 29.2, 25.0 / 24.7, ...). Cause not found.
- `after_run1` coastline_sunset Remastered had the shore and near sea in shade
  (ground luma 85 against 121.5 in `before` and `after`). It did not come back
  at the same pinned `Glob.time` (6.216 s) in `isolate/`, so it is not the
  cloud field at that time; it matches the exposure track's "engine flake"
  note. `coastline_cloud_shadow_attribution.png` also shows the known
  clear-sky cloud shadow band on the far sea (gone with `cloudShadows=0`),
  which is the separate clear-sky-clouds bug. Both unresolved here.
- Run-to-run sky, smoke and idle-animation differences (the sim clock is pinned
  per view at `time` of `capBegin`, which differs between runs).

## Why this is correct

- The pin removes the only frame-time feedback into what is drawn, so both
  shots of a view select LODs from geometry alone; the AFTER pairs have equal
  object counts on every matched view. Finest is what the adaptive detail
  converges to on a fast machine (adaptive frame 430 = pinned frame 30, 0
  pixels apart).
- The settle checks the things that can still change after a jump (draws, LOD
  width, the horizon map build) and lifts the texture budget with the engine's
  own boost, instead of guessing a frame count.
- The knob reset uses the struct's defaults, which are the shipped look by the
  header's own contract (`LightRig.hpp`, "The defaults are the shipped look").
- The lamp sync is the call `skipTime` already makes for the same reason.

## Risks

- **Gameplay:** `setDate` in any mission now switches auto street lamps at once
  (one pass over the landscape objects per `setDate`; only `StreetLamp`
  overrides `OnTimeSkipped`). This is what `skipTime` does; it changes the old
  behaviour of lamps catching up within 12 s.
- **Verb semantics:** a script that sets knobs and then calls
  `triSetLighting "remastered"` now loses them (set the mode first, or use one
  comma list). Malformed values that used to answer OK now answer FAIL. The
  tracked tests do not do either; the research copies under
  `tests/rendering/research/` were not audited.
- **Timings:** captures now measure the finest detail, so CPU and GPU times on
  long views are higher than older runs' and not comparable with them
  (sunrise_ridge Classic CPU 42 fps). The per-mode wait adds at least 30 frames
  and up to 300 in animated scenes (firefight 286, town_night Classic 276).
- `lights=` explains the draw counts but the cause (any light slot disables
  instancing for the whole scene) is the perf track's light-gate fix, not done
  here. The GL draw counter does not include the shadow depth pass
  (`EngineGL33_ShadowDepth.cpp` draws are not counted).
- The compiled-out `DENSITY_LOD` variant of `AdjustComplexity` ignores the pin.
- Not changed: the harness's per-frame centre-pixel readback under
  `--autotest` that the perf track measured (separate from Step 0's list).

## Files

`fix.patch` (16 files: engine verbs, Scene pin and object count, horizon
pending, `World::SetDate`, capture lib, `make_pairs.py`, both READMEs, the Abel
test header comment, the unit test and its CMake entry), `research_tests/`
(git-excluded research tests and the tip rig copies used for BEFORE), the run
folders above (each has `tri.log`, `udcap.log`, `timings.*`, pairs and
`contact_sheet.png`), and the comparison images named above.
