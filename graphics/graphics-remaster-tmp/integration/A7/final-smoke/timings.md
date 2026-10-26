# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | sky | skytable | post | resolve | lindepth | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| smoke_1 | classic |  |  |  |  |  |  |  |  |  |  |  |  | 177.40 | 1,291 | 740 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 5.13 | 3.93 | 0.09 | 0.11 |  | 0.99 | 0.24 |  | 0.15 | 0.09 | 0.40 | 0.11 | 160.00 | 1,285 | 740 |  | OK |
| smoke_3 | classic | 2.76 | 2.52 | 0.09 | 0.11 |  | 0.97 | 0.24 |  | 0.15 | 0.09 | 0.40 | 0.09 | 148.40 | 1,291 | 740 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 7.19 | 5.30 | 0.14 | 0.17 |  | 1.57 | 0.53 |  | 0.17 | 0.13 | 0.49 | 0.25 | 140.50 | 1,285 | 740 |  | OK |
