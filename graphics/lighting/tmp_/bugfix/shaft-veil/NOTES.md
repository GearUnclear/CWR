# Bug fix: shaft-veil (the sun-shaft composite lays a grey veil on nearby vehicles)

Scratch worktree: `D:\Arma_CWA_worktrees\fix-shaft-veil` (detached at `claude/youthful-lovelace-8imbdh` tip 3c53141, nothing committed). Build: `arma_CWR/build/fix-shaft-veil-rwdi`, staged to `arma_CWR/dist/fix-shaft-veil-rwdi`. Patch: `fix.patch` in this folder (`git diff claude/youthful-lovelace-8imbdh`, 7 files, +122 / -6).

## Root cause

Line numbers are those of the branch tip (`D:\Arma_CWA_worktrees\lighting\arma_CWR\engine\PoseidonGL33\EngineGL33_PostFX.cpp`).

- The shaft pass `s_psPostShaftsGLSL` (line 180) marches from each pixel toward the sun and sums the colour of every sample whose depth is the far plane (line 198, `sum += s * sky * decay` at line 200). Its result is the light an endless view ray would scatter toward the eye. It does not look at the depth of the pixel it is computed for.
- The composite `s_psPostCompositeGLSL` adds that term to every pixel at full weight (line 232: `texture(shaftTex, vUV).rgb * shaftColor.rgb * compParams.z`, screen-blended at line 233; the weight comes from `_lighting.sunShafts * 0.6` at line 672). A pixel on a vehicle 10 m away gets the same shaft glow as the sky next to it.
- Physically the shafts are sunlight scattered by the air between the eye and the surface, so a surface 10 m away has 10 m of air in front of it and almost no in-scatter. A backlit vehicle whose screen-space ray toward the sun crosses open sky takes the sky's full glow, which in display space lifts its dark shaded side toward grey: the veil. Measured on the tip build (this folder, `before/`): the Jeep side in `lowsun_fp` reads 63.7 of 255 with shafts on and 27.8 with `shafts=0`; the HMMWV side in `sinai_lowsun_fp` reads 57.0 against 18.4. The same mechanism lays the warm streak over the near ground under the sun in `sunrise_ridge` and the orange glow on the cypress silhouettes in front of the sun.

## The fix

1. `LightRig.hpp:133-148`, `LightRig.cpp:254-264` (pure math, unit-tested): `ShaftInscatter(distance, length) = 1 - exp(-distance / length)`, the share of the gathered shaft light that the air in front of a surface at `distance` metres scatters, and `ShaftScatterLength(fogEnd) = min(kShaftScatterLength, fogEnd / kShaftFogLengths)` with `kShaftScatterLength = 300 m` and `kShaftFogLengths = 3`. In clear weather the length is 300 m (293 m at the 900 m default view distance, 300 m from about 900 m up); fog or rain that pulls the engine's fog end in shortens it, so thick haze carries its shafts nearer. An unset fog range (0) falls back to 300 m.
2. `EngineGL33_PostFX.cpp`, composite shader (fixed file lines 232-254): a new `shaftInscatter()` reads the full-resolution resolved depth (`textureLod`, nearest, no implicit derivatives), reconstructs the euclidean distance along the view ray with the same `projInfo` / `depthMap` / `worldRect` conventions as the AO pass (so a pillarboxed world rect and both clip-control depth ranges work), returns 1 for the sky (the same `>= 0.999999` test the shaft pass uses), and the shaft term is multiplied by it before the screen blend. The depth is fetched only where the shaft term is non-zero.
3. `EngineGL33_PostFX.cpp`, `EndWorldScene` (fixed file lines 696-705): binds the already resolved `p.depthTex` on unit 5 (its sampler is already reset at the start of the pass and the unit is already unbound at the end, so the GL state hand-back is unchanged) and sets `projInfo`, `depthMap` (z = 1 / `ShaftScatterLength(GScene->GetFogMaxRange())`) and `worldRect` on the composite program.
4. Comments: the PostFX header comment and the README Post pass row say what the shafts now do.
5. Lab mirror (`tests/rendering/lighting_lab/postfx.py`, `remaster.py`): the composite gets the same new uniforms, the two constants are read from `LightRig.hpp` with the lab's existing `constexpr float k...` parser (checked: it returns 300.0 and 3.0), and `fog_end` is passed from the lab preset. Without this the lab would have rendered shafts on the sky only (depthMap.z = 0). The lab itself was not run: `moderngl` is not installed on this machine; both files parse (`ast.parse`).

## Why it is correct

