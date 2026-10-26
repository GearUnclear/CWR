# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| water_coastline_sunset | classic | 1.86 | 1.67 |  | 0.85 | 0.00 | 0.82 |  |  |  |  |  |  |  |  |  | 0.68 | 291.60 | 176 | 0 | OK:30 | OK |
| water_coastline_sunset | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| water_sea_noon | classic | 1.53 | 1.37 |  | 0.81 | 0.00 | 0.57 |  |  |  |  |  |  |  |  |  | 0.76 | 309.60 | 179 | 0 | OK:30 | OK |
| water_sea_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| water_shore_midsun | classic | 1.55 | 1.47 |  | 0.82 | 0.00 | 0.65 |  |  |  |  |  |  |  |  |  | 0.70 | 325.20 | 182 | 0 | OK:30 | OK |
| water_shore_midsun | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| water_night | classic | 1.48 | 1.27 |  | 0.60 | 0.00 | 0.68 |  |  |  |  |  |  |  |  |  | 0.77 | 294.00 | 129 | 0 | OK:30 | OK |
| water_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
