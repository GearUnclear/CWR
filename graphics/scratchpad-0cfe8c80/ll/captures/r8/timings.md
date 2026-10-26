# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 3.64 | 3.51 |  | 0.10 | 1.32 | 2.09 |  |  |  |  |  |  |  |  |  |  | 3.34 | 128.50 | 2,091 | 1,316 | OK:30 | OK |
| lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_fields | classic | 9.33 | 9.21 |  | 0.11 | 7.07 | 2.03 |  |  |  |  |  |  |  |  |  |  | 7.96 | 63.00 | 6,767 | 4,508 | OK:30 | OK |
| noon_fields | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 4.67 | 4.52 |  | 0.09 | 2.44 | 1.98 |  |  |  |  |  |  |  |  |  |  | 4.50 | 109.20 | 2,502 | 1,459 | OK:30 | OK |
| forest_edge_golden | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sunrise_ridge | classic | 21.54 | 21.40 |  | 0.12 | 17.83 | 3.44 |  |  |  |  |  |  |  |  |  |  | 17.42 | 34.10 | 13,076 | 8,897 | OK:30 | OK |
| sunrise_ridge | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| coastline_sunset | classic | 1.23 | 1.12 |  | 0.22 | 0.00 | 0.90 |  |  |  |  |  |  |  |  |  |  | 0.56 | 395.70 | 177 | 0 | OK:30 | OK |
| coastline_sunset | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night | classic | 5.88 | 5.70 |  | 0.08 | 1.43 | 4.20 |  |  |  |  |  |  |  |  |  |  | 4.98 | 116.90 | 1,289 | 1,119 | UNSETTLED:300 | OK |
| town_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| woods_pine2_evening | classic | 9.97 | 9.83 |  | 0.11 | 5.82 | 3.90 |  |  |  |  |  |  |  |  |  |  | 4.96 | 100.10 | 3,250 | 1,811 | OK:30 | OK |
| woods_pine2_evening | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| blue_hour_coast | classic | 1.39 | 1.23 |  | 0.24 | 0.00 | 0.99 |  |  |  |  |  |  |  |  |  |  | 0.55 | 379.30 | 182 | 0 | OK:30 | OK |
| blue_hour_coast | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| overcast_broken | classic | 18.07 | 17.94 |  | 0.15 | 12.66 | 5.13 |  |  |  |  |  |  |  |  |  |  | 10.83 | 37.50 | 8,243 | 5,692 | OK:30 | OK |
| overcast_broken | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 7.14 | 7.00 |  | 0.16 | 3.60 | 3.24 |  |  |  |  |  |  |  |  |  |  | 5.65 | 75.10 | 2,689 | 1,363 | OK:30 | OK |
| sinai_desert_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
