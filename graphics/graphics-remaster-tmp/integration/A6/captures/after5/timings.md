# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lighttiles_town_night | off | 5.22 | 4.44 |  | 0.41 | 1.26 | 2.78 | 0.61 | 0.35 |  | 0.15 | 0.06 |  | 0.04 | 4.42 | 106.40 | 1,178 | 1,075 | UNSETTLED:300 | OK |
| lighttiles_town_night | on | 5.16 | 4.42 |  | 0.32 | 1.23 | 2.88 | 0.58 | 0.32 |  | 0.15 | 0.06 |  | 0.04 | 4.90 | 109.40 | 1,206 | 1,101 | OK:30 | OK |
| lighttiles_town_night | delta | -0.06 | -0.02 |  | -0.09 | -0.03 | +0.10 | -0.03 | -0.03 |  | +0.00 | +0.00 |  | +0.00 | +0.48 |  | +2.3% | +2.4% |  |  |
| lighttiles_town_lamps | off | 5.04 | 3.99 |  | 0.50 | 1.97 | 1.52 | 0.79 | 0.34 |  | 0.25 | 0.12 |  | 0.08 | 4.61 | 116.40 | 877 | 552 | UNSETTLED:300 | OK |
| lighttiles_town_lamps | on | 6.00 | 5.10 |  | 0.70 | 2.57 | 1.83 | 0.70 | 0.31 |  | 0.23 | 0.09 |  | 0.06 | 5.46 | 107.60 | 804 | 552 | OK:30 | OK |
| lighttiles_town_lamps | delta | +0.96 | +1.11 |  | +0.20 | +0.60 | +0.31 | -0.09 | -0.03 |  | -0.02 | -0.03 |  | -0.02 | +0.85 |  | -8.3% | +0.0% |  |  |
| lighttiles_town_moonless | off | 3.28 | 2.53 |  | 0.28 | 1.19 | 1.06 | 0.57 | 0.28 |  | 0.17 | 0.07 |  | 0.04 | 3.01 | 132.40 | 879 | 552 | UNSETTLED:300 | OK |
| lighttiles_town_moonless | on | 3.81 | 3.12 |  | 0.45 | 1.63 | 1.04 | 0.51 | 0.25 |  | 0.15 | 0.06 |  | 0.04 | 3.64 | 124.20 | 806 | 552 | OK:30 | OK |
| lighttiles_town_moonless | delta | +0.53 | +0.59 |  | +0.17 | +0.44 | -0.02 | -0.06 | -0.03 |  | -0.02 | -0.01 |  | +0.00 | +0.63 |  | -8.3% | +0.0% |  |  |
| lighttiles_sunrise | off | 4.10 | 2.72 | 0.12 | 0.41 | 1.95 | 0.35 | 1.21 | 0.38 |  | 0.19 | 0.09 | 0.39 | 0.16 | 3.36 | 67.90 | 1,912 | 8,897 | OK:30 | OK |
| lighttiles_sunrise | on | 4.15 | 2.78 | 0.16 | 0.53 | 1.81 | 0.43 | 1.15 | 0.34 |  | 0.20 | 0.10 | 0.41 | 0.09 | 3.76 | 68.30 | 1,912 | 8,897 | OK:30 | OK |
| lighttiles_sunrise | delta | +0.05 | +0.06 | +0.04 | +0.12 | -0.14 | +0.08 | -0.06 | -0.04 |  | +0.01 | +0.01 | +0.02 | -0.07 | +0.40 |  | +0.0% | +0.0% |  |  |
| smoke_1 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 218.60 | 1,291 | 740 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 3.19 | 2.35 | 0.06 | 0.22 | 0.98 | 1.15 | 0.77 | 0.26 |  | 0.12 | 0.06 | 0.28 | 0.05 | 1.80 | 187.50 | 763 | 740 |  | OK |
| smoke_3 | classic | 2.29 | 2.11 | 0.06 | 0.11 | 1.35 | 0.66 | 0.78 | 0.25 |  | 0.12 | 0.06 | 0.29 | 0.05 | 1.80 | 163.50 | 1,291 | 740 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 3.33 | 2.48 | 0.07 | 0.23 | 0.99 | 1.26 | 0.77 | 0.23 |  | 0.12 | 0.06 | 0.29 | 0.06 | 1.80 | 154.50 | 763 | 740 |  | OK |
