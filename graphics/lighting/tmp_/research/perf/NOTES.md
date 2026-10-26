# Perf track: where the Remastered frame time goes

Track `perf`, 2026-10-17, two sessions (14:10 to 15:11, resumed 18:25 to 19:10). Question:
sunrise_ridge costs 18.4 ms GPU in Remastered against 7.2 ms in Classic in the
20261017-final capture run, all of it in the world pass. Where does the time go, and what
cheaper equivalent keeps the look?

## Short answer

1. **The 18.4 vs 7.2 ms comparison is a measurement artifact.** Replaying the capture flow
   with the draw count logged (run10) shows the Classic shot is taken while view distance
   4000 is still streaming in: 1,465 draws for Classic, 2,202 for Remastered shot right
   after it (the baseline log has 7,577 vs 9,804). Once streaming is done both modes draw
   9,500 to 10,400 objects and cost the same: GPU world 45.5 ms Classic vs 41.6 ms
   Remastered (4 interleaved samples each, same harness settings as the capture).
2. **The world pass at long view distance is bound by draw submission, in both modes.**
   Lighting off (albedo only) costs the same as full lighting; every remastered shader
   feature switched off one at a time changes nothing measurable at sunrise_ridge (run8).
   The GPU timer's "world" bracket is mostly the GPU waiting for the CPU.
