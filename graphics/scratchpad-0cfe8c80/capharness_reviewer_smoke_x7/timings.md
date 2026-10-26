# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| smoke_1 | classic |  |  |  |  |  |  |  |  | 139.90 | 1,594 | 871 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 5.67 | 4.54 | 0.13 | 0.88 | 0.62 | 0.15 |  | 0.11 | 128.10 | 1,589 | 871 |  | OK |
| smoke_3 | classic | 4.03 | 3.68 | 0.13 | 0.90 | 0.62 | 0.17 |  | 0.11 | 127.60 | 1,594 | 871 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 6.13 | 4.89 | 0.14 | 0.98 | 0.69 | 0.17 |  | 0.12 | 123.60 | 1,589 | 871 |  | OK |
