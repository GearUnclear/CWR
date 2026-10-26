# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 4.29 | 4.13 |  | 0.13 | 1.65 | 2.35 |  |  |  |  |  |  |  |  |  |  | 3.59 | 138.30 | 2,091 | 1,316 | OK:30 | OK |
| lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_fields | classic | 8.47 | 8.34 |  | 0.11 | 6.12 | 2.12 |  |  |  |  |  |  |  |  |  |  | 8.22 | 69.20 | 6,767 | 4,508 | OK:30 | OK |
| noon_fields | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 5.75 | 5.57 |  | 0.10 | 2.96 | 2.51 |  |  |  |  |  |  |  |  |  |  | 5.02 | 113.60 | 2,502 | 1,459 | OK:30 | OK |
| forest_edge_golden | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sunrise_ridge | classic | 18.61 | 18.46 |  | 0.12 | 15.35 | 2.98 |  |  |  |  |  |  |  |  |  |  | 18.04 | 35.30 | 13,076 | 8,897 | OK:30 | OK |
| sunrise_ridge | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| coastline_sunset | classic | 1.39 | 1.15 |  | 0.66 | 0.00 | 0.49 |  |  |  |  |  |  |  |  |  |  | 0.92 | 351.40 | 177 | 0 | OK:30 | OK |
| coastline_sunset | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night | classic | 6.41 | 6.18 |  | 0.11 | 1.34 | 4.73 |  |  |  |  |  |  |  |  |  |  | 6.05 | 117.00 | 1,265 | 1,100 | OK:237 | OK |
| town_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| woods_pine2_evening | classic | 5.78 | 5.63 |  | 0.12 | 3.06 | 2.45 |  |  |  |  |  |  |  |  |  |  | 5.06 | 109.20 | 3,250 | 1,811 | OK:30 | OK |
| woods_pine2_evening | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| blue_hour_coast | classic | 1.56 | 1.26 |  | 0.71 | 0.00 | 0.55 |  |  |  |  |  |  |  |  |  |  | 0.98 | 263.30 | 182 | 0 | OK:30 | OK |
| blue_hour_coast | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| overcast_broken | classic | 11.72 | 11.59 |  | 0.15 | 8.70 | 2.74 |  |  |  |  |  |  |  |  |  |  | 11.13 | 49.40 | 8,243 | 5,692 | OK:30 | OK |
| overcast_broken | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 7.94 | 7.77 |  | 0.19 | 3.69 | 3.89 |  |  |  |  |  |  |  |  |  |  | 6.76 | 73.70 | 2,689 | 1,363 | OK:30 | OK |
| sinai_desert_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