3. **Two engine problems make the draw count 4 to 7 times larger than it needs to be, and
   both have a pixel-exact fix** (prototype in `prototype.patch`):
   - **Instancing is switched off for the whole scene whenever any light slot exists**
     (`SceneDraw.cpp` Pass1, `NLights() == 0`). Abel registers 12 light slots once its street
     lamps exist, so it is off at dawn, noon and night alike. A per-object gate (the light
     list the object's own draw would select is empty) restores it.
   - **No terrain occlusion.** At sunrise_ridge 6,555 of the 7,669 objects that reach the draw
     list sit behind the ridge in front of the camera. A camera-centred terrain horizon table
     (1,024 rays over the heightfield) culls them conservatively.
   - Turning instancing on exposed a **pre-existing instancing bug** (it also hits any
     mission without a light object today): an instanced run draws every member with the
     head's per-object vertex animation, so land-following bushes take the head's terrain
     fit and change silhouette (383 pixels over 8 levels at noon_fields). Keeping
     `Object::IsAnimated` objects out of instanced runs makes it exact.
4. **Measured result of the exact set** (light gate + UBO dedupe + animated objects unbatched
   + terrain horizon occlusion), run12, medians, LOD pinned, 3 other games on the GPU:

   | View | Config | Draws | CPU frame ms | GPU world ms | GPU frame ms |
   |---|---|---|---|---|---|
   | sunrise_ridge VD 4000 | Classic | 11,172 | 28.2 | 28.14 | 28.27 |
   | | Remastered (current) | 11,172 | 31.9 | 34.11 | 35.35 |
   | | **Remastered + exact set** | **1,728** | **12.1** | **1.97** | **2.51** |
   | | Classic + exact set | 1,728 | 12.3 | 0.95 | 1.05 |
   | noon_fields VD 2500 | Classic | 5,571 | 12.6 | 15.95 | 16.18 |
   | | Remastered (current) | 5,566 | 12.8 | 18.20 | 19.13 |
   | | **Remastered + exact set** | **1,500** | **8.3** | **7.11** | **7.48** |

   Walking (camera moved 0.5 m every frame, horizon table rebuilt every moved frame):
   sunrise_ridge CPU frame 40.8 ms stock vs 14.3 ms with the exact set, GPU world 40.4 vs 6.8 ms.
   Pixel identity: sunrise_ridge 0 pixels over 2 levels outside the drifting sky (max 1);
   noon_fields max 7 levels in the horizon band, under the same-config repeat noise (max 15).
   `BEST_sunrise_ridge.png`, `BEST_noon_fields.png`.
5. **What the remastered shading really costs on the GPU** (best frame in window, light gate
   on in both modes so the GPU is not waiting): +0 to +0.9 ms in the world pass at the capture
   settings (sunrise 3.26 vs 3.25, noon 2.29 vs 1.73, lowsun 2.56 vs 1.75, forest 2.15 vs 1.28)
   plus the post pass (0.3 to 1.8 ms, AO the largest piece). On the heaviest foliage view at
   2.0x / 8x MSAA the lit shading is ~1.2 ms over unlit (2.83 vs 1.62), of which the shadow-map
   PCF is the largest piece (~0.6 to 0.9 ms). Shader-side "cheaper equivalents" (early alpha
   cutout, 4 shadow taps on foliage, 4 taps on far cascades) are visually identical and save at
   most ~0.5 ms there, inside the noise of a shared GPU.

**Budget freed**: at VD 4000 views, ~20 to 30 ms of CPU frame and ~30 ms of world-pass GPU
bracket per frame on this (contended) machine; on the quieter baseline machine state (~1.8 us of
CPU per draw), ~9,400 fewer draws at sunrise_ridge is ~17 ms of CPU, ~4,000 fewer at noon_fields
~7 ms, ~1,100 at forest_edge_golden ~2 ms, ~550 at lowsun_fp ~1 ms. The GPU does ~1 to 4 ms of
real world work at the capture settings after the fix (best frames), so a bold effect has 10+ ms of GPU to
spend at 60 fps on every view measured.

## Machine state and method (read this before trusting any absolute ms)

- Every run went through `run-captures.ps1` on hidden desktops with my own data junctions
  (`RemasterUD-perf`, `-perf4` .. `-perf12`) and research-only tests generated by
  `scripts/perfgen.py` + `scripts/gen_run*_test.py` into `tests/rendering/research/perf/` of the
  scratch worktree `D:\Arma_CWA_worktrees\gfx-perf` (git-excluded). Engine: the scratch build
  `dist/gfx-perf-rwdi` (shared build untouched).
- The machine was shared with ~20 other research agents. Session 1: 20+ compilers and up to 4
  games. Session 2: run8 started with no other game and CPU ~2% (`machine_load.log`,
  `game_count.log`), but 3 to 4 other agents' games were on the same GPU from about 18:30 on.
  The GPU is time-sliced between those processes, so the GPU timestamp brackets include other
  processes' work. Medians are therefore noisy; I report them with the best frame in each
  window (scratch build: per-window min), and I rely on deterministic counters (draws,
  culled objects, UBO flushes, instanced runs) and on pixel diffs for the claims that matter.
- Configs are interleaved in rounds (A0 classic, A0 m0, ..., A1 classic, ...), 150 frames
  each; tables give medians over rounds. `CWR_PERF_PIN_LOD` holds the object LOD at its finest
  (`Scene::AdjustComplexity` otherwise picks detail from the frame time, which made draw
  counts swing 1,465 to 11,172 in one run). `POSEIDON_OBSERVER_SAMPLE=1` turns off the
  harness's per-frame centre-pixel readback, which serialises CPU and GPU (run7; the capture
  run itself keeps it on).
- Perf flags ride on the debug knob, `triSetLighting "debug=<100 + flags>"`: 1 lit UBO
  dirty-range flush, 2 VS UBO skip-unchanged flush, 4 per-object light gate, 8 / 16 instancing
  distance band widened / removed, 32 observer every frame, **64** animated objects out of
  instanced runs, **128** (with 64) animated objects batch again beyond 400 m, **256** terrain
  horizon occlusion. Shader experiments ride on the saturation knob (`saturation=<10 + mask>`,
  mask 0 = stock look) through the overrides in `override_mask/`, `override_cutout/`,
  `override_lod/` (bits listed in `scripts/gen_run11_test.py`). Every run's
  `game_stdout.log` shows the override files loaded and the remastered programs active (lit
  UBO flushes > 0 in Remastered configs, no compile or link errors).
