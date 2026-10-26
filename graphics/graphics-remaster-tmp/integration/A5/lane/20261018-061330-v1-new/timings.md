# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | resolve | lindepth | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| temporal_forest_edge_golden | off | 4.61 | 3.95 | 0.21 | 0.50 | 0.26 |  | 0.13 | 0.07 |  | 0.04 | 134.00 | 2,142 | 1,459 | OK:30 | OK |
| temporal_forest_edge_golden | on | 6.20 | 5.22 | 0.26 | 0.79 | 0.25 |  | 0.14 | 0.07 |  | 0.03 | 117.80 | 2,142 | 1,459 | OK:30 | OK |
| temporal_forest_edge_golden | delta | +1.59 | +1.27 | +0.05 | +0.29 | -0.01 |  | +0.01 | +0.00 |  | -0.01 |  | +0.0% | +0.0% |  |  |
| a5_lowsun_fp | off | 5.20 | 4.01 | 0.08 | 1.03 | 0.28 |  | 0.16 | 0.07 | 0.44 | 0.07 | 136.50 | 1,890 | 1,316 | OK:30 | OK |
| a5_lowsun_fp | on | 6.25 | 4.91 | 0.19 | 1.17 | 0.27 |  | 0.15 | 0.07 | 0.39 | 0.05 | 117.00 | 1,890 | 1,316 | OK:30 | OK |
| a5_lowsun_fp | delta | +1.05 | +0.90 | +0.11 | +0.14 | -0.01 |  | -0.01 | +0.00 | -0.05 | -0.02 |  | +0.0% | +0.0% |  |  |
| a5_town_night | off | 8.58 | 7.71 |  | 0.66 | 0.40 |  | 0.15 | 0.06 |  | 0.04 | 108.40 | 1,247 | 1,083 | UNSETTLED:300 | OK |
| a5_town_night | on | 7.01 | 5.83 |  | 0.97 | 0.37 |  | 0.16 | 0.07 |  | 0.03 | 110.50 | 1,272 | 1,113 | OK:78 | OK |
| a5_town_night | delta | -1.57 | -1.88 |  | +0.31 | -0.03 |  | +0.01 | +0.01 |  | -0.01 |  | +2.0% | +2.7% |  |  |
