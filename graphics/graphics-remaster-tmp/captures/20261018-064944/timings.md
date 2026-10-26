# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| smoke_1 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 170.50 | 1,291 | 740 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 6.12 | 4.65 | 0.21 | 0.95 | 2.08 | 1.63 | 1.37 | 0.30 |  | 0.15 | 0.09 | 0.38 | 0.17 | 2.63 | 149.40 | 763 | 740 |  | OK |
| smoke_3 | classic | 2.72 | 2.48 | 0.32 | 0.19 | 1.42 | 0.87 | 1.32 | 0.32 |  | 0.15 | 0.09 | 0.38 | 0.12 | 2.28 | 138.40 | 1,291 | 740 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 6.16 | 4.79 | 0.26 | 0.89 | 2.12 | 1.78 | 1.27 | 0.32 |  | 0.15 | 0.09 | 0.36 | 0.10 | 2.28 | 130.50 | 763 | 740 |  | OK |
