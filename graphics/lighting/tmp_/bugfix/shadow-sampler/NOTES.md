# shadow-sampler: texture unit 2 held texture 0 under the shadow samplers

Bug from MENU.md section 5: "A GL undefined-behaviour warning (shadows, clouds):
a shadow comparison sampler bound to texture 0". Fixed in the scratch worktree
`D:\Arma_CWA_worktrees\fix-shadow-sampler` (detached at
`claude/youthful-lovelace-8imbdh` = 3c53141, nothing committed). Patch:
`fix.patch` (3 files, +117 lines: 11 engine lines, the rest a unit audit).

## Two warnings, one hole

NVIDIA's KHR_debug output (driver 610.88, RTX 4060 Ti) reports two messages.
The engine's callback logs each id only once per run (dedup by id,
`EngineGL33.cpp:91-103`), so a later hit of the same id is never shown.

| id | severity | text (shortened) | when |
|---|---|---|---|
| 131204 | LOW | texture object (0) bound to texture image unit 2 does not have a defined base level | every run of the branch tip, at start-up, about 0.1 s after the frame target is created and before the world loads |
| 131222 | MEDIUM | sampler (9) with depth comparisons enabled, with a texture object (0) with a non-depth format, by a shader that samples it with a shadow sampler: undefined behaviour | in the research logs only in the shadows and clouds tracks (13/13 and 10/12 game logs; 0 in every other track folder); reproduced here on the unmodified tip build by a shadow-map resolution change in Remastered |

Both come from the same place. Every lit program samples the shadow map on
unit 2, target `GL_TEXTURE_2D_ARRAY`:

- classic programs: `uniform sampler2DArray shadowMap` (`EngineGL33_Shaders.cpp`
  lines 226, 346, 463, 575, 667, 693), set to unit 2 at line 1684;
- remastered programs: `uniform sampler2DArrayShadow shadowMap`
  (`EngineGL33_LightingShaders.cpp:283`), read through the comparison sampler
  object `_shadowCompareSampler` that `UpdateRemasterPrograms` binds to unit 2
  (`EngineGL33_Lighting.cpp:187` at the tip) and `BindLightingTextures` rebinds
  (line 271).

Both declarations are live uniforms (the read sits behind a dynamic
`shadowCtl.x` branch), so the driver validates unit 2 at every draw whether or
not the branch is taken. Texture 0 there is an incomplete texture to the classic
sampler (131204) and undefined behaviour under the comparison sampler (131222).

## Root cause (line numbers are the branch tip's)

1. **Start-up.** `InitLightingResources` creates the 1x1 depth stand-in
   `_dummyDepthArray` (`EngineGL33_Lighting.cpp:88`) on the upload unit (7) and
   unbinds it there. Nothing puts a texture on unit 2's array target until
   `UpdateShadowMapLitState` binds a live shadow map
   (`EngineGL33_Shaders.cpp:1146`) or `BindLightingTextures` binds the stand-in
   (`EngineGL33_Lighting.cpp:270`). That function returns early unless the
   remastered programs are active (line 265). The first classic draws (loading
   screen, menu) therefore run with texture 0 on unit 2: 131204. With Classic
   lighting and shadow maps off, that state lasts the whole session.

