# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| b11_overcast_broken | off | 6.34 | 5.29 | 0.24 | 1.43 | 3.31 | 0.54 | 0.92 | 0.41 |  | 0.12 | 0.07 |  | 0.03 | 5.78 | 75.30 | 2,214 | 5,692 | OK:30 | OK |
| b11_overcast_broken | on | 6.48 | 5.43 | 0.24 | 1.44 | 3.44 | 0.55 | 0.93 | 0.46 |  | 0.12 | 0.07 |  | 0.03 | 5.80 | 75.00 | 2,214 | 5,692 | OK:30 | OK |
| b11_overcast_broken | delta | +0.14 | +0.14 | +0.00 | +0.01 | +0.13 | +0.01 | +0.01 | +0.05 |  | +0.00 | +0.00 |  | +0.00 | +0.02 |  | +0.0% | +0.0% |  |  |
| texsharp_forest_edge | off | 5.58 | 4.15 | 0.33 | 1.13 | 2.24 | 0.78 | 1.19 | 0.31 |  | 0.24 | 0.12 |  | 0.07 | 5.21 | 119.20 | 811 | 1,447 | OK:30 | OK |
| texsharp_forest_edge | on | 5.65 | 4.23 | 0.33 | 1.09 | 2.17 | 0.97 | 1.18 | 0.30 |  | 0.23 | 0.12 |  | 0.06 | 5.07 | 119.10 | 811 | 1,447 | OK:30 | OK |
| texsharp_forest_edge | delta | +0.07 | +0.08 | +0.00 | -0.04 | -0.07 | +0.19 | -0.01 | -0.01 |  | -0.01 | +0.00 |  | -0.01 | -0.14 |  | +0.0% | +0.0% |  |  |
| b11_sharpen_forest_edge | off | 4.67 | 3.73 | 0.39 | 0.50 | 2.21 | 1.02 | 0.71 | 0.27 |  | 0.23 | 0.13 |  | 0.09 | 3.60 | 130.40 | 811 | 1,447 | OK:30 | OK |
| b11_sharpen_forest_edge | on | 5.33 | 4.23 | 0.49 | 0.66 | 2.43 | 1.14 | 0.86 | 0.26 |  | 0.27 | 0.14 |  | 0.18 | 4.44 | 118.60 | 811 | 1,447 | OK:30 | OK |
| b11_sharpen_forest_edge | delta | +0.66 | +0.50 | +0.10 | +0.16 | +0.22 | +0.12 | +0.15 | -0.01 |  | +0.04 | +0.01 |  | +0.09 | +0.84 |  | +0.0% | +0.0% |  |  |
| b11_noon_fields | off | 6.24 | 5.38 | 0.11 | 0.97 | 3.97 | 0.44 | 0.73 | 0.27 |  | 0.12 | 0.07 |  | 0.03 | 5.82 | 83.00 | 1,566 | 4,508 | OK:30 | OK |
| b11_noon_fields | on | 6.18 | 5.27 | 0.11 | 0.89 | 3.79 | 0.59 | 0.78 | 0.29 |  | 0.12 | 0.07 |  | 0.03 | 5.76 | 85.50 | 1,566 | 4,508 | OK:30 | OK |
| b11_noon_fields | delta | -0.06 | -0.11 | +0.00 | -0.08 | -0.18 | +0.15 | +0.05 | +0.02 |  | +0.00 | +0.00 |  | +0.00 | -0.06 |  | +0.0% | +0.0% |  |  |
| b11_coastline_sunset | off | 3.76 | 1.47 | 0.00 | 1.13 | 0.00 | 0.34 | 2.07 | 0.32 |  | 0.21 | 0.14 | 0.65 | 0.24 | 2.95 | 270.80 | 177 | 0 | OK:30 | OK |
| b11_coastline_sunset | on | 4.80 | 1.97 | 0.00 | 1.55 | 0.00 | 0.41 | 2.58 | 0.37 |  | 0.27 | 0.24 | 0.84 | 0.18 | 4.01 | 240.10 | 177 | 0 | OK:30 | OK |
| b11_coastline_sunset | delta | +1.04 | +0.50 | +0.00 | +0.42 | +0.00 | +0.07 | +0.51 | +0.05 |  | +0.06 | +0.10 | +0.19 | -0.06 | +1.06 |  | +0.0% |  |  |  |
| b11_lowsun_fp | off | 5.32 | 4.05 | 0.18 | 0.67 | 1.87 | 1.50 | 1.14 | 0.29 |  | 0.14 | 0.06 | 0.36 | 0.05 | 5.16 | 99.40 | 1,212 | 1,316 | OK:30 | OK |
| b11_lowsun_fp | on | 7.95 | 6.57 | 0.19 | 1.09 | 3.37 | 2.11 | 1.24 | 0.31 |  | 0.14 | 0.06 | 0.37 | 0.14 | 5.14 | 93.10 | 1,212 | 1,316 | OK:30 | OK |
| b11_lowsun_fp | delta | +2.63 | +2.52 | +0.01 | +0.42 | +1.50 | +0.61 | +0.10 | +0.02 |  | +0.00 | +0.00 | +0.01 | +0.09 | -0.02 |  | +0.0% | +0.0% |  |  |
| b11cost_forest | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| b11cost_forest | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| b11cost_noon | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| b11cost_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
