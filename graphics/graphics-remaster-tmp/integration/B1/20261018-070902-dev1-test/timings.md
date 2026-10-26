# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| dev_forest | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| dev_forest | on | 4.68 | 3.59 | 0.24 | 0.72 | 1.89 | 0.98 | 0.93 | 0.29 |  | 0.13 | 0.07 |  | 0.03 | 4.10 | 111.70 | 866 | 1,459 | OK:30 | OK |
| dev_woods | off | 6.76 | 5.12 | 0.36 | 0.84 | 3.55 | 0.73 | 1.49 | 0.39 |  | 0.20 | 0.07 | 0.50 | 0.05 | 6.07 | 97.00 | 1,410 | 1,810 | OK:30 | OK |
| dev_woods | on | 7.29 | 5.63 | 0.36 | 0.82 | 4.02 | 0.78 | 1.52 | 0.42 |  | 0.20 | 0.07 | 0.50 | 0.06 | 6.51 | 88.60 | 1,410 | 1,810 | OK:30 | OK |
| dev_woods | delta | +0.53 | +0.51 | +0.00 | -0.02 | +0.47 | +0.05 | +0.03 | +0.03 |  | +0.00 | +0.00 | +0.00 | +0.01 | +0.44 |  | +0.0% | +0.0% |  |  |
