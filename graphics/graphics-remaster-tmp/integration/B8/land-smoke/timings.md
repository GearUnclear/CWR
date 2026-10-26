# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| smoke_1 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 158.00 | 1,291 | 740 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 5.40 | 4.22 | 0.13 | 1.01 | 1.90 | 1.31 | 0.07 |  |  | 1.06 | 0.31 | 0.01 | 0.09 | 0.05 |  | 0.03 | 1.47 | 140.40 | 763 | 740 |  | OK |
| smoke_3 | classic | 3.46 | 3.28 | 0.13 | 0.14 | 1.72 | 1.43 | 0.07 |  |  | 1.07 | 0.31 | 0.01 | 0.09 | 0.05 |  | 0.03 | 1.47 | 130.50 | 1,291 | 740 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 5.78 | 4.55 | 0.13 | 1.03 | 1.91 | 1.62 | 0.07 |  |  | 1.12 | 0.36 | 0.01 | 0.09 | 0.05 |  | 0.02 | 1.47 | 115.30 | 763 | 740 |  | OK |
