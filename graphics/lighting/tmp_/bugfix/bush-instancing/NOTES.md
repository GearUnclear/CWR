# bush-instancing: grouped bushes take the first bush's terrain fit

Branch: `claude/youthful-lovelace-8imbdh` at 3c53141. Scratch worktree (detached, left in
place): `D:\Arma_CWA_worktrees\fix-bush-instancing`. Nothing committed, no branch created.

## Root cause

The opaque pass draws runs of identical static shapes as one instanced draw
(`Scene::DrawObjectsAndShadowsPass1`, branch tip `SceneDraw.cpp:1521-1615`). The run's first
object (the head) goes through `DrawSortObject` once inside `BeginInstancedRunUpload` /
`EndInstancedRun` (`SceneDraw.cpp:1589-1593`), and every TL section is then drawn K times with
each member's world matrix (`EngineGL33.hpp:911-934`, `EngineGL33_Mesh.cpp:43-60`). Everything
else in that draw is the head's, including its vertex buffer.

`Object::Draw` calls `Animate(forceLOD)` before drawing and `Deanimate` after
(`Object.cpp:858`, `Object.cpp:946`). For a shape with land-following vertices,
`Object::Animate` (`Object.cpp:378-428`) rewrites every `ClipLandKeep` / `ClipLandOn` vertex so
it sits on the terrain under *that* object (`GLandscape->SurfaceY` per vertex, back into model
space through the object's inverse transform) and invalidates the vertex buffer. Bushes are such
shapes. `Object::IsAnimated` reports exactly this case (`Object.cpp:228-231`), plus damage
(`:217`) and destruction (`:221-227`). `EntityAI::IsAnimated` returns true for EntityAI
subclasses that keep its override (`VehicleAICombat.cpp:1236-1239`), but buildings do not:
`Building::IsAnimated` (`House.cpp:670-674`) calls `Entity::IsAnimated` (`Simul.cpp:1359-1366`),
which is `Object::IsAnimated` or `_animations.Size() > 0`. So only buildings with animations
(doors), damage, destruction or land-fit hints leave a run, not every building (correction from
the review). `ForestPlain::IsAnimated` is true for the non-skewed forest blocks
(`ObjectClasses.cpp:405-408`).

The grouping predicate never asked that question. The head test
(`SceneDraw.cpp:1555-1558` at the tip) and the member test (`SceneDraw.cpp:1571-1574`) checked
static, proxies, routing, shape, LOD, pass, render flags and the fog band, so a run of bushes was
batched and every member was drawn from the head's refitted vertices: each bush took the head's
terrain fit, floating over or sinking into its own slope, sheared to the head's ground.

Today this only shows where instancing runs at all: the head test also requires
`NLights() == 0` (`SceneDraw.cpp:1529`), and on Abel the street lamps hold 12 light slots
(perf track logs: `lights=12`), so the tip never instances there. A mission on a map with no
light object hits it, and so will any change that lets instancing run with lights present (the
perf track's per-object light gate, menu item 3).

## The fix

`fix.patch` (4 files, +218 / -14):

- `engine/Poseidon/World/Scene/InstancedRunRules.hpp` (new, header-only, no engine
  dependencies, like `SurfaceDrawOrder.hpp` / `AlphaSortOrder.hpp`): the per-object grouping
  rules as pure functions over a POD `RunEntry` (shape, draw LOD, pass, special, distance2,
  static, `ownVertices`). `CanHead` (`:38`) = static and not `ownVertices`. `CanJoin` (`:47`) =
  the old member test (same shape, LOD, pass, special, static, 0.90x to 1.10x fog band) and
  not `ownVertices`.
- `SceneDraw.cpp`: `InstancedRunEntry` (`:1290`) fills a `RunEntry` from a `SortObject`;
  `ownVertices` is `Object::IsAnimated(drawLOD)`, asked only when the object has the head's
  shape and LOD (a later object's draw LOD can be `LOD_INVISIBLE`, and `IsAnimated` indexes the
  shape by it; any other shape or LOD fails `CanJoin` anyway). The head test (`:1579`) uses
  `CanHead`, the member loop (`:1591`) uses `CanJoin`; the scene and routing gates (local
  lights, proxies, OnSurface, IsColored, camera object) and the engine's 256-instance cap are
  unchanged. The em dash in the comment block I touched is gone.
- Tests: `tests/unit/engine/Poseidon/World/Scene/test_instanced_run_rules.cpp` (new, tags
  `[scene][instancing]`), registered in `tests/unit/engine/Poseidon/CMakeLists.txt`.

Why it is correct: `IsAnimated` is the engine's own contract for "appearance changed with
Animate" (`Object.hpp:306`), the predicate the component code already uses to decide whether
an object's animated geometry invalidates the shared shape's components (`Object.cpp:527-577`). An object for which it is false draws the shape's own,
unmodified vertices, which is what an instanced member gets; an object for which it is true is
drawn alone through its own `Animate`, exactly as with instancing off. Instancing is not
disabled: static shapes without per-object vertices (most trees, skewed forest blocks, rocks,
props) still batch. Skewed forests (`t1` / `t2`, skew carried in the transform matrix) report
false and keep batching, correctly, since each member's own matrix carries its skew.

## Tests

New test file, 4 cases, 17 assertions: a run of five land-following bushes yields five
single draws; a damaged copy among eight twins splits the run into 3 + 1 + 4; the older rules
(shape, LOD, pass, special, static) and the fog band edges are pinned.

- **Before** (the helper with the pre-fix rule, i.e. without the two `ownVertices` terms, which
  is exactly the tip's predicate): 2 of 4 cases fail, 4 assertions:
  `CanHead(OwnVertices())` true, run lengths `{ 5 } == { 1, 1, 1, 1, 1 }` and
  `{ 8 } == { 3, 1, 4 }`. Output: `unit_tests_before_fix.txt`.
- **After**: `[instancing]` all pass (17 assertions in 4 test cases).
- Requested tags on the fix build (`unit_tests_after_fix.txt`): `[scene]`, `[remaster]`,
  `[gpu]`, `[GL33]`, `[DebugGroup]`, `[ShutdownOrder]`, `[UBORestoreGuard]`,
  `[StateOwnership]`, `[StateCacheAudit]`, `[BufferUsageAudit]`, `[IboBindAudit]`,
  `[Conventions]` all pass. `[graphics]`: 351 of 352 pass; the one failure is
  `DDSConverter: generate DDS fixture from synthetic_dxt5 DXT5`
  (`test_dds_converter.cpp:347`, `32 == 64`), a hidden `[.fixture]` test that the
  case-insensitive `[graphics]` filter pulls in. Its fixture `tests/fixtures/paa/synthetic_dxt5.paa`
  is 32x32 (`PoseidonTools image inspect`) and has not changed since the reroot commit; my diff
  touches nothing on that path. Not caused by this fix (I did not run the tip's test binary to
  confirm, to avoid writing into the lighting worktree's build dir).
- clang-format 22.1.8 `--dry-run --Werror`: clean on `SceneDraw.cpp`,
  `InstancedRunRules.hpp`, `test_instanced_run_rules.cpp`.

## In-game evidence

Captures via `run-captures.ps1`, junction `RemasterUD-fix-bush-instancing`, 960x640, 4x MSAA,
1.5x scale. Research test (git-excluded, copied to `research_test/`): Abel `noon_fields`
unchanged from the capture set, plus `noon_bushes_zoom` (same eye, heading 164, pitch -3,
fov 0.25) onto the bush-covered slope. Pixel counts: max channel difference, pixels over 2,
pixels over 8.

**1. BEFORE (tip build `dist/lighting-rwdi`) vs AFTER (`dist/fix-bush-instancing-rwdi`).**
As expected on Abel, no difference: the tip never instances there.

| View | before_repeat vs after | after vs after_repeat (noise) |
|---|---|---|
| noon_bushes_zoom, Remastered | max 1, 0 px > 2 | max 1, 0 px > 2 |
| noon_bushes_zoom, Classic | max 1, 0 px > 2 | max 1, 0 px > 2 |
| noon_fields, Remastered | max 11, 1 px > 2 | max 111, 579 px > 2 (soldier idle) |
| noon_fields, Classic | max 3, 1 px > 2 | max 3, 1 px > 2 |

The first BEFORE run's `noon_fields__classic` is an outlier against every other run of
either build (6 to 16 levels brighter at the sampled pixels, sky included, and 2,728 draw calls against
1,423 in the three other runs); `before` vs `before_repeat` shows the same difference, so it is
run-to-run variation of the first shot after boot, not the build. Images:
`before_after_noon_bushes_zoom.png`, `before_after_noon_fields.png`.

**2. The bug and the fix with instancing actually running.** To make the bug visible on Abel I
built a research-only variant (`dist/fix-bush-instancing-demo-rwdi`, NOT part of `fix.patch`;
its hunk is `research-demo-on-top-of-fix.patch`): environment variable `UD_RESEARCH_INST` bit 1
turns on the perf track's per-object light gate (instancing when the object's own light list is
empty), bit 2 restores the pre-fix grouping. Same build, same views, only the grouping differs:

| View | demo pre-fix (`=3`) vs demo fixed (`=1`) | tip (instancing off) vs demo fixed |
|---|---|---|
| noon_bushes_zoom, Remastered | max 79, 1,059 px > 2, 352 > 8 (rows 0-250) | **max 1, 0 px > 2** |
| noon_bushes_zoom, Classic | max 71, 1,094 px > 2, 353 > 8 | **max 1, 0 px > 2** |
| noon_fields, Remastered, horizon rows 250-350 | max 107, 748 px > 2, 383 > 8 | 5,948 px > 2 (see below) |
| noon_fields, Classic, rows 250-350 | max 96, 480 px > 2, 328 > 8 | 6,511 px > 2 (see below) |

On the zoom view the fixed instanced frame is identical to the tip's non-instanced frame: the
fix is exact there. The pre-fix frame differs on the far bushes (`diff_heat_noon_bushes_zoom_prefix_vs_fixed.png`,
`zoom_crop_detail_tip_vs_bug_vs_fixed.png`). On `noon_fields` the pre-fix vs fixed count (383 px
over 8 in the horizon band, Remastered) is the same number the perf track measured; the crop
`noon_fields_crop_bug_vs_fixed.png` (6x) shows the bush cluster at 150 to 300 m sheared and
reshaped under the bug and matching the tip with the fix. The remaining tip vs demo-fixed
difference on `noon_fields` is not the fit bug and not caused by the fix. Correction from the
review: it is not adaptive LOD or streaming either, since the tip's `noon_fields` images are
stable run to run (1 px > 2) even though its draw calls swing 1,423 to 5,458. Of the 4,096
Remastered px > 8 in that residual, 4,092 lie inside tip-vs-prefix and none inside
prefix-vs-fixed, so it is common to both demo grouping modes: an effect of the research light
gate or other instanced objects (a slight blue shift, Classic too), left for the perf track to
find before the light gate ships. It is not bushes: the heat map of
pre-fix vs fixed (`diff_heat_noon_fields_prefix_vs_fixed.png`) puts the bush differences only on
the bushes, and the soldier's head (idle animation, present in every repeat).

Draw calls per frame (triPerfStats `calls`, my runs): zoom view tip 2,426 (all four tip and
fix runs), demo pre-fix 823, demo fixed 1,253; `noon_fields` Remastered demo pre-fix 1,123,
demo fixed 1,640. Frame times in the logs are not comparable (3 to 4 other agents' games were
running).

The research log line `RESEARCH bush-instancing: UD_RESEARCH_INST=... lights=0` is printed on
the first pass-1 frame, before the lamps exist; it shows the variable reached the game, not the
light count during the shots.

## Risks

- **Cost once instancing runs.** Every object whose `IsAnimated` is true now draws alone:
  land-following bushes, buildings with animations, damage or land hints (not every building; see the root cause), damaged and destroyed
  objects, non-skewed forest blocks. Measured here with the research light gate: +430 draws on
  the bush slope (823 to 1,253), +517 on `noon_fields` Remastered, still about half the tip's
  count. The perf track measured sunrise_ridge 2,796 to 6,461 with the gate alone and 1,549 to
  1,728 with its terrain occlusion in front. On the tip with Abel there is no cost, since
  instancing is off there (tip 2,426 draw calls on the zoom view in all four tip and fix runs,
  against 823 to 1,253 once the research gate lets it run). Eden, Cain and Sinai also carry lamp
  models (`lampazel` / `lampadrevo` in the Cain and Eden name tables, 38 objects with "lamp" in
  the model name on Sinai by `PoseidonTools terrain objects`), so they are probably in the same
  state; not verified in game. Ways to win draws back, none needed for correctness: a GPU-side terrain fit (land
  offset in the vertex shader from a height texture), the perf track's far-only exception
  (animated objects batch again beyond 400 m, near-exact, not exact), or a narrower
  `Building::IsAnimated` after an audit of what `Entity::Animate` changes per house.
- One extra virtual `IsAnimated` call per pass-1 object, also while instancing is gated off by
  lights. A few loads per object; not measured, expected far below 0.1 ms at 7,000 objects.
- Behaviour change outside bushes: damaged or animated buildings that used to batch on a
  lamp-free map now draw individually, which is the correct look (their own damage and door
  state) instead of the head's.
- The noon views are daytime; night and the other capture views were not shot. The change is
  CPU-side grouping only and does not touch shaders or lighting.

## Files here

- `fix.patch`: `git -C D:\Arma_CWA_worktrees\fix-bush-instancing diff claude/youthful-lovelace-8imbdh` (new files added with `git add -N`).
- `research-demo-on-top-of-fix.patch`: the research-only evidence hunk (diff of `SceneDraw.cpp`, fixed vs demo). Not for the branch.
- `research_test/`: the capture test used for all runs.
- `unit_tests_before_fix.txt`, `unit_tests_after_fix.txt`.
- `captures/{before,before_repeat,after,after_repeat,demo_gate_prefix,demo_gate_fixed}/`: raw captures, pairs, timings, `udcap.log`, tri logs.
- `before_after_noon_bushes_zoom.png`, `before_after_noon_fields.png`, `zoom_crop_detail_tip_vs_bug_vs_fixed.png`, `zoom_crop_bushes_tip_vs_bug_vs_fixed.png`, `noon_fields_crop_bug_vs_fixed.png`, `diff_heat_*.png`.
