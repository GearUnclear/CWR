# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lighttiles_town_night | off | 7.54 | 6.06 |  | 0.32 | 1.24 | 4.50 | 1.29 | 0.96 |  | 0.20 | 0.08 |  | 0.05 | 6.06 | 60.10 | 1,244 | 1,131 | UNSETTLED:300 | OK |
| lighttiles_town_night | on | 7.74 | 6.38 |  | 0.36 | 1.37 | 4.65 | 1.18 | 0.88 |  | 0.18 | 0.07 |  | 0.05 | 6.13 | 58.90 | 1,235 | 1,135 | OK:32 | OK |
| lighttiles_town_night | delta | +0.20 | +0.32 |  | +0.04 | +0.13 | +0.15 | -0.11 | -0.08 |  | -0.02 | -0.01 |  | +0.00 | +0.07 |  | -0.7% | +0.4% |  |  |
| lighttiles_town_lamps | off | 5.67 | 4.42 |  | 0.53 | 2.13 | 1.77 | 0.97 | 0.50 |  | 0.27 | 0.13 |  | 0.08 | 5.11 | 93.80 | 877 | 552 | UNSETTLED:300 | OK |
| lighttiles_town_lamps | on | 6.16 | 5.01 |  | 0.66 | 2.55 | 1.80 | 0.89 | 0.47 |  | 0.24 | 0.11 |  | 0.07 | 5.44 | 95.10 | 804 | 552 | OK:30 | OK |
| lighttiles_town_lamps | delta | +0.49 | +0.59 |  | +0.13 | +0.42 | +0.03 | -0.08 | -0.03 |  | -0.03 | -0.02 |  | -0.01 | +0.33 |  | -8.3% | +0.0% |  |  |
| lighttiles_town_moonless | off | 5.72 | 4.54 |  | 0.53 | 2.13 | 1.88 | 0.89 | 0.42 |  | 0.26 | 0.12 |  | 0.09 | 5.07 | 95.90 | 879 | 552 | UNSETTLED:300 | OK |
| lighttiles_town_moonless | on | 5.97 | 4.85 |  | 0.69 | 2.41 | 1.76 | 0.86 | 0.44 |  | 0.24 | 0.11 |  | 0.06 | 5.44 | 94.60 | 806 | 552 | OK:30 | OK |
| lighttiles_town_moonless | delta | +0.25 | +0.31 |  | +0.16 | +0.28 | -0.12 | -0.03 | +0.02 |  | -0.02 | -0.01 |  | -0.03 | +0.37 |  | -8.3% | +0.0% |  |  |
| lighttiles_sunrise | off | 4.69 | 3.16 | 0.16 | 0.50 | 2.16 | 0.50 | 1.30 | 0.38 |  | 0.22 | 0.11 | 0.47 | 0.11 | 3.82 | 67.60 | 1,912 | 8,897 | OK:30 | OK |
| lighttiles_sunrise | on | 5.37 | 3.33 | 0.19 | 0.62 | 2.21 | 0.50 | 1.78 | 0.44 |  | 0.24 | 0.13 | 0.58 | 0.39 | 4.56 | 66.00 | 1,912 | 8,897 | OK:30 | OK |
| lighttiles_sunrise | delta | +0.68 | +0.17 | +0.03 | +0.12 | +0.05 | +0.00 | +0.48 | +0.06 |  | +0.02 | +0.02 | +0.11 | +0.28 | +0.74 |  | +0.0% | +0.0% |  |  |
