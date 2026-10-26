# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| lowsun_fp | remastered | 38.84 | 31.75 | 0.15 | 3.87 | 17.64 | 10.24 | 0.12 |  |  | 6.11 | 0.13 | 0.01 | 0.19 | 0.06 |  | 0.12 | 34.45 | 25.80 | 1,090 | 1,179 | OK:30 | OK |
| sunrise_ridge | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sunrise_ridge | remastered | 24.02 | 18.77 | 0.51 | 6.84 | 9.13 | 2.81 | 0.15 |  |  | 4.26 | 0.12 | 0.01 | 0.18 | 0.06 |  | 0.17 | 19.62 | 35.10 | 1,659 | 7,680 | OK:30 | OK |
| noon_fields | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_fields | remastered | 45.24 | 40.40 | 0.23 | 6.64 | 30.23 | 3.52 | 0.11 |  |  | 4.32 | 0.13 | 0.01 | 0.16 | 0.05 |  | 0.12 | 28.11 | 21.60 | 1,447 | 3,920 | OK:30 | OK |
| forest_edge_golden | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | remastered | 30.45 | 24.30 | 0.19 | 6.37 | 12.45 | 5.49 | 0.26 |  |  | 4.40 | 0.15 | 0.01 | 0.41 | 0.33 |  | 1.13 | 27.18 | 32.50 | 757 | 1,316 | OK:30 | OK |
| coastline_sunset | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| coastline_sunset | remastered | 10.83 | 6.24 | 0.00 | 4.50 | 0.00 | 1.74 | 0.12 |  |  | 4.34 | 0.13 | 0.01 | 0.16 | 1.00 |  | 0.71 | 8.60 | 119.90 | 154 | 0 | OK:30 | OK |
| town_night | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night | remastered | 49.48 | 43.30 | 0.61 | 4.58 | 14.92 | 23.79 | 0.11 |  |  | 4.33 | 0.13 | 0.01 | 0.15 | 0.06 |  | 0.43 | 46.59 | 20.20 | 1,115 | 1,001 | OK:150 | OK |
| firefight_1 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| firefight_1 | remastered | 24.24 | 17.88 |  | 2.41 | 6.61 | 8.86 | 0.15 |  |  | 4.20 | 0.11 | 0.01 | 0.16 | 0.05 |  | 0.12 | 16.25 | 45.40 | 486 | 530 | UNSETTLED:300 | OK |
| firefight_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| firefight_2 | remastered | 24.24 | 17.88 |  | 2.41 | 6.61 | 8.86 | 0.15 |  |  | 4.20 | 0.11 | 0.01 | 0.16 | 0.05 |  | 0.12 | 16.25 | 45.40 | 474 | 408 | UNSETTLED:300 | OK |
| firefight_3 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| firefight_3 | remastered | 24.24 | 17.88 |  | 2.41 | 6.61 | 8.86 | 0.15 |  |  | 4.20 | 0.11 | 0.01 | 0.16 | 0.05 |  | 0.12 | 16.25 | 45.40 | 464 | 394 | UNSETTLED:300 | OK |
| tank_gunner_interior | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| tank_gunner_interior | remastered | 38.93 | 34.85 | 0.08 | 2.56 | 14.70 | 17.59 | 0.10 |  |  | 2.11 | 0.13 |  | 1.00 | 0.84 |  | 0.10 | 36.19 | 24.90 | 1,545 | 1,219 | OK:30 | OK |
| tank_gunner_optics | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| tank_gunner_optics | remastered | 29.07 | 23.46 | 0.06 | 2.68 | 8.69 | 12.09 | 0.11 |  |  | 2.37 | 0.12 | 0.01 | 0.15 |  |  | 0.10 | 24.96 | 35.20 | 1,152 | 738 | OK:30 | OK |
