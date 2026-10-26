# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| smoke_1 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 175.50 | 1,291 | 740 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 6.13 | 4.78 | 0.18 | 0.93 | 2.06 | 1.80 | 0.10 |  | 1.17 | 0.29 |  | 0.14 | 0.08 | 0.34 | 0.10 | 2.08 | 147.10 | 763 | 740 |  | OK |
| smoke_3 | classic | 2.62 | 2.40 | 0.19 | 0.18 | 1.42 | 0.79 | 0.10 |  | 1.15 | 0.30 |  | 0.14 | 0.08 | 0.34 | 0.07 | 2.08 | 135.10 | 1,291 | 740 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 6.50 | 5.13 | 0.19 | 1.11 | 2.25 | 1.77 | 0.11 |  | 1.18 | 0.26 |  | 0.14 | 0.08 | 0.36 | 0.06 | 2.08 | 126.20 | 763 | 740 |  | OK |
