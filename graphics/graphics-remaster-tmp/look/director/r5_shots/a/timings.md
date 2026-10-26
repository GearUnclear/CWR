# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| lowsun_fp | remastered | 39.18 | 31.21 | 0.52 | 2.96 | 17.98 | 10.26 | 0.13 |  |  | 6.17 | 0.13 | 0.01 | 0.19 | 0.06 |  | 0.12 | 37.11 | 25.20 | 1,090 | 1,179 | OK:30 | OK |
| sunrise_ridge | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sunrise_ridge | remastered | 25.10 | 19.70 | 0.19 | 6.17 | 10.18 | 3.35 | 0.15 |  |  | 4.34 | 0.12 | 0.01 | 0.17 | 0.06 |  | 0.32 | 22.48 | 35.30 | 1,659 | 7,680 | OK:30 | OK |
| noon_fields | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_fields | remastered | 48.96 | 43.54 | 1.39 | 6.81 | 32.76 | 3.97 | 0.11 |  |  | 4.36 | 0.13 | 0.01 | 0.16 | 0.05 |  | 0.18 | 34.45 | 20.40 | 1,447 | 3,920 | OK:30 | OK |
| forest_edge_golden | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | remastered | 27.13 | 21.98 | 0.19 | 5.56 | 11.55 | 4.87 | 0.38 |  |  | 3.90 | 0.15 | 0.01 | 0.35 | 0.66 |  | 1.00 | 17.61 | 33.50 | 757 | 1,316 | OK:30 | OK |
| coastline_sunset | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| coastline_sunset | remastered | 11.17 | 6.33 | 0.00 | 4.66 | 0.00 | 1.67 | 0.12 |  |  | 4.58 | 0.13 | 0.01 | 0.16 | 1.67 |  | 0.13 | 9.04 | 121.70 | 154 | 0 | OK:30 | OK |
| town_night | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night | remastered | 50.88 | 43.96 | 0.49 | 4.54 | 15.46 | 23.96 | 0.16 |  |  | 4.83 | 0.13 | 0.01 | 0.15 | 0.09 |  | 0.87 | 48.26 | 19.50 | 1,059 | 919 | OK:83 | OK |
| firefight_1 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| firefight_1 | remastered | 24.25 | 17.99 |  | 2.22 | 6.48 | 9.29 | 0.15 |  |  | 4.23 | 0.11 | 0.01 | 0.16 | 0.05 |  | 0.12 | 16.37 | 43.00 | 477 | 572 | UNSETTLED:300 | OK |
| firefight_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| firefight_2 | remastered | 24.25 | 17.99 |  | 2.22 | 6.48 | 9.29 | 0.15 |  |  | 4.23 | 0.11 | 0.01 | 0.16 | 0.05 |  | 0.12 | 16.37 | 43.00 | 490 | 495 | UNSETTLED:300 | OK |
| firefight_3 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| firefight_3 | remastered | 24.25 | 17.99 |  | 2.22 | 6.48 | 9.29 | 0.15 |  |  | 4.23 | 0.11 | 0.01 | 0.16 | 0.05 |  | 0.12 | 16.37 | 43.00 | 409 | 397 | UNSETTLED:300 | OK |
| tank_gunner_interior | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| tank_gunner_interior | remastered | 30.16 | 27.03 | 0.06 | 2.09 | 10.61 | 14.33 | 0.10 |  |  | 1.69 | 0.12 |  | 0.36 | 1.08 |  | 0.10 | 26.66 | 31.40 | 1,540 | 1,214 | OK:30 | OK |
| tank_gunner_optics | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| tank_gunner_optics | remastered | 27.13 | 21.73 | 0.06 | 2.36 | 8.11 | 11.25 | 0.11 |  |  | 2.35 | 0.12 | 0.01 | 0.15 |  |  | 0.10 | 20.42 | 34.90 | 1,152 | 738 | OK:30 | OK |
