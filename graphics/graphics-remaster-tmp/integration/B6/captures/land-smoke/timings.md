# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| smoke_1 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 172.20 | 1,291 | 740 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 5.71 | 4.10 | 0.13 | 0.81 | 1.66 | 1.63 | 0.07 |  |  | 1.50 | 0.37 | 0.01 | 0.09 | 0.05 |  | 0.02 | 1.54 | 146.60 | 763 | 740 |  | OK |
| smoke_3 | classic | 2.25 | 2.07 | 0.16 | 0.13 | 1.20 | 0.73 | 0.07 |  |  | 1.31 | 0.36 | 0.01 | 0.09 | 0.05 |  | 0.02 | 1.54 | 136.90 | 1,291 | 740 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 9.74 | 8.18 | 0.32 | 1.66 | 3.57 | 2.96 | 0.07 |  |  | 1.44 | 0.39 | 0.01 | 0.09 | 0.05 |  | 0.02 | 1.54 | 116.90 | 763 | 740 |  | OK |
