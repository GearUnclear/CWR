# Shade track: low-sun shade and warmth calibration (TRACKID shade)

Two sessions. Session 1 (runs `run1` to `run5`, `diag`, `diag2`, 14:10 to 15:17) found the root causes and built the shader prototype V9. Session 2 (runs `run6` to `run14`, 18:25 to 19:15) verified the rig changes as C++ (`LightRig.cpp`), extended the calibration to evening heights (4.8 and 7.5 degrees) and to tree crowns, ran a knob-only candidate grid, found a second real-data bug (cloud shadows under a clear sky) and shot the final set. Every number below was measured on in-game captures in this folder. Builds: the shared `dist/lighting-rwdi` (shipped, untouched) and the scratch worktree `D:\Arma_CWA_worktrees\gfx-shade` (detached at 3c53141, `prototype.patch`, build `dist/gfx-shade-rwdi`, rebuilt 18:33 and 19:04).

## Verdict

Promising, and now precise. Current Remastered looks no better than Classic on real data mostly because of three bugs, not because of the light model: (1) script-spawned vehicles bypass the remastered lighting entirely (CPU T&L, Classic per-vertex light), (2) the sun-shaft composite veils near objects grey, (3) "clear" weather still casts 0.2-transmission cloud shadows over about 4% of the map (a whole view when the camera sits under one). On top of those, the rig needs three calibration terms at a low sun (evening hue from 25 degrees, extra sideways fill on vertical faces, a crown canopy term). With all of it, hulls read as solid painted objects with shade/lit 0.58 to 0.62 from 4.8 to 12.7 degrees (Classic 0.25 to 0.49, shipped 0.31 to 0.49), hull lit faces and sunlit ground are at least as warm as Classic (one exception at 4.8 degrees, 3 to 6% less red on the hull), crowns at golden hour drop from pale (F 0.78) to 0.67 (Classic 0.57) and read greener, and nothing at noon gets darker except spawned vehicle sides, which the shipped exe drew with Classic light by accident. It is a calibrated foundation, not "RTX on" by itself.

Best images (each Classic | Remastered shipped exe | prototype final, same camera, date, props, GPU frame ms per panel): `BEST_lowsun_fp.png`, `BEST_sinai_lowsun_fp.png`, `BEST_dawn_box.png`, `BEST_dawn_box5.png`, `BEST_lowsun_box.png`, `BEST_sinai_box.png`, `BEST_forest_edge_golden.png`, `BEST_forest_edge_noon.png`, `BEST_coastline_sunset.png`, `BEST_noon_fields.png`; crops `BEST_hulls_lowsun_box_crop.png`, `BEST_crowns_golden_crop.png`. Bug evidence: `cmp_rootcause_prelit.png`, `cmp_clear_sky_cloud_shadow.png`, `cmp_cloud_floor_fix.png`.

## Method

- **Albedo-normalised measurement.** Every override in this folder makes remastered debug view 3 return the surface albedo (display space). Each view is shot with `debug=3, post=0` beside the real shots, so each box gets `F` = luma(shot)/luma(albedo), the on-screen fraction of the texture brightness after the 1.2 display gamma (the README's metric), and `L` = (shot/albedo)^1.2 per channel, the colour of the light on that surface, reported as g:r and r:b (Classic's low-sun warmth is a green deficit, so CCT alone misses it). `scripts/measure.py`, boxes in `scripts/boxes.json`, `scripts/summ.py` for the tables, `scripts/crownstats.py` for crowns (foliage pixels only, plus p10:p90 luma spread as a volume measure).
- **Calibration views.** The committed views plus side-lit boxes where the sun is 90 degrees left, so the same paint shows a lit face (45 degrees from the sun) and a shaded face (135 degrees): `lowsun_box` (Abel 12.7), `sinai_box` (Sinai + @LoBo 12.2), and new in session 2: `dawn_box` (Abel 7.5), `dawn_box5` (Abel 4.8), `sinai_dusk_box` (Sinai 7.6), `forest_edge_noon` (the golden-hour forest camera at 72 degrees). The rig and Classic both depend on sun height only, so a 7.5 degree morning sun stands in for the evening (the session-1 evening spot sat in a terrain shadow).
- **Variants inside one run** through a selector in the `localLights` knob (`litGround.w`, free in daylight), the in-scatter composite through `ao=0.71`, and the C++ rig through `moonlight=1.01` (no daylight effect). Generators `scripts/gen_tests.py`, `scripts/gen_tests6.py`; multi-test runs split with `scripts/split_collect.py`; the final set merged with `scripts/merge_final.py`.
- **Rig model.** `scripts/rigmodel.py` ports `ComputeLightRig` + `shadeRemastered` + the Classic per-vertex path (within 0.01 to 0.02 of in-game numbers, session 1); `model_table.txt`; `scripts/unitcheck.py` checks the unit-test thresholds against the final rig.

