# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 4.38 | 4.26 |  |  |  |  |  |  | 163.10 | 1,575 | 1,179 | OK:30 | OK |
| lowsun_fp | remastered | 4.18 | 3.44 | 0.04 | 0.63 | 0.33 | 0.05 | 0.22 | 0.03 | 154.00 | 1,568 | 1,179 | OK:30 | OK |
| lowsun_fp | delta | -0.20 | -0.82 |  |  |  |  |  |  |  | -0.4% | +0.0% |  |  |
| sunrise_ridge | classic | 14.71 | 14.60 |  |  |  |  |  |  | 42.20 | 11,172 | 7,680 | OK:30 | OK |
| sunrise_ridge | remastered | 18.13 | 17.28 | 0.00 | 0.75 | 0.52 | 0.05 | 0.15 | 0.03 | 40.00 | 11,172 | 7,680 | OK:30 | OK |
| sunrise_ridge | delta | +3.42 | +2.68 |  |  |  |  |  |  |  | +0.0% | +0.0% |  |  |
| noon_fields | classic | 6.32 | 6.22 |  |  |  |  |  |  | 84.20 | 5,572 | 3,920 | OK:30 | OK |
| noon_fields | remastered | 7.89 | 7.36 | 0.04 | 0.43 | 0.36 | 0.05 |  | 0.03 | 76.20 | 5,567 | 3,920 | OK:30 | OK |
| noon_fields | delta | +1.57 | +1.14 |  |  |  |  |  |  |  | -0.1% | +0.0% |  |  |
| forest_edge_golden | classic | 4.71 | 4.39 |  |  |  |  |  |  | 210.00 | 1,850 | 1,316 | OK:30 | OK |
| forest_edge_golden | remastered | 4.98 | 4.30 | 0.05 | 0.52 | 0.41 | 0.07 |  | 0.04 | 123.50 | 1,847 | 1,316 | OK:30 | OK |
| forest_edge_golden | delta | +0.27 | -0.09 |  |  |  |  |  |  |  | -0.2% | +0.0% |  |  |
| coastline_sunset | classic | 2.44 | 2.01 |  |  |  |  |  |  | 230.30 | 155 | 0 | OK:30 | OK |
| coastline_sunset | remastered | 4.11 | 1.90 | 0.00 | 1.87 | 0.74 | 0.21 | 0.75 | 0.17 | 206.60 | 155 | 0 | OK:30 | OK |
| coastline_sunset | delta | +1.67 | -0.11 |  |  |  |  |  |  |  | +0.0% |  |  |  |
| town_night | classic | 7.49 | 7.13 |  |  |  |  |  |  | 108.90 | 1,207 | 1,047 | OK:276 | OK |
| town_night | remastered | 6.33 | 5.52 |  | 0.63 | 0.54 | 0.06 |  | 0.03 | 120.10 | 1,220 | 1,051 | OK:30 | OK |
| town_night | delta | -1.16 | -1.61 |  |  |  |  |  |  |  | +1.1% | +0.4% |  |  |
| firefight_1 | classic | 5.47 | 5.05 |  |  |  |  |  |  | 153.40 | 416 | 400 | OK:55 | OK |
| firefight_1 | remastered | 5.79 | 4.16 |  | 1.20 | 0.87 | 0.19 |  | 0.14 | 133.50 | 417 | 395 | OK:286 | OK |
| firefight_1 | delta | +0.32 | -0.89 |  |  |  |  |  |  |  | +0.2% | -1.2% |  |  |
| firefight_2 | classic | 5.47 | 5.05 |  |  |  |  |  |  | 153.40 | 498 | 543 | OK:55 | OK |
| firefight_2 | remastered | 5.79 | 4.16 |  | 1.20 | 0.87 | 0.19 |  | 0.14 | 133.50 | 514 | 547 | OK:286 | OK |
| firefight_2 | delta | +0.32 | -0.89 |  |  |  |  |  |  |  | +3.1% | +0.7% |  |  |
| firefight_3 | classic | 5.47 | 5.05 |  |  |  |  |  |  | 153.40 | 517 | 580 | OK:55 | OK |
| firefight_3 | remastered | 5.79 | 4.16 |  | 1.20 | 0.87 | 0.19 |  | 0.14 | 133.50 | 503 | 524 | OK:286 | OK |
| firefight_3 | delta | +0.32 | -0.89 |  |  |  |  |  |  |  | -2.7% | -9.7% |  |  |
| tank_gunner_interior | classic | 5.54 | 5.21 |  |  |  |  |  |  | 117.40 | 2,261 | 1,216 | OK:30 | OK |
| tank_gunner_interior | remastered | 6.36 | 5.57 | 0.04 | 0.51 |  | 0.48 |  | 0.03 | 102.50 | 2,272 | 1,230 | OK:30 | OK |
| tank_gunner_interior | delta | +0.82 | +0.36 |  |  |  |  |  |  |  | +0.5% | +1.1% |  |  |
| tank_gunner_optics | classic | 5.52 | 5.09 |  |  |  |  |  |  | 113.90 | 1,396 | 743 | OK:30 | OK |
| tank_gunner_optics | remastered | 5.40 | 4.55 | 0.06 | 0.58 | 0.54 |  |  | 0.04 | 109.70 | 1,377 | 743 | OK:30 | OK |
| tank_gunner_optics | delta | -0.12 | -0.54 |  |  |  |  |  |  |  | -1.4% | +0.0% |  |  |
| overcast_broken | classic | 8.79 | 8.69 |  |  |  |  |  |  | 70.40 | 6,837 | 5,083 | OK:30 | OK |
| overcast_broken | remastered | 9.82 | 9.15 | 0.03 | 0.58 | 0.51 | 0.05 |  | 0.03 | 64.60 | 6,835 | 5,083 | OK:30 | OK |
| overcast_broken | delta | +1.03 | +0.46 |  |  |  |  |  |  |  | -0.0% | +0.0% |  |  |
| overcast_heavy | classic | 8.57 | 8.47 |  |  |  |  |  |  | 71.40 | 6,593 | 5,083 | OK:30 | OK |
| overcast_heavy | remastered | 9.86 | 9.21 | 0.03 | 0.54 | 0.47 | 0.05 |  | 0.03 | 66.20 | 6,591 | 5,083 | OK:30 | OK |
| overcast_heavy | delta | +1.29 | +0.74 |  |  |  |  |  |  |  | -0.0% | +0.0% |  |  |
| sinai_lowsun_fp | classic | 5.33 | 4.99 |  |  |  |  |  |  | 105.00 | 573 | 314 | OK:30 | OK |
| sinai_lowsun_fp | remastered | 6.37 | 4.41 | 0.18 | 1.65 | 0.82 | 0.14 | 0.58 | 0.12 | 104.10 | 570 | 314 | OK:30 | OK |
| sinai_lowsun_fp | delta | +1.04 | -0.58 |  |  |  |  |  |  |  | -0.5% | +0.0% |  |  |
| sinai_desert_noon | classic | 6.44 | 6.32 |  |  |  |  |  |  | 93.50 | 2,231 | 1,100 | OK:30 | OK |
| sinai_desert_noon | remastered | 7.02 | 6.08 | 0.05 | 0.80 | 0.70 | 0.06 |  | 0.03 | 85.50 | 2,228 | 1,100 | OK:30 | OK |
| sinai_desert_noon | delta | +0.58 | -0.24 |  |  |  |  |  |  |  | -0.1% | +0.0% |  |  |
