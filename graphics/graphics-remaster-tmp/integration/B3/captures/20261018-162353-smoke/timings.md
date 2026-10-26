# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| smoke_1 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 117.40 | 1,291 | 740 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 9.20 | 7.56 | 0.17 | 1.23 | 3.26 | 3.06 | 0.07 |  |  | 1.49 | 0.37 |  | 0.09 | 0.05 | 0.22 | 0.31 | 1.60 | 101.80 | 763 | 740 |  | OK |
| smoke_3 | classic | 2.29 | 2.11 | 0.14 | 0.13 | 1.21 | 0.77 | 0.07 |  |  | 1.36 | 0.31 |  | 0.09 | 0.05 | 0.22 | 0.30 | 1.50 | 94.70 | 1,291 | 740 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 9.81 | 8.54 | 0.14 | 1.61 | 3.70 | 3.23 | 0.07 |  |  | 1.16 | 0.13 |  | 0.09 | 0.05 | 0.22 | 0.49 | 1.50 | 88.10 | 763 | 740 |  | OK |
