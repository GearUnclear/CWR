# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| lowsun_fp | on | 21.89 | 18.85 | 0.17 | 2.93 | 8.96 | 6.95 | 0.09 |  |  | 2.82 | 0.14 |  | 0.14 | 0.07 | 0.26 | 1.99 | 7.15 | 43.40 | 1,212 | 1,316 | OK:30 | OK |
| noon_fields | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_fields | on | 19.11 | 17.22 | 0.51 | 2.43 | 12.58 | 2.20 | 0.10 |  |  | 1.67 | 0.14 |  | 0.12 | 0.07 |  | 1.11 | 10.49 | 30.10 | 1,566 | 4,508 | OK:30 | OK |
| forest_edge_golden | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | on | 6.70 | 5.14 | 0.35 | 1.19 | 2.31 | 1.64 | 0.11 |  |  | 1.27 | 0.18 |  | 0.13 | 0.07 |  | 0.60 | 5.43 | 95.80 | 866 | 1,459 | OK:30 | OK |
| sunrise_ridge | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sunrise_ridge | on | 5.72 | 4.25 | 0.08 | 1.82 | 1.94 | 0.49 | 0.11 |  |  | 1.21 | 0.13 |  | 0.13 | 0.06 | 0.21 | 0.43 | 4.66 | 63.00 | 1,912 | 8,897 | OK:30 | OK |
| coastline_sunset | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| coastline_sunset | on | 3.20 | 1.86 | 0.00 | 1.43 | 0.00 | 0.42 | 0.09 |  |  | 1.14 | 0.15 |  | 0.10 | 0.07 | 0.21 | 0.37 | 2.74 | 192.00 | 177 | 0 | OK:30 | OK |
| town_night | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night | on | 8.17 | 6.92 |  | 0.80 | 2.21 | 3.91 | 0.08 |  |  | 0.99 | 0.14 |  | 0.16 | 0.07 |  | 0.38 | 7.69 | 91.80 | 1,206 | 1,104 | OK:202 | OK |
| overcast_broken | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| overcast_broken | on | 9.25 | 7.96 | 0.23 | 2.59 | 4.56 | 0.81 | 0.09 |  |  | 1.06 | 0.15 |  | 0.12 | 0.07 |  | 0.48 | 7.84 | 59.60 | 2,214 | 5,692 | OK:30 | OK |
| sinai_desert_noon | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | on | 7.88 | 6.16 | 0.14 | 0.97 | 2.76 | 2.43 | 0.10 |  |  | 1.47 | 0.18 |  | 0.13 | 0.07 |  | 0.86 | 6.79 | 73.70 | 1,266 | 1,363 | OK:30 | OK |
