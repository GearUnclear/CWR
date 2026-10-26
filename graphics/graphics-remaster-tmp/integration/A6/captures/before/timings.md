# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lighttiles_town_night | off | 6.32 | 5.41 |  | 0.35 | 1.40 | 3.66 | 0.71 | 0.34 |  | 0.22 | 0.09 |  | 0.06 | 5.66 | 103.30 | 1,126 | 1,003 | UNSETTLED:300 | OK |
| lighttiles_town_night | on | 6.09 | 5.11 |  | 0.36 | 1.49 | 3.26 | 0.79 | 0.43 |  | 0.21 | 0.09 |  | 0.06 | 5.63 | 93.30 | 1,116 | 1,010 | OK:34 | OK |
| lighttiles_town_night | delta | -0.23 | -0.30 |  | +0.01 | +0.09 | -0.40 | +0.08 | +0.09 |  | -0.01 | +0.00 |  | +0.00 | -0.03 |  | -0.9% | +0.7% |  |  |
| lighttiles_town_lamps | off | 5.29 | 4.20 |  | 0.51 | 2.10 | 1.60 | 0.81 | 0.34 |  | 0.26 | 0.12 |  | 0.08 | 4.85 | 108.90 | 877 | 552 | UNSETTLED:300 | OK |
| lighttiles_town_lamps | on | 5.53 | 4.47 |  | 0.62 | 2.25 | 1.59 | 0.78 | 0.30 |  | 0.26 | 0.13 |  | 0.09 | 5.05 | 108.30 | 877 | 552 | UNSETTLED:300 | OK |
| lighttiles_town_lamps | delta | +0.24 | +0.27 |  | +0.11 | +0.15 | -0.01 | -0.03 | -0.04 |  | +0.00 | +0.01 |  | +0.01 | +0.20 |  | +0.0% | +0.0% |  |  |
| lighttiles_sunrise | off | 3.84 | 2.59 | 0.11 | 0.35 | 1.91 | 0.33 | 1.08 | 0.38 |  | 0.17 | 0.08 | 0.36 | 0.10 | 3.37 | 70.90 | 1,912 | 8,897 | OK:30 | OK |
| lighttiles_sunrise | on | 3.86 | 2.61 | 0.11 | 0.32 | 1.93 | 0.36 | 1.08 | 0.36 |  | 0.17 | 0.08 | 0.35 | 0.13 | 3.17 | 67.10 | 1,912 | 8,897 | OK:30 | OK |
| lighttiles_sunrise | delta | +0.02 | +0.02 | +0.00 | -0.03 | +0.02 | +0.03 | +0.00 | -0.02 |  | +0.00 | +0.00 | -0.01 | +0.03 | -0.20 |  | +0.0% | +0.0% |  |  |
