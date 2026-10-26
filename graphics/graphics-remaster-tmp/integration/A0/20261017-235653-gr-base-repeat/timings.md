# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 8.58 | 8.10 |  |  |  |  |  |  | 83.70 | 1,778 | 1,179 | OK:30 | OK |
| lowsun_fp | remastered | 8.53 | 6.81 | 0.18 | 1.37 | 0.78 | 0.12 | 0.35 | 0.12 | 88.00 | 1,615 | 1,179 | OK:30 | OK |
| lowsun_fp | delta | -0.05 | -1.29 |  |  |  |  |  |  |  | -9.2% | +0.0% |  |  |
| sunrise_ridge | classic | 14.21 | 14.10 |  |  |  |  |  |  | 44.50 | 11,301 | 7,680 | OK:30 | OK |
| sunrise_ridge | remastered | 18.10 | 17.26 | 0.09 | 0.73 | 0.50 | 0.05 | 0.15 | 0.03 | 43.50 | 11,172 | 7,680 | OK:30 | OK |
| sunrise_ridge | delta | +3.89 | +3.16 |  |  |  |  |  |  |  | -1.1% | +0.0% |  |  |
| noon_fields | classic | 9.00 | 8.87 |  |  |  |  |  |  | 73.70 | 5,878 | 3,920 | OK:30 | OK |
| noon_fields | remastered | 9.72 | 9.17 | 0.08 | 0.42 | 0.32 | 0.06 |  | 0.03 | 70.10 | 5,575 | 3,920 | OK:30 | OK |
| noon_fields | delta | +0.72 | +0.30 |  |  |  |  |  |  |  | -5.2% | +0.0% |  |  |
| forest_edge_golden | classic | 9.66 | 8.79 |  |  |  |  |  |  | 181.60 | 2,189 | 1,316 | OK:30 | OK |
| forest_edge_golden | remastered | 5.43 | 4.74 | 0.30 | 0.50 | 0.38 | 0.08 |  | 0.04 | 120.40 | 1,847 | 1,316 | OK:30 | OK |
| forest_edge_golden | delta | -4.23 | -4.05 |  |  |  |  |  |  |  | -15.6% | +0.0% |  |  |
| coastline_sunset | classic | 3.04 | 2.45 |  |  |  |  |  |  | 187.90 | 155 | 0 | OK:30 | OK |
| coastline_sunset | remastered | 6.92 | 2.82 | 0.00 | 3.50 | 1.44 | 0.51 | 1.01 | 0.54 | 151.00 | 155 | 0 | OK:30 | OK |
| coastline_sunset | delta | +3.88 | +0.37 |  |  |  |  |  |  |  | +0.0% |  |  |  |
| town_night | classic | 8.81 | 8.28 |  |  |  |  |  |  | 86.20 | 1,212 | 1,047 | OK:214 | OK |
| town_night | remastered | 7.77 | 6.90 |  | 0.64 | 0.49 | 0.08 |  | 0.06 | 114.60 | 1,219 | 1,061 | OK:53 | OK |
| town_night | delta | -1.04 | -1.38 |  |  |  |  |  |  |  | +0.6% | +1.3% |  |  |
| firefight_1 | classic | 5.57 | 4.82 |  |  |  |  |  |  | 165.70 | 441 | 431 | OK:47 | OK |
| firefight_1 | remastered | 7.33 | 4.46 |  | 1.95 | 1.22 | 0.56 |  | 0.18 | 117.90 | 476 | 435 | UNSETTLED:300 | OK |
| firefight_1 | delta | +1.76 | -0.36 |  |  |  |  |  |  |  | +7.4% | +0.9% |  |  |
| firefight_2 | classic | 5.57 | 4.82 |  |  |  |  |  |  | 165.70 | 563 | 537 | OK:47 | OK |
| firefight_2 | remastered | 7.33 | 4.46 |  | 1.95 | 1.22 | 0.56 |  | 0.18 | 117.90 | 540 | 531 | UNSETTLED:300 | OK |
| firefight_2 | delta | +1.76 | -0.36 |  |  |  |  |  |  |  | -4.1% | -1.1% |  |  |
| firefight_3 | classic | 5.57 | 4.82 |  |  |  |  |  |  | 165.70 | 479 | 425 | OK:47 | OK |
| firefight_3 | remastered | 7.33 | 4.46 |  | 1.95 | 1.22 | 0.56 |  | 0.18 | 117.90 | 542 | 489 | UNSETTLED:300 | OK |
| firefight_3 | delta | +1.76 | -0.36 |  |  |  |  |  |  |  | +11.6% | +13.1% |  |  |
| tank_gunner_interior | classic | 6.78 | 6.35 |  |  |  |  |  |  | 101.30 | 2,527 | 1,214 | OK:30 | OK |
| tank_gunner_interior | remastered | 4.98 | 4.35 | 0.06 | 0.40 |  | 0.37 |  | 0.02 | 129.30 | 2,309 | 1,232 | OK:30 | OK |
| tank_gunner_interior | delta | -1.80 | -2.00 |  |  |  |  |  |  |  | -8.6% | +1.5% |  |  |
| tank_gunner_optics | classic | 3.84 | 3.53 |  |  |  |  |  |  | 125.70 | 1,728 | 741 | OK:30 | OK |
| tank_gunner_optics | remastered | 6.36 | 5.42 | 0.15 | 0.52 | 0.47 |  |  | 0.05 | 137.00 | 1,515 | 741 | OK:30 | OK |
| tank_gunner_optics | delta | +2.52 | +1.89 |  |  |  |  |  |  |  | -12.3% | +0.0% |  |  |
| overcast_broken | classic | 9.52 | 9.42 |  |  |  |  |  |  | 64.40 | 7,001 | 5,083 | OK:30 | OK |
| overcast_broken | remastered | 9.69 | 9.06 | 0.09 | 0.53 | 0.45 | 0.05 |  | 0.03 | 64.60 | 6,843 | 5,083 | OK:30 | OK |
| overcast_broken | delta | +0.17 | -0.36 |  |  |  |  |  |  |  | -2.3% | +0.0% |  |  |
| overcast_heavy | classic | 8.94 | 8.83 |  |  |  |  |  |  | 68.90 | 6,757 | 5,083 | OK:30 | OK |
| overcast_heavy | remastered | 9.80 | 9.18 | 0.08 | 0.52 | 0.44 | 0.05 |  | 0.03 | 65.90 | 6,599 | 5,083 | OK:30 | OK |
| overcast_heavy | delta | +0.86 | +0.35 |  |  |  |  |  |  |  | -2.3% | +0.0% |  |  |
| sinai_lowsun_fp | classic | 5.35 | 4.90 |  |  |  |  |  |  | 130.90 | 603 | 314 | OK:30 | OK |
| sinai_lowsun_fp | remastered | 2.84 | 1.85 | 0.12 | 0.86 | 0.51 | 0.05 | 0.24 | 0.06 | 186.90 | 583 | 314 | OK:30 | OK |
| sinai_lowsun_fp | delta | -2.51 | -3.05 |  |  |  |  |  |  |  | -3.3% | +0.0% |  |  |
| sinai_desert_noon | classic | 6.18 | 6.03 |  |  |  |  |  |  | 93.20 | 2,243 | 1,100 | OK:30 | OK |
| sinai_desert_noon | remastered | 7.08 | 6.04 | 0.05 | 0.89 | 0.78 | 0.07 |  | 0.04 | 85.10 | 2,232 | 1,100 | OK:30 | OK |
| sinai_desert_noon | delta | +0.90 | +0.01 |  |  |  |  |  |  |  | -0.5% | +0.0% |  |  |
