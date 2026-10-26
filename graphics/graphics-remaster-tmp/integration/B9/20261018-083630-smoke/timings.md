# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| smoke_1 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 113.60 | 1,291 | 740 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 7.38 | 5.96 | 0.25 | 1.07 | 1.96 | 2.92 | 0.06 |  |  | 1.33 | 0.58 |  | 0.09 | 0.05 | 0.22 | 0.07 | 1.51 | 105.60 | 763 | 740 |  | OK |
| smoke_3 | classic | 2.36 | 2.20 | 0.26 | 0.15 | 1.33 | 0.72 | 0.06 |  |  | 1.13 | 0.44 |  | 0.09 | 0.05 | 0.22 | 0.06 | 1.51 | 102.20 | 1,291 | 740 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 4.95 | 3.86 | 0.15 | 0.88 | 1.61 | 1.37 | 0.06 |  |  | 0.97 | 0.39 |  | 0.09 | 0.05 | 0.22 | 0.04 | 1.51 | 96.50 | 763 | 740 |  | OK |
