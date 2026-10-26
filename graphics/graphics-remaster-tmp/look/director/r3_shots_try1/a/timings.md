# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| lowsun_fp | remastered | 39.40 | 32.10 | 0.14 | 3.89 | 18.09 | 10.11 | 0.12 |  |  | 6.35 | 0.13 | 0.01 | 0.19 | 0.06 |  | 0.12 | 26.15 | 25.20 | 1,090 | 1,179 | OK:30 | OK |
| sunrise_ridge | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sunrise_ridge | remastered | 24.73 | 19.42 | 0.10 | 6.18 | 10.39 | 2.84 | 0.14 |  |  | 4.34 | 0.12 | 0.01 | 0.17 | 0.05 |  | 0.10 | 22.25 | 35.60 | 1,659 | 7,680 | OK:30 | OK |
| noon_fields | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_fields | remastered | 48.49 | 42.96 | 1.16 | 7.04 | 32.09 | 3.83 | 0.11 |  |  | 4.53 | 0.13 | 0.01 | 0.18 | 0.05 |  | 0.16 | 34.93 | 20.40 | 1,447 | 3,920 | OK:30 | OK |
| forest_edge_golden | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | remastered | 23.88 | 19.24 | 0.19 | 5.09 | 9.70 | 4.45 | 0.23 |  |  | 3.52 | 0.15 | 0.01 | 0.29 | 0.23 |  | 0.45 | 18.25 | 32.60 | 757 | 1,316 | OK:30 | OK |
| coastline_sunset | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| coastline_sunset | remastered | 11.06 | 6.11 | 0.00 | 4.45 | 0.00 | 1.65 | 0.12 |  |  | 4.71 | 0.13 | 0.01 | 0.16 | 1.66 |  | 0.32 | 9.34 | 121.70 | 154 | 0 | OK:30 | OK |
| town_night | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night | remastered | 44.52 | 38.54 | 0.57 | 4.02 | 13.50 | 21.02 | 0.06 |  |  | 4.15 | 0.13 | 0.01 | 0.15 | 0.11 |  | 0.38 | 35.83 | 19.20 | 1,102 | 980 | OK:163 | OK |
| firefight_1 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| firefight_1 | remastered | 24.10 | 18.15 |  | 2.38 | 6.42 | 9.35 | 0.15 |  |  | 3.83 | 0.11 | 0.01 | 0.16 | 0.05 |  | 0.12 | 15.07 | 45.20 | 517 | 552 | UNSETTLED:300 | OK |
| firefight_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| firefight_2 | remastered | 24.10 | 18.15 |  | 2.38 | 6.42 | 9.35 | 0.15 |  |  | 3.83 | 0.11 | 0.01 | 0.16 | 0.05 |  | 0.12 | 15.07 | 45.20 | 444 | 424 | UNSETTLED:300 | OK |
| firefight_3 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| firefight_3 | remastered | 24.10 | 18.15 |  | 2.38 | 6.42 | 9.35 | 0.15 |  |  | 3.83 | 0.11 | 0.01 | 0.16 | 0.05 |  | 0.12 | 15.07 | 45.20 | 410 | 375 | UNSETTLED:300 | OK |
| tank_gunner_interior | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| tank_gunner_interior | remastered | 40.27 | 35.97 | 0.07 | 2.63 | 14.68 | 18.66 | 0.09 |  |  | 2.24 | 0.13 |  | 0.87 | 1.10 |  | 0.11 | 27.20 | 26.00 | 1,546 | 1,220 | OK:30 | OK |
| tank_gunner_optics | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| tank_gunner_optics | remastered | 29.17 | 23.74 | 0.06 | 2.59 | 8.96 | 12.18 | 0.11 |  |  | 2.40 | 0.12 | 0.01 | 0.15 |  |  | 0.10 | 19.13 | 35.30 | 1,149 | 738 | OK:30 | OK |
