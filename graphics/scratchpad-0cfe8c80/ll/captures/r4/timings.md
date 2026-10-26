# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 5.23 | 5.10 |  | 0.11 | 2.21 | 2.77 |  |  |  |  |  |  |  |  |  |  | 3.46 | 127.20 | 2,091 | 1,316 | OK:30 | OK |
| lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_fields | classic | 8.62 | 8.49 |  | 0.11 | 6.35 | 2.03 |  |  |  |  |  |  |  |  |  |  | 8.35 | 69.10 | 6,767 | 4,508 | OK:30 | OK |
| noon_fields | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 5.75 | 5.57 |  | 0.10 | 3.13 | 2.34 |  |  |  |  |  |  |  |  |  |  | 5.04 | 114.10 | 2,502 | 1,459 | OK:30 | OK |
| forest_edge_golden | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sunrise_ridge | classic | 18.86 | 18.71 |  | 0.12 | 15.57 | 3.02 |  |  |  |  |  |  |  |  |  |  | 18.11 | 33.70 | 13,076 | 8,897 | OK:30 | OK |
| sunrise_ridge | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| coastline_sunset | classic | 1.42 | 1.18 |  | 0.69 | 0.00 | 0.49 |  |  |  |  |  |  |  |  |  |  | 0.95 | 341.10 | 177 | 0 | OK:30 | OK |
| coastline_sunset | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night | classic | 7.09 | 6.87 |  | 0.10 | 1.69 | 5.07 |  |  |  |  |  |  |  |  |  |  | 6.34 | 106.60 | 1,291 | 1,121 | OK:250 | OK |
| town_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| woods_pine2_evening | classic | 6.94 | 6.79 |  | 0.13 | 3.73 | 2.93 |  |  |  |  |  |  |  |  |  |  | 5.14 | 88.20 | 3,250 | 1,811 | OK:30 | OK |
| woods_pine2_evening | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| blue_hour_coast | classic | 1.65 | 1.35 |  | 0.72 | 0.00 | 0.63 |  |  |  |  |  |  |  |  |  |  | 1.01 | 271.00 | 182 | 0 | OK:30 | OK |
| blue_hour_coast | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| overcast_broken | classic | 12.87 | 12.74 |  | 0.15 | 9.14 | 3.45 |  |  |  |  |  |  |  |  |  |  | 11.35 | 43.20 | 8,243 | 5,692 | OK:30 | OK |
| overcast_broken | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 9.15 | 9.00 |  | 0.16 | 4.59 | 4.25 |  |  |  |  |  |  |  |  |  |  | 7.18 | 59.30 | 2,689 | 1,363 | OK:30 | OK |
| sinai_desert_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
