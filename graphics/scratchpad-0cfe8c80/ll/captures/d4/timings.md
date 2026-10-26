# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| town_night | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night | on | 9.89 | 8.62 | 0.16 | 1.12 | 3.25 | 4.25 | 0.08 |  |  | 0.88 | 0.18 | 0.01 | 0.15 | 0.07 |  | 0.12 | 9.51 | 75.70 | 1,138 | 1,026 | UNSETTLED:300 | OK |
| woods_pine2_evening | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| woods_pine2_evening | on | 9.72 | 7.86 | 0.39 | 1.52 | 5.44 | 0.90 | 0.13 |  |  | 1.54 | 0.19 | 0.01 | 0.20 | 0.07 |  | 0.11 | 9.34 | 77.90 | 1,421 | 1,811 | OK:30 | OK |
