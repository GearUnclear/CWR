# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 6.84 | 6.71 |  | 0.11 | 2.13 | 4.47 |  |  |  |  |  |  |  |  |  |  | 3.35 | 130.50 | 2,091 | 1,316 | OK:30 | OK |
| lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_fields | classic | 8.66 | 8.53 |  | 0.11 | 6.32 | 2.11 |  |  |  |  |  |  |  |  |  |  | 8.33 | 67.30 | 6,767 | 4,508 | OK:30 | OK |
| noon_fields | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 4.73 | 4.57 |  | 0.09 | 2.48 | 2.01 |  |  |  |  |  |  |  |  |  |  | 4.53 | 119.10 | 2,502 | 1,459 | OK:30 | OK |
| forest_edge_golden | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sunrise_ridge | classic | 25.11 | 24.95 |  | 0.12 | 21.74 | 3.09 |  |  |  |  |  |  |  |  |  |  | 21.45 | 28.50 | 13,076 | 8,897 | OK:30 | OK |
| sunrise_ridge | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| coastline_sunset | classic | 1.95 | 1.81 |  | 1.00 | 0.00 | 0.81 |  |  |  |  |  |  |  |  |  |  | 0.56 | 386.90 | 177 | 0 | OK:30 | OK |
| coastline_sunset | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night | classic | 6.78 | 6.57 |  | 0.08 | 1.74 | 4.76 |  |  |  |  |  |  |  |  |  |  | 4.89 | 92.10 | 1,292 | 1,129 | OK:200 | OK |
| town_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| woods_pine2_evening | classic | 16.08 | 15.93 |  | 0.12 | 8.78 | 7.04 |  |  |  |  |  |  |  |  |  |  | 5.07 | 53.60 | 3,250 | 1,811 | OK:30 | OK |
| woods_pine2_evening | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| blue_hour_coast | classic | 2.49 | 2.33 |  | 0.26 | 0.00 | 2.07 |  |  |  |  |  |  |  |  |  |  | 0.58 | 275.00 | 182 | 0 | OK:30 | OK |
| blue_hour_coast | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| overcast_broken | classic | 34.86 | 34.73 |  | 0.15 | 26.54 | 8.03 |  |  |  |  |  |  |  |  |  |  | 15.31 | 25.60 | 8,243 | 5,692 | OK:30 | OK |
| overcast_broken | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 20.53 | 20.39 |  | 0.15 | 8.41 | 11.83 |  |  |  |  |  |  |  |  |  |  | 17.69 | 36.60 | 2,688 | 1,363 | OK:30 | OK |
| sinai_desert_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
