# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| lowsun_fp | on | 11.46 | 9.31 | 0.17 | 1.50 | 3.98 | 3.83 | 0.09 |  |  | 1.92 | 0.14 |  | 0.14 | 0.07 | 0.26 | 1.08 | 7.20 | 65.50 | 1,212 | 1,316 | OK:30 | OK |
| sunrise_ridge | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sunrise_ridge | on | 7.39 | 5.32 | 0.19 | 1.68 | 2.61 | 1.03 | 0.11 |  |  | 1.81 | 0.16 |  | 0.13 | 0.07 | 0.20 | 1.01 | 4.76 | 49.90 | 1,912 | 8,897 | OK:30 | OK |
| coastline_sunset | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| coastline_sunset | on | 4.44 | 2.78 | 0.00 | 1.97 | 0.00 | 0.81 | 0.09 |  |  | 1.46 | 0.18 |  | 0.10 | 0.07 | 0.21 | 0.66 | 2.76 | 190.90 | 177 | 0 | OK:30 | OK |
