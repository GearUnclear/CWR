# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| smoke_1 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 153.00 | 612 | 218 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 4.53 | 3.09 | 0.09 | 0.77 | 1.07 | 1.25 | 0.07 |  |  | 1.30 | 0.40 | 0.01 | 0.09 | 0.05 |  | 0.03 | 1.54 | 137.00 | 433 | 218 |  | OK |
| smoke_3 | classic | 1.95 | 1.76 | 0.09 | 0.11 | 0.47 | 1.19 | 0.07 |  |  | 1.32 | 0.48 | 0.01 | 0.09 | 0.05 |  | 0.03 | 1.42 | 127.60 | 612 | 218 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 4.27 | 2.92 | 0.10 | 0.84 | 1.08 | 1.00 | 0.07 |  |  | 1.20 | 0.42 | 0.01 | 0.09 | 0.05 |  | 0.02 | 1.42 | 132.50 | 433 | 218 |  | OK |
