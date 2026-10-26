# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| smoke_1 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 171.20 | 1,291 | 740 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 6.56 | 5.31 | 0.18 | 1.17 | 2.46 | 1.68 | 0.10 |  | 1.08 | 0.26 |  | 0.14 | 0.08 | 0.34 | 0.06 | 2.00 | 143.60 | 763 | 740 |  | OK |
| smoke_3 | classic | 2.36 | 2.17 | 0.22 | 0.16 | 1.33 | 0.68 | 0.10 |  | 1.06 | 0.27 |  | 0.13 | 0.08 | 0.33 | 0.05 | 1.95 | 131.90 | 1,291 | 740 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 6.18 | 4.90 | 0.23 | 1.08 | 2.19 | 1.63 | 0.09 |  | 1.13 | 0.28 |  | 0.13 | 0.07 | 0.32 | 0.13 | 1.95 | 122.40 | 763 | 740 |  | OK |
