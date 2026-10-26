# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 4.35 | 4.18 |  | 0.14 | 1.74 | 2.31 |  |  |  |  |  |  |  |  |  |  | 3.54 | 140.20 | 2,091 | 1,316 | OK:30 | OK |
| lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_fields | classic | 8.76 | 8.64 |  | 0.11 | 6.30 | 2.23 |  |  |  |  |  |  |  |  |  |  | 8.49 | 68.40 | 6,767 | 4,508 | OK:30 | OK |
| noon_fields | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 5.60 | 5.41 |  | 0.10 | 2.95 | 2.36 |  |  |  |  |  |  |  |  |  |  | 4.91 | 116.00 | 2,502 | 1,459 | OK:30 | OK |
| forest_edge_golden | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sunrise_ridge | classic | 18.83 | 18.67 |  | 0.12 | 15.55 | 3.00 |  |  |  |  |  |  |  |  |  |  | 18.10 | 34.60 | 13,076 | 8,897 | OK:30 | OK |
| sunrise_ridge | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| coastline_sunset | classic | 1.43 | 1.19 |  | 0.70 | 0.00 | 0.49 |  |  |  |  |  |  |  |  |  |  | 0.95 | 339.60 | 177 | 0 | OK:30 | OK |
| coastline_sunset | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night | classic | 6.24 | 6.00 |  | 0.11 | 1.34 | 4.55 |  |  |  |  |  |  |  |  |  |  | 5.73 | 121.40 | 1,210 | 1,041 | OK:197 | OK |
| town_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| woods_pine2_evening | classic | 6.04 | 5.88 |  | 0.13 | 3.28 | 2.47 |  |  |  |  |  |  |  |  |  |  | 5.06 | 109.30 | 3,250 | 1,811 | OK:30 | OK |
| woods_pine2_evening | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| blue_hour_coast | classic | 1.64 | 1.33 |  | 0.84 | 0.00 | 0.49 |  |  |  |  |  |  |  |  |  |  | 1.01 | 290.10 | 182 | 0 | OK:30 | OK |
| blue_hour_coast | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| overcast_broken | classic | 11.77 | 11.64 |  | 0.15 | 8.73 | 2.75 |  |  |  |  |  |  |  |  |  |  | 11.15 | 51.40 | 8,243 | 5,692 | OK:30 | OK |
| overcast_broken | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 8.91 | 8.76 |  | 0.16 | 4.48 | 4.12 |  |  |  |  |  |  |  |  |  |  | 6.41 | 70.50 | 2,689 | 1,363 | OK:30 | OK |
| sinai_desert_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
