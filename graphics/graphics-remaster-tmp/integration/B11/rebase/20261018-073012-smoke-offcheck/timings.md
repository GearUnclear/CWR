# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| b11_sharpen_forest_edge | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| b11_sharpen_forest_edge | on | 5.67 | 4.50 | 0.37 | 1.31 | 2.38 | 0.80 | 0.18 |  | 0.78 | 0.26 |  | 0.23 | 0.13 |  | 0.06 | 5.09 | 117.40 | 811 | 1,447 | OK:30 | OK |
| smoke_1 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 171.80 | 1,291 | 740 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 6.14 | 4.73 | 0.18 | 1.16 | 1.96 | 1.60 | 0.09 |  | 1.24 | 0.31 |  | 0.13 | 0.08 | 0.33 | 0.18 | 2.03 | 149.80 | 763 | 740 |  | OK |
| smoke_3 | classic | 2.31 | 2.11 | 0.18 | 0.17 | 1.24 | 0.70 | 0.09 |  | 1.20 | 0.34 |  | 0.13 | 0.08 | 0.33 | 0.11 | 2.03 | 141.60 | 1,291 | 740 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 5.98 | 4.72 | 0.18 | 1.08 | 2.11 | 1.53 | 0.09 |  | 1.10 | 0.30 |  | 0.13 | 0.07 | 0.33 | 0.07 | 2.03 | 137.70 | 763 | 740 |  | OK |
