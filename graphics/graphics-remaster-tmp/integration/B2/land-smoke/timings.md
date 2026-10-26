# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| smoke_1 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 131.30 | 1,291 | 740 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 10.26 | 8.49 | 0.21 | 1.20 | 2.57 | 4.73 | 0.10 |  | 1.58 | 0.75 |  | 0.14 | 0.08 | 0.34 | 0.07 | 2.15 | 107.10 | 763 | 740 |  | OK |
| smoke_3 | classic | 5.73 | 5.51 | 0.19 | 0.17 | 3.23 | 2.12 | 0.10 |  | 1.38 | 0.58 |  | 0.14 | 0.08 | 0.33 | 0.05 | 2.12 | 94.60 | 1,291 | 740 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 7.16 | 5.73 | 0.17 | 1.25 | 2.50 | 1.98 | 0.09 |  | 1.28 | 0.54 |  | 0.12 | 0.07 | 0.31 | 0.05 | 2.12 | 88.00 | 1,440 | 1,177 |  | OK |
