# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| woods_pine2_evening | off | 9.37 | 7.32 | 0.36 | 1.34 | 5.15 | 0.83 | 0.10 |  |  | 1.78 | 0.55 | 0.01 | 0.20 | 0.06 |  | 0.04 | 8.91 | 68.90 | 1,421 | 1,811 | OK:42 | OK |
| woods_pine2_evening | on | 9.37 | 7.52 | 0.37 | 1.38 | 5.27 | 0.87 | 0.13 |  |  | 1.53 | 0.27 | 0.01 | 0.20 | 0.06 |  | 0.08 | 9.10 | 71.30 | 1,421 | 1,811 | OK:40 | OK |
| woods_pine2_evening | delta | +0.00 | +0.20 | +0.01 | +0.04 | +0.12 | +0.04 | +0.03 |  |  | -0.25 | -0.28 | +0.00 | +0.00 | +0.00 |  | +0.04 | +0.19 |  | +0.0% | +0.0% |  |  |
