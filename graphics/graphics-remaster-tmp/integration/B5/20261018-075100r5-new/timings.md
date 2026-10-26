# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| water_coastline_sunset | classic | 3.63 | 2.48 |  | 1.72 | 0.00 | 0.76 |  |  |  |  |  |  |  |  |  | 0.97 | 270.40 | 176 | 0 | OK:30 | OK |
| water_coastline_sunset | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| water_sea_noon | classic | 1.98 | 1.75 |  | 1.22 | 0.00 | 0.52 |  |  |  |  |  |  |  |  |  | 1.28 | 293.10 | 179 | 0 | OK:30 | OK |
| water_sea_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| water_shore_midsun | classic | 0.97 | 0.91 |  | 0.55 | 0.00 | 0.37 |  |  |  |  |  |  |  |  |  | 0.60 | 317.90 | 182 | 0 | OK:30 | OK |
| water_shore_midsun | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| water_night | classic | 0.85 | 0.69 |  | 0.37 | 0.00 | 0.32 |  |  |  |  |  |  |  |  |  | 0.56 | 412.50 | 129 | 0 | OK:30 | OK |
| water_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
