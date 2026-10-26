# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lighttiles_town_night | off | 6.13 | 5.02 |  | 0.38 | 1.42 | 3.21 | 0.71 | 0.35 |  | 0.21 | 0.09 |  | 0.05 | 5.60 | 102.00 | 1,120 | 1,011 | OK:148 | OK |
| lighttiles_town_night | on | 6.30 | 5.47 |  | 0.43 | 1.68 | 3.36 | 0.63 | 0.31 |  | 0.20 | 0.08 |  | 0.05 | 5.73 | 102.50 | 1,145 | 1,059 | OK:82 | OK |
| lighttiles_town_night | delta | +0.17 | +0.45 |  | +0.05 | +0.26 | +0.15 | -0.08 | -0.04 |  | -0.01 | -0.01 |  | +0.00 | +0.13 |  | +2.2% | +4.5% |  |  |
| lighttiles_town_lamps | off | 5.23 | 4.18 |  | 0.54 | 2.07 | 1.57 | 0.77 | 0.32 |  | 0.26 | 0.12 |  | 0.08 | 4.73 | 114.50 | 877 | 552 | UNSETTLED:300 | OK |
| lighttiles_town_lamps | on | 5.85 | 4.98 |  | 0.74 | 2.62 | 1.61 | 0.66 | 0.27 |  | 0.24 | 0.09 |  | 0.06 | 5.37 | 108.30 | 804 | 552 | OK:30 | OK |
| lighttiles_town_lamps | delta | +0.62 | +0.80 |  | +0.20 | +0.55 | +0.04 | -0.11 | -0.05 |  | -0.02 | -0.03 |  | -0.02 | +0.64 |  | -8.3% | +0.0% |  |  |
| lighttiles_town_moonless | off | 5.95 | 4.44 |  | 0.67 | 2.13 | 1.64 | 0.86 | 0.40 |  | 0.26 | 0.12 |  | 0.08 | 5.06 | 102.00 | 879 | 552 | UNSETTLED:300 | OK |
| lighttiles_town_moonless | on | 6.20 | 5.07 |  | 0.70 | 2.69 | 1.68 | 0.79 | 0.41 |  | 0.23 | 0.09 |  | 0.06 | 5.48 | 97.10 | 806 | 552 | OK:30 | OK |
| lighttiles_town_moonless | delta | +0.25 | +0.63 |  | +0.03 | +0.56 | +0.04 | -0.07 | +0.01 |  | -0.03 | -0.03 |  | -0.02 | +0.42 |  | -8.3% | +0.0% |  |  |
| lighttiles_sunrise | off | 5.04 | 3.45 | 0.16 | 0.62 | 2.35 | 0.48 | 1.35 | 0.42 |  | 0.21 | 0.12 | 0.48 | 0.11 | 3.91 | 58.60 | 1,913 | 8,897 | OK:30 | OK |
| lighttiles_sunrise | on | 5.78 | 3.75 | 0.20 | 0.82 | 2.39 | 0.54 | 1.76 | 0.50 |  | 0.26 | 0.14 | 0.61 | 0.25 | 4.75 | 54.20 | 1,913 | 8,897 | OK:30 | OK |
| lighttiles_sunrise | delta | +0.74 | +0.30 | +0.04 | +0.20 | +0.04 | +0.06 | +0.41 | +0.08 |  | +0.05 | +0.02 | +0.13 | +0.14 | +0.84 |  | +0.0% | +0.0% |  |  |
| smoke_1 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 160.60 | 1,291 | 740 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 5.20 | 3.87 | 0.10 | 0.44 | 1.47 | 1.96 | 1.22 | 0.37 |  | 0.17 | 0.11 | 0.48 | 0.10 | 2.57 | 139.60 | 763 | 740 |  | OK |
| smoke_3 | classic | 4.13 | 3.64 | 0.10 | 0.53 | 1.95 | 1.17 | 1.25 | 0.39 |  | 0.17 | 0.11 | 0.48 | 0.09 | 2.57 | 125.50 | 1,291 | 740 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 7.05 | 4.89 | 0.26 | 0.73 | 1.61 | 2.55 | 1.88 | 0.75 |  | 0.17 | 0.21 | 0.42 | 0.33 | 2.57 | 114.40 | 763 | 740 |  | OK |
