# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | resolve | lindepth | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| smoke_1 | classic |  |  |  |  |  |  |  |  |  |  | 152.10 | 1,291 | 740 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 8.61 | 7.31 | 0.55 | 1.23 | 0.13 |  | 0.09 | 0.05 | 0.22 | 0.48 | 127.80 | 1,285 | 740 |  | OK |
| smoke_3 | classic | 3.85 | 3.68 | 0.95 | 1.22 | 0.11 |  | 0.09 | 0.05 | 0.22 | 0.57 | 114.60 | 1,291 | 740 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 11.58 | 10.04 | 1.08 | 1.46 | 0.10 |  | 0.09 | 0.05 | 0.22 | 0.87 | 100.30 | 1,285 | 740 |  | OK |