## Root causes (with evidence)

1. **Script-spawned shapes bypass the remastered lighting (engine bug, the biggest one).** `Shapes.OptimizeAll()` creates vertex buffers only for shapes loaded at mission start (`WorldImpl.cpp:2382`, `WorldInit.cpp:796`, `Landscape.cpp:2837`). A shape first loaded later (any `createVehicle` of a class the mission did not place) has no `_buffer`, so `Shape::Draw` (`ShapeDraw.cpp:89`) takes the CPU T&L path: Classic light baked into pre-lit vertices, passed through by the remastered shaders. No per-pixel light, no sky fill, no shadow maps received. `cmp_rootcause_prelit.png` (debug paints pre-lit draws magenta: Jeep and M113 are magenta). In the shipped exe `sinai_box` hull values equal Classic exactly (Zelda lit 0.92 / 0.88 / 1.32 in both). Every hull judgement on earlier captures, `20261017-final` included, judged Classic CPU lighting; in Guerrilla Mode every runtime-spawned vehicle is affected. `PortraitRenderer.cpp:100` already calls `OptimizeRendering()` on private shape copies it makes at run time, so building buffers after mission start has precedent.
2. **The sun-shaft composite veils near objects.** `psPostComposite` screen-blends the shaft term at full weight whatever the depth: in `lowsun_fp` the Jeep side goes from 27 to 68 (of 255) with shafts on (session-1 `diag`). Final run: Jeep side F Classic 0.40, shipped 0.92 (veiled grey), prototype 0.63; Sinai HMMWV 0.38 / 1.18 / 0.61.
3. **Clear weather casts deep cloud shadows (new, session 2).** `setOvercast 0` is not the clearest keyframe: `Weather::SetSky` interpolates from the -0.08 "jasno" keyframe (`Landscape.cpp:425`), so skyThrough is 0.959 and `ComputeLightRig` sees cover 0.041 > 0.02 and turns cloud shadows on, at transmission 0.2, with no cloud visible in the sky. About 4% of the map sits in 80% sun loss. `run12` (debug 1 vs `cloudShadows=0`): `forest_edge_noon` 74% of the frame shadowed, frame luma -14.5%; `coastline_sunset` 12.6% (in `run13s` the shipped exe lost the whole sun path on the sea to it, `BEST_coastline_sunset.png`). This is also why the forest at noon measured slope F 0.67 vs Classic 0.87 until it was found (`diag_noon_sun_visibility.png`: uniform 0.2 = `kCloudTransmit`; `terrainShadows=0`, `terrainOcclusion=0`, `ao=0`, `post=0` changed nothing).
4. **Evening hue starts too late** (`EveningSunChroma`: `w = (1 - Smoothstep(5, 16, e)) * 0.65`): at 12.7 degrees only 14% of Classic's sunset hue reaches the sun, while Classic ramps from 25 degrees (`Lights.cpp`). Sunlit ground at 12.7: Classic g:r 0.84 to 0.87, shipped 0.92 (greener, greyer).
5. **Vertical faces lose fill exactly at a low sun** (`skyHorizon = zenith * 0.55 * sky * (1 - 0.25 low) + 0.5 ground`): the anti-sun face of a hull sees 0.55 of a sky already at 44% of noon, minus 20%. Hull shade/lit with per-pixel light and the shipped rig: 0.46 at 12.7 degrees, 0.43 at 7.5, 0.41 at 4.8.
6. **Crowns overshoot at a low sun.** Half-lit (foliage) materials are `CreateMaterialConstant(col, 0.5)`: ambient 1, forced 0.25, diffuse 0.25. Classic applies the diffuse 0.25 with each leaf card's own normal (on average about half of it lands); the remastered ellipsoid normal with a wrapped terminator gives nearly all of it on the lit side, and the exposure meter adds 1.3 to 1.5x at 12 degrees. Crown F 0.78 vs Classic 0.57 at golden hour, light g:r 0.86 vs 0.78: the pale grey-green.
7. **Not wrong:** the exposure meter (sunlit ground at 12.7 degrees F 0.67 vs Classic 0.48 to 0.59) and ground cast shadows (0.70 to 0.79 of lit in every variant).

