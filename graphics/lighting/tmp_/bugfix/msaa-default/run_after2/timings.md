# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| forest_edge_golden | classic | 3.56 | 3.40 |  |  |  |  |  |  | 225.00 | 1,850 | 1,316 | OK:30 | OK |
| forest_edge_golden | remastered | 5.42 | 4.83 | 0.06 | 0.42 | 0.32 | 0.07 |  | 0.03 | 112.70 | 1,847 | 1,316 | OK:30 | OK |
| forest_edge_golden | delta | +1.86 | +1.43 |  |  |  |  |  |  |  | -0.2% | +0.0% |  |  |
| sinai_lowsun_fp | classic | 5.72 | 5.23 |  |  |  |  |  |  | 117.40 | 586 | 314 | OK:30 | OK |
| sinai_lowsun_fp | remastered | 4.72 | 3.17 | 0.26 | 1.25 | 0.63 | 0.16 | 0.37 | 0.09 | 138.50 | 583 | 314 | OK:30 | OK |
| sinai_lowsun_fp | delta | -1.00 | -2.06 |  |  |  |  |  |  |  | -0.5% | +0.0% |  |  |
