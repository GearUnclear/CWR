# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 6.58 | 6.44 |  | 0.10 | 1.93 | 4.41 |  |  |  |  |  |  |  |  |  |  | 3.36 | 125.60 | 2,091 | 1,316 | OK:30 | OK |
| lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_fields | classic | 10.81 | 10.69 |  | 0.11 | 7.95 | 2.64 |  |  |  |  |  |  |  |  |  |  | 8.54 | 63.40 | 6,767 | 4,508 | OK:30 | OK |
| noon_fields | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 5.43 | 5.28 |  | 0.09 | 2.54 | 2.65 |  |  |  |  |  |  |  |  |  |  | 4.54 | 117.40 | 2,502 | 1,459 | OK:30 | OK |
| forest_edge_golden | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sunrise_ridge | classic | 27.77 | 27.62 |  | 0.12 | 22.98 | 4.52 |  |  |  |  |  |  |  |  |  |  | 18.14 | 27.60 | 13,076 | 8,897 | OK:30 | OK |
| sunrise_ridge | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| coastline_sunset | classic | 0.90 | 0.78 |  | 0.22 | 0.00 | 0.57 |  |  |  |  |  |  |  |  |  |  | 0.55 | 416.70 | 177 | 0 | OK:30 | OK |
| coastline_sunset | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night | classic | 6.09 | 5.86 |  | 0.09 | 1.38 | 4.38 |  |  |  |  |  |  |  |  |  |  | 4.88 | 126.10 | 1,266 | 1,089 | OK:209 | OK |
| town_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| woods_pine2_evening | classic | 5.78 | 5.63 |  | 0.12 | 3.19 | 2.33 |  |  |  |  |  |  |  |  |  |  | 5.10 | 105.30 | 3,250 | 1,811 | OK:30 | OK |
| woods_pine2_evening | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| blue_hour_coast | classic | 1.56 | 1.25 |  | 0.71 | 0.00 | 0.54 |  |  |  |  |  |  |  |  |  |  | 1.04 | 302.50 | 182 | 0 | OK:30 | OK |
| blue_hour_coast | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| overcast_broken | classic | 15.31 | 15.18 |  | 0.15 | 10.96 | 4.07 |  |  |  |  |  |  |  |  |  |  | 11.53 | 38.20 | 8,243 | 5,692 | OK:30 | OK |
| overcast_broken | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 10.10 | 9.96 |  | 0.15 | 4.08 | 5.73 |  |  |  |  |  |  |  |  |  |  | 5.80 | 69.40 | 2,689 | 1,363 | OK:30 | OK |
| sinai_desert_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
