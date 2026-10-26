# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| smoke_1 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 158.00 | 1,291 | 740 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 4.93 | 3.76 | 0.13 | 0.81 | 1.66 | 1.29 | 0.06 |  | 1.06 | 0.36 | 0.01 | 0.10 | 0.07 |  | 0.03 | 1.89 | 141.50 | 763 | 740 |  | OK |
| smoke_3 | classic | 2.01 | 1.85 | 0.13 | 0.12 | 1.08 | 0.64 | 0.06 |  | 1.05 | 0.35 | 0.01 | 0.09 | 0.07 |  | 0.03 | 1.55 | 134.00 | 1,291 | 740 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 4.96 | 3.78 | 0.13 | 0.90 | 1.62 | 1.26 | 0.07 |  | 1.08 | 0.36 | 0.01 | 0.10 | 0.06 |  | 0.03 | 1.55 | 127.90 | 763 | 740 |  | OK |
