# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| smoke_1 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 177.90 | 1,291 | 740 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 4.72 | 3.57 | 0.21 | 0.35 | 1.41 | 1.81 | 1.05 | 0.27 |  | 0.16 | 0.10 | 0.44 | 0.09 | 2.00 | 158.10 | 763 | 740 |  | OK |
| smoke_3 | classic | 3.86 | 3.44 | 0.15 | 0.53 | 1.92 | 0.99 | 1.04 | 0.24 |  | 0.16 | 0.10 | 0.45 | 0.08 | 2.00 | 140.80 | 1,291 | 740 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 6.13 | 4.28 | 0.16 | 0.96 | 1.57 | 1.75 | 1.38 | 0.57 |  | 0.17 | 0.14 | 0.40 | 0.12 | 2.00 | 130.30 | 763 | 740 |  | OK |
