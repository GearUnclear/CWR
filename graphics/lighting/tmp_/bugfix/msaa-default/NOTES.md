# Bug fix msaa-default: Remastered ships without anti-aliasing

**Current deliverable: `fix2.patch`** (round 2, after review). It is a diff
against `claude/youthful-lovelace-8imbdh` at 84408f8 (the tip after the other
nine bug fixes landed) and applies there cleanly (`git apply --check` in the
lighting worktree, exit 0). Nothing is committed.

- Round 2 scratch worktree: `D:\Arma_CWA_worktrees\fix2-msaa-default`, created
  with `git worktree add --detach ... claude/youthful-lovelace-8imbdh` (detached
  at 84408f8, no branch). `fix.patch` applied there with `git apply --3way`:
  all eight files applied cleanly, no conflicts. Build:
  `build/fix2-msaa-default-rwdi`, staged to `arma_CWR/dist/fix2-msaa-default-rwdi`.
- Round 1 (kept for reference): `D:\Arma_CWA_worktrees\fix-msaa-default`
  (detached at 3c53141), build `build/fix-msaa-default-rwdi`, patch `fix.patch`.

## Round 2: what the review changed

1. `GraphicsConfig::Migrate`: the one-time MSAA lift now also needs
   `renderScale < 1.001f` (the same threshold `ApplyPendingRenderScale` uses
   for "no supersampling"). A pre-v3 profile with supersampling on already has
   anti-aliasing, so its MSAA Off is kept. The comment says so.
2. New test `GraphicsConfig: Migrate keeps MSAA Off on a supersampled profile`:
   v2, `msaaSamples = 0`, `renderScale` 1.5 and 2.0; it migrates to `kVersion`
   and keeps `msaaSamples == 0` and the render scale. The v0/v1/v2 lift test now
   sets `renderScale = 1.0f` explicitly and still expects 4.
3. README Anti-aliasing bullet and the Risks below now say the lift applies only
   to profiles without supersampling.

Red check (`fix2_unit_red.txt`, `fix2_build_red.log`): with the renderScale
condition removed again (the round-1 rule) and PoseidonTests rebuilt, the new
test fails at `test_graphicsConfig.cpp(210)`, `CHECK( c.msaaSamples == 0 )`
with `4 == 0`, for render scale 1.5 and for 2 (1 case, 2 of 8 assertions
failed). The file was restored and the whole tree rebuilt (`fix2_build_final.log`,
exit 0) before every result below.

## Root cause

Line numbers are on the branch (the code files below are the same at 3c53141
and 84408f8, except `EngineGL33_PostFX.cpp`, quoted at 84408f8).

- `engine/Poseidon/UI/Settings/GraphicsConfig.hpp:89`:
  `int msaaSamples = 0;`. That is the factory default a fresh profile gets.
  `apps/cwr/Game/GameApplication.cpp:394-422` (`LoadAndApplyGraphicsConfig`)
  writes it to `graphics.cfg` on first boot, and `GraphicsApply.cpp:97` hands it
  to `EngineGL33::SetMsaaSamples`. `ApplyPendingRenderScale`
  (`EngineGL33_VertexBuffer.cpp:434-498`) then builds a single-sampled frame
  target, so `_msaaActive` stays false. That also switches off
  alpha-to-coverage (`EngineGL33.hpp:950`, the gate at
  `EngineGL33_State.cpp:242-243`). Cutouts fall back to a hard `discard` at
  alpha 0.753, and polygon edges are not anti-aliased at all.
- Existing profiles: `GraphicsConfig::Save` (`GraphicsConfig.cpp:286`)
  always writes `msaaSamples`. So every profile at schema version 2 holds
  `msaaSamples=0` unless the user changed the row. Changing only the default
  would not reach any existing install. A migration is needed.
- Latent trap found on the way: `GraphicsConfig::Migrate` (`GraphicsConfig.cpp:113`)
  reset `fpsCap == 0` to the refresh-rate cap without a version check. At
  `kVersion = 2` that was harmless, because only files below v2 reach that line.
  Any version bump would have silently overwritten a v2 user's chosen Unlimited.

Observed with a tool (round 1): the BEFORE runs' game logs (branch-tip build at
3c53141, fresh tri user dir) show `LoadGraphicsConfig: autodetected preset=3 ...
wrote '...graphics.cfg'` and only `GL33: frame target 960x640 (0x MSAA, render
scale 1)`, in all three game launches (`run_before/tri/*/game_stdout.log`,
`run_before/tri/retry-1/msaa_abel/game_stdout.log`).

