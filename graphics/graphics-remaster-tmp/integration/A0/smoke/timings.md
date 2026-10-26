# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| smoke_1 | classic |  |  |  |  |  |  |  |  | 135.80 | 612 | 218 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 2.61 | 1.47 | 0.06 | 1.03 | 0.56 | 0.07 | 0.31 | 0.09 | 129.80 | 583 | 218 |  | OK |
| smoke_3 | classic | 1.79 | 1.56 | 0.06 | 1.07 | 0.61 | 0.07 | 0.31 | 0.08 | 125.10 | 612 | 218 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 4.30 | 2.56 | 0.10 | 1.56 | 0.66 | 0.12 | 0.56 | 0.22 | 131.60 | 583 | 218 |  | OK |
