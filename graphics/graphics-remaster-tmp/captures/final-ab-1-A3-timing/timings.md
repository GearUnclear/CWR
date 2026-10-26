# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| valley_0515_r1 | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| valley_0515_r1 | on | 31.41 | 30.07 | 0.04 |  |  |  |  |  |  | 1.20 | 0.70 |  | 0.06 | 0.07 | 0.31 | 0.06 |  | 30.40 | 11,406 | 7,966 | OK:30 | OK |
| sunrise_ridge_r1 | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sunrise_ridge_r1 | on | 24.28 | 23.03 | 0.15 |  |  |  |  |  |  | 1.10 | 0.56 |  | 0.13 | 0.06 | 0.27 | 0.08 |  | 29.90 | 12,910 | 8,897 | OK:30 | OK |
| valley_0515_r2 | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| valley_0515_r2 | on | 53.33 | 51.38 | 0.08 |  |  |  |  |  |  | 1.81 | 0.76 |  | 0.06 | 0.07 | 0.31 | 0.60 |  | 18.10 | 11,406 | 7,966 | OK:30 | OK |
| sunrise_ridge_r2 | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sunrise_ridge_r2 | on | 27.63 | 26.23 | 0.08 |  |  |  |  |  |  | 1.24 | 0.60 |  | 0.13 | 0.06 | 0.27 | 0.18 |  | 27.40 | 12,910 | 8,897 | OK:30 | OK |
| valley_0515_r3 | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| valley_0515_r3 | on | 57.97 | 55.46 | 0.04 |  |  |  |  |  |  | 2.36 | 1.14 |  | 0.06 | 0.07 | 0.31 | 0.78 |  | 16.70 | 11,406 | 7,966 | OK:30 | OK |
| sunrise_ridge_r3 | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sunrise_ridge_r3 | on | 24.75 | 23.55 | 0.08 |  |  |  |  |  |  | 1.04 | 0.47 |  | 0.13 | 0.06 | 0.27 | 0.11 |  | 30.40 | 12,910 | 8,897 | OK:30 | OK |
