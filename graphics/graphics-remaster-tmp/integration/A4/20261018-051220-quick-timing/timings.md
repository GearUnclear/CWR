# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| sunrise_ridge_r1 | off | 19.34 | 18.15 | 0.08 | 0.27 | 15.73 | 2.14 | 1.03 | 0.52 |  | 0.13 | 0.06 | 0.27 | 0.06 | 18.68 | 34.40 | 12,910 | 8,897 | OK:30 | OK |
| sunrise_ridge_r1 | on | 4.00 | 2.63 | 0.14 | 0.40 | 1.79 | 0.44 | 1.17 | 0.38 |  | 0.21 | 0.10 | 0.40 | 0.09 | 2.34 | 70.20 | 1,895 | 8,897 | OK:30 | OK |
| sunrise_ridge_r1 | delta | -15.34 | -15.52 | +0.06 | +0.13 | -13.94 | -1.70 | +0.14 | -0.14 |  | +0.08 | +0.04 | +0.13 | +0.03 | -16.34 |  | -85.3% | +0.0% |  |  |
| sunrise_walk_r1 | off | 20.95 | 19.78 | 0.10 | 3.49 | 15.19 | 1.10 | 1.01 | 0.47 |  | 0.12 | 0.06 | 0.27 | 0.08 | 16.96 | 37.30 | 12,939 | 8,936 | OK:30 | OK |
| sunrise_walk_r1 | on | 5.32 | 3.59 | 0.28 | 0.56 | 2.45 | 0.59 | 1.47 | 0.41 |  | 0.23 | 0.13 | 0.56 | 0.15 | 2.31 | 65.90 | 1,920 | 8,936 | OK:30 | OK |
| sunrise_walk_r1 | delta | -15.63 | -16.19 | +0.18 | -2.93 | -12.74 | -0.51 | +0.46 | -0.06 |  | +0.11 | +0.07 | +0.29 | +0.07 | -14.65 |  | -85.2% | +0.0% |  |  |