## The fix

1. `GraphicsConfig.hpp`: new `kDefaultMsaaSamples = 4`. The factory default
   `msaaSamples` is now that constant. `kVersion` goes 2 -> 3.
2. `GraphicsConfig::Migrate`: below v3, `msaaSamples == 0` on a profile without
   supersampling (`renderScale < 1.001f`) is taken as never chosen and becomes
   4. 2x, 4x and 8x are kept, and so is Off on a supersampled profile (1.5x or
   2x already anti-aliases). From v3 on, 0 is the user's Off and survives every
   later boot. The `fpsCap` lift is now gated on `version < 2`, so the bump
   cannot touch a v2 Unlimited. This is the same rule the existing `fpsCap`
   migration already follows (a value that equals the old stamped default is
   "never set"). The boot path already calls `Migrate` and persists the result
   (`GameApplication.cpp:414`). Its log line now also prints the MSAA count.
   `Normalize` (which clamps `renderScale` to 1.0..2.0) runs after `Migrate`
   on that path, so an out-of-range scale below 1 counts as no supersampling.
3. Comments that stated the old defaults were corrected:
   `EngineGL33.cpp:295` said "8x at render scale 1, 4x at SSAA scales", and
   `EngineGL33_VertexBuffer.cpp:444` said "Both default OFF". The Remaster
   README gains one Tuning bullet on the Anti-aliasing row.
4. `test_graphics_page.cpp`: the AA row default index is now 2 (4x). The
   round trip now writes Off instead of 4x, so the row still visibly changes.

Why it is correct: the engine side already supports MSAA end to end. The frame
target is clamped to `GL_MAX_SAMPLES`, and an incomplete FBO falls back to no
AA with a warning (`EngineGL33_VertexBuffer.cpp:454, 478-487`). The post pass
resolves colour and depth itself (`EngineGL33_PostFX.cpp:521-527` at 84408f8).
Every lighting capture test on this branch already ran at `triSetMsaa 4`. The
fix only changes what a profile that never chose gets. An explicit choice is
respected, with one unavoidable exception (see Risks).

Alpha-to-coverage check (code plus images). A2C is enabled only for draws that
are alpha-tested, `AlphaMode::Test` (not TestAndBlend), `BlendMode::Opaque`, on
mesh vertex input, in the 3D pass, and with real MSAA samples
(`EngineGL33_State.cpp:232-244`). Blended alpha draws keep plain blending plus
their `ref = 1/255` reject. That covers the @LoBo camouflage net, vehicle glass
and flares. Remastered adds alpha-to-one for covered samples
(`EngineGL33_State.cpp:151-162`), so the post pass does not read cutout edges as
extra sky-light share. The round-1 images show the net intact and unchanged in
weave, and the foliage cutouts graded instead of stair-stepped.

## Tests

Cases in `tests/unit/engine/Poseidon/UI/Settings/test_graphicsConfig.cpp`:
- factory defaults now also pin `msaaSamples == 4`, `alphaToCoverage`, and `renderScale == 1`;
- `a v2 Unlimited cap survives the v3 migration` (guards the fpsCap trap);
- `Migrate turns an untouched MSAA Off into 4x` (from v0, v1, v2, at render scale 1.0);
- `Migrate keeps MSAA Off on a supersampled profile` (v2 at 1.5 and 2.0; round 2);
- `Migrate keeps an MSAA count the user chose` (2, 4, 8 at v2);
- `MSAA Off survives once the file is current`;
- `an old file gets 4x MSAA once, a later Off sticks` (file round trip: v2
  file with 0 -> Load -> Migrate -> 4 -> Save; reload is v3 with no migration;
  user sets Off -> Save -> reload keeps 0).

`test_graphics_page.cpp`: AA row default is index 2.

