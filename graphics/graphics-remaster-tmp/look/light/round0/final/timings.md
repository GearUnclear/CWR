# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 6.77 | 6.61 |  | 0.11 | 2.46 | 4.04 |  |  |  |  |  |  |  |  |  |  | 3.31 | 99.60 | 2,091 | 1,316 | OK:30 | OK |
| lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_fields | classic | 10.03 | 9.90 |  | 0.10 | 7.58 | 2.21 |  |  |  |  |  |  |  |  |  |  | 8.70 | 61.30 | 6,767 | 4,508 | OK:30 | OK |
| noon_fields | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 7.44 | 7.28 |  | 0.09 | 4.14 | 3.05 |  |  |  |  |  |  |  |  |  |  | 4.59 | 82.10 | 2,502 | 1,459 | OK:30 | OK |
| forest_edge_golden | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sunrise_ridge | classic | 32.15 | 32.00 |  | 0.12 | 27.07 | 4.80 |  |  |  |  |  |  |  |  |  |  | 18.28 | 23.70 | 13,076 | 8,897 | OK:30 | OK |
| sunrise_ridge | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| coastline_sunset | classic | 0.89 | 0.76 |  | 0.28 | 0.00 | 0.48 |  |  |  |  |  |  |  |  |  |  | 0.56 | 394.60 | 177 | 0 | OK:30 | OK |
| coastline_sunset | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night | classic | 6.84 | 6.66 |  | 0.08 | 1.37 | 5.21 |  |  |  |  |  |  |  |  |  |  | 4.60 | 119.60 | 1,213 | 1,029 | OK:195 | OK |
| town_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| woods_pine2_evening | classic | 5.80 | 5.65 |  | 0.12 | 3.10 | 2.43 |  |  |  |  |  |  |  |  |  |  | 5.06 | 106.20 | 3,250 | 1,811 | OK:30 | OK |
| woods_pine2_evening | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| blue_hour_coast | classic | 1.05 | 0.87 |  | 0.29 | 0.00 | 0.58 |  |  |  |  |  |  |  |  |  |  | 0.58 | 356.50 | 182 | 0 | OK:30 | OK |
| blue_hour_coast | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| overcast_broken | classic | 11.82 | 11.69 |  | 0.15 | 8.79 | 2.75 |  |  |  |  |  |  |  |  |  |  | 11.21 | 48.40 | 8,243 | 5,692 | OK:30 | OK |
| overcast_broken | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 7.89 | 7.71 |  | 0.20 | 3.62 | 3.89 |  |  |  |  |  |  |  |  |  |  | 7.13 | 73.10 | 2,689 | 1,363 | OK:30 | OK |
| sinai_desert_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
