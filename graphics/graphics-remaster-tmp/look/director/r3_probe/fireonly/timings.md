# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| firefight_1 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| firefight_1 | remastered | 12.98 | 9.52 |  | 1.72 | 4.03 | 3.76 | 0.15 |  |  | 2.18 | 0.19 | 0.01 | 0.16 | 0.10 |  | 0.19 | 4.16 | 106.00 | 424 | 379 | UNSETTLED:300 | OK |
| firefight_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| firefight_2 | remastered | 12.98 | 9.52 |  | 1.72 | 4.03 | 3.76 | 0.15 |  |  | 2.18 | 0.19 | 0.01 | 0.16 | 0.10 |  | 0.19 | 4.16 | 106.00 | 461 | 479 | UNSETTLED:300 | OK |
| firefight_3 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| firefight_3 | remastered | 12.98 | 9.52 |  | 1.72 | 4.03 | 3.76 | 0.15 |  |  | 2.18 | 0.19 | 0.01 | 0.16 | 0.10 |  | 0.19 | 4.16 | 106.00 | 503 | 577 | UNSETTLED:300 | OK |
