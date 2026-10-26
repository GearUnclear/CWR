# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| forest_edge_golden | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | remastered | 5.43 | 4.75 | 0.28 | 0.50 | 0.38 | 0.08 |  | 0.05 | 116.70 | 1,847 | 1,316 | OK:30 | OK |
| noon_fields | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_fields | remastered | 7.80 | 7.29 | 0.07 | 0.41 | 0.34 | 0.05 |  | 0.03 | 79.90 | 5,575 | 3,920 | OK:30 | OK |
| town_day | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |
| town_day | remastered | 4.62 | 2.97 | 0.44 | 1.30 | 1.03 | 0.14 |  | 0.13 | 158.10 | 697 | 472 | OK:30 | OK |
