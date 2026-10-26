# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| valley_0515_r1 | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| valley_0515_r1 | on | 26.29 | 23.42 | 0.04 | 2.25 | 18.88 | 2.29 | 0.09 |  |  | 2.60 | 0.36 | 0.01 | 0.06 | 0.06 |  | 0.04 | 25.47 | 28.00 | 5,242 | 7,966 | OK:30 | OK |
| sunrise_ridge_r1 | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sunrise_ridge_r1 | on | 16.20 | 13.00 | 0.11 | 9.78 | 2.56 | 0.66 | 0.16 |  |  | 2.80 | 1.25 | 0.01 | 0.17 | 0.08 |  | 0.05 | 5.09 | 46.00 | 1,912 | 8,897 | OK:30 | OK |
| valley_0515_r2 | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| valley_0515_r2 | on | 25.75 | 23.13 | 0.04 | 2.26 | 18.94 | 1.93 | 0.09 |  |  | 2.35 | 0.16 | 0.01 | 0.06 | 0.06 |  | 0.05 | 24.54 | 29.40 | 5,242 | 7,966 | OK:30 | OK |
| sunrise_ridge_r2 | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sunrise_ridge_r2 | on | 9.95 | 8.14 | 0.08 | 5.59 | 2.14 | 0.41 | 0.12 |  |  | 1.51 | 0.40 | 0.01 | 0.13 | 0.06 |  | 0.03 | 5.05 | 61.40 | 1,912 | 8,897 | OK:30 | OK |
| valley_0515_r3 | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| valley_0515_r3 | on | 55.71 | 51.98 | 0.23 | 4.68 | 42.70 | 4.61 | 0.09 |  |  | 3.46 | 0.17 | 0.01 | 0.06 | 0.23 |  | 0.30 | 52.91 | 16.20 | 5,242 | 7,966 | OK:30 | OK |
| sunrise_ridge_r3 | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sunrise_ridge_r3 | on | 14.19 | 11.50 | 0.08 | 6.18 | 4.10 | 1.21 | 0.12 |  |  | 2.37 | 0.26 | 0.01 | 0.13 | 0.06 |  | 0.04 | 7.44 | 53.90 | 1,912 | 8,897 | OK:30 | OK |