- It is the in-scatter integral of a homogeneous medium, `1 - exp(-sigma d)`, with the radial-blur result taken as the value for an endless ray (the sky). The sky keeps exactly what it had (weight 1), distant ground keeps nearly all (1 km: 0.96), and nearby surfaces keep next to nothing (7 m: 0.02, 10 m: 0.03, 16 m: 0.05). The 300 m length is the value the shade research track tuned and its critic reviewed (`research\shade\NOTES.md`, "In-scatter shafts"); the research prototype read the half-resolution AO target's depth and so only worked with AO on, and bled across silhouettes; this version reads full-resolution depth and does not depend on AO.
- The evidence below matches the expectation pixel for pixel: the difference map is black over the sky and lights up only on near geometry and near ground.
- Nothing outside the composite changes: the shaft pass, its weight, the bloom, AO and dither logic are untouched, and Classic never runs the post pass.

## Tests

New Catch2 case `Remaster sun shafts brighten the air, not the surfaces near the eye` (`tests/unit/engine/Poseidon/Graphics/test_remaster_lighting.cpp:488`, tags `[Graphics][Shaders][remaster]`):
- `ShaftScatterLength` at the fog ends of 900, 1500 and 4000 m view distances is at most 300 m and above 270 m; at those lengths the acceptance props keep under 3% (soldier 7 m), 4% (Jeep 10 m) and 6% (M113 16 m) of the shafts, ground at 1 km keeps over 95%, the sky (infinite distance) keeps exactly 1, and the share rises monotonically with distance; an unset fog range gives 300 m; a 150 m fog end gives 50 m and at least doubles the share at 30 m.
- A GLSL contract: the composite declares `uniform sampler2D depthTex;`, contains the `1.0 - exp(-max(dist, 0.0) * depthMap.z)` falloff and applies `shaft *= shaftInscatter()`, and the engine passes `1.0f / scatterLength`.

Before / after (observed):
- With the branch-tip `EngineGL33_PostFX.cpp` copied into the scratch tree (the test reads the source at run time), the case FAILED: 4 of 149 assertions, all four GLSL contract checks (`test_before_tip_source.txt`). The pure-function half cannot run before the fix at all, because `ShaftInscatter` / `ShaftScatterLength` do not exist there. The fixed file was restored afterwards (diff re-checked).
- With the fix: passed, 149 assertions.

Tag sweep on `PoseidonTests.exe` from the D: drive (`unit_tests.txt`): `[remaster]` 14 cases / 356 assertions passed; `[gpu]` 3 / 29; `[GL33]` 104 / 554; `[DebugGroup]` 6 / 67; `[ShutdownOrder]` 8 / 36; `[UBORestoreGuard]` 1 / 3; `[StateOwnership]` 4 / 40; `[StateCacheAudit]` 5 / 48; `[BufferUsageAudit]` 4 / 40; `[IboBindAudit]` 4 / 31; `[Conventions]` 7 / 25; `[Shaders]` 6 / 209, all passed. `[graphics]`: 352 of 353 passed; the one failure is `DDSConverter: generate DDS fixture from synthetic_dxt5 DXT5` (`test_dds_converter.cpp:347`, `dds2.width == 64` read 32), a hidden `[.fixture]` case that fails the same way when run alone. It reads `paa/synthetic_dxt5.paa` and runs the DDS converter, neither of which this patch touches; I did not run it on a tip build, so "pre-existing" is an inference from the diff, not an observation.

clang-format `--dry-run --Werror` exit 0 on `LightRig.hpp`, `LightRig.cpp`, `EngineGL33_PostFX.cpp`, `test_remaster_lighting.cpp`. The build (`cmake --build build/fix-shaft-veil-rwdi -j 6`, all targets) finished with exit 0 and no compiler errors (`build.log`). The game logs of both capture runs show no post shader compile or link error.

## In-game evidence

