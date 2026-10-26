# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| smoke_1 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 167.40 | 1,291 | 740 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 6.18 | 5.02 | 0.17 | 1.07 | 2.23 | 1.72 | 0.09 |  | 1.01 | 0.23 |  | 0.13 | 0.07 | 0.32 | 0.07 | 1.95 | 147.40 | 763 | 740 |  | OK |
| smoke_3 | classic | 2.33 | 2.14 | 0.17 | 0.16 | 1.28 | 0.70 | 0.09 |  | 1.05 | 0.25 |  | 0.13 | 0.07 | 0.32 | 0.09 | 1.95 | 139.80 | 1,291 | 740 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 6.26 | 5.07 | 0.17 | 1.18 | 2.27 | 1.61 | 0.09 |  | 1.04 | 0.28 |  | 0.13 | 0.07 | 0.31 | 0.06 | 1.95 | 132.30 | 763 | 740 |  | OK |
