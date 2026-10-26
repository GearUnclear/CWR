# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| valley_0515_r1 | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| valley_0515_r1 | on | 26.68 | 23.84 | 0.04 | 2.27 | 19.17 | 2.41 | 0.09 |  |  | 2.57 | 0.16 | 0.01 | 0.06 | 0.06 |  | 0.04 | 24.92 | 29.40 | 5,242 | 7,966 | OK:30 | OK |
| valley_0515_r2 | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| valley_0515_r2 | on | 26.08 | 23.45 | 0.04 | 2.31 | 19.14 | 2.00 | 0.09 |  |  | 2.36 | 0.18 | 0.01 | 0.06 | 0.08 |  | 0.04 | 25.08 | 27.20 | 5,242 | 7,966 | OK:30 | OK |