Runs through `tests\rendering\captures\run-captures.ps1` with `-DataJunction RemasterUD-fix-shaft-veil` on the hidden desktop, focused research-only tests `tests\rendering\research\shaft-veil\abel_shaftveil.test.sqf` and `sinai_shaftveil.test.sqf` (git-excluded; they copy the `lowsun_fp`, `sunrise_ridge` and `sinai_lowsun_fp` views of the committed capture tests verbatim and add a third shot per view, Remastered with `shafts=0`, as the no-shaft reference).
- BEFORE: `before\`, the branch-tip build `D:\Arma_CWA_worktrees\lighting\arma_CWR\dist\lighting-rwdi` (not rebuilt).
- AFTER: `after\`, `dist/fix-shaft-veil-rwdi`.
- Copies of the focused tests: `research-tests\`. Image scripts: `scripts\cmp.py`, `scripts\measure.py`.
- Comparisons: `cmp_<view>.png` (before | after | shafts off), `crop_<view>.png` (2x crops, before | after), `diff_<view>_x6.png` (|before - after| luma x6), `measure.json`.

Box luma (display 0-255; "veil" = shot minus the same build's `shafts=0` shot; `scripts\measure.py` in this folder, boxes from the shade track's `measure.py` / `boxes.json`):

| view, box | before | after | shafts off | veil before | veil after |
|---|---|---|---|---|---|
| lowsun_fp Jeep side front (10 m) | 63.7 | 31.7 | 27.8 | +35.9 | +3.9 |
| lowsun_fp Jeep side rear | 65.0 | 41.0 | 39.4 | +25.6 | +1.6 |
| lowsun_fp M113 side (16 m) | 43.7 | 31.9 | 30.9 | +12.8 | +0.9 |
| lowsun_fp tent (22 m) | 63.1 | 42.9 | 40.7 | +22.5 | +2.2 |
| lowsun_fp soldier (7 m) | 67.8 | 59.2 | 58.9 | +8.8 | +0.3 |
| lowsun_fp far hills | 77.0 | 76.3 | 73.4 | +3.6 | +2.9 |
| lowsun_fp sky near the sun | 193.3 | 193.2 | 185.5 | +7.8 | +7.8 |
| sinai_lowsun_fp HMMWV side (10 m) | 57.0 | 22.3 | 18.4 | +38.6 | +3.9 |
| sinai_lowsun_fp HMMWV side 2 | 50.7 | 21.9 | 18.8 | +31.9 | +3.0 |
| sinai_lowsun_fp camouflage net | 51.8 | 46.1 | 45.8 | +6.0 | +0.3 |
| sinai_lowsun_fp sky top | 170.0 | 170.0 | 167.7 | +2.3 | +2.3 |
| sunrise_ridge near ground in the beam (about 10-70 m) | 50.3 | 44.1 | 43.5 | +6.8 | +0.6 |
| sunrise_ridge ridge, left | 131.7 | 131.7 | 130.3 | +1.4 | +1.4 |
| sunrise_ridge sky near the sun | 171.2 | 171.2 | 162.3 | +8.9 | +8.9 |

What the images show (read with the Read tool): in `crop_lowsun_fp.png` and `cmp_sinai_lowsun_fp.png` the BEFORE Jeep and HMMWV are flat grey-violet cut-outs; AFTER they are solid, with their paint, wheels and interior detail readable, and indistinguishable from the shafts-off reference. The sky, the sun's rays and the far hills look the same before and after. In `crop_sunrise_ridge.png` the BEFORE warm streak over the near ground below the sun and the orange glow on the cypresses are gone AFTER (the cypresses are dark backlit silhouettes); the rays in the sky around the sun are unchanged. `diff_lowsun_fp_x6.png` and `diff_sunrise_ridge_x6.png` are black over the whole sky and bright only on the props, the cypresses and the near ground in the beam.

Run-to-run noise: the two builds' `shafts=0` shots agree on Abel (pixels differing by more than 3: 0.06% in `lowsun_fp`, 0.18% in `sunrise_ridge`; frame means equal to 0.01). On Sinai 9.8% of pixels differ by more than 3 between the runs although the frame means agree (101.67 vs 101.70); something in that view moved between runs, so Sinai pixel diffs are less clean than Abel's. The box values above compare each shot with its own run's reference.

GPU time: composite 0.49 / 0.03 / 0.08 ms before and 0.59 / 0.03 / 0.04 ms after (lowsun_fp / sunrise_ridge / sinai_lowsun_fp, `before\timings.md`, `after\timings.md`), single smoothed readings on a GPU shared with other test games: no measurable change. The added work is one nearest depth fetch, a reciprocal and an exp per full-resolution pixel where the shaft term is non-zero.

## Risks

- **Darker near ground under a low sun.** The warm beam over the ground in front of the camera is gone (sunrise_ridge beam region 50.3 to 44.1, red/blue 1.77 to 1.47; shafts-off 43.5). It is still far brighter than Classic there (24.8 in the same run). The research critic flagged the same loss ("the sunrise foreground loses its sun-shaft streak"). The beam on the near ground and the veil on near vehicles are the same term at the same distance, so no scattering length keeps one and removes the other; `kShaftScatterLength` only moves how much of the shafts sits on ground 100 m to 1 km away.
- **Backlit spawned vehicles read darker in luma.** Their veil-lifted value goes back to what the surface itself shows (Jeep side 31.7, same as Classic's 27.8 within the residual 3 to 4). In this build that surface is still the Classic per-vertex light, because script-spawned vehicles skip the remastered lighting (the separate spawned-vehicle bug); the shade track measured shade/lit near 0.6 once that fix and its fill retune are in. On its own, this fix trades a grey haze for a dark but solid hull.
- **Fog path not shot in game.** The shorter length in fog or rain is unit-tested only; all three views ran in clear weather (fog end above 900 m, so 293 to 300 m).
- **Merge conflicts.** The new LightRig declarations and functions sit at the ends of `LightRig.hpp` / `LightRig.cpp`, and the new test case sits between the two GLSL contract cases of `test_remaster_lighting.cpp`; other bug fixes that append to the same places will need a trivial merge.
- **Lab not run** (no `moderngl` here).
- Untouched by design: vehicle interiors and gunner optics already turn the shafts off for the frame, and the composite's gating for them is unchanged; the pillarbox case uses the same `worldRect` mapping as the AO pass but was not shot.

## Directory listing

(Generated by `ls -la` / `ls` after this file was written; see the end of this file.)

```
$ ls -la D:\Arma_CWA_worktrees\lighting\tmp\bugfix\shaft-veil
total 7452
drwxr-xr-x 1 Dane 197121       0 Oct 17 21:39 .
drwxr-xr-x 1 Dane 197121       0 Oct 17 21:37 ..
-rw-r--r-- 1 Dane 197121   13461 Oct 17 21:39 NOTES.md
drwxr-xr-x 1 Dane 197121       0 Oct 17 21:35 after
-rw-r--r-- 1 Dane 197121    1490 Oct 17 21:35 after-run.log
drwxr-xr-x 1 Dane 197121       0 Oct 17 21:33 before
-rw-r--r-- 1 Dane 197121    1371 Oct 17 21:33 before-run.log
-rw-r--r-- 1 Dane 197121  149033 Oct 17 21:34 build.log
-rw-r--r-- 1 Dane 197121 1760694 Oct 17 21:36 cmp_lowsun_fp.png
-rw-r--r-- 1 Dane 197121 1859272 Oct 17 21:36 cmp_sinai_lowsun_fp.png
-rw-r--r-- 1 Dane 197121  959131 Oct 17 21:36 cmp_sunrise_ridge.png
-rw-r--r-- 1 Dane 197121   15438 Oct 17 21:26 configure.log
-rw-r--r-- 1 Dane 197121  742352 Oct 17 21:36 crop_lowsun_fp.png
-rw-r--r-- 1 Dane 197121  891096 Oct 17 21:36 crop_sinai_lowsun_fp.png
-rw-r--r-- 1 Dane 197121  597464 Oct 17 21:36 crop_sunrise_ridge.png
-rw-r--r-- 1 Dane 197121  218129 Oct 17 21:36 diff_lowsun_fp_x6.png
-rw-r--r-- 1 Dane 197121  282951 Oct 17 21:36 diff_sinai_lowsun_fp_x6.png
-rw-r--r-- 1 Dane 197121   65127 Oct 17 21:36 diff_sunrise_ridge_x6.png
-rw-r--r-- 1 Dane 197121   15902 Oct 17 21:37 fix.patch
-rw-r--r-- 1 Dane 197121    3939 Oct 17 21:35 measure.json
drwxr-xr-x 1 Dane 197121       0 Oct 17 21:39 research-tests
drwxr-xr-x 1 Dane 197121       0 Oct 17 21:39 scripts
-rw-r--r-- 1 Dane 197121    2105 Oct 17 21:34 test_before_tip_source.txt
-rw-r--r-- 1 Dane 197121    1868 Oct 17 21:34 unit_tests.txt

