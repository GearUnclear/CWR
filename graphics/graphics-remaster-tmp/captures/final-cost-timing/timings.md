# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| valley_0515_r1 | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| valley_0515_r1 | on | 27.42 | 24.66 | 0.04 | 2.36 | 19.87 | 2.43 | 0.09 |  |  | 2.49 | 0.30 | 0.01 | 0.06 | 0.06 |  | 0.04 | 26.26 | 28.60 | 5,242 | 7,966 | OK:30 | OK |
| sunrise_ridge_r1 | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sunrise_ridge_r1 | on | 10.09 | 8.15 | 0.09 | 5.82 | 1.97 | 0.36 | 0.13 |  |  | 1.62 | 0.44 | 0.01 | 0.14 | 0.06 |  | 0.04 | 5.04 | 65.40 | 1,912 | 8,897 | OK:30 | OK |
| valley_0515_r2 | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| valley_0515_r2 | on | 26.50 | 23.95 | 0.04 | 2.32 | 19.62 | 2.01 | 0.09 |  |  | 2.28 | 0.16 | 0.01 | 0.06 | 0.06 |  | 0.04 | 25.42 | 29.40 | 5,242 | 7,966 | OK:30 | OK |
| sunrise_ridge_r2 | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sunrise_ridge_r2 | on | 9.88 | 7.86 | 0.11 | 5.38 | 2.07 | 0.41 | 0.13 |  |  | 1.68 | 0.39 | 0.01 | 0.14 | 0.09 |  | 0.04 | 5.02 | 65.00 | 1,912 | 8,897 | OK:30 | OK |
