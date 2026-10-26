# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| town_night_lamps | off | 3.73 | 2.93 |  |  |  |  | 0.59 | 0.30 |  | 0.17 | 0.07 |  | 0.04 |  | 159.10 | 777 | 546 | OK:30 | OK |
| town_night_lamps | on | 2.95 | 2.14 |  |  |  |  | 0.60 | 0.30 |  | 0.18 | 0.07 |  | 0.04 |  | 178.30 | 777 | 546 | OK:30 | OK |
| town_night_lamps | delta | -0.78 | -0.79 |  |  |  |  | +0.01 | +0.00 |  | +0.01 | +0.00 |  | +0.00 |  |  | +0.0% | +0.0% |  |  |