Round 2 results (`fix2_unit.txt`, PoseidonTests from the fix2 build, run from `D:\`):
- `[GraphicsConfig]` 30/30 cases (186 assertions); `[GraphicsPage]` 11/11 (274);
  `[Settings]` 184/184 (2919); the new case alone 1/1 (8 assertions);
- `[remaster]` 27/27 (3612), `[gpu]` 3/3 (29), `[GL33]` 107/107 (575);
- audits all pass: `[DebugGroup]` 6, `[ShutdownOrder]` 8, `[UBORestoreGuard]` 1,
  `[StateOwnership]` 4, `[StateCacheAudit]` 8, `[BufferUsageAudit]` 4,
  `[IboBindAudit]` 4, `[Conventions]` 7 cases;
- `[UI]` 282/288: the 6 failing cases are `test_map_legend_localized`,
  `test_equipment_localization` and `test_mp_version_message_localization`.
  They read `arma_CWR\packages\Remaster\...` game data, which no scratch
  worktree has. The same 6 fail in the branch-tip integration run
  (`../integ_full_poseidon.txt`);
- `[graphics]` 364/365: `test_dds_converter.cpp:347` (`32 == 64`, a hidden
  `[.fixture]` case that a tag filter selects) fails the same way on the
  branch-tip build `lighting/arma_CWR/build/lighting-rwdi` (364/365, same line).
  Both failures are unrelated to this change.

Round 1 (`unit_before.txt`, `unit_after.txt`) showed the then-new cases red on
the unfixed `GraphicsConfig` (6 failed) and green on the fix.

clang-format 22.1.8 `--dry-run --Werror`, whole file, fix2 vs branch tip:
`GraphicsConfig.cpp`, `GameApplication.cpp`, `test_graphicsConfig.cpp` and
`test_graphics_page.cpp` are clean (0). `EngineGL33.cpp` 3 -> 3 and
`EngineGL33_VertexBuffer.cpp` 15 -> 15, none on changed lines (violations at
lines 13, 266, 267 and 17 to 675 outside the changed hunks at 295-297 and
444-446). `GraphicsConfig.hpp` 146 -> 148: that header is tab-indented,
column-aligned upstream code that was never clang-formatted, and the two new
lines (comment and constant) keep the file's own tab style.

## In-game evidence

All runs go through `tests/rendering/captures/run-captures.ps1` on a hidden
desktop, 960x640 windowed. The tests are research copies (git-excluded) under
`<scratch>/arma_CWR/tests/rendering/research/msaa-default/`:
`msaa_abel.test.sqf` (forest_edge_golden) and `msaa_sinai.test.sqf`
(sinai_lowsun_fp with @LoBo's camouflage net). Both are the stock capture
blocks with `triSetMsaa` and `triSetRenderScale` removed, so each game runs on
the default a fresh profile gets (tri gives every test an empty user dir).

Round 2, `run_after2/` (`-GameDir dist/fix2-msaa-default-rwdi
-DataJunction RemasterUD-fix2-msaa-default`, capture_lib from 84408f8, so with
the new object-detail pin and settle): 2/2 passed. Both game logs
(`run_after2/tri/msaa_*/game_stdout.log`) show `LoadGraphicsConfig:
autodetected preset=3 ... wrote '...graphics.cfg'` and then `GL33: frame
target 960x640 (4x MSAA, render scale 1)`. Both views settled (`OK:30`).
The pairs (`*__pair.png`) render normally: trees, soldier, HMMWV, M113 and the
camouflage net all present. The round-2 change only affects pre-v3 profiles
with supersampling on, which a fresh tri profile is not, so this run checks
that the rebased fix still gives a new profile 4x; the supersampling rule is
covered by the unit tests above.

Round 1 (at 3c53141):
- `run_before/`: branch-tip build `lighting/arma_CWR/dist/lighting-rwdi` (not
  rebuilt). Log: `frame target 960x640 (0x MSAA, render scale 1)` only.
  The first `msaa_abel` copy lacked `triEndTest`, so that attempt timed out
  after its screenshots, and tri's retry (`tri/retry-1/`) re-ran it with the
  fixed file. Both attempts are the same build and settings, and both logs
  show 0x.
- `run_after/`: fix build `dist/fix-msaa-default-rwdi`. Log: first-boot
  `wrote '...graphics.cfg'`, then `frame target 960x640 (4x MSAA, render
  scale 1)` in both games. Both tests passed (2/2).

Round-1 figures in `figures/` (BEFORE left, AFTER right; crops are nearest-neighbour x3):
- `sinai_lowsun_fp__remastered__crop0_x3.png`: the HMMWV roof, hood and wheel
  silhouettes lose their stair steps. Before, the radio antenna showed only as
  two or three isolated dots. After, it is a continuous dotted line. The
  camouflage net (blended draw) keeps the same shape, weave and translucency.
- `sinai_lowsun_fp__remastered__crop1_x3.png`: net close-up. The net is intact
  (no hole or opacity change, which is the failure mode A2C would cause on a
  blended draw), with smoother outer edges.
- `forest_edge_golden__remastered__crop0_x3.png`: mid-range pines. Cutout
  crowns are graded at the edges instead of hard 1-pixel steps. The trunks are
  anti-aliased and the crowns are not thinned.
- `forest_edge_golden__classic__crop1_x3.png`: the same on Classic programs (A2C
  without alpha-to-one). Thin twigs at the birch top appear as graded lines.
- Full frames: `*__before_after.png`; raw shots in `run_before/` and `run_after/`.

Light is unchanged. `figures/luma.md` gives the whole-frame mean luma change,
after minus before: -0.03 (forest remastered), -0.04 (forest classic), -0.03
(sinai remastered), +0.00 (sinai classic), on a 0-255 scale. Crops range from
-0.16 to +0.04. That is edge pixels only: no haze, no darkening, no tint.

GPU time (`run_*/timings.md`, smoothed `triGpuTimes`; other agents' games shared
the GPU, so treat these as rough): round 1 forest_edge_golden remastered
3.78 ms -> 4.67 ms, sinai_lowsun_fp remastered 9.46 -> 7.38 ms. The noise is
larger than the effect at 960x640. The post-pass column grows by the MSAA
resolve: 0.12 -> 0.19 ms (forest) and 0.20 -> 0.62 ms (sinai; ao 0.08 -> 0.50
is the biggest part). Round 2 at 4x (`run_after2/timings.md`): forest 5.42 ms
(post 0.42), sinai 4.72 ms (post 1.25). Not measured here: 1080p cost, and
motion shimmer. The aa research track measured shimmer with 4x MSAA at 1.0x vs
no AA on the same views (track `aa`, `shimmer_v4v6.txt`, run_v6): the
town_night tree residual went 6.96 -> 5.39 and its p99 82.7 -> 34.4; on
forest_edge_golden, 4.81 -> 3.79 and p99 51.7 -> 28.7.

## Risks

- A v2 profile without supersampling (render scale 1.0) where the user
  deliberately chose Off is lifted to 4x once. The file cannot tell that apart
  from the never-touched default, because every save wrote 0. The existing
  `fpsCap` migration accepts the same trade. After that one boot, Off sticks.
  Explicit 2x/8x are never touched, and a profile with supersampling on (render
  scale above 1.0) keeps its Off, because it already has anti-aliasing.
- The default now also applies to the Classic lighting mode, since MSAA is one
  per-machine knob. The captures show Classic only gains edge AA (luma
  unchanged). Classic is no longer "bit-identical" to its no-AA look at the
  default; set Anti-aliasing Off to get that back.
- The engine still starts single-sampled and recreates the frame target at 4x
  when the config applies at boot (two `frame target` log lines about 0.1 s
  apart, seen again in `run_after2`). That is a one-time reallocation, which I
  left in place to keep the engine-level default and non-game GL33 users
  (Tetris, Studio) unchanged.
- Cost: 4x MSAA multiplies frame-target memory (RGBA8 + D24S8, 4 samples)
  and adds the resolve and post-pass work above. The aa track rates 4x at
  native resolution as near free, but nobody has timed it at 1080p on an
  RTX 3060.
- The stale Pester smoke tests `tests/smoke/graphics_config.tests.ps1` (they
  expect brightness 1.2 and gamma 1.0, and a verbatim version-less file) were
  already out of date before this change and were not run. A v2 file there is
  now rewritten to v3 on boot.
- Not done here (aa track items, out of scope): the 1.5x render-scale preset,
  the Lanczos2+RCAS present pass, the coverage-threshold shader change, and
  FXAA when MSAA is off.

## Files in this folder

- `fix2.patch`: round 2 deliverable (diff vs the branch at 84408f8).
- `fix.patch`: round 1 patch (vs 3c53141).
- Round 2 logs: `fix2_configure.log`, `fix2_build.log`, `fix2_build_red.log`,
  `fix2_build_final.log`, `fix2_unit.txt`, `fix2_unit_red.txt`,
  `run_after2.console.txt`, `run_after2/`.
- Round 1: `configure.log`, `build*.log`, `unit_before.txt`, `unit_after.txt`,
  `run_before*`, `run_after*` (not `run_after2`), `figures/`, `make_figures.py`.
