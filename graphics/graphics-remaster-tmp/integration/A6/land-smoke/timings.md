# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| smoke_1 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 163.10 | 1,291 | 740 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 6.20 | 5.00 | 0.19 | 1.10 | 2.18 | 1.71 | 0.09 |  | 1.02 | 0.26 |  | 0.13 | 0.07 | 0.32 | 0.06 | 1.91 | 142.90 | 763 | 740 |  | OK |
| smoke_3 | classic | 2.29 | 2.10 | 0.18 | 0.16 | 1.24 | 0.70 | 0.09 |  | 1.02 | 0.27 |  | 0.13 | 0.07 | 0.31 | 0.05 | 1.91 | 135.60 | 1,291 | 740 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 6.06 | 4.86 | 0.17 | 1.10 | 2.08 | 1.68 | 0.09 |  | 1.03 | 0.20 |  | 0.13 | 0.07 | 0.31 | 0.13 | 1.91 | 132.00 | 763 | 740 |  | OK |
