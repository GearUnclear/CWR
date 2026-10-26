# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 8.00 | 7.87 |  | 0.11 | 3.46 | 4.31 |  |  |  |  |  |  |  |  |  |  | 3.33 | 90.10 | 2,091 | 1,316 | OK:30 | OK |
| lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_fields | classic | 25.10 | 24.97 |  | 0.11 | 16.95 | 7.92 |  |  |  |  |  |  |  |  |  |  | 16.21 | 30.10 | 6,767 | 4,508 | OK:30 | OK |
| noon_fields | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 10.76 | 10.61 |  | 0.09 | 6.28 | 4.23 |  |  |  |  |  |  |  |  |  |  | 8.01 | 55.50 | 2,502 | 1,459 | OK:30 | OK |
| forest_edge_golden | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sunrise_ridge | classic | 57.27 | 57.12 |  | 0.12 | 48.23 | 8.76 |  |  |  |  |  |  |  |  |  |  | 30.27 | 14.80 | 13,076 | 8,897 | OK:30 | OK |
| sunrise_ridge | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| coastline_sunset | classic | 1.79 | 1.67 |  | 0.22 | 0.00 | 1.45 |  |  |  |  |  |  |  |  |  |  | 0.60 | 260.00 | 177 | 0 | OK:30 | OK |
| coastline_sunset | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night | classic | 16.89 | 16.62 |  | 0.08 | 3.07 | 13.48 |  |  |  |  |  |  |  |  |  |  | 6.09 | 57.20 | 1,314 | 1,132 | OK:179 | OK |
| town_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| woods_pine2_evening | classic | 18.67 | 18.52 |  | 0.11 | 10.13 | 8.27 |  |  |  |  |  |  |  |  |  |  | 5.03 | 55.50 | 3,250 | 1,811 | OK:30 | OK |
| woods_pine2_evening | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| blue_hour_coast | classic | 1.70 | 1.52 |  | 0.23 | 0.00 | 1.29 |  |  |  |  |  |  |  |  |  |  | 1.57 | 277.30 | 182 | 0 | OK:30 | OK |
| blue_hour_coast | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| overcast_broken | classic | 21.54 | 21.41 |  | 0.15 | 16.12 | 5.13 |  |  |  |  |  |  |  |  |  |  | 11.99 | 35.50 | 8,243 | 5,692 | OK:30 | OK |
| overcast_broken | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 14.47 | 14.33 |  | 0.15 | 6.12 | 8.06 |  |  |  |  |  |  |  |  |  |  | 5.99 | 54.60 | 2,688 | 1,363 | OK:30 | OK |
| sinai_desert_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
