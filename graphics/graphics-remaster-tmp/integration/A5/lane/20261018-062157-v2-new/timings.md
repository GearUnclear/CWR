# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | resolve | lindepth | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| temporal_forest_edge_golden | off | 5.31 | 4.59 | 0.24 | 0.52 | 0.24 |  | 0.16 | 0.08 |  | 0.05 | 126.70 | 2,142 | 1,459 | OK:30 | OK |
| temporal_forest_edge_golden | on | 6.33 | 5.32 | 0.28 | 0.81 | 0.28 |  | 0.15 | 0.08 |  | 0.04 | 114.30 | 2,142 | 1,459 | OK:30 | OK |
| temporal_forest_edge_golden | delta | +1.02 | +0.73 | +0.04 | +0.29 | +0.04 |  | -0.01 | +0.00 |  | -0.01 |  | +0.0% | +0.0% |  |  |
| a5_lowsun_fp | off | 5.19 | 4.01 | 0.08 | 1.01 | 0.25 |  | 0.16 | 0.07 | 0.44 | 0.08 | 140.50 | 1,890 | 1,316 | OK:30 | OK |
| a5_lowsun_fp | on | 6.27 | 4.95 | 0.17 | 1.15 | 0.28 |  | 0.14 | 0.07 | 0.38 | 0.06 | 118.00 | 1,890 | 1,316 | OK:30 | OK |
| a5_lowsun_fp | delta | +1.08 | +0.94 | +0.09 | +0.14 | +0.03 |  | -0.02 | +0.00 | -0.06 | -0.02 |  | +0.0% | +0.0% |  |  |
| a5_town_night | off | 6.53 | 5.68 |  | 0.64 | 0.34 |  | 0.18 | 0.07 |  | 0.04 | 120.70 | 1,311 | 1,135 | OK:203 | OK |
| a5_town_night | on | 7.16 | 6.06 |  | 0.83 | 0.34 |  | 0.16 | 0.07 |  | 0.03 | 116.20 | 1,301 | 1,127 | OK:35 | OK |
| a5_town_night | delta | +0.63 | +0.38 |  | +0.19 | +0.00 |  | -0.02 | +0.00 |  | -0.01 |  | -0.8% | -0.7% |  |  |
