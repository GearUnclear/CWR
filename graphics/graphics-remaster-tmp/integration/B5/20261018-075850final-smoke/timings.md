# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| smoke_1 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 147.50 | 1,291 | 740 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 10.15 | 8.45 | 0.43 | 0.88 | 3.67 | 3.90 | 0.06 |  | 1.58 | 0.10 |  | 0.09 | 0.05 | 0.22 | 0.99 | 2.76 | 125.10 | 763 | 740 |  | OK |
| smoke_3 | classic | 2.65 | 2.47 | 0.26 | 0.12 | 1.36 | 0.99 | 0.06 |  | 1.48 | 0.10 |  | 0.09 | 0.05 | 0.22 | 0.89 | 1.48 | 114.50 | 1,291 | 740 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 10.23 | 9.11 | 0.14 | 1.61 | 5.07 | 2.44 | 0.06 |  | 1.04 | 0.21 |  | 0.09 | 0.05 | 0.22 | 0.27 | 1.48 | 101.80 | 763 | 740 |  | OK |