## Prototype (final)

| Piece | Where | What it does |
|---|---|---|
| Lazy vertex buffer | `prototype.patch`: `Shape.hpp`, `ShapeDraw.cpp` | On first draw a shape with no buffer gets one (`VBDynamic`), with `OptimizeRendering`'s exclusions plus any special per-vertex lighting (sun, moon, halo, sky, stars). Used only while the remastered lighting is active, so Classic keeps its CPU path. `CWR_NO_LAZY_VB=1` turns it off. |
| Evening hue | `prototype.patch`: `LightRig.cpp` `ComputeLightRig` | After the shipped blend: `share = 0.4 (1 - S(6, 25, e)) + 0.3 (1 - S(4, 9, e))`, `chroma = Normalise(Mix(chroma, Normalise(ToLinear(classicSun)), share * warmth))`. |
| Vertical-face fill | same | `fill = (1 - S(8, 40, e)) S(-2, 3, e)`; sky part of `skyHorizon` x(1 + 2.6 fill), sky part of `skyHorizonSun` x(1 + 0.5 fill), `skyGround` x(1 + 1.3 fill). Up-facing ground (zenith only) and noon unchanged. |
| Clear-sky cloud floor | same | `shadowCover = (cover - 0.05) / 0.95`; cloud shadows only above that; the rest of Classic's dimming stays uniform (`overcastDim = classicDim / (1 - shadowCover (1 - T))`), so the mean still equals Classic's. |
| Crown term (V18) | `ovr_final/psRemasterCommon.glsl` | Leaf albedo saturated at equal luminance (`pow(a, 1.3)` renormalised); sun optical depth through the crown (sphere approximation from `vCrown`: path to the crown surface toward the sun, `T = mix(1, exp(-t), 0.75)`); sky toward the core x0.45 (was 0.6); at a low sun only (`1 - S(12, 35, e)`), crown sun x0.65 and crown sky x0.75. |
| In-scatter shafts | `ovr_final/psPostComposite.glsl` | Shaft term weighted by `1 - exp(-viewZ / 300 m)` (view depth from the AO target's `.g`): a hull at 10 m keeps its contrast, distant terrain and sky keep the shafts. |

The research gate: the C++ terms run only with `triSetLighting "moonlight=1.01"`; `ovr_final` has the rig terms switched off in GLSL (`kRigInCpp`) and V18 fixed on. **C++ port verified:** `run9` shot the session-1 V4 rig in C++ and in GLSL side by side: every box in 5 views equal to two decimals (`run10/run9/measure_*.json`). The final C++ (V20 hue + V21 fill together) is consistent with the separately shot GLSL V20 and V21 (`run10/run10` vs `final`): hull shade/lit within 0.01, lit-face r:b within 0.06.

## Results (final run: `run13` scratch build + `run13s` shipped exe, measured against `run13`'s albedo; `final/measure_final.md`, `final/crowns_final.md`)

Hull shade/lit, same paint (F of the 135-degree face / F of the 45-degree face):

| view, sun | Classic | Remastered shipped exe | prototype final |
|---|---|---|---|
| lowsun_box 12.7, M113 | 0.40 | 0.46 | **0.62** |
| lowsun_box 12.7, Jeep | 0.63 | 0.66 | 0.76 |
| sinai_box 12.2, Zelda | 0.49 | 0.49 | **0.61** |
| dawn_box 7.5, M113 | 0.32 | 0.38 | **0.60** |
| sinai_dusk_box 7.6, Zelda | 0.43 | 0.43 | **0.58** |
| dawn_box5 4.8, M113 | 0.25 | 0.31 | **0.58** |

Hull lit-face light (g:r / r:b; lower g:r and higher r:b are warmer):

| view | Classic | shipped | final |
|---|---|---|---|
| lowsun_box M113 | 0.88 / 1.42 | 0.89 / 1.30 | 0.82 / 1.48 |
| sinai_box Zelda | 0.88 / 1.32 | 0.88 / 1.32 | 0.78 / 1.59 |
| dawn_box M113 | 0.63 / 2.07 | 0.67 / 1.78 | 0.62 / 2.16 |
| sinai_dusk_box Zelda | 0.68 / 1.76 | 0.68 / 1.76 | 0.62 / 2.21 |
| dawn_box5 M113 | 0.50 / 2.66 | 0.55 / 2.20 | 0.53 / 2.57 |

Sunlit ground (F, g:r / r:b) and cast shadow / lit ground:

| view | Classic | shipped | final |
|---|---|---|---|
| lowsun_fp 12.7 | 0.59, 0.86 / 1.10 | 0.68, 0.92 / 1.14 | 0.67, 0.86 / 1.25 |
| lowsun_box 12.7 | 0.48, 0.84 / 1.13; shadow 0.74 | 0.67, 0.92 / 1.14; 0.70 | 0.66, 0.86 / 1.24; 0.71 |
| sinai_lowsun_fp 12.2 | 0.48, 0.93 / 0.93 | 0.58, 0.93 / 1.09 | 0.58, 0.89 / 1.16 |
| dawn_box 7.5 | 0.28, 0.77 / 1.18; 0.75 | 0.52, 0.77 / 1.38; 0.79 | 0.51, 0.73 / 1.46; 0.79 |
| dawn_box5 4.8 | 0.18, 0.73 / 1.22 | 0.44, 0.73 / 1.39 | 0.43, 0.69 / 1.46 |

Crowns (`crownstats.py`, foliage pixels; F / g:r / r:b / p10:p90):

| view | Classic | shipped | final |
|---|---|---|---|
| forest_edge_golden 12, big crown | 0.57 / 0.78 / 1.45 / 0.38 | 0.78 / 0.86 / 1.39 / 0.44 | 0.67 / 0.84 / 1.60 / 0.42 |
| forest_edge_golden 12, conifer | 0.48 / 0.83 / 1.33 / 0.19 | 0.71 / 0.85 / 1.43 / 0.25 | 0.60 / 0.81 / 1.65 / 0.22 |
| forest_edge_noon 72, big crown | 0.86 / 1.02 / 1.05 | 0.74 (under the clear-sky cloud) | 0.79 / 1.06 / 1.09 |

Noon: `noon_fields` ground F 1.04 to 1.05 in all three; `forest_edge_noon` slope F Classic 0.87, shipped 0.67 (cloud bug), final 0.87; spawned Jeep side F 0.97 in Classic and in the shipped exe (Classic CPU light, bug 1), 0.60 in the final (per-pixel at a 72-degree sun). `sinai_desert_noon` (38.8 degrees) is visually unchanged.

How it looks (read off the images): `BEST_lowsun_fp.png`, `BEST_sinai_lowsun_fp.png`: shipped hulls are grey shapes in a haze, final ones are olive/tan, solid, detailed, on Classic's pink-tan ground. `BEST_dawn_box.png`, `BEST_dawn_box5.png`: shaded hull rears readable, lit sides warm red-orange, ground warm instead of Classic's near-black. `BEST_forest_edge_golden.png`, `BEST_crowns_golden_crop.png`: the big crown is greener and less silvery than shipped, golden slope; still lighter than Classic's. `BEST_forest_edge_noon.png`: shipped sits under an invisible cloud (dim, grey); final is sunny like Classic, with shading. `BEST_noon_fields.png`: the final Jeep side is darker than shipped (see risks). `final/cmp_sunrise_ridge.png`: final removes the shaft streak over the near ground, so that ground is darker than shipped (still brighter than Classic).

## Candidate grid and variants (session 2)

- **Knob-only grid** (`grid_knobs_lowsun_box.png`, `grid_knobs_dawn_box.png`, `run10/run8`, lazy-VB build so hulls are lit per pixel; captions carry the measured numbers): hull shade/lit at 12.7 / 7.5 degrees: shipped rig 0.46 / 0.43; k1 `sky=1.4` 0.49 / 0.47 (ground shadows flatten 0.70 to 0.75); k2 `+warmth=1.5` 0.50 / 0.47; k3 `sky=1.8,sun=1.2,warmth=1.5` 0.50 / 0.47 (everything brighter, shadows 0.76); **k4 `bounce=2.5,warmth=1.5` 0.54 / 0.50 (the best knob-only set, ground shadows unchanged)**; k5 `k2 + shafts=0.4` 0.50 (veil halved: lowsun_fp Jeep 0.93 to 0.75); k6 `exposure=1.15,sky=1.3,warmth=1.5,saturation=1.2` 0.49; shader V9 0.59 / 0.56. No knob reaches 0.6 without flattening the ground shadows, and `warmth` extrapolates the physical keyframes (greener) instead of Classic's hue. If only knobs can ship, k4 plus the in-scatter composite is the fallback.
- **Crown variants** (`crowns_v12_v16_golden_crops.png`, `crowns_v17_v18_golden_crops.png`; big crown F at 12 degrees): V9 0.74; V12 leaf-card normals 0.74 (no change: the forced term dominates); V13 crown optical depth 0.72 (conifers 0.69 to 0.63); V14 0.72; V15 self-shadow floor 0.25 0.72 (no change: the visible leaves are sunlit); V16 sigma 1.6 0.71; V17 low-sun canopy x0.75 0.70; **V18 x0.65 0.67** (chosen). The p10:p90 spread stays 0.42 to 0.44 in every variant (Classic 0.38): these terms change level and hue, not the crown's internal contrast. That needs per-leaf shading, which is the foliage track's subject.
- **Evening hull variants** (`variants_hulls_dawn_box5.png`): V20 (hue share +0.3 below 9 degrees) brings the 4.8-degree M113 lit face to 0.52 / 2.63 (Classic 0.50 / 2.66); V21 (anti-sun fill x2.6, bounce x1.3) lifts shade/lit by 0.02 to 0.03 to the 0.6 target. Both are in the final C++.
- Session-1 variants (V2 to V11, `variants_*.png`, `s1_BEST_*.png`, `ovr_v4`, `ovr_recommended`) are superseded by the final set; V9 = V4 rig + canopy + the V5 neutral ground tint on crowns.

## GPU cost

Not resolvable on this machine today: every run shared the GPU with three other games. `timings_run7_interleaved.md` (runs `run7a` shipped exe and `run7b` lazy-VB build, four views, Classic / shipped rig / V9 / V14 interleaved three times, 120 frames each): the same mode's GPU frame time swings up to 4x between rounds (sunrise_ridge Classic 13.6 / 31.3 / 52.5 ms) and V9 minus the shipped rig is -4.5, -1.3, 0.0, +13.8 ms by view, no consistent sign. Per-shot numbers of the final set are in `final/timings_final.md` (same noise). Analytically: the rig terms are uniforms computed once per frame in C++ (zero per-pixel cost); the crown term adds a sqrt, an exp, three pows and about 20 ALU per foliage fragment; the in-scatter adds one texture fetch and an exp per pixel in the composite (about 1.4 MP at 960x640 x1.5 scale); the cloud floor is free. The lazy buffer moves spawned vehicles from CPU T&L to the GPU path every other object already uses: less CPU, and those draws now pay per-pixel lighting and shadow-map sampling like any placed vehicle. A clean pass on an idle machine is still owed.

## Recommended defaults (production)

1. Land the lazy vertex buffer, or call `LODShape::OptimizeRendering()` for shapes loaded after mission start (the patch's exclusions are required: without them the 8-vertex sun/halo shape got a buffer and the sun disc vanished, session-1 `run2`). Keep Classic on its CPU path.
2. `LightRig.cpp`: the hue, fill and cloud-floor blocks of `prototype.patch`, without the `moonlight` sentinel.
3. `s_psRemasterCommon`: the V18 crown term from `ovr_final`; `psPostComposite`: the in-scatter weighting.
4. Leave the tuning-knob defaults as they are.
5. Tell the clouds track about the clear-sky floor: their overcast-0.5 work sits on the same `cover`.

## Production needs

- Lazy buffer: review for animated LODs (it uses `VBDynamic`), proxies, multiplayer, memory for many spawned classes, and the 3D UI previews (they draw with the classic programs while Remastered is active; a lazily buffered shape would draw there through the GPU classic path).
- `test_remaster_lighting.cpp` and the README calibration table must be re-pinned: the hue test at 7 degrees ("evening sun keeps the original's sunset hue") compares against Classic with 10% tolerance and will move; the 7-degree "wall facing the sun lit well above one facing away" check still passes by the model (ratio 4.97 vs the threshold 4; shipped 14.8; `scripts/unitcheck.py`); add pins for the clear-sky floor (no cloud shadows at skyThrough 0.959) and for the fill (anti-sun wall about 0.59 of the sun wall at 7 to 14 degrees).
- Drop the research debug-3 albedo hack, or keep albedo as a real debug view: it made this calibration possible.
- Capture harness: props spawned by script are what exposed bug 1; keep them, and pin weather so `setOvercast 0` really means no cloud shadows (fixed by the floor).

## Risks vs the owner taste list

- **Warmth:** ground r:b 1.16 to 1.46 vs Classic 0.93 to 1.22 at 4.8 to 12.7 degrees, with lower g:r (pinker), not mustard. Hull lit faces at least as warm as Classic except at 4.8 degrees (r:b 2.57 vs 2.66, g:r 0.53 vs 0.50). If the owner finds the 12-degree ground too orange, the session-1 V11 (hue share 0.3) is the gentler step.
- **Shade:** 0.58 to 0.62 on M113 and Zelda paint (target 0.6, crushed was 0.42); ground cast shadows unchanged (0.70 to 0.79 of lit). Shaded hull faces are neutral (r:b 0.95 to 1.10), where Classic's are bluish to neutral (0.76 to 1.02).
- **Only darker:** crowns at golden hour are darker than shipped (0.78 to 0.67) but above Classic (0.57) and read greener; the shaft veil removal darkens near ground toward a 5-degree sun (`sunrise_ridge`); and spawned vehicle sides at noon go from 0.97 (shipped, accidentally Classic-lit) to 0.60. That last one is what per-pixel light gives at a 72-degree sun; the knob grid's k4 (bounce 2.5) is the lever if the owner wants noon vehicle sides lighter. Frame-level brightness at noon is unchanged (ground 1.04).
- **Flattened crowns:** not flattened (spread 0.42 vs shipped 0.44, Classic 0.38), but not more voluminous either.
- **Classic:** the final build keeps Classic on the CPU path; whole Classic frames vary about 18% run to run on the shipped exe itself (lowsun_box Classic ground F 0.48 in most runs, 0.59 in `run8`), so compare Classic within a run.

## Side findings

- Classic's frame brightness for the same view is unstable between runs (above); Classic references should come from the same run.
- Any clear-weather capture, `20261017-final` included, could carry a random 0.2 cloud shadow; the cloud field drifts during a run even with `triSetSimTime` re-pinned (`run10` forest_edge_noon ground F 0.74 to 0.91 across shots of one view).
- GPU timings from shared-machine runs are not usable for cost decisions.

## Files

- `BEST_*.png` (final set, session 2); `s1_BEST_*.png` (session-1 set, V9 on the first lazy-VB build); `final/cmp_<view>.png` for all 13 views, `final/crop_*.png`, `final/measure_final.md`, `final/crowns_final.md`, `final/timings_final.md`.
- `prototype.patch`: lazy vertex buffer + the C++ rig (hue, fill, clear-sky cloud floor), research-gated by `moonlight=1.01`. Scratch worktree `D:\Arma_CWA_worktrees\gfx-shade`, build `dist/gfx-shade-rwdi`.
- `ovr_final/`: the shader half of the final (crown V18 + in-scatter; rig terms off, pairs with the patch). `ovr_v17/`: every GLSL variant behind the `localLights` selector (V1 to V21, rig in GLSL, runs on any build). `ovr_v12/`, `ovr_v4/`, `ovr_recommended/`, `ovr_base/`, `ovr_v2*`, `ovr_v3*`, `ovr_diag2/`: earlier steps.
- Evidence images: `cmp_rootcause_prelit.png`, `cmp_clear_sky_cloud_shadow.png`, `cmp_cloud_floor_fix.png`, `diag_noon_sun_visibility.png`, `cmp_sunrise_ridge.png`, `grid_knobs_*.png`, `crowns_*_crops.png`, `variants_*.png`.
- Runs: `run1` to `run5`, `diag`, `diag2` (session 1); `run6` (crown V12-V16, evening boxes), `run6s` (shipped exe, new views), `run7a`/`run7b` (interleaved timing), `run10` (`run10/run10` crown V17/V18 and hull V20/V21, `run10/run9` C++ port check, `run10/run8` knob grid), `run11` (noon debug views), `run12` (cloud-shadow debug, 8 views), `run13`/`run13s` (final set), `run14` (cloud floor, same-run check), `final/` (merged). Each has `udcap.log`, `measure_<view>.json`, tri's tree.
- Tables: `measure_summary.md`, `model_table.txt`, `timings_run1_run3.md`, `timings_run4_run5.md`, `timings_run7_interleaved.md`.
- `scripts/` (measure, summ, crownstats, cloudfrac, rigmodel, unitcheck, gen_tests, gen_tests6, split_collect, merge_final, make_cmp, crop_grid, timings, timing7); `tests/` and the scratch worktree's `tests/rendering/research/shade/` (generated research tests, git-excluded).
