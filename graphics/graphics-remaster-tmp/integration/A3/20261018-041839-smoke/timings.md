# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | resolve | lindepth | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| smoke_1 | classic |  |  |  |  |  |  |  |  |  |  | 189.20 | 612 | 218 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 3.71 | 2.61 | 0.04 | 1.02 | 0.19 |  | 0.09 | 0.05 | 0.22 | 0.47 | 170.90 | 583 | 218 |  | OK |
| smoke_3 | classic | 2.04 | 1.87 | 0.04 | 1.09 | 0.22 |  | 0.09 | 0.05 | 0.22 | 0.50 | 155.20 | 612 | 218 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 2.16 | 1.27 | 0.04 | 0.83 | 0.38 |  | 0.09 | 0.05 | 0.22 | 0.08 | 164.80 | 583 | 218 |  | OK |