- Pixel identity: `scripts/banddiff.py` per row band (sky 0-250, horizon 250-350, ground
  350-640): max channel difference / pixels over 2 / pixels over 8 of 614,400. Clouds drift
  between shots, so the sky band is never identical; the soldier props idle-animate (face and
  head), so the band holding them has a repeat noise floor. Same-config repeat shots give
  that floor in every run.

## Runs

| Run | When | Build / shaders | What | Outcome |
|---|---|---|---|---|
| run1_diag | s1 | shared, mask | one pass of every mask bit; scale / MSAA / VD sweeps | draw count swung with frame time; mask bits verified visually (`run1_diag/mask_check.png`) |
| run2_rounds | s1 | shared, mask | interleaved feature masks, VD 4000 and 1000 | VD 4000: every config 40 to 43 ms whatever the shading (submission bound) |
| run3_scratch | s1 | scratch | rounds | tri hung after the settle; stopped with my own PIDs |
| run4_amp | s1 | scratch, amp | phase split, UBO counters, amplification | one full lit evaluation 0.13 to 0.22 ms at 2.46 MP 8x MSAA |
| run5_inst | s1 | scratch, mask | light gate, UBO dedupe, distance band | 11,172 to 2,797 draws |
| run6_identity | s1 | scratch | repeat shots, town_night | gate changes ~25 far pixels at sunrise_ridge |
| run7_observer | s1 | scratch | harness readback | adds 3 to 16 ms of CPU frame under the harness |
| run8_quiet | s2 | scratch, mask | the cost table on 4 views, feature masks at sunrise | below |
| run9_cutout | s2 | scratch (+ flag 64), cutout | early cutout, forest masks, flag 64 identity | below |
| run10_baseline_replay | s2 | scratch, **stock shaders, adaptive LOD, observer every frame** | the capture flow with draws logged | explains the 18.4 vs 7.2 ms |
| run11_foliage | s2 | scratch (+ 128, + 256), lod | forest shading pieces at 2x/8x, far-only batching, terrain occlusion | below |
| run12_exact | s2 | scratch, mask | the exact set, walking camera | headline table above |

### run10: the capture flow replayed (why 18.4 vs 7.2)

Stock shaders, no LOD pin, observer every frame: exactly what `run-captures.ps1` measures.

| Shot | Draws | GPU world ms | CPU frame ms |
|---|---|---|---|
| Classic after 150 + 90 frames (the capture order) | 1,465 | 1.91 | 6.5 |
| Remastered, right after | 2,202 | 12.32 | 15.2 |
| Classic, 4 samples after +600 frames (interleaved with Remastered, both orders) | 9,487 to 10,388 | 45.5 (median) | 52.0 |
| Remastered, same | 9,487 to 9,987 | 41.6 (median) | 49.8 |
| Classic + gate + UBO, 2 samples | 2,644 / 2,793 | 11.6 | 19.5 |
| Remastered + gate + UBO, 2 samples | 2,644 / 2,797 | 12.4 | 21.0 |

The objects that stream in late are mostly behind the ridge, so the pictures barely differ
(`run10_baseline_replay/capture_flow_replay.png`). The capture's GPU column compared a
half-loaded Classic scene with a more loaded Remastered one.

### run8: cost table, LOD pinned (medians of 3 rounds; best = best frame in window)

| View | Config | Draws | CPU frame | GPU world (best) | GPU post |
|---|---|---|---|---|---|
| sunrise_ridge VD 4000 | Classic | 11,172 | 43.2 | 37.70 (20.17) | |
| | Remastered | 11,172 | 42.1 | 35.37 (17.42) | 0.94 |
| | Classic + gate + UBO | 2,797 | 16.5 | 7.21 (3.25) | |
| | Remastered + gate + UBO | 2,797 | 17.8 | 7.81 (3.26) | 1.04 |
| noon_fields VD 2500 | Classic | 5,571 | 21.9 | 22.29 (6.78) | |
| | Remastered | 5,566 | 30.8 | 31.12 (7.92) | 0.59 |
| | Classic + gate + UBO | 1,345 | 14.0 | 5.62 (1.73) | |
| | Remastered + gate + UBO | 1,340 | 12.6 | 9.82 (2.29) | 1.86 |
| lowsun_fp VD 1500 | Classic | 1,576 | 9.9 | 7.18 (2.90) | |
| | Remastered | 1,569 | 11.3 | 12.06 (3.96) | 1.41 |
| | Classic + gate + UBO | 1,005 | 9.2 | 5.47 (1.75) | |
| | Remastered + gate + UBO | 998 | 9.1 | 10.11 (2.56) | 1.76 |
| forest_edge_golden VD 1500 | Classic | 1,849 | 6.9 | 10.44 (2.94) | |
| | Remastered | 1,846 | 8.7 | 12.68 (3.30) | 0.36 |
| | Classic + gate + UBO | 973 | 5.5 | 4.74 (1.28) | |
| | Remastered + gate + UBO | 970 | 8.4 | 8.57 (2.15) | 0.27 |

