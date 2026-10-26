# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | resolve | lindepth | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 10.58 | 10.48 |  |  |  |  |  |  |  |  | 78.70 | 1,778 | 1,179 | OK:30 | OK |
| lowsun_fp | remastered | 11.14 | 9.34 | 0.75 | 1.69 | 0.15 |  | 0.09 | 0.05 | 0.22 | 1.18 | 75.60 | 1,615 | 1,179 | OK:30 | OK |
| lowsun_fp | delta | +0.56 | -1.14 |  |  |  |  |  |  |  |  |  | -9.2% | +0.0% |  |  |
| sunrise_ridge | classic | 47.82 | 47.69 |  |  |  |  |  |  |  |  | 19.40 | 11,301 | 7,680 | OK:30 | OK |
| sunrise_ridge | remastered | 35.66 | 34.88 | 0.31 | 0.64 | 0.32 |  | 0.09 | 0.05 | 0.15 | 0.03 | 21.40 | 11,172 | 7,680 | OK:30 | OK |
| sunrise_ridge | delta | -12.16 | -12.81 |  |  |  |  |  |  |  |  |  | -1.1% | +0.0% |  |  |
| noon_fields | classic | 21.47 | 21.37 |  |  |  |  |  |  |  |  | 44.00 | 5,878 | 3,920 | OK:30 | OK |
| noon_fields | remastered | 12.39 | 11.90 | 0.07 | 0.37 | 0.22 |  | 0.08 | 0.05 |  | 0.03 | 50.60 | 5,575 | 3,920 | OK:30 | OK |
| noon_fields | delta | -9.08 | -9.47 |  |  |  |  |  |  |  |  |  | -5.2% | +0.0% |  |  |
| forest_edge_golden | classic | 3.93 | 3.81 |  |  |  |  |  |  |  |  | 185.60 | 2,189 | 1,316 | OK:30 | OK |
| forest_edge_golden | remastered | 7.12 | 6.46 | 0.18 | 0.51 | 0.35 |  | 0.09 | 0.05 |  | 0.03 | 106.10 | 1,847 | 1,316 | OK:30 | OK |
| forest_edge_golden | delta | +3.19 | +2.65 |  |  |  |  |  |  |  |  |  | -15.6% | +0.0% |  |  |
| coastline_sunset | classic | 0.72 | 0.62 |  |  |  |  |  |  |  |  | 489.70 | 155 | 0 | OK:30 | OK |
| coastline_sunset | remastered | 1.49 | 0.78 | 0.00 | 0.60 | 0.28 |  | 0.07 | 0.05 | 0.16 | 0.03 | 398.50 | 155 | 0 | OK:30 | OK |
| coastline_sunset | delta | +0.77 | +0.16 |  |  |  |  |  |  |  |  |  | +0.0% |  |  |  |
| town_night | classic | 5.95 | 5.78 |  |  |  |  |  |  |  |  | 106.10 | 1,247 | 1,079 | OK:281 | OK |
| town_night | remastered | 7.92 | 7.37 |  | 0.39 | 0.21 |  | 0.10 | 0.05 |  | 0.03 | 104.90 | 1,236 | 1,080 | OK:107 | OK |
| town_night | delta | +1.97 | +1.59 |  |  |  |  |  |  |  |  |  | -0.9% | +0.1% |  |  |
| firefight_1 | classic | 3.35 | 3.22 |  |  |  |  |  |  |  |  | 239.10 | 469 | 439 | OK:55 | OK |
| firefight_1 | remastered | 2.39 | 1.47 |  | 0.78 | 0.62 |  | 0.08 | 0.05 |  | 0.03 | 209.50 | 458 | 417 | UNSETTLED:300 | OK |
| firefight_1 | delta | -0.96 | -1.75 |  |  |  |  |  |  |  |  |  | -2.3% | -5.0% |  |  |
| firefight_2 | classic | 3.35 | 3.22 |  |  |  |  |  |  |  |  | 239.10 | 528 | 483 | OK:55 | OK |
| firefight_2 | remastered | 2.39 | 1.47 |  | 0.78 | 0.62 |  | 0.08 | 0.05 |  | 0.03 | 209.50 | 522 | 500 | UNSETTLED:300 | OK |
| firefight_2 | delta | -0.96 | -1.75 |  |  |  |  |  |  |  |  |  | -1.1% | +3.4% |  |  |
| firefight_3 | classic | 3.35 | 3.22 |  |  |  |  |  |  |  |  | 239.10 | 506 | 433 | OK:55 | OK |
| firefight_3 | remastered | 2.39 | 1.47 |  | 0.78 | 0.62 |  | 0.08 | 0.05 |  | 0.03 | 209.50 | 498 | 489 | UNSETTLED:300 | OK |
| firefight_3 | delta | -0.96 | -1.75 |  |  |  |  |  |  |  |  |  | -1.6% | +11.5% |  |  |
| tank_gunner_interior | classic | 8.75 | 8.05 |  |  |  |  |  |  |  |  | 81.20 | 2,530 | 1,220 | OK:30 | OK |
| tank_gunner_interior | remastered | 12.52 | 11.56 | 0.62 | 0.29 | 0.21 |  |  | 0.05 |  | 0.02 | 73.40 | 2,311 | 1,232 | OK:30 | OK |
| tank_gunner_interior | delta | +3.77 | +3.51 |  |  |  |  |  |  |  |  |  | -8.7% | +1.0% |  |  |
| tank_gunner_optics | classic | 4.98 | 4.66 |  |  |  |  |  |  |  |  | 133.60 | 1,730 | 744 | OK:30 | OK |
| tank_gunner_optics | remastered | 11.76 | 10.42 | 0.66 | 0.21 | 0.11 |  | 0.08 |  |  | 0.02 | 77.80 | 1,521 | 744 | OK:30 | OK |
| tank_gunner_optics | delta | +6.78 | +5.76 |  |  |  |  |  |  |  |  |  | -12.1% | +0.0% |  |  |
| overcast_broken | classic | 30.95 | 30.84 |  |  |  |  |  |  |  |  | 26.10 | 7,001 | 5,083 | OK:30 | OK |
| overcast_broken | remastered | 43.42 | 42.98 | 0.08 | 0.33 | 0.17 |  | 0.08 | 0.05 |  | 0.03 | 21.20 | 6,843 | 5,083 | OK:30 | OK |
| overcast_broken | delta | +12.47 | +12.14 |  |  |  |  |  |  |  |  |  | -2.3% | +0.0% |  |  |
| overcast_heavy | classic | 34.67 | 34.55 |  |  |  |  |  |  |  |  | 24.60 | 6,757 | 5,083 | OK:30 | OK |
| overcast_heavy | remastered | 34.47 | 33.71 | 0.61 | 0.64 | 0.48 |  | 0.08 | 0.05 |  | 0.03 | 24.70 | 6,599 | 5,083 | OK:30 | OK |
| overcast_heavy | delta | -0.20 | -0.84 |  |  |  |  |  |  |  |  |  | -2.3% | +0.0% |  |  |
| sinai_lowsun_fp | classic | 6.17 | 6.06 |  |  |  |  |  |  |  |  | 104.40 | 603 | 314 | OK:30 | OK |
| sinai_lowsun_fp | remastered | 6.55 | 5.18 | 0.46 | 1.25 | 0.12 |  | 0.10 | 0.05 | 0.22 | 0.74 | 125.70 | 583 | 314 | OK:30 | OK |
| sinai_lowsun_fp | delta | +0.38 | -0.88 |  |  |  |  |  |  |  |  |  | -3.3% | +0.0% |  |  |
| sinai_desert_noon | classic | 7.02 | 6.90 |  |  |  |  |  |  |  |  | 88.70 | 2,242 | 1,100 | OK:30 | OK |
| sinai_desert_noon | remastered | 7.51 | 6.59 | 0.12 | 0.80 | 0.64 |  | 0.08 | 0.05 |  | 0.03 | 76.80 | 2,231 | 1,100 | OK:30 | OK |
| sinai_desert_noon | delta | +0.49 | -0.31 |  |  |  |  |  |  |  |  |  | -0.5% | +0.0% |  |  |
