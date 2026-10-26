# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| smoke_1 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 123.70 | 1,291 | 740 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 10.80 | 9.21 | 0.13 | 2.13 | 3.90 | 3.18 | 0.07 |  | 1.48 | 0.17 |  | 0.09 | 0.05 | 0.22 | 0.68 | 1.55 | 103.50 | 763 | 740 |  | OK |
| smoke_3 | classic | 3.00 | 2.82 | 0.13 | 0.19 | 1.52 | 1.10 | 0.07 |  | 1.44 | 0.20 |  | 0.09 | 0.05 | 0.22 | 0.63 | 1.47 | 95.70 | 1,291 | 740 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 13.13 | 11.47 | 0.13 | 2.42 | 4.90 | 4.15 | 0.07 |  | 1.55 | 0.11 |  | 0.09 | 0.05 | 0.22 | 0.89 | 1.47 | 84.90 | 763 | 740 |  | OK |
