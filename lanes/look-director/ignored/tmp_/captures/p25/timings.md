# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| firefight_1 | classic | 4.79 | 4.19 |  | 0.32 | 0.60 | 3.27 |  |  |  |  |  |  |  |  |  |  | 1.25 | 161.00 | 453 | 391 | OK:47 | OK |
| firefight_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| firefight_2 | classic | 4.79 | 4.19 |  | 0.32 | 0.60 | 3.27 |  |  |  |  |  |  |  |  |  |  | 1.25 | 161.00 | 530 | 541 | OK:47 | OK |
| firefight_2 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| firefight_3 | classic | 4.79 | 4.19 |  | 0.32 | 0.60 | 3.27 |  |  |  |  |  |  |  |  |  |  | 1.25 | 161.00 | 441 | 395 | OK:47 | OK |
| firefight_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
