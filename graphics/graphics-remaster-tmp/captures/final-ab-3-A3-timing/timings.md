# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| valley_0515_r1 | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| valley_0515_r1 | on | 29.12 | 27.83 | 0.04 |  |  |  |  |  |  | 1.15 | 0.66 |  | 0.06 | 0.07 | 0.31 | 0.06 |  | 30.10 | 11,406 | 7,966 | OK:30 | OK |
| sunrise_ridge_r1 | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sunrise_ridge_r1 | on | 30.29 | 28.70 | 0.13 |  |  |  |  |  |  | 1.43 | 0.53 |  | 0.13 | 0.06 | 0.27 | 0.44 |  | 25.40 | 12,910 | 8,897 | OK:30 | OK |
| valley_0515_r2 | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| valley_0515_r2 | on | 31.14 | 29.51 | 0.04 |  |  |  |  |  |  | 1.48 | 0.69 |  | 0.06 | 0.07 | 0.31 | 0.35 |  | 19.20 | 11,406 | 7,966 | OK:30 | OK |
| sunrise_ridge_r2 | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sunrise_ridge_r2 | on | 24.99 | 23.77 | 0.09 |  |  |  |  |  |  | 1.06 | 0.55 |  | 0.13 | 0.06 | 0.27 | 0.05 |  | 27.00 | 12,910 | 8,897 | OK:30 | OK |
| valley_0515_r3 | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| valley_0515_r3 | on | 36.53 | 34.82 | 0.04 |  |  |  |  |  |  | 1.57 | 0.74 |  | 0.06 | 0.07 | 0.31 | 0.39 |  | 17.50 | 11,406 | 7,966 | OK:30 | OK |
| sunrise_ridge_r3 | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sunrise_ridge_r3 | on | 25.48 | 24.20 | 0.13 |  |  |  |  |  |  | 1.12 | 0.45 |  | 0.13 | 0.06 | 0.27 | 0.20 |  | 29.60 | 12,910 | 8,897 | OK:30 | OK |
