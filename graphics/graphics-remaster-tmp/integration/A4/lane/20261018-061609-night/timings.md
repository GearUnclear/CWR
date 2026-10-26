# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| town_night_lamps | off | 3.26 | 2.49 |  | 0.37 | 1.31 | 0.81 | 0.57 | 0.28 |  | 0.17 | 0.07 |  | 0.04 | 2.35 | 163.80 | 777 | 546 | OK:30 | OK |
| town_night_lamps | on | 3.75 | 2.82 |  | 0.33 | 1.28 | 1.21 | 0.75 | 0.41 |  | 0.18 | 0.07 |  | 0.09 | 2.95 | 143.90 | 707 | 546 | OK:30 | OK |
| town_night_lamps | delta | +0.49 | +0.33 |  | -0.04 | -0.03 | +0.40 | +0.18 | +0.13 |  | +0.01 | +0.00 |  | +0.05 | +0.60 |  | -9.0% | +0.0% |  |  |
