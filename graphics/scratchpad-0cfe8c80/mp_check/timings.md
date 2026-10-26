# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 6.38 | 6.10 |  |  |  |  |  |  | 115.20 | 1,778 | 1,179 | OK:30 | OK |
| lowsun_fp | remastered | 4.57 | 3.64 | 0.10 | 0.80 | 0.39 | 0.06 | 0.28 | 0.07 | 143.30 | 1,615 | 1,179 | OK:30 | OK |
| lowsun_fp | delta | -1.81 | -2.46 |  |  |  |  |  |  |  | -9.2% | +0.0% |  |  |
| sunrise_ridge | classic | 14.49 | 14.38 |  |  |  |  |  |  | 43.90 | 11,301 | 7,680 | OK:30 | OK |
| sunrise_ridge | remastered | 18.28 | 17.42 | 0.09 | 0.74 | 0.51 | 0.05 | 0.15 | 0.03 | 43.10 | 11,172 | 7,680 | OK:30 | OK |
| sunrise_ridge | delta | +3.79 | +3.04 |  |  |  |  |  |  |  | -1.1% | +0.0% |  |  |
| noon_fields | classic | 7.11 | 7.01 |  |  |  |  |  |  | 83.20 | 5,878 | 3,920 | OK:30 | OK |
| noon_fields | remastered | 7.92 | 7.41 | 0.07 | 0.40 | 0.32 | 0.05 |  | 0.03 | 80.00 | 5,575 | 3,920 | OK:30 | OK |
| noon_fields | delta | +0.81 | +0.40 |  |  |  |  |  |  |  | -5.2% | +0.0% |  |  |
| forest_edge_golden | classic | 6.71 | 6.39 |  |  |  |  |  |  | 182.60 | 2,189 | 1,316 | OK:30 | OK |
| forest_edge_golden | remastered | 5.47 | 4.82 | 0.29 | 0.48 | 0.36 | 0.07 |  | 0.04 | 132.90 | 1,847 | 1,316 | OK:30 | OK |
| forest_edge_golden | delta | -1.24 | -1.57 |  |  |  |  |  |  |  | -15.6% | +0.0% |  |  |
| coastline_sunset | classic | 2.05 | 1.63 |  |  |  |  |  |  | 241.40 | 155 | 0 | OK:30 | OK |
| coastline_sunset | remastered | 4.00 | 1.65 | 0.00 | 1.76 | 0.71 | 0.18 | 0.59 | 0.27 | 200.30 | 155 | 0 | OK:30 | OK |
| coastline_sunset | delta | +1.95 | +0.02 |  |  |  |  |  |  |  | +0.0% |  |  |  |
| town_night | classic | 6.04 | 5.79 |  |  |  |  |  |  | 121.80 | 1,149 | 986 | OK:252 | OK |
| town_night | remastered | 6.45 | 5.69 |  | 0.57 | 0.46 | 0.07 |  | 0.04 | 126.70 | 1,151 | 984 | OK:52 | OK |
| town_night | delta | +0.41 | -0.10 |  |  |  |  |  |  |  | +0.2% | -0.2% |  |  |
| firefight_1 | classic | 4.81 | 4.30 |  |  |  |  |  |  | 179.70 | 452 | 431 | OK:47 | OK |
| firefight_1 | remastered | 4.76 | 3.14 |  | 1.19 | 0.97 | 0.11 |  | 0.10 | 147.90 | 468 | 418 | OK:275 | OK |
| firefight_1 | delta | -0.05 | -1.16 |  |  |  |  |  |  |  | +3.4% | -3.0% |  |  |
| firefight_2 | classic | 4.81 | 4.30 |  |  |  |  |  |  | 179.70 | 520 | 514 | OK:47 | OK |
| firefight_2 | remastered | 4.76 | 3.14 |  | 1.19 | 0.97 | 0.11 |  | 0.10 | 147.90 | 583 | 537 | OK:275 | OK |
| firefight_2 | delta | -0.05 | -1.16 |  |  |  |  |  |  |  | +10.8% | +4.3% |  |  |
| firefight_3 | classic | 4.81 | 4.30 |  |  |  |  |  |  | 179.70 | 464 | 404 | OK:47 | OK |
| firefight_3 | remastered | 4.76 | 3.14 |  | 1.19 | 0.97 | 0.11 |  | 0.10 | 147.90 | 525 | 431 | OK:275 | OK |
| firefight_3 | delta | -0.05 | -1.16 |  |  |  |  |  |  |  | +11.6% | +6.3% |  |  |
| tank_gunner_interior | classic | 6.12 | 5.81 |  |  |  |  |  |  | 111.70 | 2,531 | 1,219 | OK:30 | OK |
| tank_gunner_interior | remastered | 6.14 | 5.43 | 0.07 | 0.42 |  | 0.39 |  | 0.03 | 107.90 | 2,311 | 1,232 | OK:30 | OK |
| tank_gunner_interior | delta | +0.02 | -0.38 |  |  |  |  |  |  |  | -8.7% | +1.1% |  |  |
| tank_gunner_optics | classic | 5.97 | 5.42 |  |  |  |  |  |  | 119.90 | 1,729 | 743 | OK:30 | OK |
| tank_gunner_optics | remastered | 4.74 | 3.99 | 0.08 | 0.49 | 0.46 |  |  | 0.03 | 144.10 | 1,516 | 743 | OK:30 | OK |
| tank_gunner_optics | delta | -1.23 | -1.43 |  |  |  |  |  |  |  | -12.3% | +0.0% |  |  |
| overcast_broken | classic | 9.75 | 9.65 |  |  |  |  |  |  | 63.90 | 7,001 | 5,083 | OK:30 | OK |
| overcast_broken | remastered | 9.97 | 9.36 | 0.08 | 0.52 | 0.44 | 0.05 |  | 0.03 | 62.10 | 6,843 | 5,083 | OK:30 | OK |
| overcast_broken | delta | +0.22 | -0.29 |  |  |  |  |  |  |  | -2.3% | +0.0% |  |  |
| overcast_heavy | classic | 9.42 | 9.31 |  |  |  |  |  |  | 67.80 | 6,757 | 5,083 | OK:30 | OK |
| overcast_heavy | remastered | 10.30 | 9.63 | 0.09 | 0.56 | 0.49 | 0.05 |  | 0.03 | 63.50 | 6,599 | 5,083 | OK:30 | OK |
| overcast_heavy | delta | +0.88 | +0.32 |  |  |  |  |  |  |  | -2.3% | +0.0% |  |  |
| sinai_lowsun_fp | classic | 4.95 | 4.43 |  |  |  |  |  |  | 128.60 | 602 | 314 | OK:30 | OK |
| sinai_lowsun_fp | remastered | 5.28 | 3.14 | 0.35 | 1.75 | 0.95 | 0.17 | 0.47 | 0.17 | 133.10 | 582 | 314 | OK:30 | OK |
| sinai_lowsun_fp | delta | +0.33 | -1.29 |  |  |  |  |  |  |  | -3.3% | +0.0% |  |  |
| sinai_desert_noon | classic | 6.20 | 6.03 |  |  |  |  |  |  | 93.70 | 2,242 | 1,100 | OK:30 | OK |
| sinai_desert_noon | remastered | 6.14 | 5.23 | 0.06 | 0.80 | 0.72 | 0.05 |  | 0.03 | 93.10 | 2,231 | 1,100 | OK:30 | OK |
| sinai_desert_noon | delta | -0.06 | -0.80 |  |  |  |  |  |  |  | -0.5% | +0.0% |  |  |
