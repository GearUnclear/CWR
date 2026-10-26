# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | resolve | lindepth | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| temporal_forest_edge_golden | off | 5.41 | 4.69 | 0.26 | 0.54 | 0.25 |  | 0.16 | 0.08 |  | 0.05 | 123.70 | 2,142 | 1,459 | OK:30 | OK |
| temporal_forest_edge_golden | on | 6.11 | 5.10 | 0.26 | 0.81 | 0.25 |  | 0.14 | 0.07 |  | 0.03 | 116.20 | 2,142 | 1,459 | OK:30 | OK |
| temporal_forest_edge_golden | delta | +0.70 | +0.41 | +0.00 | +0.27 | +0.00 |  | -0.02 | -0.01 |  | -0.02 |  | +0.0% | +0.0% |  |  |
| a5_lowsun_fp | off | 5.21 | 4.02 | 0.08 | 1.02 | 0.27 |  | 0.16 | 0.07 | 0.44 | 0.08 | 126.90 | 1,890 | 1,316 | OK:30 | OK |
| a5_lowsun_fp | on | 6.29 | 4.93 | 0.18 | 1.19 | 0.29 |  | 0.14 | 0.07 | 0.38 | 0.08 | 116.70 | 1,890 | 1,316 | OK:30 | OK |
| a5_lowsun_fp | delta | +1.08 | +0.91 | +0.10 | +0.17 | +0.02 |  | -0.02 | +0.00 | -0.06 | +0.00 |  | +0.0% | +0.0% |  |  |
| a5_town_night | off | 6.63 | 5.76 |  | 0.66 | 0.35 |  | 0.18 | 0.07 |  | 0.05 | 122.40 | 1,262 | 1,104 | OK:208 | OK |
| a5_town_night | on | 6.79 | 5.75 |  | 0.83 | 0.31 |  | 0.16 | 0.07 |  | 0.03 | 117.40 | 1,233 | 1,057 | OK:37 | OK |
| a5_town_night | delta | +0.16 | -0.01 |  | +0.17 | -0.04 |  | -0.02 | +0.00 |  | -0.02 |  | -2.3% | -4.3% |  |  |
