# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 15.63 | 15.49 |  | 0.11 | 5.91 | 9.47 |  |  |  |  |  |  |  |  |  |  | 11.77 | 55.30 | 2,091 | 1,316 | OK:30 | OK |
| lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_fields | classic | 24.12 | 23.99 |  | 0.11 | 17.39 | 6.50 |  |  |  |  |  |  |  |  |  |  | 9.71 | 31.80 | 6,767 | 4,508 | OK:30 | OK |
| noon_fields | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 14.53 | 14.38 |  | 0.09 | 8.12 | 6.17 |  |  |  |  |  |  |  |  |  |  | 4.85 | 54.90 | 2,502 | 1,459 | OK:30 | OK |
| forest_edge_golden | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sunrise_ridge | classic | 52.06 | 51.90 |  | 0.13 | 43.09 | 8.69 |  |  |  |  |  |  |  |  |  |  | 25.60 | 15.70 | 13,076 | 8,897 | OK:30 | OK |
| sunrise_ridge | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| coastline_sunset | classic | 0.82 | 0.70 |  | 0.22 | 0.00 | 0.49 |  |  |  |  |  |  |  |  |  |  | 0.56 | 338.80 | 177 | 0 | OK:30 | OK |
| coastline_sunset | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night | classic | 8.47 | 8.29 |  | 0.08 | 1.57 | 6.64 |  |  |  |  |  |  |  |  |  |  | 4.47 | 100.50 | 1,173 | 987 | OK:123 | OK |
| town_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| woods_pine2_evening | classic | 15.06 | 14.92 |  | 0.11 | 8.30 | 6.50 |  |  |  |  |  |  |  |  |  |  | 9.06 | 52.60 | 3,250 | 1,811 | OK:30 | OK |
| woods_pine2_evening | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| blue_hour_coast | classic | 1.58 | 1.40 |  | 0.22 | 0.00 | 1.18 |  |  |  |  |  |  |  |  |  |  | 0.58 | 310.80 | 182 | 0 | OK:30 | OK |
| blue_hour_coast | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| overcast_broken | classic | 34.18 | 34.05 |  | 0.15 | 25.76 | 8.13 |  |  |  |  |  |  |  |  |  |  | 12.87 | 26.10 | 8,243 | 5,692 | OK:30 | OK |
| overcast_broken | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 14.32 | 14.18 |  | 0.15 | 5.37 | 8.66 |  |  |  |  |  |  |  |  |  |  | 7.90 | 49.90 | 2,689 | 1,363 | OK:30 | OK |
| sinai_desert_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
