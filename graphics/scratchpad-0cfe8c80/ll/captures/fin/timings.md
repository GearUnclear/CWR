# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 9.40 | 9.27 |  | 0.11 | 3.57 | 5.59 |  |  |  |  |  |  |  |  |  |  | 7.72 | 67.80 | 2,091 | 1,316 | OK:30 | OK |
| lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_fields | classic | 30.41 | 30.28 |  | 0.11 | 22.78 | 7.39 |  |  |  |  |  |  |  |  |  |  | 29.10 | 26.70 | 6,767 | 4,508 | OK:30 | OK |
| noon_fields | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 16.53 | 16.37 |  | 0.09 | 8.96 | 7.32 |  |  |  |  |  |  |  |  |  |  | 15.25 | 50.50 | 2,502 | 1,459 | OK:30 | OK |
| forest_edge_golden | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sunrise_ridge | classic | 62.98 | 62.83 |  | 0.12 | 53.00 | 9.71 |  |  |  |  |  |  |  |  |  |  | 34.39 | 14.10 | 13,076 | 8,897 | OK:30 | OK |
| sunrise_ridge | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| coastline_sunset | classic | 1.78 | 1.66 |  | 0.22 | 0.00 | 1.44 |  |  |  |  |  |  |  |  |  |  | 0.55 | 324.20 | 177 | 0 | OK:30 | OK |
| coastline_sunset | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night | classic | 17.02 | 16.83 |  | 0.08 | 2.40 | 14.35 |  |  |  |  |  |  |  |  |  |  | 5.37 | 56.10 | 1,293 | 1,127 | OK:111 | OK |
| town_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| woods_pine2_evening | classic | 16.23 | 16.09 |  | 0.11 | 9.07 | 6.91 |  |  |  |  |  |  |  |  |  |  | 5.10 | 60.70 | 3,250 | 1,811 | OK:30 | OK |
| woods_pine2_evening | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| blue_hour_coast | classic | 1.40 | 1.23 |  | 0.27 | 0.00 | 0.96 |  |  |  |  |  |  |  |  |  |  | 0.60 | 299.90 | 182 | 0 | OK:30 | OK |
| blue_hour_coast | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| overcast_broken | classic | 21.04 | 20.91 |  | 0.15 | 15.35 | 5.41 |  |  |  |  |  |  |  |  |  |  | 11.17 | 37.40 | 8,243 | 5,692 | OK:30 | OK |
| overcast_broken | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 9.75 | 9.61 |  | 0.15 | 4.33 | 5.12 |  |  |  |  |  |  |  |  |  |  | 5.95 | 57.00 | 2,689 | 1,363 | OK:30 | OK |
| sinai_desert_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
