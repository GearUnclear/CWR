# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 6.25 | 5.83 |  |  |  |  |  |  | 116.40 | 1,778 | 1,179 | OK:30 | OK |
| lowsun_fp | remastered | 5.51 | 4.47 | 0.09 | 0.88 | 0.39 | 0.08 | 0.34 | 0.07 | 131.50 | 1,615 | 1,179 | OK:30 | OK |
| lowsun_fp | delta | -0.74 | -1.36 |  |  |  |  |  |  |  | -9.2% | +0.0% |  |  |
| sunrise_ridge | classic | 14.94 | 14.82 |  |  |  |  |  |  | 43.40 | 11,301 | 7,680 | OK:30 | OK |
| sunrise_ridge | remastered | 19.04 | 18.14 | 0.12 | 0.78 | 0.55 | 0.05 | 0.15 | 0.03 | 39.30 | 11,172 | 7,680 | OK:30 | OK |
| sunrise_ridge | delta | +4.10 | +3.32 |  |  |  |  |  |  |  | -1.1% | +0.0% |  |  |
| noon_fields | classic | 7.43 | 7.32 |  |  |  |  |  |  | 74.50 | 5,878 | 3,920 | OK:30 | OK |
| noon_fields | remastered | 9.81 | 9.42 | 0.25 | 0.27 | 0.20 | 0.05 |  | 0.03 | 75.60 | 5,575 | 3,920 | OK:30 | OK |
| noon_fields | delta | +2.38 | +2.10 |  |  |  |  |  |  |  | -5.2% | +0.0% |  |  |
| forest_edge_golden | classic | 7.30 | 7.18 |  |  |  |  |  |  | 175.40 | 2,189 | 1,316 | OK:30 | OK |
| forest_edge_golden | remastered | 7.22 | 6.74 | 0.18 | 0.34 | 0.27 | 0.05 |  | 0.03 | 98.40 | 1,847 | 1,316 | OK:30 | OK |
| forest_edge_golden | delta | -0.08 | -0.44 |  |  |  |  |  |  |  | -15.6% | +0.0% |  |  |
| coastline_sunset | classic | 0.76 | 0.66 |  |  |  |  |  |  | 472.60 | 155 | 0 | OK:30 | OK |
| coastline_sunset | remastered | 1.18 | 0.58 | 0.00 | 0.50 | 0.25 | 0.05 | 0.16 | 0.03 | 429.40 | 155 | 0 | OK:30 | OK |
| coastline_sunset | delta | +0.42 | -0.08 |  |  |  |  |  |  |  | +0.0% |  |  |  |
| town_night | classic | 5.69 | 5.54 |  |  |  |  |  |  | 128.90 | 1,232 | 1,070 | UNSETTLED:300 | OK |
| town_night | remastered | 5.64 | 4.94 |  | 0.53 | 0.46 | 0.05 |  | 0.03 | 135.50 | 1,226 | 1,073 | OK:41 | OK |
| town_night | delta | -0.05 | -0.60 |  |  |  |  |  |  |  | -0.5% | +0.3% |  |  |
| firefight_1 | classic | 3.13 | 2.93 |  |  |  |  |  |  | 251.70 | 450 | 377 | OK:45 | OK |
| firefight_1 | remastered | 2.50 | 1.53 |  | 0.81 | 0.72 | 0.06 |  | 0.03 | 216.40 | 432 | 373 | UNSETTLED:300 | OK |
| firefight_1 | delta | -0.63 | -1.40 |  |  |  |  |  |  |  | -4.0% | -1.1% |  |  |
| firefight_2 | classic | 3.13 | 2.93 |  |  |  |  |  |  | 251.70 | 547 | 474 | OK:45 | OK |
| firefight_2 | remastered | 2.50 | 1.53 |  | 0.81 | 0.72 | 0.06 |  | 0.03 | 216.40 | 499 | 457 | UNSETTLED:300 | OK |
| firefight_2 | delta | -0.63 | -1.40 |  |  |  |  |  |  |  | -8.8% | -3.6% |  |  |
| firefight_3 | classic | 3.13 | 2.93 |  |  |  |  |  |  | 251.70 | 438 | 345 | OK:45 | OK |
| firefight_3 | remastered | 2.50 | 1.53 |  | 0.81 | 0.72 | 0.06 |  | 0.03 | 216.40 | 452 | 349 | UNSETTLED:300 | OK |
| firefight_3 | delta | -0.63 | -1.40 |  |  |  |  |  |  |  | +3.1% | +1.1% |  |  |
| tank_gunner_interior | classic | 6.01 | 5.48 |  |  |  |  |  |  | 122.00 | 2,536 | 1,223 | OK:30 | OK |
| tank_gunner_interior | remastered | 5.97 | 5.26 | 0.06 | 0.45 |  | 0.43 |  | 0.02 | 121.60 | 2,317 | 1,237 | OK:30 | OK |
| tank_gunner_interior | delta | -0.04 | -0.22 |  |  |  |  |  |  |  | -8.6% | +1.1% |  |  |
| tank_gunner_optics | classic | 3.35 | 3.09 |  |  |  |  |  |  | 153.00 | 1,735 | 749 | OK:30 | OK |
| tank_gunner_optics | remastered | 4.88 | 4.14 | 0.07 | 0.48 | 0.46 |  |  | 0.03 | 144.30 | 1,521 | 748 | OK:30 | OK |
| tank_gunner_optics | delta | +1.53 | +1.05 |  |  |  |  |  |  |  | -12.3% | -0.1% |  |  |
| overcast_broken | classic | 9.49 | 9.39 |  |  |  |  |  |  | 61.30 | 7,001 | 5,083 | OK:30 | OK |
| overcast_broken | remastered | 11.15 | 10.47 | 0.09 | 0.57 | 0.49 | 0.05 |  | 0.03 | 56.40 | 6,843 | 5,083 | OK:30 | OK |
| overcast_broken | delta | +1.66 | +1.08 |  |  |  |  |  |  |  | -2.3% | +0.0% |  |  |
| overcast_heavy | classic | 9.29 | 9.18 |  |  |  |  |  |  | 63.10 | 6,757 | 5,083 | OK:30 | OK |
| overcast_heavy | remastered | 13.52 | 12.63 | 0.09 | 0.78 | 0.70 | 0.05 |  | 0.03 | 56.60 | 6,599 | 5,083 | OK:30 | OK |
| overcast_heavy | delta | +4.23 | +3.45 |  |  |  |  |  |  |  | -2.3% | +0.0% |  |  |
| sinai_lowsun_fp | classic | 3.82 | 3.71 |  |  |  |  |  |  | 158.10 | 603 | 314 | OK:30 | OK |
| sinai_lowsun_fp | remastered | 3.00 | 1.99 | 0.11 | 0.89 | 0.58 | 0.05 | 0.22 | 0.03 | 183.40 | 583 | 314 | OK:30 | OK |
| sinai_lowsun_fp | delta | -0.82 | -1.72 |  |  |  |  |  |  |  | -3.3% | +0.0% |  |  |
| sinai_desert_noon | classic | 5.84 | 5.72 |  |  |  |  |  |  | 97.80 | 2,242 | 1,100 | OK:30 | OK |
| sinai_desert_noon | remastered | 7.92 | 6.86 | 0.05 | 0.91 | 0.82 | 0.06 |  | 0.03 | 85.00 | 2,231 | 1,100 | OK:30 | OK |
| sinai_desert_noon | delta | +2.08 | +1.14 |  |  |  |  |  |  |  | -0.5% | +0.0% |  |  |
