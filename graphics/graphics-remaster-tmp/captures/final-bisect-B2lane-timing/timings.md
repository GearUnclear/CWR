# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| valley_0515_r1 | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| valley_0515_r1 | on | 21.24 | 19.62 | 0.04 | 1.52 | 16.01 | 2.09 | 0.08 |  |  | 1.43 | 0.69 |  | 0.06 | 0.07 | 0.32 | 0.04 | 20.72 | 34.20 | 5,242 | 7,966 | OK:30 | OK |
| valley_0515_r2 | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| valley_0515_r2 | on | 20.63 | 19.09 | 0.04 | 1.52 | 15.85 | 1.71 | 0.08 |  |  | 1.35 | 0.61 |  | 0.06 | 0.07 | 0.32 | 0.04 | 20.14 | 34.80 | 5,242 | 7,966 | OK:30 | OK |
