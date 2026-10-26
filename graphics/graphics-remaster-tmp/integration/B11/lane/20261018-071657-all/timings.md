# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| b11_overcast_broken | off | 9.26 | 8.23 | 0.24 | 2.67 | 4.93 | 0.63 | 0.93 | 0.48 |  | 0.12 | 0.07 |  | 0.03 | 5.70 | 66.30 | 2,214 | 5,692 | OK:30 | OK |
| b11_overcast_broken | on | 6.67 | 5.55 | 0.29 | 1.53 | 3.38 | 0.64 | 1.00 | 0.45 |  | 0.12 | 0.07 |  | 0.03 | 5.82 | 69.20 | 2,214 | 5,692 | OK:30 | OK |
| b11_overcast_broken | delta | -2.59 | -2.68 | +0.05 | -1.14 | -1.55 | +0.01 | +0.07 | -0.03 |  | +0.00 | +0.00 |  | +0.00 | +0.12 |  | +0.0% | +0.0% |  |  |
| texsharp_forest_edge | off | 3.78 | 2.79 | 0.23 | 0.78 | 1.46 | 0.55 | 0.83 | 0.25 |  | 0.16 | 0.08 |  | 0.04 | 3.51 | 150.50 | 811 | 1,447 | OK:30 | OK |
| texsharp_forest_edge | on | 6.09 | 4.94 | 0.49 | 1.18 | 2.31 | 1.44 | 1.01 | 0.21 |  | 0.14 | 0.07 |  | 0.05 | 3.48 | 135.60 | 811 | 1,447 | OK:30 | OK |
| texsharp_forest_edge | delta | +2.31 | +2.15 | +0.26 | +0.40 | +0.85 | +0.89 | +0.18 | -0.04 |  | -0.02 | -0.01 |  | +0.01 | -0.03 |  | +0.0% | +0.0% |  |  |
| b11_sharpen_forest_edge | off | 2.95 | 2.33 | 0.22 | 0.59 | 1.24 | 0.51 | 0.49 | 0.23 |  | 0.14 | 0.07 |  | 0.05 | 2.50 | 160.80 | 811 | 1,447 | OK:30 | OK |
| b11_sharpen_forest_edge | on | 2.95 | 2.29 | 0.24 | 0.53 | 1.13 | 0.64 | 0.52 | 0.23 |  | 0.14 | 0.07 |  | 0.03 | 2.53 | 159.40 | 811 | 1,447 | OK:30 | OK |
| b11_sharpen_forest_edge | delta | +0.00 | -0.04 | +0.02 | -0.06 | -0.11 | +0.13 | +0.03 | +0.00 |  | +0.00 | +0.00 |  | -0.02 | +0.03 |  | +0.0% | +0.0% |  |  |
| b11_noon_fields | off | 6.00 | 5.15 | 0.11 | 0.88 | 3.81 | 0.46 | 0.72 | 0.27 |  | 0.12 | 0.06 |  | 0.03 | 5.56 | 84.30 | 1,566 | 4,508 | OK:30 | OK |
| b11_noon_fields | on | 6.16 | 5.28 | 0.11 | 1.16 | 3.67 | 0.45 | 0.74 | 0.30 |  | 0.12 | 0.07 |  | 0.03 | 5.78 | 75.20 | 1,566 | 4,508 | OK:30 | OK |
| b11_noon_fields | delta | +0.16 | +0.13 | +0.00 | +0.28 | -0.14 | -0.01 | +0.02 | +0.03 |  | +0.00 | +0.01 |  | +0.00 | +0.22 |  | +0.0% | +0.0% |  |  |
| b11_coastline_sunset | off | 3.50 | 1.84 | 0.00 | 0.83 | 0.00 | 1.01 | 1.54 | 0.49 |  | 0.10 | 0.06 | 0.29 | 0.38 | 1.68 | 216.40 | 177 | 0 | OK:30 | OK |
| b11_coastline_sunset | on | 3.40 | 1.79 | 0.00 | 0.91 | 0.00 | 0.88 | 1.49 | 0.32 |  | 0.10 | 0.06 | 0.29 | 0.50 | 1.80 | 235.80 | 177 | 0 | OK:30 | OK |
| b11_coastline_sunset | delta | -0.10 | -0.05 | +0.00 | +0.08 | +0.00 | -0.13 | -0.05 | -0.17 |  | +0.00 | +0.00 | +0.00 | +0.12 | +0.12 |  | +0.0% |  |  |  |
| b11_lowsun_fp | off | 9.66 | 7.98 | 0.28 | 1.02 | 3.79 | 3.17 | 1.54 | 0.36 |  | 0.14 | 0.07 | 0.36 | 0.39 | 5.28 | 71.30 | 1,212 | 1,316 | OK:30 | OK |
| b11_lowsun_fp | on | 10.23 | 8.55 | 0.20 | 1.67 | 3.27 | 3.61 | 1.54 | 0.44 |  | 0.14 | 0.07 | 0.36 | 0.32 | 5.59 | 67.30 | 1,212 | 1,316 | OK:30 | OK |
| b11_lowsun_fp | delta | +0.57 | +0.57 | -0.08 | +0.65 | -0.52 | +0.44 | +0.00 | +0.08 |  | +0.00 | +0.00 | +0.00 | -0.07 | +0.31 |  | +0.0% | +0.0% |  |  |
| b11cost_forest | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| b11cost_forest | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| b11cost_noon | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| b11cost_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