2. **Recreating the cascade array.** `EnsureArrayTarget`
   (`EngineGL33_ShadowDepth.cpp:151`) deletes the old array at line 157 when the
   shadow-map resolution or cascade count changes. GL then treats every unit
   that held it as bound to texture 0. The replacement is bound at line 167 on
   whichever unit happens to be active, not on unit 2. The depth pass runs
   inside the world pass (`SceneDraw.cpp:1708-1726`), with the remastered
   program still current: `RenderCascadeArray` captures it and restores it
   after the pass. The first `clear::WithMask` at line 388 also runs before
   `glUseProgram(s_prog)` at line 398. In Remastered, unit 2 only gets the new
   array back when `RenderShadowDepthScene` calls `UpdateShadowMapLitState`
   after the pass (line 514). In between, the comparison sampler sits on
   texture 0: 131222. The two reproduction runs below place the warning in the
   frames right after `triShadowSetRes 2048` in Remastered. I did not
   instrument the driver, so I don't know which GL call between line 157 and
   line 514 the driver flagged.

   From reading the code (not observed on screen): Classic has the same hole
   and a worse result. `RenderShadowDepthScene` rebinds only when
   `_lightingActive`, so after a Classic resolution change, the draws later in
   that frame run the classic shadow test, still armed, against texture 0. An
   incomplete texture reads depth 0, and `sc.z - bias > 0` holds for every
   receiver inside a cascade, so those draws are shadowed for that one frame.
   The startup 131204 masks this case in the log, because the callback logs
   each id once.

   The research prototypes hit the MEDIUM warning more readily. For example,
   the clouds track's `SetRemasterSkyMode` binds the comparison sampler before
   the 3D pass without binding a texture to unit 2.

## The fix

The rule: once the lighting resources exist, unit 2's `GL_TEXTURE_2D_ARRAY`
always holds a complete depth array. That is the 1x1 stand-in until a shadow
map exists, then a cascade array. Two edits close the two holes:

- `EngineGL33_Lighting.cpp` `InitLightingResources` (fixed file lines 111-117):
  after the stand-ins are made and the bind cache is invalidated, bind
  `_dummyDepthArray` on unit 2 (`GL33Bind::ActiveUnit(2)`, `glBindTexture`,
  back to unit 0). This runs at every `InitPixelShaders`: start-up, soft reset
  and shader reload.
- `EngineGL33_ShadowDepth.cpp` `EnsureArrayTarget` (fixed file lines 166-169):
  select unit 2 before `glGenTextures` / `glBindTexture`, so the new array goes
  straight onto the unit the deleted one emptied. Only `glDeleteFramebuffers`
  and `glGenTextures` run between the delete and the new bind, and neither
  validates program state.

Nothing else changes: `BindLightingTextures` and `UpdateShadowMapLitState`
keep their logic, there are no per-draw binds, and no shader changes.

### Why it is correct

- Both texture kinds are complete under every sampler that reads unit 2. The
  stand-in is `GL_DEPTH_COMPONENT24`, 1x1x1, `NEAREST`, `MAX_LEVEL 0`. The new
  cascade array has level 0 allocated by `glTexImage3D` and a `NEAREST` filter
  with no mips, so it is complete even if its FBO then fails. Both are depth
  formats, which the comparison sampler requires. With no sampler object bound
  (classic, `glBindSampler(2, 0)`), the texture's own compare mode is `NONE`,
  which the classic raw-depth read requires.
- What they contain does not matter. Both programs read the map only when
  `shadowCtl.x` is armed, and `UpdateShadowMapLitState` arms it only with a live
  map bound. The stand-in's depth of 1.0 would read as fully lit under both
  tests anyway (`LEQUAL` against a reference no greater than 1 passes;
  `sc.z - bias > 1.0` fails).
- Keeping the array bound on unit 2 while the depth pass renders into it is not
  a feedback loop. GL 3.3 makes one undefined only when the current program can
  sample that unit, and the depth programs never do (`s_prog` has no sampler;
  `s_alphaProg` samples unit 0). It is also the state of every steady frame on
  the tip: unit 2 already held the live array when the next depth pass rendered
  into it.
- `EnsureArrayTarget` now leaves unit 2 active. The bind cache records that.
  Every texture bind in the backend picks its unit, through `GL33Bind` or an
  explicit `glActiveTexture`, so none inherits it. On the tip the unit active
  after the pass already varied (unit 0 after alpha casters, otherwise
  whatever was active before).
- Cost: one bind at init and one per array re-creation (a settings change).
  There is no per-frame or per-draw work, so no GPU cost.
- The look: the before and after screenshots match (numbers below). This
  removes UB; it is not a look change.

## Tests

