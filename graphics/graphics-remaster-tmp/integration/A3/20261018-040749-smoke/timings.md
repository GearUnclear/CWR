# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | resolve | lindepth | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| smoke_1 | classic |  |  |  |  |  |  |  |  |  |  | 89.10 | 612 | 218 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 6.84 | 5.08 | 0.04 | 1.74 | 0.10 |  | 0.09 | 0.05 | 0.22 | 1.27 | 83.30 | 583 | 218 |  | OK |
| smoke_3 | classic | 5.48 | 5.29 | 0.04 | 1.61 | 0.09 |  | 0.10 | 0.05 | 0.22 | 1.16 | 77.80 | 612 | 218 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 6.19 | 4.67 | 0.04 | 1.48 | 0.27 |  | 0.10 | 0.05 | 0.22 | 0.85 | 80.90 | 583 | 218 |  | OK |
