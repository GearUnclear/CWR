# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| sunrise_ridge_r1 | classic | 26.24 | 26.09 |  | 7.96 | 15.17 | 2.96 |  |  |  |  |  |  |  |  |  |  | 25.03 | 29.00 | 13,076 | 8,897 | OK:30 | OK |
| sunrise_ridge_r1 | remastered | 9.42 | 7.55 | 0.09 | 5.32 | 1.87 | 0.36 | 0.12 |  |  | 1.57 | 0.43 | 0.01 | 0.13 | 0.06 |  | 0.04 | 5.06 | 61.50 | 1,912 | 8,897 | OK:30 | OK |
| sunrise_ridge_r1 | delta | -16.82 | -18.54 |  | -2.64 | -13.30 | -2.60 |  |  |  |  |  |  |  |  |  |  | -19.97 |  | -85.4% | +0.0% |  |  |
| forest_edge_golden_r1 | classic | 6.87 | 6.72 |  | 0.09 | 4.49 | 2.14 |  |  |  |  |  |  |  |  |  |  | 4.54 | 100.50 | 2,502 | 1,459 | OK:30 | OK |
| forest_edge_golden_r1 | remastered | 8.89 | 6.85 | 0.30 | 2.07 | 3.50 | 1.29 | 0.12 |  |  | 1.71 | 0.41 | 0.01 | 0.13 | 0.23 |  | 0.03 | 5.78 | 88.50 | 866 | 1,459 | OK:30 | OK |
| forest_edge_golden_r1 | delta | +2.02 | +0.13 |  | +1.98 | -0.99 | -0.85 |  |  |  |  |  |  |  |  |  |  | +1.24 |  | -65.4% | +0.0% |  |  |
