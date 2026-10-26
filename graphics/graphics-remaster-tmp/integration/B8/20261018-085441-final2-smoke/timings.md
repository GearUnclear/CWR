# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| smoke_1 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 185.30 | 1,291 | 740 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 4.53 | 3.43 | 0.14 | 0.71 | 1.47 | 1.25 | 0.06 |  | 1.01 | 0.32 | 0.01 | 0.09 | 0.09 |  | 0.02 | 1.47 | 159.00 | 763 | 740 |  | OK |
| smoke_3 | classic | 1.82 | 1.66 | 0.13 | 0.11 | 0.94 | 0.61 | 0.06 |  | 1.02 | 0.32 | 0.01 | 0.09 | 0.10 |  | 0.02 | 1.47 | 146.20 | 1,291 | 740 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 6.71 | 5.53 | 0.38 | 1.35 | 2.17 | 2.01 | 0.06 |  | 1.08 | 0.25 | 0.01 | 0.09 | 0.10 |  | 0.02 | 1.47 | 133.00 | 763 | 740 |  | OK |
