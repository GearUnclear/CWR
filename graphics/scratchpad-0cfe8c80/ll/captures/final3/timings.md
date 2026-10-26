# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 4.72 | 4.58 |  | 0.10 | 1.58 | 2.90 |  |  |  |  |  |  |  |  |  |  | 3.32 | 137.10 | 2,091 | 1,316 | OK:30 | OK |
| lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_fields | classic | 8.86 | 8.74 |  | 0.11 | 6.22 | 2.41 |  |  |  |  |  |  |  |  |  |  | 8.69 | 66.90 | 6,767 | 4,508 | OK:30 | OK |
| noon_fields | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 7.17 | 7.01 |  | 0.09 | 3.19 | 3.73 |  |  |  |  |  |  |  |  |  |  | 4.66 | 79.20 | 2,502 | 1,459 | OK:30 | OK |
| forest_edge_golden | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sunrise_ridge | classic | 51.18 | 51.03 |  | 0.12 | 42.39 | 8.51 |  |  |  |  |  |  |  |  |  |  | 28.76 | 14.00 | 13,076 | 8,897 | OK:30 | OK |
| sunrise_ridge | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| coastline_sunset | classic | 2.31 | 2.19 |  | 0.22 | 0.00 | 1.97 |  |  |  |  |  |  |  |  |  |  | 0.56 | 284.70 | 177 | 0 | OK:30 | OK |
| coastline_sunset | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night | classic | 13.91 | 13.72 |  | 0.08 | 2.12 | 11.52 |  |  |  |  |  |  |  |  |  |  | 5.65 | 62.60 | 1,266 | 1,099 | OK:208 | OK |
| town_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| woods_pine2_evening | classic | 16.14 | 16.00 |  | 0.11 | 8.97 | 6.91 |  |  |  |  |  |  |  |  |  |  | 5.18 | 52.60 | 3,250 | 1,811 | OK:30 | OK |
| woods_pine2_evening | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| blue_hour_coast | classic | 1.08 | 0.91 |  | 0.22 | 0.00 | 0.69 |  |  |  |  |  |  |  |  |  |  | 0.59 | 293.70 | 182 | 0 | OK:30 | OK |
| blue_hour_coast | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| overcast_broken | classic | 19.85 | 19.72 |  | 0.15 | 14.11 | 5.46 |  |  |  |  |  |  |  |  |  |  | 12.84 | 31.40 | 8,243 | 5,692 | OK:30 | OK |
| overcast_broken | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 15.76 | 15.61 |  | 0.15 | 6.87 | 8.60 |  |  |  |  |  |  |  |  |  |  | 7.89 | 39.80 | 2,689 | 1,363 | OK:30 | OK |
| sinai_desert_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
