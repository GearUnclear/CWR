# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| smoke_1 | classic |  |  |  |  |  |  |  |  | 131.80 | 612 | 218 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 4.53 | 2.67 | 0.15 | 1.55 | 0.69 | 0.13 | 0.50 | 0.23 | 122.80 | 583 | 218 |  | OK |
| smoke_3 | classic | 3.66 | 3.14 | 0.19 | 1.71 | 0.82 | 0.13 | 0.52 | 0.23 | 113.70 | 612 | 218 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 5.72 | 3.53 | 0.19 | 1.83 | 0.89 | 0.20 | 0.60 | 0.14 | 116.40 | 583 | 218 |  | OK |
