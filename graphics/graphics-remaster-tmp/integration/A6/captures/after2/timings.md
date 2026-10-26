# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lighttiles_town_night | off | 5.02 | 4.24 |  | 0.33 | 1.06 | 2.85 | 0.61 | 0.35 |  | 0.15 | 0.07 |  | 0.04 | 4.50 | 114.50 | 1,211 | 1,106 | OK:260 | OK |
| lighttiles_town_night | on | 10.06 | 9.28 |  | 0.82 | 2.63 | 5.83 | 0.62 | 0.36 |  | 0.15 | 0.06 |  | 0.04 | 5.63 | 78.70 | 1,207 | 1,094 | OK:40 | OK |
| lighttiles_town_night | delta | +5.04 | +5.04 |  | +0.49 | +1.57 | +2.98 | +0.01 | +0.01 |  | +0.00 | -0.01 |  | +0.00 | +1.13 |  | -0.3% | -1.1% |  |  |
| lighttiles_town_lamps | off | 5.09 | 4.04 |  | 0.46 | 1.93 | 1.65 | 0.78 | 0.33 |  | 0.25 | 0.12 |  | 0.08 | 4.51 | 116.90 | 877 | 552 | UNSETTLED:300 | OK |
| lighttiles_town_lamps | on | 8.43 | 7.72 |  | 1.47 | 4.52 | 1.73 | 0.54 | 0.28 |  | 0.15 | 0.06 |  | 0.04 | 8.01 | 84.90 | 804 | 552 | OK:30 | OK |
| lighttiles_town_lamps | delta | +3.34 | +3.68 |  | +1.01 | +2.59 | +0.08 | -0.24 | -0.05 |  | -0.10 | -0.06 |  | -0.04 | +3.50 |  | -8.3% | +0.0% |  |  |
| lighttiles_town_moonless | off | 5.24 | 4.21 |  | 0.60 | 2.09 | 1.52 | 0.75 | 0.29 |  | 0.26 | 0.12 |  | 0.08 | 4.75 | 115.20 | 879 | 552 | UNSETTLED:300 | OK |
| lighttiles_town_moonless | on | 8.51 | 7.77 |  | 1.33 | 4.74 | 1.69 | 0.57 | 0.31 |  | 0.15 | 0.06 |  | 0.04 | 8.30 | 82.90 | 806 | 552 | OK:30 | OK |
| lighttiles_town_moonless | delta | +3.27 | +3.56 |  | +0.73 | +2.65 | +0.17 | -0.18 | +0.02 |  | -0.11 | -0.06 |  | -0.04 | +3.55 |  | -8.3% | +0.0% |  |  |
| lighttiles_sunrise | off | 4.46 | 2.85 | 0.15 | 0.48 | 1.92 | 0.44 | 1.38 | 0.39 |  | 0.22 | 0.11 | 0.46 | 0.20 | 3.75 | 68.50 | 1,913 | 8,897 | OK:30 | OK |
| lighttiles_sunrise | on | 5.10 | 3.34 | 0.19 | 0.62 | 2.18 | 0.54 | 1.51 | 0.41 |  | 0.24 | 0.13 | 0.56 | 0.17 | 4.49 | 66.40 | 1,913 | 8,897 | OK:30 | OK |
| lighttiles_sunrise | delta | +0.64 | +0.49 | +0.04 | +0.14 | +0.26 | +0.10 | +0.13 | +0.02 |  | +0.02 | +0.02 | +0.10 | -0.03 | +0.74 |  | +0.0% | +0.0% |  |  |