New source audits, tag `[ShadowSampler]` (also `[Graphics][GL33][StateCacheAudit]`),
appended to `tests/unit/engine/Poseidon/Graphics/test_gl_state_cache_audit.cpp`,
in the style of the I-03/I-04 audits there. The unit-2 binding is GL state, and
no unit test here owns a GL context, so the audits read the backend sources:

1. "Shadow-map unit 2 holds the depth stand-in from init on": in
   `InitLightingResources`, after the stand-in is made, `ActiveUnit(2)` and then
   `glBindTexture(GL_TEXTURE_2D_ARRAY, _dummyDepthArray)`, with no unit change or
   cache invalidation between them.
2. "A recreated cascade array takes unit 2, where the deleted one was sampled":
   in `EnsureArrayTarget`, after `glDeleteTextures(1, &s_arrTex)`,
   `ActiveUnit(2)` and then the bind of the new array, with no unit change
   between them.
3. "The texture-array target is unbound only on the upload unit": the one
   `glBindTexture(GL_TEXTURE_2D_ARRAY, 0)` in the backend is in
   `UploadTerrainHorizonMap`, after `ActiveUnit(kUploadUnit - GL_TEXTURE0)`. A new
   unbind anywhere else has to be added to this list.

Results (full output in `unit_tests.txt`; `PoseidonTests.exe` from
`build/fix-shadow-sampler-rwdi`, run from the D: drive):

- **Before** (the two engine files temporarily replaced by the tip versions;
  the audits read source at run time, then the fix was restored):
  `[ShadowSampler]` 3 cases, 1 passed, **2 failed** (cases 1 and 2,
  `REQUIRE(unit2 != npos)`).
- **After**: `[ShadowSampler]` 3/3 passed (21 assertions).
- Sweep with the fix: `[remaster]` 13 cases / 207 assertions, `[gpu]` 3/29,
  `[GL33]` 107/575, `[DebugGroup]` 6/67, `[ShutdownOrder]` 8/36,
  `[UBORestoreGuard]` 1/3, `[StateOwnership]` 4/40, `[StateCacheAudit]` 8/69,
  `[BufferUsageAudit]` 4/40, `[IboBindAudit]` 4/31, `[Conventions]` 7/25,
  `[Shadow]` 2/23, `[Shaders]` 5/60: all passed.
- `[graphics]`: 354 of 355 cases passed. The one failure is the hidden fixture
  case `DDSConverter: generate DDS fixture from synthetic_dxt5 DXT5`
  (`test_dds_converter.cpp:347`, `dds2.width == 64` read 32). It also fails when
  run alone. It reads `paa/synthetic_dxt5.paa` and runs the DDS converter, and
  this patch touches neither. The shaft-veil fix agent reports the same failure.
  I did not run it on a tip build, so calling it pre-existing is an inference.
- clang-format 22.1.8 `--dry-run --Werror`: `EngineGL33_Lighting.cpp` and
  `test_gl_state_cache_audit.cpp` are clean. `EngineGL33_ShadowDepth.cpp`
  reports violations only at lines 470-472, the `rendered` expression in
  `RenderShadowDepthScene`. The tip has the same violation at line 466
  (checked with `git show HEAD:... | clang-format --dry-run --Werror`). I left
  it alone to keep the diff minimal; my four added lines are clean.

## In-game evidence

All runs went through `tests\rendering\captures\run-captures.ps1` on the hidden
desktop, with `-DataJunction RemasterUD-fix-shadow-sampler` (the Remaster shadow
`D:\Arma_CWA\tmp\remaster-shadow-67`). The cleanup stopped only that junction's
games and left other agents' games running (see the `LEFT ALONE` lines in the
run logs). BEFORE used the tip build
`D:\Arma_CWA_worktrees\lighting\arma_CWR\dist\lighting-rwdi`, which I did not
rebuild. AFTER used `dist/fix-shadow-sampler-rwdi`.

Tests:
- `smoke_abel.test.sqf` (unchanged capture smoke): Classic, Remastered,
  Classic, Remastered, one shot each.