Charts: `chart_run8_{Q,F,L,G}_base.png`. Shots and three-way images: `run8_quiet/threeway_*.png`.

Shader feature masks at sunrise_ridge, all on top of Remastered + gate + UBO (2,797 draws):
GPU world median 7.70 to 9.49 ms and best frame 3.6 to 4.7 ms for shadow map off, 1 shadow tap,
primary cascade only, horizon map off, cloud shadow off, fog toward the sun off, crown ellipsoid
off, all features off and unlit, against 7.81 / 3.26 for the stock shading. None is cheaper
than stock; switching the post pass off saves 1.0 to 1.5 ms of GPU frame
(`chart_run8_Q_features.png`).

### run9 and run11: instancing correctness (flags 64 and 128)

| View | Gate + UBO | + 64 (exact) | + 64 + 128 (far only) |
|---|---|---|---|
| sunrise_ridge draws | 2,796 | 6,461 | 2,808 |
| noon_fields draws | 1,340 | 2,166 | 1,365 |
| forest_edge_golden draws | 971 | 1,210 | 986 |
| lowsun_fp draws | 998 | 1,045 | 1,025 |
| noon horizon band vs stock (px > 2 / > 8, max) | 742 / 383, 107 | 0 / 0, 2 | 254 / 71, 43 |
| forest horizon band vs stock | 45 / 23, 73 | 0 / 0, 2 | 45 / 23, 73 (the differing bushes are beyond 400 m) |
| sunrise horizon band vs stock | 33 / 4, 19 | 0 / 0, 2 | 20 / 4, 21 |

`run8_quiet/noon_treeline_zoom_m0_top_m0all_bottom.png` shows the cause: the bushes at
150 to 300 m change outline under the gate alone. `Object::Animate` rewrites land-following
(`ClipLandKeep` / `ClipLandOn`) vertices per object to fit the terrain; an instanced run draws
the head's rewritten buffer for every member. Flag 64 (`!IsAnimated(drawLOD)` for head and
members) is exact but gives back much of the draw saving at sunrise_ridge, where the far
hillsides are full of such bushes. Flag 128 lets them batch again beyond 400 m: near-exact
(`run11_foliage/noon_far_animated_zoom_stock_top_192_bottom.png`, differences not visible at
4x zoom), not bit-exact. With terrain occlusion in front, flag 64 alone costs little (run12:
1,728 vs 1,549 draws at sunrise_ridge), so the recommended set uses 64 without 128.

### run11 and run12: terrain horizon occlusion (flag 256)

From the camera, 1,024 rays march the heightfield (`GLandscape->SurfaceY`) in 25 m steps out
to the fog range and keep the running maximum elevation tangent per ray and step, with the
terrain lowered by 2 m so a coarser rendered terrain LOD cannot reveal what the table hides.
An object is culled when the top of its bounding sphere (radius x1.25 + 1 m) lies under the
table along every ray its angular width spans (plus one ray each side), using only terrain
samples in front of its near edge. Objects closer than 75 m are never tested. The table is
rebuilt when the camera moves more than 0.25 m.

