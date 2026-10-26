# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| forest_edge_golden | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | remastered | 5.82 | 5.29 | 0.03 | 0.38 | 0.31 | 0.05 |  | 0.03 | 105.00 | 1,847 | 1,316 | OK:30 | OK |
| noon_fields | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_fields | remastered | 7.81 | 7.31 | 0.04 | 0.39 | 0.32 | 0.05 |  | 0.03 | 54.30 | 5,575 | 3,920 | OK:30 | OK |
| town_day | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |
| town_day | remastered | 4.40 | 2.95 | 0.09 | 1.09 | 0.82 | 0.15 |  | 0.12 | 191.50 | 697 | 472 | OK:30 | OK |
