# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lighttiles_town_night | off | 7.61 | 6.84 |  | 0.83 | 1.85 | 4.17 | 0.61 | 0.35 |  | 0.15 | 0.06 |  | 0.04 | 4.37 | 98.90 | 1,146 | 1,047 | OK:208 | OK |
| lighttiles_town_night | on | 5.36 | 4.39 |  | 0.40 | 1.21 | 2.78 | 0.63 | 0.37 |  | 0.15 | 0.06 |  | 0.04 | 4.71 | 109.80 | 1,145 | 1,037 | OK:40 | OK |
| lighttiles_town_night | delta | -2.25 | -2.45 |  | -0.43 | -0.64 | -1.39 | +0.02 | +0.02 |  | +0.00 | +0.00 |  | +0.00 | +0.34 |  | -0.1% | -1.0% |  |  |
| lighttiles_town_lamps | off | 3.87 | 3.13 |  | 0.57 | 1.51 | 1.06 | 0.56 | 0.29 |  | 0.17 | 0.07 |  | 0.04 | 2.88 | 128.80 | 877 | 552 | UNSETTLED:300 | OK |
| lighttiles_town_lamps | on | 4.20 | 3.47 |  | 0.52 | 1.73 | 1.22 | 0.56 | 0.31 |  | 0.15 | 0.06 |  | 0.04 | 3.77 | 115.00 | 804 | 552 | OK:30 | OK |
| lighttiles_town_lamps | delta | +0.33 | +0.34 |  | -0.05 | +0.22 | +0.16 | +0.00 | +0.02 |  | -0.02 | -0.01 |  | +0.00 | +0.89 |  | -8.3% | +0.0% |  |  |
| lighttiles_town_moonless | off | 5.74 | 5.04 |  | 0.67 | 2.51 | 1.86 | 0.52 | 0.27 |  | 0.16 | 0.06 |  | 0.04 | 2.83 | 112.60 | 879 | 552 | UNSETTLED:300 | OK |
| lighttiles_town_moonless | on | 4.85 | 4.03 |  | 0.47 | 1.81 | 1.75 | 0.65 | 0.39 |  | 0.16 | 0.06 |  | 0.04 | 3.77 | 113.90 | 806 | 552 | OK:30 | OK |
| lighttiles_town_moonless | delta | -0.89 | -1.01 |  | -0.20 | -0.70 | -0.11 | +0.13 | +0.12 |  | +0.00 | +0.00 |  | +0.00 | +0.94 |  | -8.3% | +0.0% |  |  |
| lighttiles_sunrise | off | 5.38 | 4.05 | 0.24 | 0.51 | 2.44 | 1.09 | 1.20 | 0.31 |  | 0.14 | 0.07 | 0.28 | 0.40 | 3.34 | 65.80 | 1,912 | 8,897 | OK:30 | OK |
| lighttiles_sunrise | on | 3.56 | 2.56 | 0.08 | 0.28 | 1.99 | 0.30 | 0.86 | 0.33 |  | 0.13 | 0.06 | 0.27 | 0.07 | 2.85 | 68.90 | 1,912 | 8,897 | OK:30 | OK |
| lighttiles_sunrise | delta | -1.82 | -1.49 | -0.16 | -0.23 | -0.45 | -0.79 | -0.34 | +0.02 |  | -0.01 | -0.01 | -0.01 | -0.33 | -0.49 |  | +0.0% | +0.0% |  |  |
| timing_lamps | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| timing_lamps | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_1 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 161.60 | 1,291 | 740 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 4.97 | 3.66 | 0.10 | 0.43 | 1.51 | 1.72 | 1.21 | 0.29 |  | 0.17 | 0.11 | 0.47 | 0.18 | 2.00 | 144.30 | 763 | 740 |  | OK |
| smoke_3 | classic | 3.33 | 3.06 | 0.10 | 0.22 | 1.75 | 1.09 | 1.16 | 0.28 |  | 0.17 | 0.11 | 0.48 | 0.13 | 2.00 | 133.00 | 1,291 | 740 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 6.95 | 4.89 | 0.16 | 0.74 | 2.04 | 2.10 | 1.76 | 0.73 |  | 0.17 | 0.20 | 0.42 | 0.24 | 2.00 | 123.60 | 763 | 740 |  | OK |
