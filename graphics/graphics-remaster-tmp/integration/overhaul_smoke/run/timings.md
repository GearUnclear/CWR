# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| smoke_1 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 63.40 | 612 | 218 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 12.09 | 9.26 | 0.09 | 1.90 | 2.77 | 4.59 | 0.13 |  |  | 2.55 | 0.14 | 0.01 | 0.19 | 0.06 |  | 0.37 | 5.09 | 59.70 | 432 | 218 |  | OK |
| smoke_3 | classic | 13.34 | 13.02 | 0.09 | 0.20 | 1.72 | 11.10 | 0.13 |  |  | 3.11 | 0.13 | 0.01 | 0.19 | 0.06 |  | 0.45 | 5.09 | 57.20 | 612 | 218 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 18.35 | 13.48 | 0.09 | 2.33 | 4.86 | 6.29 | 0.13 |  |  | 3.58 | 0.12 | 0.01 | 0.19 | 0.06 |  | 0.50 | 1.90 | 55.20 | 432 | 218 |  | OK |
