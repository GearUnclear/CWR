# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| smoke_1 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 162.30 | 1,291 | 740 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 6.80 | 5.31 | 0.19 | 1.23 | 2.42 | 1.66 | 0.09 |  | 1.34 | 0.34 | 0.01 | 0.13 | 0.08 |  | 0.04 | 2.09 | 141.40 | 763 | 740 |  | OK |
| smoke_3 | classic | 2.32 | 2.12 | 0.18 | 0.17 | 1.24 | 0.71 | 0.09 |  | 1.25 | 0.30 | 0.01 | 0.13 | 0.07 |  | 0.04 | 1.87 | 132.90 | 1,291 | 740 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 6.25 | 4.79 | 0.17 | 1.01 | 2.01 | 1.78 | 0.09 |  | 1.32 | 0.35 | 0.01 | 0.12 | 0.07 |  | 0.03 | 1.87 | 125.40 | 763 | 740 |  | OK |
