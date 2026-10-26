# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| b8_lowsun_fp | off | 10.70 | 9.11 | 0.17 | 1.52 | 4.66 | 2.94 | 0.09 |  | 1.39 | 0.24 |  | 0.14 | 0.07 | 0.37 | 0.36 | 6.14 | 77.20 | 1,212 | 1,316 | OK:30 | OK |
| b8_lowsun_fp | on | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| b8_pine_eve | off | 10.46 | 9.18 | 0.26 | 1.30 | 7.02 | 0.86 | 0.11 |  | 1.06 | 0.33 |  | 0.10 | 0.07 | 0.27 | 0.05 | 10.12 | 75.40 | 2,043 | 2,100 | OK:30 | OK |
| b8_pine_eve | on | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