| View | Tested | Culled | Draws stock / tocc only / exact set | Table build |
|---|---|---|---|---|
| sunrise_ridge VD 4000 | 7,669 | 6,555 | 11,173 / 2,724 / 1,728 | 5.4 to 5.7 ms |
| noon_fields VD 2500 | 3,915 | 1,682 | 5,566 / 3,297 / 1,500 | 4.6 ms |
| forest_edge_golden VD 1500 | 1,276 | 812 | 1,846 / 931 / 740 (with 128) | 2.6 ms |
| lowsun_fp VD 1500 | 831 | 5 | 1,569 / 1,565 / 1,024 (with 128) | 2.3 ms |
| sunrise_ridge after a 60 m walk | 7,653 | 5,162 | | rebuilt 120 times in 120 moved frames |

Identity of the cull alone, horizon band: sunrise_ridge 17 px over 2 (max 4) Remastered, 0
Classic; noon_fields 0 and 0; lowsun_fp 3 (max 16, soldier noise floor 363 / 213). Charts:
`chart_run11_Q_prototypes.png`, `chart_run11_F_prototypes.png`.

Classic alone with terrain occlusion at sunrise_ridge: 11,173 to 2,724 draws, GPU world 40.5 to
2.9 ms, CPU frame 42.9 to 12.3 ms (run11); with the exact set 0.95 ms GPU world (run12).

### run9 and run11: the foliage-heavy view and shader-side cheaper equivalents

forest_edge_golden, VD 1500, all on top of Remastered + gate + UBO. Best frame in window, ms
(medians in `chart_run11_H_forest_shading.png`, `timings.md`):

| Config | 2.0x / 8x MSAA (run11) | 1.5x / 4x (run9 G) |
|---|---|---|
| Classic + gate | 1.56 | 1.28 |
| Remastered + gate | 2.83 | 2.17 |
| unlit (albedo, same pipeline) | 1.62 | 1.35 |
| all features off | 1.80 | 1.56 |
| shadow map off | 1.93 | 1.69 |
| 1 shadow tap | 2.19 | |
| early alpha cutout (`override_cutout`) | 2.65 | 2.09 |
| cutout + 4 taps on foliage | 2.43 | |
| cutout + 4 taps on cascades past the first | 2.36 | |
| cutout + both | 2.33 | |

The remastered lighting costs ~1.2 ms per frame on this view at 2.0x / 8x and ~0.9 ms at the
capture settings; the shadow-map PCF is the largest piece. The early cutout (the alpha test
moved before the lighting, with coverage computed once in uniform control flow) is pixel-exact
(forest: max 2 levels outside the sky) and the tap reductions are visually identical
(foliage 4 taps: 0 px over 2 outside the sky; both: ground 3 px, max 28, against a ground noise
floor of max 16), but none saves more than ~0.5 ms and run9 and run11 disagree on the cutout
(run9 2x/8x best 6.42 vs 4.66 for stock, run11 2.65 vs 2.83). Not worth production effort
before the draw-count fixes; the medians in the late configs of each round (crown, horizon,
cloud, fog off) drift upward with the other games' load and are not usable.

### Session 1 results that still stand

- Light gate + UBO dedupe (run5): sunrise_ridge 11,172 to 2,797 draws, lit UBO 1,084 KB to
  10 KB a frame, VS full-block flushes 2,986 to 322. The UBO dedupe alone does not move the
  frame time measurably but is exact by construction. The instancing distance band (flags 8 /
  16) saves more draws but changes ~150 far pixels; not recommended.
- Amplification (run4): one full lit evaluation is 0.13 ms (best frame) to 0.22 ms (median) at
  2.46 MP, 8x MSAA, VD 1000; the 12-tap shadow lookup ~0.02 ms, the horizon / sky / ground tint
  fetches not measurable.
- Harness readback (run7): under `--autotest` the frame observer reads one pixel every frame
  (MSAA resolve + `glReadPixels`), so CPU and GPU never overlap in capture runs; it adds 3 to
  16 ms of CPU frame and lands Remastered's GPU additions on the CPU frame in full. The capture
  tables' CPU fps columns overstate the Remastered cost.

