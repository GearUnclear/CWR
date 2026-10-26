# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lighttiles_town_night | off | 6.32 | 5.40 |  | 0.33 | 1.34 | 3.73 | 0.73 | 0.39 |  | 0.20 | 0.08 |  | 0.05 | 5.70 | 100.60 | 1,210 | 1,093 | OK:253 | OK |
| lighttiles_town_night | on | 6.14 | 5.28 |  | 0.32 | 1.29 | 3.66 | 0.67 | 0.33 |  | 0.20 | 0.08 |  | 0.05 | 5.62 | 103.20 | 1,191 | 1,099 | OK:35 | OK |
| lighttiles_town_night | delta | -0.18 | -0.12 |  | -0.01 | -0.05 | -0.07 | -0.06 | -0.06 |  | +0.00 | +0.00 |  | +0.00 | -0.08 |  | -1.6% | +0.5% |  |  |
| lighttiles_town_lamps | off | 5.42 | 4.04 |  | 0.49 | 2.07 | 1.48 | 0.82 | 0.36 |  | 0.26 | 0.12 |  | 0.08 | 4.74 | 113.30 | 877 | 552 | UNSETTLED:300 | OK |
| lighttiles_town_lamps | on | 5.98 | 4.69 |  | 0.62 | 2.26 | 1.80 | 0.95 | 0.48 |  | 0.26 | 0.12 |  | 0.08 | 5.19 | 89.20 | 877 | 552 | UNSETTLED:300 | OK |
| lighttiles_town_lamps | delta | +0.56 | +0.65 |  | +0.13 | +0.19 | +0.32 | +0.13 | +0.12 |  | +0.00 | +0.00 |  | +0.00 | +0.45 |  | +0.0% | +0.0% |  |  |
| lighttiles_town_moonless | off | 5.67 | 4.36 |  | 0.53 | 2.11 | 1.72 | 1.02 | 0.54 |  | 0.26 | 0.13 |  | 0.09 | 5.17 | 92.10 | 879 | 552 | UNSETTLED:300 | OK |
| lighttiles_town_moonless | on | 6.51 | 5.17 |  | 0.55 | 2.12 | 2.50 | 1.06 | 0.59 |  | 0.26 | 0.12 |  | 0.08 | 5.12 | 73.00 | 879 | 552 | UNSETTLED:300 | OK |
| lighttiles_town_moonless | delta | +0.84 | +0.81 |  | +0.02 | +0.01 | +0.78 | +0.04 | +0.05 |  | +0.00 | -0.01 |  | -0.01 | -0.05 |  | +0.0% | +0.0% |  |  |
| lighttiles_sunrise | off | 4.95 | 3.27 | 0.21 | 0.63 | 2.13 | 0.51 | 1.42 | 0.36 |  | 0.23 | 0.13 | 0.56 | 0.15 | 4.29 | 66.80 | 1,912 | 8,897 | OK:30 | OK |
| lighttiles_sunrise | on | 5.62 | 3.87 | 0.21 | 0.66 | 2.46 | 0.75 | 1.47 | 0.31 |  | 0.26 | 0.14 | 0.62 | 0.14 | 4.87 | 64.10 | 1,912 | 8,897 | OK:30 | OK |
| lighttiles_sunrise | delta | +0.67 | +0.60 | +0.00 | +0.03 | +0.33 | +0.24 | +0.05 | -0.05 |  | +0.03 | +0.01 | +0.06 | -0.01 | +0.58 |  | +0.0% | +0.0% |  |  |
