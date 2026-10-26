# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| valley_0515_r1 | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| valley_0515_r1 | on | 25.96 | 23.32 | 0.04 | 2.23 | 18.78 | 2.31 | 0.09 |  |  | 2.38 | 0.28 | 0.01 | 0.06 | 0.06 |  | 0.04 | 25.29 | 27.50 | 5,242 | 7,966 | OK:30 | OK |
| sunrise_ridge_r1 | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sunrise_ridge_r1 | on | 10.13 | 8.24 | 0.10 | 5.95 | 1.88 | 0.40 | 0.12 |  |  | 1.58 | 0.41 | 0.01 | 0.13 | 0.06 |  | 0.04 | 5.02 | 65.70 | 1,912 | 8,897 | OK:30 | OK |
| valley_0515_r2 | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| valley_0515_r2 | on | 46.42 | 42.79 | 0.11 | 3.87 | 35.21 | 3.71 | 0.09 |  |  | 3.36 | 0.17 | 0.01 | 0.06 | 0.09 |  | 0.39 | 24.83 | 18.50 | 5,242 | 7,966 | OK:30 | OK |
| sunrise_ridge_r2 | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sunrise_ridge_r2 | on | 10.26 | 8.48 | 0.08 | 5.87 | 2.26 | 0.35 | 0.12 |  |  | 1.48 | 0.38 | 0.01 | 0.13 | 0.06 |  | 0.03 | 5.02 | 64.30 | 1,912 | 8,897 | OK:30 | OK |
| valley_0515_r3 | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| valley_0515_r3 | on | 33.29 | 30.13 | 0.04 | 2.77 | 24.59 | 2.77 | 0.09 |  |  | 2.89 | 0.17 | 0.01 | 0.06 | 0.13 |  | 0.18 | 24.68 | 20.10 | 5,242 | 7,966 | OK:30 | OK |
| sunrise_ridge_r3 | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sunrise_ridge_r3 | on | 10.25 | 8.41 | 0.08 | 5.89 | 2.15 | 0.38 | 0.12 |  |  | 1.54 | 0.38 | 0.01 | 0.13 | 0.06 |  | 0.04 | 5.03 | 65.30 | 1,912 | 8,897 | OK:30 | OK |
