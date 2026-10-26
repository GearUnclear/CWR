# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| noon_fields | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_fields | on | 8.23 | 7.12 | 0.11 | 1.28 | 5.14 | 0.70 | 0.10 |  |  | 0.90 | 0.14 |  | 0.12 | 0.07 |  | 0.35 | 7.92 | 69.60 | 1,566 | 4,508 | OK:30 | OK |
| sunrise_ridge | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sunrise_ridge | on | 5.04 | 3.59 | 0.09 | 1.37 | 1.73 | 0.49 | 0.11 |  |  | 1.19 | 0.13 |  | 0.13 | 0.06 | 0.20 | 0.42 | 4.67 | 61.70 | 1,912 | 8,897 | OK:30 | OK |
| coastline_sunset | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| coastline_sunset | on | 4.88 | 2.65 | 0.00 | 2.14 | 0.00 | 0.51 | 0.16 |  |  | 1.91 | 0.27 |  | 0.16 | 0.11 | 0.39 | 0.50 | 2.72 | 190.30 | 177 | 0 | OK:30 | OK |
| overcast_broken | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| overcast_broken | on | 13.43 | 11.87 | 0.31 | 3.22 | 7.30 | 1.36 | 0.09 |  |  | 1.32 | 0.15 |  | 0.12 | 0.07 |  | 0.74 | 8.25 | 42.20 | 2,214 | 5,692 | OK:30 | OK |
| sinai_desert_noon | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | on | 19.01 | 16.79 | 0.10 | 2.54 | 7.54 | 6.71 | 0.11 |  |  | 1.97 | 0.15 |  | 0.13 | 0.07 |  | 1.38 | 7.24 | 39.00 | 1,266 | 1,363 | OK:30 | OK |
