# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| dev_forest | off | 6.13 | 4.80 | 0.39 | 1.04 | 2.77 | 0.99 | 1.13 | 0.42 |  | 0.20 | 0.10 |  | 0.05 | 5.21 | 93.70 | 866 | 1,459 | OK:30 | OK |
| dev_forest | on | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| dev_woods | off | 6.72 | 5.02 | 0.36 | 0.80 | 3.50 | 0.72 | 1.56 | 0.44 |  | 0.20 | 0.07 | 0.49 | 0.08 | 6.14 | 93.20 | 1,410 | 1,810 | OK:30 | OK |
| dev_woods | on | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
