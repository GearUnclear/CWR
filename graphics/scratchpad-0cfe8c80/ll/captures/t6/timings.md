# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| lowsun_fp | on | 8.33 | 6.82 | 0.20 | 1.20 | 3.49 | 2.13 | 0.09 |  |  | 1.28 | 0.16 |  | 0.14 | 0.07 | 0.26 | 0.43 | 6.74 | 84.60 | 1,212 | 1,316 | OK:30 | OK |
| noon_fields | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_fields | on | 9.86 | 8.51 | 0.13 | 1.46 | 6.20 | 0.84 | 0.10 |  |  | 1.14 | 0.26 |  | 0.12 | 0.07 |  | 0.47 | 7.97 | 59.00 | 1,566 | 4,508 | OK:30 | OK |
| forest_edge_golden | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | on | 6.74 | 5.42 | 0.24 | 1.54 | 2.72 | 1.16 | 0.11 |  |  | 1.04 | 0.18 |  | 0.13 | 0.07 |  | 0.37 | 5.36 | 95.20 | 866 | 1,459 | OK:30 | OK |
| sunrise_ridge | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sunrise_ridge | on | 7.26 | 5.32 | 0.11 | 1.59 | 2.83 | 0.90 | 0.11 |  |  | 1.67 | 0.20 |  | 0.13 | 0.07 | 0.20 | 0.84 | 4.74 | 46.10 | 1,913 | 8,897 | OK:30 | OK |
| coastline_sunset | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| coastline_sunset | on | 4.54 | 2.46 | 0.00 | 1.65 | 0.00 | 0.81 | 0.09 |  |  | 1.87 | 0.16 |  | 0.10 | 0.07 | 0.21 | 1.09 | 2.74 | 176.90 | 177 | 0 | OK:30 | OK |
| town_night | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night | on | 13.72 | 12.26 |  | 1.44 | 3.66 | 7.16 | 0.08 |  |  | 1.19 | 0.15 |  | 0.16 | 0.07 |  | 0.57 | 7.41 | 63.80 | 1,213 | 1,105 | OK:261 | OK |
| overcast_broken | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| overcast_broken | on | 15.84 | 13.94 | 0.57 | 3.29 | 8.90 | 1.75 | 0.09 |  |  | 1.67 | 0.21 |  | 0.12 | 0.07 |  | 1.03 | 8.14 | 40.90 | 2,214 | 5,692 | OK:30 | OK |
| sinai_desert_noon | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | on | 7.44 | 5.95 | 0.10 | 0.92 | 2.92 | 2.10 | 0.11 |  |  | 1.25 | 0.14 |  | 0.12 | 0.07 |  | 0.69 | 6.76 | 69.90 | 1,265 | 1,363 | OK:30 | OK |
