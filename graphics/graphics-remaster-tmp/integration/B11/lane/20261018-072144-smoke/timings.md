# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| smoke_1 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 167.80 | 1,291 | 740 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 6.24 | 4.83 | 0.21 | 1.16 | 2.05 | 1.62 | 1.32 | 0.43 |  | 0.14 | 0.08 | 0.34 | 0.11 | 2.57 | 140.50 | 763 | 740 |  | OK |
| smoke_3 | classic | 2.59 | 2.36 | 0.25 | 0.18 | 1.37 | 0.81 | 1.32 | 0.45 |  | 0.14 | 0.08 | 0.33 | 0.09 | 2.17 | 124.40 | 1,291 | 740 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 5.96 | 4.53 | 0.19 | 1.16 | 1.85 | 1.51 | 1.34 | 0.45 |  | 0.14 | 0.08 | 0.34 | 0.11 | 2.17 | 116.90 | 763 | 740 |  | OK |
