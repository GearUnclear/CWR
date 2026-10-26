# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| valley_0515_r1 | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| valley_0515_r1 | on | 27.28 | 24.51 | 0.05 | 2.36 | 19.77 | 2.38 | 0.09 |  |  | 2.50 | 0.33 | 0.01 | 0.06 | 0.06 |  | 0.04 | 26.29 | 28.60 | 5,242 | 7,966 | OK:30 | OK |
| valley_0515_r2 | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| valley_0515_r2 | on | 26.48 | 23.93 | 0.04 | 2.31 | 19.60 | 2.02 | 0.09 |  |  | 2.28 | 0.16 | 0.01 | 0.06 | 0.07 |  | 0.04 | 25.55 | 29.40 | 5,242 | 7,966 | OK:30 | OK |
