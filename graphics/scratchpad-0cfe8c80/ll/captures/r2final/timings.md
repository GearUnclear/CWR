# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 4.55 | 4.27 |  | 0.11 | 2.11 | 2.06 |  |  |  |  |  |  |  |  |  |  | 3.38 | 136.60 | 2,091 | 1,316 | OK:30 | OK |
| lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_fields | classic | 15.01 | 14.89 |  | 0.11 | 11.14 | 3.64 |  |  |  |  |  |  |  |  |  |  | 8.86 | 46.30 | 6,767 | 4,508 | OK:30 | OK |
| noon_fields | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 9.63 | 9.47 |  | 0.09 | 5.74 | 3.64 |  |  |  |  |  |  |  |  |  |  | 4.58 | 95.20 | 2,502 | 1,459 | OK:30 | OK |
| forest_edge_golden | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sunrise_ridge | classic | 53.55 | 53.40 |  | 0.13 | 44.09 | 9.19 |  |  |  |  |  |  |  |  |  |  | 18.25 | 21.10 | 13,076 | 8,897 | OK:30 | OK |
| sunrise_ridge | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| coastline_sunset | classic | 1.56 | 1.45 |  | 0.22 | 0.00 | 1.23 |  |  |  |  |  |  |  |  |  |  | 0.63 | 325.40 | 177 | 0 | OK:30 | OK |
| coastline_sunset | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night | classic | 16.70 | 16.53 |  | 0.08 | 2.28 | 14.17 |  |  |  |  |  |  |  |  |  |  | 7.67 | 58.20 | 1,269 | 1,109 | OK:236 | OK |
| town_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| woods_pine2_evening | classic | 14.45 | 14.31 |  | 0.11 | 7.67 | 6.52 |  |  |  |  |  |  |  |  |  |  | 10.32 | 52.20 | 3,250 | 1,811 | OK:30 | OK |
| woods_pine2_evening | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| blue_hour_coast | classic | 0.99 | 0.79 |  | 0.40 | 0.00 | 0.38 |  |  |  |  |  |  |  |  |  |  | 0.58 | 347.40 | 182 | 0 | OK:30 | OK |
| blue_hour_coast | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| overcast_broken | classic | 18.25 | 18.12 |  | 0.15 | 12.51 | 5.46 |  |  |  |  |  |  |  |  |  |  | 13.37 | 38.00 | 8,243 | 5,692 | OK:30 | OK |
| overcast_broken | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 10.82 | 10.67 |  | 0.15 | 5.76 | 4.76 |  |  |  |  |  |  |  |  |  |  | 6.97 | 60.10 | 2,689 | 1,363 | OK:30 | OK |
| sinai_desert_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