- `research_test/sampler_paths_abel.test.sqf` (git-excluded research copy, also
  saved here). It goes through every path that can empty unit 2: start-up;
  Classic with shadow maps (`triEnableShadowMaps`); `triShadowSetRes 1024` and
  `triShadowSetCascades 2` in Classic; then Remastered with
  `triShadowSetRes 2048` and `triShadowSetCascades 4`; the shadow-map
  self-test from script (`triGetShadowMapCacheTest`, which recreates the array
  at 256 px with 1 cascade outside the world pass; it answered 1 in every
  run); then Classic and Remastered again.

| run | test | id 131204 | id 131222 | tri |
|---|---|---|---|---|
| before | smoke_abel | 1 (21:35:46.373, start-up) | 0 | passed |
| before | sampler_paths_abel | 1 (21:35:29.755, start-up) | 1 (21:35:42.606) | passed |
| before-rerun | sampler_paths_abel | 1 (21:39:35.295) | 1 (21:39:47.116) | passed |
| after | smoke_abel | 0 | 0 | passed |
| after | sampler_paths_abel | 0 | 0 | passed |
| after-rerun | sampler_paths_abel | 0 | 0 | passed |

The logs are in `<run>\tri\<test>\game_stdout.log`. In `before-rerun`, the
131222 lands right after the "simulated 30 frames" that follows
`triSetLighting remastered` (21:39:47.093) and before the next "simulated 10
frames" (21:39:47.296). That window is the frames that follow
`triShadowSetRes 2048`. In the after runs, no other unit shows up as a 131204
either. Before the fix, the once-per-id log could have hidden one behind unit
2's. The remaining GL lines (131169 renderbuffer info, 131218 shader recompile,
131154 pixel transfer) are performance and info messages. They appear in the
before runs too and are not part of this bug.

Images (not a visual bug; they show the fix leaves the frame unchanged):
- `res_change_remastered_before_after.png`: the Remastered shot after the
  resolution and cascade change, before-rerun on the left, after-rerun on the
  right.
- `smoke_classic_before_after.png`: smoke shot 1 (Classic), before and after.
- `before\`, `after\` hold `make_pairs.py`'s pairs and contact sheet for the
  smoke run. Its "missing captures" note is expected: the smoke labels
  alternate modes and do not form pairs.

Pixel differences (mean absolute difference over RGB, 0-255; share of pixels
differing by more than 8):
- before-rerun vs after-rerun, all four research shots: mean 0.008 to 0.014,
  0.00% of pixels over 8.
- after vs after-rerun: mean 0.007 to 0.011, 0.00%.
- The first before run's Remastered shot differs from both the after run and
  its own rerun by the same amount (mean 0.36, 1.42%). The difference sits on
  the soldier, the shadow edges and the distant trees, so it is that run's
  frame-to-frame variation, not the fix. The smoke run's last Remastered shot
  shows the same (1.45%). I have no rerun of the smoke test to separate it
  further.

## Risks and limits

- Tested on one NVIDIA driver only. AMD, Intel and Mesa were not run. The fix
  uses only core GL 3.3 binds.
- The audits match source text. A refactor that renames
  `InitLightingResources` or `EnsureArrayTarget`, or rewrites the binds, has to
  update them. That is intended, as with the I-03/I-04 audits.
- A shader reload or soft reset now puts the stand-in on unit 2 even if a live
  map was bound there. The reload runs in `InitDraw` before `BeginPass` rebinds
  the map, so no world draw should see the stand-in, but this was not
  exercised in game (no reload in these runs).
- Out of scope, noticed only: `s_arrTex` and `s_arrFbo` are file statics that
  are never released and are not reset on a context re-creation. A raw-depth
  second binding of the array on another unit, as in the shadows track's
  prototype on unit 6, would need the same rule for that unit if it is adopted.
- The scratch worktree is still at `D:\Arma_CWA_worktrees\fix-shadow-sampler`,
  with build `build/fix-shadow-sampler-rwdi`, dist `dist/fix-shadow-sampler-rwdi`,
  junction `packages\RemasterUD-fix-shadow-sampler` and the git-excluded
  research test under `tests\rendering\research\shadow-sampler\`.
