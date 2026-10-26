# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| water_coastline_sunset | classic | 0.74 | 0.62 |  | 0.18 | 0.00 | 0.44 |  |  |  |  |  |  |  |  |  | 0.48 | 434.30 | 176 | 0 | OK:30 | OK |
| water_coastline_sunset | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| water_sea_noon | classic | 1.03 | 0.94 |  | 0.27 | 0.00 | 0.67 |  |  |  |  |  |  |  |  |  | 0.55 | 396.00 | 179 | 0 | OK:30 | OK |
| water_sea_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| water_shore_midsun | classic | 1.35 | 1.30 |  | 0.32 | 0.00 | 0.97 |  |  |  |  |  |  |  |  |  | 0.56 | 349.40 | 182 | 0 | OK:30 | OK |
| water_shore_midsun | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| water_night | classic | 0.93 | 0.80 |  | 0.25 | 0.00 | 0.55 |  |  |  |  |  |  |  |  |  | 0.56 | 361.30 | 129 | 0 | OK:30 | OK |
| water_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
