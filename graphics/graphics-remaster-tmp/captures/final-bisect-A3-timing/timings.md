# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| valley_0515_r1 | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| valley_0515_r1 | on | 26.05 | 24.65 | 0.04 |  |  |  |  |  |  | 1.26 | 0.76 |  | 0.06 | 0.07 | 0.31 | 0.07 |  | 31.10 | 11,406 | 7,966 | OK:30 | OK |
| valley_0515_r2 | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| valley_0515_r2 | on | 22.39 | 21.09 | 0.04 |  |  |  |  |  |  | 1.17 | 0.67 |  | 0.06 | 0.07 | 0.31 | 0.06 |  | 33.20 | 11,406 | 7,966 | OK:30 | OK |
