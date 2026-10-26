# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| valley_0515_r1 | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| valley_0515_r1 | on | 15.91 | 14.70 | 0.04 | 0.45 | 12.69 | 1.55 |  |  |  | 1.10 | 0.61 |  | 0.06 | 0.07 | 0.31 | 0.06 | 15.34 | 40.30 | 5,242 | 7,966 | OK:30 | OK |
| valley_0515_r2 | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| valley_0515_r2 | on | 15.45 | 14.29 | 0.04 | 0.45 | 12.59 | 1.24 |  |  |  | 1.06 | 0.56 |  | 0.06 | 0.07 | 0.31 | 0.06 | 14.91 | 42.60 | 5,242 | 7,966 | OK:30 | OK |
