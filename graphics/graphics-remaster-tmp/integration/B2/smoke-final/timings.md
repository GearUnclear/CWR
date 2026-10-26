# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| smoke_1 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 153.40 | 1,291 | 740 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 6.24 | 5.06 | 0.17 | 1.21 | 2.22 | 1.63 | 0.09 |  | 1.02 | 0.25 |  | 0.13 | 0.07 | 0.32 | 0.05 | 1.95 | 134.80 | 763 | 740 |  | OK |
| smoke_3 | classic | 2.24 | 2.05 | 0.17 | 0.16 | 1.21 | 0.68 | 0.09 |  | 0.99 | 0.22 |  | 0.13 | 0.07 | 0.32 | 0.05 | 1.93 | 126.90 | 1,291 | 740 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 5.96 | 4.69 | 0.17 | 1.04 | 2.07 | 1.58 | 0.09 |  | 1.11 | 0.26 |  | 0.13 | 0.07 | 0.31 | 0.15 | 1.93 | 121.90 | 763 | 740 |  | OK |
