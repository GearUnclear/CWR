# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| smoke_1 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 100.00 | 1,291 | 740 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 15.74 | 13.54 | 0.64 | 2.43 | 5.70 | 5.42 | 0.07 |  |  | 1.74 | 0.10 |  | 0.09 | 0.05 | 0.22 | 1.13 | 4.01 | 83.20 | 763 | 740 |  | OK |
| smoke_3 | classic | 4.23 | 4.02 | 0.69 | 0.20 | 2.18 | 1.64 | 0.07 |  |  | 1.69 | 0.10 |  | 0.09 | 0.05 | 0.22 | 1.09 | 1.50 | 77.40 | 1,291 | 740 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 9.70 | 8.30 | 0.20 | 1.81 | 3.42 | 3.07 | 0.07 |  |  | 1.30 | 0.19 |  | 0.09 | 0.05 | 0.22 | 0.50 | 1.50 | 73.20 | 763 | 740 |  | OK |
