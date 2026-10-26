# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| b11_overcast_broken | off | 8.73 | 7.70 | 0.23 | 2.75 | 4.40 | 0.54 | 0.92 | 0.48 |  | 0.12 | 0.07 |  | 0.03 | 5.64 | 62.40 | 2,214 | 5,692 | OK:30 | OK |
| b11_overcast_broken | on | 8.69 | 7.70 | 0.24 | 3.02 | 4.08 | 0.60 | 0.87 | 0.43 |  | 0.12 | 0.07 |  | 0.03 | 5.78 | 63.70 | 2,214 | 5,692 | OK:30 | OK |
| b11_overcast_broken | delta | -0.04 | +0.00 | +0.01 | +0.27 | -0.32 | +0.06 | -0.05 | -0.05 |  | +0.00 | +0.00 |  | +0.00 | +0.14 |  | +0.0% | +0.0% |  |  |
| texsharp_forest_edge | off | 5.12 | 3.94 | 0.21 | 0.89 | 2.02 | 1.02 | 1.03 | 0.26 |  | 0.14 | 0.07 |  | 0.03 | 3.31 | 110.90 | 811 | 1,447 | OK:30 | OK |
| texsharp_forest_edge | on | 4.15 | 3.06 | 0.22 | 0.97 | 1.39 | 0.70 | 0.94 | 0.37 |  | 0.14 | 0.07 |  | 0.03 | 3.69 | 113.50 | 811 | 1,447 | OK:30 | OK |
| texsharp_forest_edge | delta | -0.97 | -0.88 | +0.01 | +0.08 | -0.63 | -0.32 | -0.09 | +0.11 |  | +0.00 | +0.00 |  | +0.00 | +0.38 |  | +0.0% | +0.0% |  |  |
| b11_sharpen_forest_edge | off | 4.01 | 3.27 | 0.33 | 0.76 | 1.68 | 0.83 | 0.60 | 0.34 |  | 0.14 | 0.07 |  | 0.04 | 2.83 | 114.50 | 811 | 1,447 | OK:30 | OK |
| b11_sharpen_forest_edge | on | 4.08 | 3.33 | 0.37 | 0.57 | 1.57 | 1.19 | 0.61 | 0.30 |  | 0.14 | 0.07 |  | 0.03 | 2.74 | 118.30 | 811 | 1,447 | OK:30 | OK |
| b11_sharpen_forest_edge | delta | +0.07 | +0.06 | +0.04 | -0.19 | -0.11 | +0.36 | +0.01 | -0.04 |  | +0.00 | +0.00 |  | -0.01 | -0.09 |  | +0.0% | +0.0% |  |  |
| b11_noon_fields | off | 6.17 | 5.17 | 0.11 | 0.94 | 3.75 | 0.48 | 0.87 | 0.41 |  | 0.12 | 0.07 |  | 0.03 | 5.78 | 73.70 | 1,566 | 4,508 | OK:30 | OK |
| b11_noon_fields | on | 6.31 | 5.37 | 0.13 | 1.21 | 3.68 | 0.47 | 0.80 | 0.36 |  | 0.12 | 0.07 |  | 0.03 | 5.83 | 74.20 | 1,566 | 4,508 | OK:30 | OK |
| b11_noon_fields | delta | +0.14 | +0.20 | +0.02 | +0.27 | -0.07 | -0.01 | -0.07 | -0.05 |  | +0.00 | +0.00 |  | +0.00 | +0.05 |  | +0.0% | +0.0% |  |  |
| b11_coastline_sunset | off | 4.03 | 1.67 | 0.00 | 1.20 | 0.00 | 0.47 | 2.14 | 0.49 |  | 0.21 | 0.14 | 0.61 | 0.21 | 2.96 | 239.00 | 177 | 0 | OK:30 | OK |
| b11_coastline_sunset | on | 5.32 | 2.37 | 0.00 | 1.89 | 0.00 | 0.49 | 2.71 | 0.53 |  | 0.24 | 0.16 | 0.82 | 0.16 | 4.38 | 190.20 | 177 | 0 | OK:30 | OK |
| b11_coastline_sunset | delta | +1.29 | +0.70 | +0.00 | +0.69 | +0.00 | +0.02 | +0.57 | +0.04 |  | +0.03 | +0.02 | +0.21 | -0.05 | +1.42 |  | +0.0% |  |  |  |
| b11_lowsun_fp | off | 6.16 | 4.63 | 0.22 | 0.73 | 2.14 | 1.76 | 1.39 | 0.39 |  | 0.15 | 0.07 | 0.41 | 0.11 | 5.63 | 96.80 | 1,212 | 1,316 | OK:30 | OK |
| b11_lowsun_fp | on | 6.20 | 4.70 | 0.18 | 0.95 | 2.00 | 1.75 | 1.36 | 0.36 |  | 0.15 | 0.07 | 0.39 | 0.12 | 5.81 | 94.80 | 1,212 | 1,316 | OK:30 | OK |
| b11_lowsun_fp | delta | +0.04 | +0.07 | -0.04 | +0.22 | -0.14 | -0.01 | -0.03 | -0.03 |  | +0.00 | +0.00 | -0.02 | +0.01 | +0.18 |  | +0.0% | +0.0% |  |  |
| b11cost_forest | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| b11cost_forest | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| b11cost_noon | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| b11cost_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
