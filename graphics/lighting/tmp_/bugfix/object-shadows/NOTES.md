# Bug fix: tree and building shadows are off at start-up (object-shadows)

**Current deliverable: `fix2.patch` (round 2, after review).** `fix.patch` is the
round-1 patch, kept for the record and superseded.

| | Round 1 | Round 2 (current) |
|---|---|---|
| Scratch worktree | `D:\Arma_CWA_worktrees\fix-object-shadows` | `D:\Arma_CWA_worktrees\fix2-object-shadows` |
| Base | `claude/youthful-lovelace-8imbdh` = 3c53141 | `claude/youthful-lovelace-8imbdh` = 84408f8 (9 other fixes landed) |
| Build / game | `build/fix-object-shadows-rwdi`, `dist/fix-object-shadows-rwdi` | `build/fix2-object-shadows-rwdi`, `dist/fix2-object-shadows-rwdi` |
| Patch | `fix.patch` | `fix2.patch` (`git diff claude/youthful-lovelace-8imbdh`, new test file in the index) |

Both scratch copies are detached worktrees: no branch, nothing committed. Round 2
was made with `git worktree add --detach ... claude/youthful-lovelace-8imbdh` and
`git apply --3way fix.patch`, which applied without conflicts. `git apply --check
fix2.patch` passes against the lighting worktree.

## Summary

Fixed. Trees, bushes and buildings cast sun shadows from the first frame of a
session, as the Graphics options say. The foliage track's "shadow maps vanish when
the camera faces the sun" is the same bug.

Round 2 narrows what a newly built Scene takes from the Graphics options to the
draw-only tiers: object and vehicle shadows (the shadow tier) and smoke (the
particles tier). The terrain grid and the object LOD bias stay out of that path.
Round 1 applied them too, and the terrain grid is not only a render setting: it
changes the physical terrain heights (see "Why the terrain grid stays out"). Those
two tiers still do not reach the scene at boot; that gap is recorded under
Follow-ups.

## Review changes in round 2

| Reviewer asked for | Done |
|---|---|
| `Scene::LoadConfig` applies only the shadow tier (cloudlets may stay); the terrain grid and LOD bias stay out of the construction path, through one helper that both callers use | New `ApplyGraphicsShadowsToScene(cfg, scene)` sets object shadows, vehicle shadows and cloudlets. `ApplyGraphicsConfigToScene` (the live path, Graphics page) sets the terrain grid and LOD bias, then calls it. `Scene::LoadConfig` calls only `ApplyGraphicsShadowsToScene`. |
| Boot test: shadows (and cloudlets) take the config, terrain grid and LOD bias keep the Scene defaults; keep the Off and profile-switch cases | Done. The boot case first builds a reference Scene and reads its own terrain grid and LOD bias (12.5 m and 1.0 in this run), REQUIREs that the config's tiers differ from them (50 m, 0.5), then checks that a Scene built after the boot apply has the config's shadows and cloudlets and the reference grid and bias. Off and profile-switch cases unchanged. |
| NOTES: the terrain grid changes physical terrain heights; `compare_hightiers` is the primary round-1 after-evidence; record the terrain-grid / LOD-bias boot gap as a follow-up | Below. |

## Root cause

Line numbers are from 84408f8 (the round-2 base).

1. `Scene::Scene` starts with `_objectShadows(false)` (`engine/Poseidon/World/Scene/Scene.cpp:105`).
2. Boot applies `graphics.cfg` before the world exists:
   `apps/cwr/Game/GameApplication.cpp:681` `LoadAndApplyGraphicsConfig()`, then `:683`
   `InitializeWorld()`, which creates `GWorld` and its Scene (`:1610`). In
   `ApplyGraphicsConfigToEngine` (`engine/Poseidon/UI/Settings/GraphicsApply.cpp:74`)
   the whole scene half sits behind `if (GScene)` (`:76`), so at boot it does nothing.
   The engine half (lighting model, shadow maps, MSAA and so on) works at boot,
   because `GEngine` exists by then.
3. Only the Graphics page applies the scene half (`GraphicsPage.cpp:404`). The other
   source is a set of legacy per-profile keys: the Scene constructor reads
   `objectShadows` / `vehicleShadows` / `cloudlets` / `terrainGrid` from the user
   profile (`Scene.cpp:112` -> `LoadConfig`, `:634`, keys at `:661-685`), and `~Scene`
   writes back whatever the scene held (`Scene.cpp:214` -> `SaveConfig`, `:690`). Every
   session that never opened the Graphics page therefore saved `objectShadows=0` and
   booted with it. A profile switch (`UpdateUserProfile` -> `GScene->LoadConfig()`)
   reads those stale keys again.