$ ls before after research-tests scripts
after:
contact_sheet.png
lowsun_fp__classic.png
lowsun_fp__pair.png
lowsun_fp__remastered.png
lowsun_fp_noshafts__pair.png
lowsun_fp_noshafts__remastered.png
run.txt
sinai_lowsun_fp__classic.png
sinai_lowsun_fp__pair.png
sinai_lowsun_fp__remastered.png
sinai_lowsun_fp_noshafts__pair.png
sinai_lowsun_fp_noshafts__remastered.png
sunrise_ridge__classic.png
sunrise_ridge__pair.png
sunrise_ridge__remastered.png
sunrise_ridge_noshafts__pair.png
sunrise_ridge_noshafts__remastered.png
timings.csv
timings.json
timings.md
tri
tri.log
udcap.log

before:
contact_sheet.png
lowsun_fp__classic.png
lowsun_fp__pair.png
lowsun_fp__remastered.png
lowsun_fp_noshafts__pair.png
lowsun_fp_noshafts__remastered.png
run.txt
sinai_lowsun_fp__classic.png
sinai_lowsun_fp__pair.png
sinai_lowsun_fp__remastered.png
sinai_lowsun_fp_noshafts__pair.png
sinai_lowsun_fp_noshafts__remastered.png
sunrise_ridge__classic.png
sunrise_ridge__pair.png
sunrise_ridge__remastered.png
sunrise_ridge_noshafts__pair.png
sunrise_ridge_noshafts__remastered.png
timings.csv
timings.json
timings.md
tri
tri.log
udcap.log

research-tests:
abel_shaftveil.test.sqf
abel_shaftveil.test.toml
capture_lib.sqf
sinai_shaftveil.test.sqf
sinai_shaftveil.test.toml

scripts:
cmp.py
measure.py
```
