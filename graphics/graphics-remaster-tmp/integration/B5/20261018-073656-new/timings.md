# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| water_coastline_sunset | classic | 1.61 | 1.37 |  | 0.75 | 0.00 | 0.62 |  |  |  |  |  |  |  |  |  | 0.85 | 334.10 | 176 | 0 | OK:30 | OK |
| water_coastline_sunset | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| water_sea_noon | classic | 1.59 | 1.41 |  | 0.92 | 0.00 | 0.49 |  |  |  |  |  |  |  |  |  | 0.93 | 324.80 | 179 | 0 | OK:30 | OK |
| water_sea_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| water_shore_midsun | classic | 1.59 | 1.50 |  | 1.03 | 0.00 | 0.46 |  |  |  |  |  |  |  |  |  | 0.93 | 312.70 | 182 | 0 | OK:30 | OK |
| water_shore_midsun | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| water_night | classic | 2.24 | 2.02 |  | 1.01 | 0.00 | 1.01 |  |  |  |  |  |  |  |  |  | 0.96 | 306.50 | 129 | 0 | OK:30 | OK |
| water_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
