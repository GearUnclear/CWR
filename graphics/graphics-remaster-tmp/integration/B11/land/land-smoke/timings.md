# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| smoke_1 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 179.10 | 1,291 | 740 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 5.95 | 4.98 | 0.13 | 1.77 | 1.95 | 1.25 | 0.06 |  | 0.88 | 0.32 |  | 0.09 | 0.05 | 0.22 | 0.03 | 1.49 | 149.90 | 763 | 740 |  | OK |
| smoke_3 | classic | 2.79 | 2.63 | 0.13 | 0.20 | 1.29 | 1.14 | 0.06 |  | 0.95 | 0.31 |  | 0.09 | 0.05 | 0.22 | 0.07 | 1.48 | 133.20 | 1,291 | 740 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 6.49 | 5.41 | 0.13 | 1.57 | 2.22 | 1.62 | 0.06 |  | 0.98 | 0.24 |  | 0.09 | 0.05 | 0.22 | 0.19 | 1.48 | 122.00 | 763 | 740 |  | OK |
