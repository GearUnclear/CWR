# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| firefight_1 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| firefight_1 | remastered | 14.92 | 10.68 |  | 1.80 | 4.48 | 4.41 | 0.15 |  |  | 2.71 | 0.19 | 0.01 | 0.16 | 0.16 |  | 0.13 | 4.65 | 88.30 | 399 | 368 | UNSETTLED:300 | OK |
| firefight_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| firefight_2 | remastered | 14.92 | 10.68 |  | 1.80 | 4.48 | 4.41 | 0.15 |  |  | 2.71 | 0.19 | 0.01 | 0.16 | 0.16 |  | 0.13 | 4.65 | 88.30 | 485 | 493 | UNSETTLED:300 | OK |
| firefight_3 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| firefight_3 | remastered | 14.92 | 10.68 |  | 1.80 | 4.48 | 4.41 | 0.15 |  |  | 2.71 | 0.19 | 0.01 | 0.16 | 0.16 |  | 0.13 | 4.65 | 88.30 | 401 | 403 | UNSETTLED:300 | OK |