4. With `_objectShadows` false, `Object::CastShadow` and `Building::CastShadow`
   return false (`Object.cpp:766-769`, `House.cpp:782-785`, `IS_SHADOW_OBJECT`), and
   the shadow depth pass skips every static caster (`SceneShadowPass.cpp:279`). Only
   spawned soldiers and vehicles (`Entity::CastShadow` -> vehicle shadows, default
   true) cast. With no spawned unit in view there are no casters at all, so
   `RenderCascadeArray` (`EngineGL33_ShadowDepth.cpp:332`) has nothing to draw and
   `_shadowMapActive` goes false (`:486`): the "shadows vanish into the sun" report.
   Every capture run starts in a fresh user dir, so every capture so far, including
   the `20261018-bugfix` baseline, had no tree or building shadows.

## The fix (fix2.patch)

| File | Change |
|---|---|
| `engine/Poseidon/UI/Settings/GraphicsApply.{hpp,cpp}` | `ApplyGraphicsShadowsToScene(cfg, Scene&)` (`.cpp:79`): shadow tier Off turns object and vehicle shadows off, Low and up turn them on; particles tier Off turns cloudlets off. `ApplyGraphicsConfigToScene(cfg, Scene&)` (`:92`): terrain grid, LOD bias, then the shadows helper; `ApplyGraphicsConfigToEngine` calls it when `GScene` exists. `ApplyGraphicsConfigToEngine` keeps a copy of the config it pushed (`std::optional<GraphicsConfig>`) and returns it through `AppliedGraphicsConfig()` (null until the first push). The tier mappings are unchanged. |
| `engine/Poseidon/World/Scene/Scene.cpp` | `Scene::LoadConfig` ends by calling `ApplyGraphicsShadowsToScene(*AppliedGraphicsConfig(), *this)` when a config was applied, after the legacy profile keys and before the view distance (`:693-696` in the patched file). |
| `engine/Poseidon/Game/Commands/GameStateExtTestViewer.cpp` | Comment only: the `triEnableShadows` doc said the scene could never get object shadows outside the Options page. The verb still makes a test independent of the machine's autodetected preset (the Low preset has shadows Off). |
| `tests/unit/engine/Poseidon/UI/Settings/test_graphicsApply.cpp` (+1 line in the tests' `CMakeLists.txt`) | Three tests, tags `[Settings][GraphicsApply][graphics]`, below. |

Why this is correct:

- `Scene::LoadConfig` runs in the two places where a Scene can come up with stale
  settings: its constructor, for every new World (boot, and the mod re-mount path,
  which re-runs `InitializeWorld` at `GameApplication.cpp:1797`), and the profile
  switch. No boot call site has to remember a second apply.
- The dedicated server, the Tetris sample and any app that never applies a
  `GraphicsConfig` get null from `AppliedGraphicsConfig()`, so their Scenes behave as
  before.
- The construction path touches only flags that change what is drawn: the two
  shadow flags (`CastShadow`, the shadow pass, the object draw-range border at
  `World.cpp:1429`, the track-mark fade in `Tracks.cpp`) and the cloudlet draw gate
  (`Scene::CloudletForDrawing`). A grep for their getters finds no simulation or
  physics reader.
- `graphics.cfg` (one per machine) now wins over the legacy per-profile shadow and
  smoke keys, which is the rule the Graphics page already follows live.

### Why the terrain grid stays out

`Scene::SetPreferredTerrainGrid` (`Scene.cpp:523`) calls `World::AdjustSubdivision`
when a World exists, and mission init calls it anyway (`WorldInit.cpp:552`). It
reads the grid (`WorldImpl.cpp:1091`) and calls `Landscape::SubdivideTerrain`
(`WorldImpl.cpp:1078`, `LandSave.cpp:931`), which interpolates the height field,
adds random relief from `_randGen` per step (`LandSave.cpp:876-893`) and replaces
the heights (`_data = result`, `:923`). `Landscape::SurfaceY` (`Landscape.cpp:1536`)
reads those heights, so the grid tier changes where the ground physically is for
every unit, vehicle and object, not only what the terrain costs to draw.
Multiplayer already pins it (`PreferredGridSizeMP` = 25, `WorldImpl.cpp:1096`).

Observed in the capture logs (`UDCAP ... eyeASL` is the camera's ground height plus
1.75 m at a fixed map point; the horizon-map line names the grid):

| view | 12.5 m grid (round-1 `before` and `after_hightiers`; round-2 baseline, `after2`, `research2_*`) | 6.25 m grid (round-1 `after`, autodetected Ultra) |
|---|---|---|
| forest_edge_golden | 44.3747 | 44.708 |
| backlit_hornbeam | 49.563 | 49.9146 |
| pine_stand_noon | 79.109 | 79.6736 |
| town_night / town_day | 105.83 | 105.847 |
| noon_fields | 121.135 | 121.135 |
| horizon map | "from 1024x1024 at 12.5 m" | "from 2048x2048 at 6.25 m" |

So round 1, on this 98 GB machine (autodetected Ultra), raised the ground under the
forest_edge_golden camera by 0.33 m and under pine_stand_noon by 0.56 m. Every
single-player session and every Trident run on a 16 GB+ machine would have played
on different terrain from a smaller machine.

The object LOD bias stays out too. Nothing reads it today: `_objectLODBias` is read
only by `GetObjectLODBias`, whose only caller outside the new test is the
`triGetObjectLodBias` verb (`GameStateExtTestAudio.cpp:504`). The Object LOD tier
has no runtime effect, at boot or live.

## Tests

`PoseidonTests [GraphicsApply]`, 3 test cases, 10 assertions:

| Code under test | Result | Log |
|---|---|---|
| fix2 (final) | All tests passed (10 assertions in 3 test cases) | `unit2_graphicsapply.log` |
| branch tip 84408f8 engine files, new test | 3 of 3 failed: object shadows false after the boot apply, cloudlets true with particles Off, vehicle shadows true with shadows Off, object shadows false after the profile reload | `unit2_before_tip.log` |
| round-1 broad apply (`LoadConfig` calling `ApplyGraphicsConfigToScene`), new test | boot case failed: terrain grid `50.0f == Approx( 12.5 )`, LOD bias `0.5f == Approx( 1.0 )`; the other two passed | `unit2_broad_apply.log` |

So the new boot case fails on both the original bug and the round-1 over-reach.
The engine files were swapped in place for the two red runs and restored; the final
test binary was rebuilt from the final sources and rerun (first row).

Requested tag run, final build (`[remaster],[graphics],[gpu],[GL33],[DebugGroup],[ShutdownOrder],[UBORestoreGuard],[StateOwnership],[StateCacheAudit],[BufferUsageAudit],[IboBindAudit],[Conventions],[Settings],[GraphicsConfig],[GraphicsApply]`):
563 test cases, 562 passed, 1 failed (`unit2_tags.log`). The failure is
`DDSConverter: generate DDS fixture from synthetic_dxt5 DXT5` (`[Graphics][DDS][.fixture]`,
pulled in by the case-insensitive `[graphics]` filter): `REQUIRE( dds2.width == 64 )`,
`32 == 64`. It fails the same way on the branch-tip test binary
(`D:\Arma_CWA_worktrees\lighting\arma_CWR\build\lighting-rwdi`, built 22:29), and
the patch touches neither the test nor the DDS converter.

Full PoseidonTests, final build: 3094 test cases, 3084 passed, 6 failed, 4 skipped
(`unit2_full.log`). The 6 failures are localization tests that read
`packages\Remaster` (`test_map_legend_localized.cpp`, `test_equipment_localization.cpp`,
`test_mp_version_message_localization.cpp`), which does not exist in the scratch
worktree; the skips report missing game data (WRP, PBO inspect).

`clang-format --dry-run --Werror` (22.1.8) exits 0 on all five touched C++ files.
Build log: `build2.log` (its 5 warnings are all in untouched Network headers).

## In-game evidence

All runs: `tests\rendering\captures\run-captures.ps1` on the hidden desktop,
`-DataJunction RemasterUD-fix2-shadows`. Each tri run starts in a fresh user dir and
autodetected `preset=3 (RAM=98117 MB)` (Ultra: shadows High, particles High,
terrain Ultra, Object LOD Ultra) in every round-2 game log.

| Folder | Binary | Test |
|---|---|---|
| `D:\Arma_CWA_worktrees\lighting\tmp\captures\20261018-bugfix` (baseline, not shot by me) | `D:\Arma_CWA_worktrees\lighting\arma_CWR\dist\lighting-rwdi` | abel, eden, sinai capture tests |
| `after2/` | `dist/fix2-object-shadows-rwdi` | `abel_lighting.test.sqf` (all 11 views) |
| `research2_before/` | the baseline's binary (`dist\lighting-rwdi`) | `evidence_scripts/objshadows2_abel.test.sqf` |
| `research2_after/` | `dist/fix2-object-shadows-rwdi` | same |

`objshadows2_abel.test.sqf` is a research test (not in the patch; it sat in the
scratch worktree for the two runs and was then removed): `forest_edge_golden__boot`
(no `triSetLighting` call, so exactly what the boot path left), `__remastered` and
`__dbgsunvis` (`triSetLighting "debug=1"`, sun visibility: white is lit), and the
same remastered / debug pair for noon_fields and town_day (town_night's camera at
17:00 on 5 July, so the houses are in sunlight; no Abel capture view shows
buildings in daylight).

Side-by-sides: `compare2/` (baseline vs `after2`, `<label>__ba.png`, plus
`noon_fields__bushes_zoom.png`) and `compare2_research/` (`research2_before` vs
`research2_after`). These are the after-evidence for `fix2.patch`.

**Terrain unchanged.** Every round-2 run logs the horizon map "from 1024x1024 at
12.5 m", and the `eyeASL` of all eight Abel views in `after2` equals the baseline
exactly (lowsun_fp 34.915, sunrise_ridge 54.355, noon_fields 121.135,
forest_edge_golden 44.3747, coastline_sunset 8.085, town_night 105.83, firefight
32.24, tank_gunner_interior 34.915), with the autodetected Ultra terrain tier in
graphics.cfg. Objects with a visible LOD and the LOD width match in every daylight
view (lowsun_fp 1,179, sunrise_ridge 7,680, noon_fields 3,920, forest_edge_golden
1,316, coastline_sunset 0; `lod=0.0208333` everywhere), and the before / after frames
line up in terrain silhouette and object placement.

What the images show (all read with the Read tool):

- **forest_edge_golden** (`compare2/forest_edge_golden__remastered__ba.png`,
  `compare2_research/forest_edge_golden__boot__ba.png`, `__dbgsunvis__ba.png`).
  Before: only the soldier casts; sun visibility is white over the whole slope and
  every tree. After: long diagonal tree and bush shadows across the slope and shade
  under the crowns, already in the boot shot with no tri lighting call.
- **town_day** (`compare2_research/town_day__remastered__ba.png`, `__dbgsunvis__ba.png`).
  Before: sun visibility white over the whole town. After: the birch throws a
  dappled shadow across the lawn, the half-timbered house and the houses at the left
  edge are in shadow, and the brick houses on the right shade their sun-away walls
  and the road edge beside them. Buildings are casters again.
- **noon_fields** (`compare2/noon_fields__remastered__ba.png`, `__bushes_zoom.png`,
  `compare2_research/noon_fields__dbgsunvis__ba.png`). Small change at a 72 degree
  sun, as expected: the bushes and trees on the far slope get short shadows (dark
  patches under each in the debug view); the near frame, soldier and jeep are
  unchanged.
- **lowsun_fp** (`compare2/lowsun_fp__remastered__ba.png`). No visible change: every
  caster in this view is spawned (jeep, M113, soldier, the CampEast tent) and cast
  before the fix too. Shadow pass 0.08 / 0.07 ms.
- **town_night**: unchanged apart from smoke and fire, which move between runs.
- **Classic** (`compare2/forest_edge_golden__classic__ba.png`): the shadow tier
  drives object shadows in Classic too, as the Graphics page always did. Classic
  draw calls went up (forest_edge_golden 1,850 -> 2,189, noon_fields 5,580 -> 5,878,
  lowsun_fp 1,622 -> 1,778) with the same object counts; the visible change in that
  frame is small.

Shadow-pass GPU time (`triGpuTimes`, the `(shadow x)` part of `world`, ms, one read
each): forest_edge_golden 0.05 -> 0.30 (boot shot 0.05 -> 0.26), town_day 0.09 ->
0.44, noon_fields 0.04 -> 0.07, sunrise_ridge 0.00 -> 0.11, lowsun_fp 0.08 -> 0.07.
Whole-frame times swing by several ms between runs here (forest_edge_golden boot
shot GPU frame 9.56 before vs 5.65 after, the opposite direction to the added work),
so these runs do not resolve whole-frame cost. Tables: `compare2/timings.md`,
`compare2_research/timings.md`.

Frame brightness (`brightness2.md`, mean display luma 0-255, whole frame / lower
half): forest_edge_golden 100.7/89.2 -> 97.0/86.0, town_day 97.2/84.0 -> 90.3/72.8,
noon_fields and lowsun_fp within 0.3.

### Round-1 evidence (base 3c53141)

`before/`, `after/`, `after_hightiers/`, `pine_diag/`, `compare/`, `compare_hightiers/`,
`brightness.md`. **`compare_hightiers/` is the primary round-1 after-evidence**: its
seeded `graphics.cfg` (`evidence_scripts/graphics_hightiers.cfg`) kept the terrain
grid at 12.5 m and the LOD bias at 1.0, so only the casters differ from `before`.
`compare/` (autodetected Ultra) is not a shadows-only comparison: that build also
moved the terrain grid to 6.25 m, which changed the terrain heights (table above)
and the terrain silhouette. Round 1 also attributed changed tree LODs to the LOD
bias; nothing reads the bias, and the round-1 rig did not yet pin object detail
(874638c landed after 3c53141), so adaptive object detail is the more likely source
(an inference, not measured). Round-1 findings that still hold: the into-the-sun
grove (`backlit_hornbeam`) is this bug. The flat shade in `pine_stand_noon` was the
clear-sky cloud shadow, which the branch has since addressed in 5cf85cf ("a clear
sky casts no cloud shadows"); not re-shot here.

## Risks

- **Only darker.** Wooded and built-up frames at a low sun get darker (town_day -7%
  mean, -13% in the lower half; forest edge -4%). The shade keeps the sky fill, but
  the shadows track's "grey shade" tuning question now applies to every such frame.
- **Crown self-shadow.** The sun-visibility view shows halftone patterns on tree
  crowns (leaf cards shading each other through alpha test), as the shadows track
  found.
- **Frame cost.** Shadow pass +0.03 to +0.35 ms GPU in these views, plus caster
  collection on the CPU.
- **Classic** boots with its projected object shadows when the shadow tier is not
  Off (more draw calls, small visible change).
- **Every later capture changes.** From this fix on, captures include tree and
  building casters; the `20261018-bugfix` baseline and earlier ones are not
  comparable for wooded or built-up views without re-shooting.
- **Legacy Video options.** Its object-shadow and smoke toggles
  (`OptionsUIImplVideo.cpp`) still work for the session, but the next Scene
  construction or profile switch re-applies `graphics.cfg`. Its terrain toggle is
  not affected by this patch.
- **Global state.** `AppliedGraphicsConfig()` is process-wide. A unit test that calls
  `ApplyGraphicsConfigToEngine` leaves it set for later tests that build a Scene. Only
  the new tests build one today, and the boot case measures its reference values
  instead of assuming them.

## Follow-ups (not in this patch)

1. **Terrain grid and object LOD bias do not reach the Scene at boot.** A Scene still
   starts with the legacy `terrainGrid` profile key or its default (12.5 m with
   hardware T&L, `Scene.cpp:106`) and a LOD bias of 1.0, whatever `graphics.cfg`
   says; only the Graphics page applies those tiers. A fresh 16 GB+ machine shows
   Terrain detail Ultra (6.25 m) in the options and plays on 12.5 m (observed in every
   round-2 run). Because the grid is physical (above), the fix needs a decision
   first: either a fixed gameplay grid for single-player and test runs with the tier
   made render-only, or terrain that follows the option in single-player with tri
   runs pinned through a seeded `graphics.cfg`. The Object LOD tier needs a reader in
   LOD selection before its boot path matters; wire both together.
2. The `[graphics]` tag pulls in the hidden `[.fixture]` DDS test, which fails on
   its own fixture (32x32 PAA, asserts 64).

## Files in this folder

- `fix2.patch` (current), `fix.patch` (round 1), `NOTES.md`
- Round 2: `configure2.log`, `build2.log`, `build2_tests_*.log`, `unit2_graphicsapply.log`,
  `unit2_before_tip.log`, `unit2_broad_apply.log`, `unit2_tags.log`, `unit2_full.log`,
  `after2/`, `after2_run.log`, `research2_before/`, `research2_after/`,
  `research2_*_run.log`, `compare2/`, `compare2_research/`, `brightness2.md`
- Round 1: `configure.log`, `build0.log`, `build1.log`, `build_tests_oldcode.log`,
  `unit_*.log`, `before/`, `after/`, `after_hightiers/`, `pine_diag/`, `*_run.log`,
  `compare/`, `compare_hightiers/`, `brightness.md`
- `evidence_scripts/`: the research tests (round 1 `objshadows_abel`, `objshadows_pine`;
  round 2 `objshadows2_abel`), the seeded `graphics.cfg`, and the Python used for the
  comparisons (`compare.py`, `bright.py`)
