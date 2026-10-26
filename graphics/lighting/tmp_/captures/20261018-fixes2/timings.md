# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 6.04 | 5.76 |  |  |  |  |  |  | 106.80 | 1,778 | 1,179 | OK:30 | OK |
| lowsun_fp | remastered | 4.47 | 3.54 | 0.07 | 0.81 | 0.40 | 0.06 | 0.28 | 0.08 | 146.50 | 1,615 | 1,179 | OK:30 | OK |
| lowsun_fp | delta | -1.57 | -2.22 |  |  |  |  |  |  |  | -9.2% | +0.0% |  |  |
| sunrise_ridge | classic | 14.26 | 14.15 |  |  |  |  |  |  | 44.10 | 11,301 | 7,680 | OK:30 | OK |
| sunrise_ridge | remastered | 17.93 | 17.08 | 0.09 | 0.73 | 0.51 | 0.05 | 0.15 | 0.03 | 43.60 | 11,172 | 7,680 | OK:30 | OK |
| sunrise_ridge | delta | +3.67 | +2.93 |  |  |  |  |  |  |  | -1.1% | +0.0% |  |  |
| noon_fields | classic | 7.00 | 6.90 |  |  |  |  |  |  | 83.60 | 5,878 | 3,920 | OK:30 | OK |
| noon_fields | remastered | 7.75 | 7.28 | 0.07 | 0.37 | 0.30 | 0.05 |  | 0.03 | 81.30 | 5,575 | 3,920 | OK:30 | OK |
| noon_fields | delta | +0.75 | +0.38 |  |  |  |  |  |  |  | -5.2% | +0.0% |  |  |
| forest_edge_golden | classic | 5.76 | 5.54 |  |  |  |  |  |  | 177.50 | 2,189 | 1,316 | OK:30 | OK |
| forest_edge_golden | remastered | 5.73 | 5.04 | 0.28 | 0.50 | 0.37 | 0.08 |  | 0.05 | 123.90 | 1,847 | 1,316 | OK:30 | OK |
| forest_edge_golden | delta | -0.03 | -0.50 |  |  |  |  |  |  |  | -15.6% | +0.0% |  |  |
| coastline_sunset | classic | 2.24 | 1.70 |  |  |  |  |  |  | 236.30 | 155 | 0 | OK:30 | OK |
| coastline_sunset | remastered | 4.49 | 1.95 | 0.00 | 2.07 | 0.74 | 0.38 | 0.61 | 0.34 | 203.70 | 155 | 0 | OK:30 | OK |
| coastline_sunset | delta | +2.25 | +0.25 |  |  |  |  |  |  |  | +0.0% |  |  |  |
| town_night | classic | 6.66 | 6.44 |  |  |  |  |  |  | 119.00 | 1,183 | 1,021 | OK:219 | OK |
| town_night | remastered | 6.71 | 5.92 |  | 0.60 | 0.50 | 0.07 |  | 0.04 | 119.60 | 1,198 | 1,036 | OK:95 | OK |
| town_night | delta | +0.05 | -0.52 |  |  |  |  |  |  |  | +1.3% | +1.4% |  |  |
| firefight_1 | classic | 5.36 | 4.74 |  |  |  |  |  |  | 160.40 | 417 | 385 | OK:46 | OK |
| firefight_1 | remastered | 5.35 | 3.53 |  | 1.26 | 1.04 | 0.13 |  | 0.10 | 142.60 | 470 | 425 | OK:82 | OK |
| firefight_1 | delta | -0.01 | -1.21 |  |  |  |  |  |  |  | +11.3% | +9.4% |  |  |
| firefight_2 | classic | 5.36 | 4.74 |  |  |  |  |  |  | 160.40 | 534 | 558 | OK:46 | OK |
| firefight_2 | remastered | 5.35 | 3.53 |  | 1.26 | 1.04 | 0.13 |  | 0.10 | 142.60 | 545 | 556 | OK:82 | OK |
| firefight_2 | delta | -0.01 | -1.21 |  |  |  |  |  |  |  | +2.0% | -0.4% |  |  |
| firefight_3 | classic | 5.36 | 4.74 |  |  |  |  |  |  | 160.40 | 565 | 515 | OK:46 | OK |
| firefight_3 | remastered | 5.35 | 3.53 |  | 1.26 | 1.04 | 0.13 |  | 0.10 | 142.60 | 568 | 559 | OK:82 | OK |
| firefight_3 | delta | -0.01 | -1.21 |  |  |  |  |  |  |  | +0.5% | +7.9% |  |  |
| tank_gunner_interior | classic | 5.67 | 5.38 |  |  |  |  |  |  | 118.00 | 2,533 | 1,220 | OK:30 | OK |
| tank_gunner_interior | remastered | 6.21 | 5.36 | 0.07 | 0.54 |  | 0.51 |  | 0.03 | 107.90 | 2,316 | 1,237 | OK:30 | OK |
| tank_gunner_interior | delta | +0.54 | -0.02 |  |  |  |  |  |  |  | -8.6% | +1.4% |  |  |
| tank_gunner_optics | classic | 6.83 | 6.24 |  |  |  |  |  |  | 113.00 | 1,724 | 743 | OK:30 | OK |
| tank_gunner_optics | remastered | 5.15 | 4.39 | 0.13 | 0.48 | 0.44 |  |  | 0.03 | 132.80 | 1,520 | 741 | OK:30 | OK |
| tank_gunner_optics | delta | -1.68 | -1.85 |  |  |  |  |  |  |  | -11.8% | -0.3% |  |  |
| overcast_broken | classic | 9.89 | 9.78 |  |  |  |  |  |  | 64.60 | 7,001 | 5,083 | OK:30 | OK |
| overcast_broken | remastered | 10.05 | 9.40 | 0.08 | 0.55 | 0.48 | 0.05 |  | 0.03 | 63.00 | 6,843 | 5,083 | OK:30 | OK |
| overcast_broken | delta | +0.16 | -0.38 |  |  |  |  |  |  |  | -2.3% | +0.0% |  |  |
| overcast_heavy | classic | 9.15 | 9.04 |  |  |  |  |  |  | 66.90 | 6,757 | 5,083 | OK:30 | OK |
| overcast_heavy | remastered | 9.86 | 9.23 | 0.08 | 0.53 | 0.46 | 0.05 |  | 0.03 | 65.40 | 6,599 | 5,083 | OK:30 | OK |
| overcast_heavy | delta | +0.71 | +0.19 |  |  |  |  |  |  |  | -2.3% | +0.0% |  |  |
| sinai_lowsun_fp | classic | 4.06 | 3.71 |  |  |  |  |  |  | 145.80 | 603 | 314 | OK:30 | OK |
| sinai_lowsun_fp | remastered | 4.94 | 2.95 | 0.37 | 1.68 | 0.85 | 0.15 | 0.57 | 0.11 | 137.10 | 583 | 314 | OK:30 | OK |
| sinai_lowsun_fp | delta | +0.88 | -0.76 |  |  |  |  |  |  |  | -3.3% | +0.0% |  |  |
| sinai_desert_noon | classic | 6.03 | 5.86 |  |  |  |  |  |  | 98.10 | 2,243 | 1,100 | OK:30 | OK |
| sinai_desert_noon | remastered | 6.45 | 5.45 | 0.05 | 0.87 | 0.76 | 0.07 |  | 0.04 | 92.70 | 2,232 | 1,100 | OK:30 | OK |
| sinai_desert_noon | delta | +0.42 | -0.41 |  |  |  |  |  |  |  | -0.5% | +0.0% |  |  |
