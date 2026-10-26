# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 8.78 | 8.65 |  | 0.11 | 2.99 | 5.55 |  |  |  |  |  |  |  |  |  |  | 4.41 | 85.30 | 2,091 | 1,316 | OK:30 | OK |
| lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_fields | classic | 23.90 | 23.77 |  | 0.11 | 17.32 | 6.34 |  |  |  |  |  |  |  |  |  |  | 9.18 | 34.60 | 6,767 | 4,508 | OK:30 | OK |
| noon_fields | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 6.79 | 6.62 |  | 0.09 | 2.78 | 3.76 |  |  |  |  |  |  |  |  |  |  | 4.60 | 106.60 | 2,502 | 1,459 | OK:30 | OK |
| forest_edge_golden | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sunrise_ridge | classic | 36.81 | 36.66 |  | 0.13 | 30.48 | 6.05 |  |  |  |  |  |  |  |  |  |  | 28.79 | 21.30 | 13,076 | 8,897 | OK:30 | OK |
| sunrise_ridge | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| coastline_sunset | classic | 1.34 | 1.23 |  | 0.22 | 0.00 | 1.01 |  |  |  |  |  |  |  |  |  |  | 0.57 | 342.40 | 177 | 0 | OK:30 | OK |
| coastline_sunset | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night | classic | 9.31 | 9.13 |  | 0.08 | 1.59 | 7.46 |  |  |  |  |  |  |  |  |  |  | 5.09 | 89.20 | 1,310 | 1,122 | OK:223 | OK |
| town_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| woods_pine2_evening | classic | 10.68 | 10.54 |  | 0.12 | 5.87 | 4.55 |  |  |  |  |  |  |  |  |  |  | 6.78 | 67.30 | 3,250 | 1,811 | OK:30 | OK |
| woods_pine2_evening | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| blue_hour_coast | classic | 1.54 | 1.38 |  | 0.22 | 0.00 | 1.15 |  |  |  |  |  |  |  |  |  |  | 0.58 | 343.20 | 182 | 0 | OK:30 | OK |
| blue_hour_coast | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| overcast_broken | classic | 25.29 | 25.16 |  | 0.15 | 19.02 | 5.98 |  |  |  |  |  |  |  |  |  |  | 11.20 | 32.20 | 8,243 | 5,692 | OK:30 | OK |
| overcast_broken | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 10.15 | 10.00 |  | 0.15 | 5.86 | 3.99 |  |  |  |  |  |  |  |  |  |  | 6.81 | 61.10 | 2,688 | 1,363 | OK:30 | OK |
| sinai_desert_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
