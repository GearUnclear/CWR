# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 4.66 | 4.28 |  | 0.15 | 1.80 | 2.33 |  |  |  |  |  |  |  |  |  |  | 3.68 | 140.30 | 2,091 | 1,316 | OK:30 | OK |
| lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_fields | classic | 8.17 | 8.05 |  | 0.11 | 5.98 | 1.96 |  |  |  |  |  |  |  |  |  |  | 7.93 | 71.30 | 6,767 | 4,508 | OK:30 | OK |
| noon_fields | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 5.73 | 5.54 |  | 0.11 | 3.06 | 2.37 |  |  |  |  |  |  |  |  |  |  | 5.24 | 115.10 | 2,502 | 1,459 | OK:30 | OK |
| forest_edge_golden | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sunrise_ridge | classic | 17.73 | 17.58 |  | 0.12 | 14.66 | 2.80 |  |  |  |  |  |  |  |  |  |  | 17.38 | 36.50 | 13,076 | 8,897 | OK:30 | OK |
| sunrise_ridge | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| coastline_sunset | classic | 1.48 | 1.23 |  | 0.71 | 0.00 | 0.52 |  |  |  |  |  |  |  |  |  |  | 0.97 | 351.40 | 177 | 0 | OK:30 | OK |
| coastline_sunset | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night | classic | 6.84 | 6.66 |  | 0.08 | 1.49 | 5.09 |  |  |  |  |  |  |  |  |  |  | 4.65 | 123.80 | 1,301 | 1,127 | OK:233 | OK |
| town_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| woods_pine2_evening | classic | 17.73 | 17.59 |  | 0.12 | 9.97 | 7.50 |  |  |  |  |  |  |  |  |  |  | 5.15 | 47.80 | 3,250 | 1,811 | OK:30 | OK |
| woods_pine2_evening | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| blue_hour_coast | classic | 1.63 | 1.47 |  | 0.22 | 0.00 | 1.25 |  |  |  |  |  |  |  |  |  |  | 0.58 | 273.40 | 182 | 0 | OK:30 | OK |
| blue_hour_coast | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| overcast_broken | classic | 36.63 | 36.50 |  | 0.15 | 27.81 | 8.54 |  |  |  |  |  |  |  |  |  |  | 10.92 | 24.10 | 8,243 | 5,692 | OK:30 | OK |
| overcast_broken | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 12.59 | 12.45 |  | 0.15 | 5.57 | 6.73 |  |  |  |  |  |  |  |  |  |  | 7.80 | 58.10 | 2,688 | 1,363 | OK:30 | OK |
| sinai_desert_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