## What a production version needs

1. **Per-object light gate** (`SceneDraw.cpp` Pass1): batch an object when the light list its
   own draw would select (`SelectLights`) is empty, for the head and every member, instead of
   `NLights() == 0` for the whole scene. Daytime selection returns empty at once.
2. **Instancing correctness**: keep `Object::IsAnimated(drawLOD)` objects (land-following
   vertices, damage, skewed forests) out of instanced runs. This is a fix for today's engine
   too: stock instancing already batches them whenever a mission has no light object. A
   GPU-side terrain fit (land-keep offset in the vertex shader from a height texture) would let
   them batch exactly; the 400 m far-only rule is a near-exact shortcut.
3. **Terrain horizon occlusion**: the prototype's table build is too slow for a moving camera
   at VD 4000 (5.5 ms per rebuild). Options: steps that grow with distance (25 m near, 100 m
   far), 512 rays with a wider angular margin, SIMD over rays, a worker thread with the table
   built for an eye raised by the expected motion until the next rebuild (stays conservative),
   or reuse of `TerrainHorizon.cpp`'s per-cell map. Move the cull before LOD selection and
   sorting so culled objects cost no CPU at all (the CPU frame floor of ~12 ms at sunrise_ridge
   is the per-object work done before the cull). Check two edge cases the prototype ignores:
   long sun shadows from culled objects onto visible ground (the shadow-map pass selects its
   casters separately; not verified) and notches narrower than one ray (0.35 degrees).
4. **UBO uploads**: mirror what each UBO holds and upload only the dirty slot range (lit block)
   or skip unchanged full flushes (VS block); keep the mirror in step in every partial upload
   (`SetLitOpaque`, `UploadVSCrown`, the per-draw world-matrix path) and reset it when the
   buffer is recreated.
5. **Classic**: the exact set changes no pixel by more than 2 levels outside the drifting sky in
   Classic on the views tested (sunrise_ridge max 1, noon_fields horizon band max 0, ground max 2) and cuts its sunrise_ridge GPU
   world from 28 to 1 ms. Whether Classic takes it is the owner's call; it is a pure cost change.
6. **Capture methodology**: settle long enough for VD 4000 streaming before the first shot (or
   log draws and refuse a pair whose counts differ by more than a few percent), pin or log the
   adaptive LOD, run with `POSEIDON_OBSERVER_SAMPLE=1`, and report the best frame next to the
   EMA when other games share the GPU.
7. Shader-side LOD (early cutout, fewer taps on foliage and far cascades) is safe and visually
   exact but worth at most ~0.5 ms on the heaviest view; the post pass (AO: 12 taps + 4 normal
   taps per half-res pixel from full-res depth) is the largest fixed Remastered GPU item and a
   half-res linear-depth prepass would trim it (untried).

## Risks against the owner's taste list

None for the look: every prototype here is a cost change, checked pixel for pixel against a
same-config repeat. The exact set is within the repeat noise on sunrise_ridge and noon_fields.
The near-exact variants (flag 128, the instancing distance band) change a few dozen to a few
hundred far pixels and are not recommended for Classic, which must stay bit-identical.

## Honesty notes

- No run had the machine to itself for its whole length; run8's first configs (sunrise_ridge)
  ran with no other game, the rest with 3 to 4 other agents' games on the GPU. Absolute ms are
  inflated and noisy; draws, culled counts, UBO counters and pixel diffs are deterministic.
- I did not rerun the 13-view capture set with the prototype; the views measured are
  sunrise_ridge, noon_fields, lowsun_fp, forest_edge_golden (and town_night in session 1).
- run3 and the first run7 attempt hung in tri after the settle in session 1; I stopped them
  with their own tri PIDs and `stop-owned-games.ps1 -DataDir <my junction>`. In session 2 I
  stopped three of my own tri processes while they were still waiting for a game slot (no game
  had started) to rebuild the scratch exe; no other agent's process was touched.
- The terrain cull's edge cases in "production" item 3 were not exercised.
