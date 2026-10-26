# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| smoke_1 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 180.40 | 1,291 | 740 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 6.39 | 5.21 | 0.17 | 1.21 | 2.31 | 1.69 | 0.09 |  | 1.01 | 0.30 |  | 0.12 | 0.07 | 0.30 | 0.04 | 1.95 | 154.20 | 763 | 740 |  | OK |
| smoke_3 | classic | 2.23 | 2.05 | 0.16 | 0.15 | 1.18 | 0.73 | 0.08 |  | 0.98 | 0.29 |  | 0.12 | 0.07 | 0.29 | 0.04 | 1.67 | 143.70 | 1,291 | 740 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 6.32 | 5.31 | 0.13 | 1.42 | 2.36 | 1.53 | 0.07 |  | 0.90 | 0.30 |  | 0.09 | 0.05 | 0.23 | 0.04 | 1.64 | 128.90 | 763 | 740 |  | OK |
