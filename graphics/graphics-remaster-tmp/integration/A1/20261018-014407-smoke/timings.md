# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| smoke_1 | classic |  |  |  |  |  |  |  |  | 131.90 | 612 | 218 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 5.68 | 3.32 | 0.22 | 2.11 | 0.81 | 0.20 | 0.91 | 0.19 | 121.80 | 583 | 218 |  | OK |
| smoke_3 | classic | 4.34 | 3.81 | 0.20 | 2.17 | 0.87 | 0.20 | 0.91 | 0.19 | 113.10 | 612 | 218 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 6.56 | 3.94 | 0.19 | 2.27 | 0.90 | 0.21 | 0.97 | 0.20 | 115.90 | 583 | 218 |  | OK |
