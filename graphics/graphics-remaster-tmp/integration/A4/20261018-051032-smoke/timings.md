# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| smoke_1 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 141.80 | 612 | 218 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 4.41 | 2.47 | 0.20 | 0.70 | 0.38 | 1.40 | 1.57 | 0.60 |  | 0.20 | 0.14 | 0.49 | 0.14 | 3.59 | 132.00 | 429 | 218 |  | OK |
| smoke_3 | classic | 4.44 | 3.77 | 0.17 | 0.49 | 0.75 | 2.53 | 1.74 | 0.64 |  | 0.20 | 0.14 | 0.49 | 0.27 | 3.42 | 121.10 | 612 | 218 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 5.16 | 2.92 | 0.16 | 0.80 | 0.41 | 1.71 | 1.87 | 0.69 |  | 0.21 | 0.16 | 0.54 | 0.27 | 3.42 | 124.30 | 429 | 218 |  | OK |
