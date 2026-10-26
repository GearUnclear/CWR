# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| lowsun_fp | remastered | 39.23 | 32.49 | 0.15 | 4.12 | 17.17 | 11.20 | 0.12 |  |  | 5.78 | 0.13 | 0.01 | 0.18 | 0.06 |  | 0.12 | 24.82 | 25.30 | 1,090 | 1,179 | OK:30 | OK |
| sunrise_ridge | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sunrise_ridge | remastered | 24.16 | 18.74 | 0.28 | 6.26 | 10.00 | 2.48 | 0.14 |  |  | 4.48 | 0.12 | 0.01 | 0.17 | 0.05 |  | 0.10 | 21.92 | 35.60 | 1,659 | 7,680 | OK:30 | OK |
| noon_fields | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_fields | remastered | 49.25 | 43.76 | 0.53 | 7.08 | 32.80 | 3.88 | 0.11 |  |  | 4.55 | 0.13 | 0.01 | 0.16 | 0.05 |  | 0.12 | 33.39 | 20.50 | 1,447 | 3,920 | OK:30 | OK |
| forest_edge_golden | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | remastered | 28.33 | 22.91 | 0.19 | 6.05 | 11.81 | 5.05 | 0.23 |  |  | 4.04 | 0.15 | 0.01 | 0.32 | 0.36 |  | 0.54 | 19.99 | 32.30 | 757 | 1,316 | OK:30 | OK |
| coastline_sunset | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| coastline_sunset | remastered | 10.78 | 6.18 | 0.00 | 4.46 | 0.00 | 1.72 | 0.12 |  |  | 4.36 | 0.13 | 0.01 | 0.16 | 1.15 |  | 0.58 | 9.08 | 125.40 | 154 | 0 | OK:30 | OK |
| town_night | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night | remastered | 55.02 | 48.28 | 1.00 | 4.71 | 16.43 | 27.14 | 0.06 |  |  | 4.72 | 0.13 | 0.01 | 0.15 | 0.16 |  | 0.78 | 52.49 | 18.10 | 1,167 | 1,067 | OK:81 | OK |
| firefight_1 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| firefight_1 | remastered | 23.60 | 17.69 |  | 2.27 | 7.56 | 7.87 | 0.15 |  |  | 3.85 | 0.11 | 0.01 | 0.16 | 0.05 |  | 0.12 | 15.36 | 43.60 | 482 | 589 | UNSETTLED:300 | OK |
| firefight_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| firefight_2 | remastered | 23.60 | 17.69 |  | 2.27 | 7.56 | 7.87 | 0.15 |  |  | 3.85 | 0.11 | 0.01 | 0.16 | 0.05 |  | 0.12 | 15.36 | 43.60 | 463 | 419 | UNSETTLED:300 | OK |
| firefight_3 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| firefight_3 | remastered | 23.60 | 17.69 |  | 2.27 | 7.56 | 7.87 | 0.15 |  |  | 3.85 | 0.11 | 0.01 | 0.16 | 0.05 |  | 0.12 | 15.36 | 43.60 | 417 | 372 | UNSETTLED:300 | OK |
| tank_gunner_interior | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| tank_gunner_interior | remastered | 30.02 | 26.81 | 0.06 | 2.04 | 10.35 | 14.42 | 0.09 |  |  | 1.75 | 0.12 |  | 0.28 | 1.23 |  | 0.10 | 25.18 | 31.30 | 1,541 | 1,215 | OK:30 | OK |
| tank_gunner_optics | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| tank_gunner_optics | remastered | 27.77 | 22.77 | 0.06 | 2.50 | 8.62 | 11.65 | 0.11 |  |  | 2.43 | 0.11 | 0.01 | 0.15 |  |  | 0.10 | 17.32 | 36.60 | 1,149 | 738 | OK:30 | OK |
