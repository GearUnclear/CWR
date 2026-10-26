# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 6.43 | 6.29 |  | 0.10 | 2.96 | 3.23 |  |  |  |  |  |  |  |  |  |  | 3.28 | 111.10 | 2,091 | 1,316 | OK:30 | OK |
| lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_fields | classic | 25.89 | 25.77 |  | 0.11 | 18.45 | 7.21 |  |  |  |  |  |  |  |  |  |  | 12.36 | 36.30 | 6,767 | 4,508 | OK:30 | OK |
| noon_fields | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 16.26 | 16.10 |  | 0.09 | 8.59 | 7.42 |  |  |  |  |  |  |  |  |  |  | 12.97 | 51.70 | 2,502 | 1,459 | OK:30 | OK |
| forest_edge_golden | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sunrise_ridge | classic | 59.85 | 59.69 |  | 0.12 | 50.01 | 9.55 |  |  |  |  |  |  |  |  |  |  | 35.44 | 14.20 | 13,076 | 8,897 | OK:30 | OK |
| sunrise_ridge | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| coastline_sunset | classic | 2.30 | 2.18 |  | 0.22 | 0.00 | 1.96 |  |  |  |  |  |  |  |  |  |  | 0.57 | 285.20 | 177 | 0 | OK:30 | OK |
| coastline_sunset | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night | classic | 15.69 | 15.50 |  | 0.08 | 2.28 | 13.15 |  |  |  |  |  |  |  |  |  |  | 10.12 | 57.70 | 1,296 | 1,121 | OK:129 | OK |
| town_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| woods_pine2_evening | classic | 15.82 | 15.67 |  | 0.12 | 8.40 | 7.16 |  |  |  |  |  |  |  |  |  |  | 10.53 | 49.90 | 3,250 | 1,811 | OK:30 | OK |
| woods_pine2_evening | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| blue_hour_coast | classic | 1.69 | 1.53 |  | 0.24 | 0.00 | 1.29 |  |  |  |  |  |  |  |  |  |  | 0.59 | 309.80 | 182 | 0 | OK:30 | OK |
| blue_hour_coast | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| overcast_broken | classic | 38.36 | 38.23 |  | 0.15 | 29.22 | 8.86 |  |  |  |  |  |  |  |  |  |  | 24.40 | 22.30 | 8,243 | 5,692 | OK:30 | OK |
| overcast_broken | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 14.58 | 14.43 |  | 0.15 | 6.70 | 7.58 |  |  |  |  |  |  |  |  |  |  | 10.55 | 42.60 | 2,688 | 1,363 | OK:30 | OK |
| sinai_desert_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
