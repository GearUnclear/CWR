# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 11.18 | 11.08 |  |  |  |  |  |  | 72.30 | 1,778 | 1,179 | OK:30 | OK |
| lowsun_fp | remastered | 10.52 | 9.25 | 0.44 | 1.16 | 0.21 | 0.05 | 0.22 | 0.68 | 77.60 | 1,615 | 1,179 | OK:30 | OK |
| lowsun_fp | delta | -0.66 | -1.83 |  |  |  |  |  |  |  | -9.2% | +0.0% |  |  |
| sunrise_ridge | classic | 52.61 | 52.49 |  |  |  |  |  |  | 17.20 | 11,301 | 7,680 | OK:30 | OK |
| sunrise_ridge | remastered | 58.12 | 56.47 | 0.23 | 1.49 | 1.25 | 0.05 | 0.15 | 0.03 | 15.90 | 11,172 | 7,680 | OK:30 | OK |
| sunrise_ridge | delta | +5.51 | +3.98 |  |  |  |  |  |  |  | -1.1% | +0.0% |  |  |
| noon_fields | classic | 26.00 | 25.90 |  |  |  |  |  |  | 31.60 | 5,878 | 3,920 | OK:30 | OK |
| noon_fields | remastered | 23.80 | 23.15 | 0.07 | 0.54 | 0.46 | 0.05 |  | 0.03 | 33.20 | 5,575 | 3,920 | OK:30 | OK |
| noon_fields | delta | -2.20 | -2.75 |  |  |  |  |  |  |  | -5.2% | +0.0% |  |  |
| forest_edge_golden | classic | 15.11 | 14.93 |  |  |  |  |  |  | 164.10 | 2,189 | 1,316 | OK:30 | OK |
| forest_edge_golden | remastered | 6.58 | 6.14 | 0.18 | 0.30 | 0.23 | 0.05 |  | 0.03 | 106.20 | 1,847 | 1,316 | OK:30 | OK |
| forest_edge_golden | delta | -8.53 | -8.79 |  |  |  |  |  |  |  | -15.6% | +0.0% |  |  |
| coastline_sunset | classic | 0.90 | 0.72 |  |  |  |  |  |  | 401.30 | 155 | 0 | OK:30 | OK |
| coastline_sunset | remastered | 2.59 | 1.15 | 0.00 | 1.25 | 0.39 | 0.11 | 0.55 | 0.19 | 309.80 | 155 | 0 | OK:30 | OK |
| coastline_sunset | delta | +1.69 | +0.43 |  |  |  |  |  |  |  | +0.0% |  |  |  |
| town_night | classic | 5.20 | 5.02 |  |  |  |  |  |  | 128.90 | 1,106 | 929 | OK:155 | OK |
| town_night | remastered | 4.87 | 4.24 |  | 0.48 | 0.40 | 0.05 |  | 0.03 | 140.60 | 1,110 | 937 | OK:37 | OK |
| town_night | delta | -0.33 | -0.78 |  |  |  |  |  |  |  | +0.4% | +0.9% |  |  |
| firefight_1 | classic | 1.77 | 1.63 |  |  |  |  |  |  | 253.50 | 459 | 369 | OK:51 | OK |
| firefight_1 | remastered | 5.20 | 3.39 |  | 1.30 | 1.06 | 0.11 |  | 0.13 | 167.90 | 450 | 390 | OK:295 | OK |
| firefight_1 | delta | +3.43 | +1.76 |  |  |  |  |  |  |  | -2.0% | +5.4% |  |  |
| firefight_2 | classic | 1.77 | 1.63 |  |  |  |  |  |  | 253.50 | 513 | 447 | OK:51 | OK |
| firefight_2 | remastered | 5.20 | 3.39 |  | 1.30 | 1.06 | 0.11 |  | 0.13 | 167.90 | 515 | 473 | OK:295 | OK |
| firefight_2 | delta | +3.43 | +1.76 |  |  |  |  |  |  |  | +0.4% | +5.5% |  |  |
| firefight_3 | classic | 1.77 | 1.63 |  |  |  |  |  |  | 253.50 | 487 | 410 | OK:51 | OK |
| firefight_3 | remastered | 5.20 | 3.39 |  | 1.30 | 1.06 | 0.11 |  | 0.13 | 167.90 | 477 | 371 | OK:295 | OK |
| firefight_3 | delta | +3.43 | +1.76 |  |  |  |  |  |  |  | -2.1% | -9.5% |  |  |
| tank_gunner_interior | classic | 5.56 | 5.27 |  |  |  |  |  |  | 116.90 | 2,532 | 1,219 | OK:30 | OK |
| tank_gunner_interior | remastered | 6.65 | 5.93 | 0.08 | 0.41 |  | 0.38 |  | 0.03 | 108.40 | 2,312 | 1,233 | OK:30 | OK |
| tank_gunner_interior | delta | +1.09 | +0.66 |  |  |  |  |  |  |  | -8.7% | +1.1% |  |  |
| tank_gunner_optics | classic | 4.28 | 3.98 |  |  |  |  |  |  | 147.10 | 1,730 | 744 | OK:30 | OK |
| tank_gunner_optics | remastered | 9.75 | 8.45 | 0.14 | 0.65 | 0.60 |  |  | 0.05 | 67.50 | 1,516 | 743 | OK:30 | OK |
| tank_gunner_optics | delta | +5.47 | +4.47 |  |  |  |  |  |  |  | -12.4% | -0.1% |  |  |
| overcast_broken | classic | 17.57 | 17.45 |  |  |  |  |  |  | 48.60 | 7,001 | 5,083 | OK:30 | OK |
| overcast_broken | remastered | 11.47 | 10.72 | 0.08 | 0.65 | 0.58 | 0.05 |  | 0.03 | 33.90 | 6,843 | 5,083 | OK:30 | OK |
| overcast_broken | delta | -6.10 | -6.73 |  |  |  |  |  |  |  | -2.3% | +0.0% |  |  |
| overcast_heavy | classic | 8.87 | 8.76 |  |  |  |  |  |  | 70.10 | 6,757 | 5,083 | OK:30 | OK |
| overcast_heavy | remastered | 11.45 | 10.69 | 0.09 | 0.64 | 0.57 | 0.05 |  | 0.03 | 61.00 | 6,599 | 5,083 | OK:30 | OK |
| overcast_heavy | delta | +2.58 | +1.93 |  |  |  |  |  |  |  | -2.3% | +0.0% |  |  |
| sinai_lowsun_fp | classic | 4.74 | 4.24 |  |  |  |  |  |  | 134.80 | 603 | 314 | OK:30 | OK |
| sinai_lowsun_fp | remastered | 5.67 | 3.44 | 0.42 | 1.87 | 0.99 | 0.15 | 0.50 | 0.24 | 131.60 | 583 | 314 | OK:30 | OK |
| sinai_lowsun_fp | delta | +0.93 | -0.80 |  |  |  |  |  |  |  | -3.3% | +0.0% |  |  |
| sinai_desert_noon | classic | 5.99 | 5.83 |  |  |  |  |  |  | 93.80 | 2,243 | 1,100 | OK:30 | OK |
| sinai_desert_noon | remastered | 6.72 | 5.69 | 0.05 | 0.88 | 0.76 | 0.07 |  | 0.04 | 89.30 | 2,232 | 1,100 | OK:30 | OK |
| sinai_desert_noon | delta | +0.73 | -0.14 |  |  |  |  |  |  |  | -0.5% | +0.0% |  |  |
