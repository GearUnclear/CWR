# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | resolve | lindepth | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| temporal_forest_edge_golden | off | 6.06 | 5.29 | 0.27 | 0.58 | 0.26 |  | 0.18 | 0.09 |  | 0.06 | 114.20 | 2,142 | 1,459 | OK:30 | OK |
| temporal_forest_edge_golden | on | 5.99 | 5.21 | 0.34 | 0.57 | 0.24 |  | 0.18 | 0.09 |  | 0.06 | 115.40 | 2,142 | 1,459 | OK:30 | OK |
| temporal_forest_edge_golden | delta | -0.07 | -0.08 | +0.07 | -0.01 | -0.02 |  | +0.00 | +0.00 |  | +0.00 |  | +0.0% | +0.0% |  |  |
| a5_lowsun_fp | off | 6.02 | 4.72 | 0.09 | 1.13 | 0.27 |  | 0.19 | 0.09 | 0.50 | 0.08 | 123.40 | 1,890 | 1,316 | OK:30 | OK |
| a5_lowsun_fp | on | 5.96 | 4.59 | 0.09 | 1.19 | 0.27 |  | 0.19 | 0.09 | 0.50 | 0.14 | 122.20 | 1,890 | 1,316 | OK:30 | OK |
| a5_lowsun_fp | delta | -0.06 | -0.13 | +0.00 | +0.06 | +0.00 |  | +0.00 | +0.00 | +0.00 | +0.06 |  | +0.0% | +0.0% |  |  |
| a5_town_night | off | 6.46 | 5.60 |  | 0.65 | 0.34 |  | 0.18 | 0.07 |  | 0.05 | 121.70 | 1,277 | 1,101 | OK:274 | OK |
| a5_town_night | on | 6.01 | 5.18 |  | 0.62 | 0.30 |  | 0.19 | 0.08 |  | 0.06 | 127.00 | 1,208 | 1,033 | OK:30 | OK |
| a5_town_night | delta | -0.45 | -0.42 |  | -0.03 | -0.04 |  | +0.01 | +0.01 |  | +0.01 |  | -5.4% | -6.2% |  |  |
