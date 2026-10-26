# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| smoke_1 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 176.10 | 1,291 | 740 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 5.69 | 4.46 | 0.14 | 1.22 | 1.93 | 1.31 | 0.07 |  |  | 1.12 | 0.30 | 0.01 | 0.09 | 0.05 |  | 0.02 | 1.49 | 151.10 | 763 | 740 |  | OK |
| smoke_3 | classic | 2.22 | 2.06 | 0.13 | 0.15 | 1.24 | 0.67 | 0.07 |  |  | 1.09 | 0.30 | 0.01 | 0.09 | 0.05 |  | 0.02 | 1.49 | 136.20 | 1,291 | 740 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 5.80 | 4.53 | 0.23 | 0.97 | 1.97 | 1.59 | 0.07 |  |  | 1.13 | 0.27 | 0.01 | 0.09 | 0.05 |  | 0.02 | 1.49 | 123.60 | 763 | 740 |  | OK |
