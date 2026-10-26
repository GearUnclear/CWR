# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 4.49 | 4.14 |  | 0.14 | 1.76 | 2.24 |  |  |  |  |  |  |  |  |  |  | 3.33 | 145.40 | 2,091 | 1,316 | OK:30 | OK |
| lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_fields | classic | 8.60 | 8.47 |  | 0.11 | 6.37 | 2.00 |  |  |  |  |  |  |  |  |  |  | 8.32 | 68.40 | 6,767 | 4,508 | OK:30 | OK |
| noon_fields | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 5.90 | 5.72 |  | 0.11 | 3.15 | 2.46 |  |  |  |  |  |  |  |  |  |  | 4.98 | 115.80 | 2,502 | 1,459 | OK:30 | OK |
| forest_edge_golden | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sunrise_ridge | classic | 18.13 | 17.97 |  | 0.12 | 14.96 | 2.89 |  |  |  |  |  |  |  |  |  |  | 17.32 | 35.40 | 13,076 | 8,897 | OK:30 | OK |
| sunrise_ridge | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| coastline_sunset | classic | 1.44 | 1.19 |  | 0.70 | 0.00 | 0.50 |  |  |  |  |  |  |  |  |  |  | 0.94 | 368.40 | 177 | 0 | OK:30 | OK |
| coastline_sunset | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night | classic | 6.05 | 5.78 |  | 0.13 | 1.27 | 4.38 |  |  |  |  |  |  |  |  |  |  | 5.72 | 122.40 | 1,195 | 1,021 | OK:195 | OK |
| town_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| woods_pine2_evening | classic | 5.75 | 5.59 |  | 0.13 | 3.06 | 2.40 |  |  |  |  |  |  |  |  |  |  | 5.03 | 109.70 | 3,250 | 1,811 | OK:30 | OK |
| woods_pine2_evening | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| blue_hour_coast | classic | 1.02 | 0.83 |  | 0.41 | 0.00 | 0.42 |  |  |  |  |  |  |  |  |  |  | 0.56 | 386.90 | 182 | 0 | OK:30 | OK |
| blue_hour_coast | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| overcast_broken | classic | 18.25 | 18.12 |  | 0.15 | 12.85 | 5.12 |  |  |  |  |  |  |  |  |  |  | 11.54 | 40.20 | 8,243 | 5,692 | OK:30 | OK |
| overcast_broken | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 8.87 | 8.72 |  | 0.15 | 3.60 | 4.97 |  |  |  |  |  |  |  |  |  |  | 6.05 | 73.90 | 2,688 | 1,363 | OK:30 | OK |
| sinai_desert_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
