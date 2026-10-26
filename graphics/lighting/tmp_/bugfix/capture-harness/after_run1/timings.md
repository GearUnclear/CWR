# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 5.91 | 5.63 |  |  |  |  |  |  | 111.50 | 1,575 | 1,179 | OK:30 | OK |
| lowsun_fp | remastered | 5.06 | 4.15 | 0.05 | 0.79 | 0.36 | 0.07 | 0.31 | 0.06 | 132.30 | 1,568 | 1,179 | OK:30 | OK |
| lowsun_fp | delta | -0.85 | -1.48 |  |  |  |  |  |  |  | -0.4% | +0.0% |  |  |
| sunrise_ridge | classic | 13.49 | 13.38 |  |  |  |  |  |  | 45.60 | 11,172 | 7,680 | OK:30 | OK |
| sunrise_ridge | remastered | 17.41 | 16.58 | 0.00 | 0.72 | 0.50 | 0.05 | 0.15 | 0.03 | 44.00 | 11,172 | 7,680 | OK:30 | OK |
| sunrise_ridge | delta | +3.92 | +3.20 |  |  |  |  |  |  |  | +0.0% | +0.0% |  |  |
| noon_fields | classic | 6.44 | 6.07 |  |  |  |  |  |  | 93.30 | 5,572 | 3,920 | OK:30 | OK |
| noon_fields | remastered | 7.71 | 7.24 | 0.04 | 0.38 | 0.30 | 0.05 |  | 0.03 | 82.70 | 5,567 | 3,920 | OK:30 | OK |
| noon_fields | delta | +1.27 | +1.17 |  |  |  |  |  |  |  | -0.1% | +0.0% |  |  |
| forest_edge_golden | classic | 4.78 | 4.43 |  |  |  |  |  |  | 229.10 | 1,850 | 1,316 | OK:30 | OK |
| forest_edge_golden | remastered | 4.96 | 4.36 | 0.05 | 0.44 | 0.33 | 0.07 |  | 0.04 | 131.20 | 1,847 | 1,316 | OK:30 | OK |
| forest_edge_golden | delta | +0.18 | -0.07 |  |  |  |  |  |  |  | -0.2% | +0.0% |  |  |
| coastline_sunset | classic | 3.40 | 2.78 |  |  |  |  |  |  | 236.40 | 155 | 0 | OK:30 | OK |
| coastline_sunset | remastered | 3.95 | 1.78 | 0.00 | 1.85 | 0.74 | 0.20 | 0.73 | 0.18 | 224.10 | 155 | 0 | OK:30 | OK |
| coastline_sunset | delta | +0.55 | -1.00 |  |  |  |  |  |  |  | +0.0% |  |  |  |
| town_night | classic | 6.85 | 6.47 |  |  |  |  |  |  | 107.20 | 1,124 | 964 | OK:131 | OK |
| town_night | remastered | 6.28 | 5.50 |  | 0.56 | 0.44 | 0.08 |  | 0.05 | 132.00 | 1,114 | 951 | OK:63 | OK |
| town_night | delta | -0.57 | -0.97 |  |  |  |  |  |  |  | -0.9% | -1.3% |  |  |
| firefight_1 | classic | 5.62 | 5.16 |  |  |  |  |  |  | 162.40 | 454 | 418 | OK:54 | OK |
| firefight_1 | remastered | 6.16 | 4.48 |  | 1.25 | 0.90 | 0.20 |  | 0.15 | 128.30 | 419 | 416 | OK:285 | OK |
| firefight_1 | delta | +0.54 | -0.68 |  |  |  |  |  |  |  | -7.7% | -0.5% |  |  |
| firefight_2 | classic | 5.62 | 5.16 |  |  |  |  |  |  | 162.40 | 498 | 534 | OK:54 | OK |
| firefight_2 | remastered | 6.16 | 4.48 |  | 1.25 | 0.90 | 0.20 |  | 0.15 | 128.30 | 477 | 489 | OK:285 | OK |
| firefight_2 | delta | +0.54 | -0.68 |  |  |  |  |  |  |  | -4.2% | -8.4% |  |  |
| firefight_3 | classic | 5.62 | 5.16 |  |  |  |  |  |  | 162.40 | 483 | 458 | OK:54 | OK |
| firefight_3 | remastered | 6.16 | 4.48 |  | 1.25 | 0.90 | 0.20 |  | 0.15 | 128.30 | 489 | 566 | OK:285 | OK |
| firefight_3 | delta | +0.54 | -0.68 |  |  |  |  |  |  |  | +1.2% | +19.1% |  |  |
| tank_gunner_interior | classic | 5.22 | 4.90 |  |  |  |  |  |  | 126.20 | 2,263 | 1,217 | OK:30 | OK |
| tank_gunner_interior | remastered | 6.70 | 5.99 | 0.06 | 0.42 |  | 0.39 |  | 0.03 | 103.50 | 2,276 | 1,234 | OK:30 | OK |
| tank_gunner_interior | delta | +1.48 | +1.09 |  |  |  |  |  |  |  | +0.6% | +1.4% |  |  |
| tank_gunner_optics | classic | 5.41 | 4.96 |  |  |  |  |  |  | 126.80 | 1,399 | 746 | OK:30 | OK |
| tank_gunner_optics | remastered | 5.22 | 4.48 | 0.06 | 0.46 | 0.43 |  |  | 0.04 | 117.90 | 1,383 | 746 | OK:30 | OK |
| tank_gunner_optics | delta | -0.19 | -0.48 |  |  |  |  |  |  |  | -1.1% | +0.0% |  |  |
| overcast_broken | classic | 8.81 | 8.71 |  |  |  |  |  |  | 69.20 | 6,837 | 5,083 | OK:30 | OK |
| overcast_broken | remastered | 9.66 | 9.03 | 0.03 | 0.54 | 0.47 | 0.05 |  | 0.03 | 66.20 | 6,835 | 5,083 | OK:30 | OK |
| overcast_broken | delta | +0.85 | +0.32 |  |  |  |  |  |  |  | -0.0% | +0.0% |  |  |
| overcast_heavy | classic | 8.41 | 8.30 |  |  |  |  |  |  | 74.30 | 6,593 | 5,083 | OK:30 | OK |
| overcast_heavy | remastered | 9.79 | 9.16 | 0.04 | 0.53 | 0.46 | 0.05 |  | 0.03 | 65.50 | 6,591 | 5,083 | OK:30 | OK |
| overcast_heavy | delta | +1.38 | +0.86 |  |  |  |  |  |  |  | -0.0% | +0.0% |  |  |
| sinai_lowsun_fp | classic | 5.83 | 5.46 |  |  |  |  |  |  | 99.20 | 573 | 314 | OK:30 | OK |
| sinai_lowsun_fp | remastered | 6.21 | 4.30 | 0.18 | 1.60 | 0.75 | 0.14 | 0.58 | 0.14 | 110.50 | 570 | 314 | OK:30 | OK |
| sinai_lowsun_fp | delta | +0.38 | -1.16 |  |  |  |  |  |  |  | -0.5% | +0.0% |  |  |
| sinai_desert_noon | classic | 6.40 | 6.25 |  |  |  |  |  |  | 91.80 | 2,233 | 1,100 | OK:30 | OK |
| sinai_desert_noon | remastered | 7.24 | 6.20 | 0.06 | 0.88 | 0.76 | 0.08 |  | 0.04 | 87.40 | 2,230 | 1,100 | OK:30 | OK |
| sinai_desert_noon | delta | +0.84 | -0.05 |  |  |  |  |  |  |  | -0.1% | +0.0% |  |  |
