# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| smoke_1 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 186.30 | 1,291 | 740 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 6.20 | 5.07 | 0.17 | 1.48 | 2.30 | 1.29 | 0.06 |  | 1.04 | 0.32 | 0.01 | 0.09 | 0.05 |  | 0.02 | 3.42 | 153.20 | 763 | 740 |  | OK |
| smoke_3 | classic | 2.08 | 1.92 | 0.29 | 0.15 | 1.13 | 0.64 | 0.06 |  | 1.03 | 0.32 | 0.01 | 0.09 | 0.05 |  | 0.02 | 1.47 | 139.40 | 1,291 | 740 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 8.93 | 7.45 | 0.60 | 1.09 | 2.88 | 3.48 | 0.06 |  | 1.38 | 0.09 | 0.01 | 0.09 | 0.05 |  | 0.02 | 1.47 | 125.60 | 763 | 740 |  | OK |
