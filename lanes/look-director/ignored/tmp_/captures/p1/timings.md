# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| rain_day | classic | 5.83 | 5.69 |  | 0.09 | 3.33 | 2.28 |  |  |  |  |  |  |  |  |  |  | 2.79 | 97.70 | 1,751 | 783 | OK:30 | OK |
| rain_day | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| firefight_1 | classic | 2.57 | 2.44 |  | 0.06 | 0.27 | 2.11 |  |  |  |  |  |  |  |  |  |  | 0.84 | 236.70 | 470 | 409 | OK:41 | OK |
| firefight_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| firefight_2 | classic | 2.57 | 2.44 |  | 0.06 | 0.27 | 2.11 |  |  |  |  |  |  |  |  |  |  | 0.84 | 236.70 | 555 | 527 | OK:41 | OK |
| firefight_2 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| firefight_3 | classic | 2.57 | 2.44 |  | 0.06 | 0.27 | 2.11 |  |  |  |  |  |  |  |  |  |  | 0.84 | 236.70 | 491 | 475 | OK:41 | OK |
| firefight_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
