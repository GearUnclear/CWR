# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| lowsun_fp | on | 30.39 | 25.94 | 0.17 | 3.90 | 11.83 | 10.22 | 0.10 |  |  | 4.21 | 0.14 |  | 0.14 | 0.07 | 0.26 | 3.38 | 16.77 | 30.30 | 1,212 | 1,316 | OK:30 | OK |
| noon_fields | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_fields | on | 35.84 | 33.41 | 0.11 | 4.39 | 25.58 | 3.44 | 0.10 |  |  | 2.20 | 0.14 |  | 0.12 | 0.07 |  | 1.64 | 23.18 | 25.30 | 1,566 | 4,508 | OK:30 | OK |
| forest_edge_golden | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | on | 19.41 | 17.10 | 1.50 | 3.59 | 8.78 | 4.73 | 0.11 |  |  | 2.02 | 0.15 |  | 0.13 | 0.07 |  | 1.37 | 15.70 | 40.20 | 866 | 1,459 | OK:30 | OK |
| sunrise_ridge | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sunrise_ridge | on | 9.86 | 7.32 | 0.08 | 2.32 | 4.13 | 0.86 | 0.11 |  |  | 2.28 | 0.15 |  | 0.13 | 0.07 | 0.20 | 1.48 | 5.28 | 41.40 | 1,912 | 8,897 | OK:30 | OK |
| coastline_sunset | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| coastline_sunset | on | 6.30 | 4.02 | 0.00 | 2.49 | 0.00 | 1.52 | 0.09 |  |  | 2.08 | 0.14 |  | 0.10 | 0.07 | 0.21 | 1.31 | 2.74 | 131.60 | 177 | 0 | OK:30 | OK |
| town_night | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night | on | 15.10 | 13.35 |  | 1.46 | 4.21 | 7.67 | 0.07 |  |  | 1.49 | 0.15 |  | 0.16 | 0.07 |  | 0.86 | 7.74 | 60.30 | 1,235 | 1,134 | UNSETTLED:300 | OK |
| overcast_broken | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| overcast_broken | on | 20.18 | 18.08 | 0.88 | 4.74 | 10.89 | 2.45 | 0.09 |  |  | 1.87 | 0.22 |  | 0.12 | 0.07 |  | 1.21 | 8.06 | 39.40 | 2,214 | 5,692 | OK:30 | OK |
| sinai_desert_noon | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | on | 21.34 | 18.97 | 0.10 | 2.89 | 8.73 | 7.35 | 0.11 |  |  | 2.11 | 0.14 |  | 0.13 | 0.07 |  | 1.53 | 9.08 | 38.80 | 1,266 | 1,363 | OK:30 | OK |
