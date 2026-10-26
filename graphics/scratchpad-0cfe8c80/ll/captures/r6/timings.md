# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 3.76 | 3.44 |  | 0.10 | 1.37 | 1.97 |  |  |  |  |  |  |  |  |  |  | 3.34 | 149.10 | 2,091 | 1,316 | OK:30 | OK |
| lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_fields | classic | 11.31 | 11.18 |  | 0.11 | 8.81 | 2.27 |  |  |  |  |  |  |  |  |  |  | 8.63 | 60.40 | 6,767 | 4,508 | OK:30 | OK |
| noon_fields | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 7.49 | 7.33 |  | 0.09 | 3.73 | 3.51 |  |  |  |  |  |  |  |  |  |  | 4.55 | 96.30 | 2,502 | 1,459 | OK:30 | OK |
| forest_edge_golden | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sunrise_ridge | classic | 18.59 | 18.44 |  | 0.12 | 15.38 | 2.94 |  |  |  |  |  |  |  |  |  |  | 18.00 | 34.70 | 13,076 | 8,897 | OK:30 | OK |
| sunrise_ridge | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| coastline_sunset | classic | 1.27 | 1.04 |  | 0.58 | 0.00 | 0.46 |  |  |  |  |  |  |  |  |  |  | 0.91 | 351.70 | 177 | 0 | OK:30 | OK |
| coastline_sunset | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night | classic | 6.55 | 6.32 |  | 0.11 | 1.35 | 4.86 |  |  |  |  |  |  |  |  |  |  | 5.99 | 115.40 | 1,253 | 1,080 | OK:238 | OK |
| town_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| woods_pine2_evening | classic | 6.45 | 6.29 |  | 0.13 | 3.49 | 2.68 |  |  |  |  |  |  |  |  |  |  | 5.09 | 96.50 | 3,250 | 1,811 | OK:30 | OK |
| woods_pine2_evening | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| blue_hour_coast | classic | 1.51 | 1.33 |  | 0.42 | 0.00 | 0.92 |  |  |  |  |  |  |  |  |  |  | 0.61 | 256.50 | 182 | 0 | OK:30 | OK |
| blue_hour_coast | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| overcast_broken | classic | 17.92 | 17.79 |  | 0.15 | 13.73 | 3.90 |  |  |  |  |  |  |  |  |  |  | 14.09 | 34.10 | 8,243 | 5,692 | OK:30 | OK |
| overcast_broken | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 12.74 | 12.59 |  | 0.15 | 5.51 | 6.93 |  |  |  |  |  |  |  |  |  |  | 7.02 | 48.10 | 2,689 | 1,363 | OK:30 | OK |
| sinai_desert_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
