# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| smoke_1 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 115.50 | 1,291 | 740 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 10.28 | 8.68 | 0.25 | 1.99 | 3.69 | 3.00 | 0.07 |  |  | 1.50 | 0.26 | 0.01 | 0.09 | 0.05 |  | 0.02 | 1.65 | 101.30 | 763 | 740 |  | OK |
| smoke_3 | classic | 5.05 | 4.86 | 0.26 | 0.21 | 2.65 | 2.00 | 0.07 |  |  | 1.65 | 0.17 | 0.01 | 0.09 | 0.05 |  | 0.02 | 1.65 | 93.70 | 1,291 | 740 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 13.51 | 11.68 | 0.24 | 2.47 | 5.23 | 3.99 | 0.07 |  |  | 1.73 | 0.17 | 0.01 | 0.09 | 0.05 |  | 0.02 | 1.48 | 83.40 | 763 | 740 |  | OK |
