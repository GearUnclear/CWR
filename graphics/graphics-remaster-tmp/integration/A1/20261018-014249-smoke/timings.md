# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| smoke_1 | classic |  |  |  |  |  |  |  |  | 130.90 | 612 | 218 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 5.33 | 3.13 | 0.17 | 1.95 | 0.79 | 0.18 | 0.83 | 0.16 | 122.30 | 583 | 218 |  | OK |
| smoke_3 | classic | 3.74 | 3.25 | 0.17 | 2.05 | 0.87 | 0.19 | 0.84 | 0.15 | 114.20 | 612 | 218 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 6.59 | 3.96 | 0.19 | 2.34 | 0.88 | 0.22 | 0.99 | 0.25 | 116.10 | 583 | 218 |  | OK |
