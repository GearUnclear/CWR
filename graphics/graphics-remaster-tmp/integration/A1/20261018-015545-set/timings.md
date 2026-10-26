# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 6.03 | 5.92 |  |  |  |  |  |  | 116.30 | 1,778 | 1,179 | OK:30 | OK |
| lowsun_fp | remastered | 4.44 | 3.60 | 0.08 | 0.73 | 0.29 | 0.05 | 0.22 | 0.18 | 120.70 | 1,615 | 1,179 | OK:30 | OK |
| lowsun_fp | delta | -1.59 | -2.32 |  |  |  |  |  |  |  | -9.2% | +0.0% |  |  |
| sunrise_ridge | classic | 20.93 | 20.80 |  |  |  |  |  |  | 33.70 | 11,301 | 7,680 | OK:30 | OK |
| sunrise_ridge | remastered | 23.00 | 22.16 | 0.12 | 0.71 | 0.48 | 0.05 | 0.15 | 0.03 | 34.80 | 11,172 | 7,680 | OK:30 | OK |
| sunrise_ridge | delta | +2.07 | +1.36 |  |  |  |  |  |  |  | -1.1% | +0.0% |  |  |
| noon_fields | classic | 7.46 | 7.35 |  |  |  |  |  |  | 75.40 | 5,878 | 3,920 | OK:30 | OK |
| noon_fields | remastered | 12.05 | 11.56 | 0.07 | 0.38 | 0.31 | 0.05 |  | 0.03 | 60.10 | 5,575 | 3,920 | OK:30 | OK |
| noon_fields | delta | +4.59 | +4.21 |  |  |  |  |  |  |  | -5.2% | +0.0% |  |  |
| forest_edge_golden | classic | 5.35 | 5.23 |  |  |  |  |  |  | 183.70 | 2,189 | 1,316 | OK:30 | OK |
| forest_edge_golden | remastered | 4.30 | 3.75 | 0.20 | 0.40 | 0.32 | 0.05 |  | 0.03 | 132.40 | 1,847 | 1,316 | OK:30 | OK |
| forest_edge_golden | delta | -1.05 | -1.48 |  |  |  |  |  |  |  | -15.6% | +0.0% |  |  |
| coastline_sunset | classic | 1.04 | 0.92 |  |  |  |  |  |  | 396.30 | 155 | 0 | OK:30 | OK |
| coastline_sunset | remastered | 1.43 | 0.77 | 0.00 | 0.55 | 0.31 | 0.05 | 0.16 | 0.03 | 432.10 | 155 | 0 | OK:30 | OK |
| coastline_sunset | delta | +0.39 | -0.15 |  |  |  |  |  |  |  | +0.0% |  |  |  |
| town_night | classic | 6.93 | 6.77 |  |  |  |  |  |  | 114.50 | 1,228 | 1,065 | OK:239 | OK |
| town_night | remastered | 5.79 | 5.17 |  | 0.46 | 0.39 | 0.05 |  | 0.03 | 120.50 | 1,209 | 1,063 | OK:43 | OK |
| town_night | delta | -1.14 | -1.60 |  |  |  |  |  |  |  | -1.5% | -0.2% |  |  |
| firefight_1 | classic | 1.89 | 1.75 |  |  |  |  |  |  | 266.50 | 460 | 413 | OK:42 | OK |
| firefight_1 | remastered | 3.20 | 2.25 |  | 0.79 | 0.69 | 0.06 |  | 0.04 | 226.50 | 501 | 398 | OK:94 | OK |
| firefight_1 | delta | +1.31 | +0.50 |  |  |  |  |  |  |  | +8.2% | -3.6% |  |  |
| firefight_2 | classic | 1.89 | 1.75 |  |  |  |  |  |  | 266.50 | 530 | 521 | OK:42 | OK |
| firefight_2 | remastered | 3.20 | 2.25 |  | 0.79 | 0.69 | 0.06 |  | 0.04 | 226.50 | 540 | 502 | OK:94 | OK |
| firefight_2 | delta | +1.31 | +0.50 |  |  |  |  |  |  |  | +1.9% | -3.6% |  |  |
| firefight_3 | classic | 1.89 | 1.75 |  |  |  |  |  |  | 266.50 | 468 | 420 | OK:42 | OK |
| firefight_3 | remastered | 3.20 | 2.25 |  | 0.79 | 0.69 | 0.06 |  | 0.04 | 226.50 | 515 | 443 | OK:94 | OK |
| firefight_3 | delta | +1.31 | +0.50 |  |  |  |  |  |  |  | +9.1% | +5.2% |  |  |
| tank_gunner_interior | classic | 6.92 | 6.68 |  |  |  |  |  |  | 101.60 | 2,529 | 1,217 | OK:30 | OK |
| tank_gunner_interior | remastered | 7.93 | 7.25 | 0.11 | 0.44 |  | 0.42 |  | 0.02 | 110.10 | 2,313 | 1,234 | OK:30 | OK |
| tank_gunner_interior | delta | +1.01 | +0.57 |  |  |  |  |  |  |  | -8.5% | +1.4% |  |  |
| tank_gunner_optics | classic | 6.76 | 6.10 |  |  |  |  |  |  | 145.40 | 1,726 | 740 | OK:30 | OK |
| tank_gunner_optics | remastered | 3.54 | 2.90 | 0.06 | 0.38 | 0.36 |  |  | 0.02 | 157.20 | 1,510 | 740 | OK:30 | OK |
| tank_gunner_optics | delta | -3.22 | -3.20 |  |  |  |  |  |  |  | -12.5% | +0.0% |  |  |
| overcast_broken | classic | 12.49 | 12.39 |  |  |  |  |  |  | 56.90 | 7,001 | 5,083 | OK:30 | OK |
| overcast_broken | remastered | 17.61 | 17.02 | 0.15 | 0.49 | 0.42 | 0.05 |  | 0.03 | 44.30 | 6,843 | 5,083 | OK:30 | OK |
| overcast_broken | delta | +5.12 | +4.63 |  |  |  |  |  |  |  | -2.3% | +0.0% |  |  |
| overcast_heavy | classic | 9.76 | 9.65 |  |  |  |  |  |  | 62.00 | 6,757 | 5,083 | OK:30 | OK |
| overcast_heavy | remastered | 11.56 | 10.77 | 0.08 | 0.66 | 0.58 | 0.05 |  | 0.03 | 58.10 | 6,599 | 5,083 | OK:30 | OK |
| overcast_heavy | delta | +1.80 | +1.12 |  |  |  |  |  |  |  | -2.3% | +0.0% |  |  |
| sinai_lowsun_fp | classic | 3.80 | 3.69 |  |  |  |  |  |  | 190.80 | 603 | 314 | OK:30 | OK |
| sinai_lowsun_fp | remastered | 4.10 | 3.01 | 0.13 | 0.97 | 0.37 | 0.05 | 0.22 | 0.33 | 173.90 | 583 | 314 | OK:30 | OK |
| sinai_lowsun_fp | delta | +0.30 | -0.68 |  |  |  |  |  |  |  | -3.3% | +0.0% |  |  |
| sinai_desert_noon | classic | 7.87 | 7.76 |  |  |  |  |  |  | 80.30 | 2,243 | 1,100 | OK:30 | OK |
| sinai_desert_noon | remastered | 8.79 | 8.12 | 0.16 | 0.55 | 0.47 | 0.05 |  | 0.03 | 72.60 | 2,232 | 1,100 | OK:30 | OK |
| sinai_desert_noon | delta | +0.92 | +0.36 |  |  |  |  |  |  |  | -0.5% | +0.0% |  |  |
