# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| smoke_1 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 160.80 | 1,291 | 740 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 7.54 | 6.39 | 0.12 | 1.51 | 2.93 | 1.94 | 0.07 |  | 1.01 | 0.28 |  | 0.09 | 0.05 | 0.22 | 0.12 | 1.50 | 136.90 | 763 | 740 |  | OK |
| smoke_3 | classic | 2.42 | 2.25 | 0.12 | 0.16 | 1.22 | 0.87 | 0.07 |  | 1.23 | 0.23 |  | 0.09 | 0.05 | 0.22 | 0.30 | 1.46 | 125.60 | 1,291 | 740 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 4.90 | 3.98 | 0.12 | 1.04 | 1.73 | 1.21 | 0.07 |  | 0.80 | 0.24 |  | 0.09 | 0.05 | 0.22 | 0.05 | 1.46 | 117.10 | 763 | 740 |  | OK |
