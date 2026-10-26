# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 9.15 | 9.04 |  | 0.07 | 3.02 | 5.96 |  |  |  |  |  |  |  |  |  |  | 2.95 | 111.10 | 1,778 | 1,179 | OK:30 | OK |
| lowsun_fp | remastered | 8.61 | 6.73 | 0.19 | 1.11 | 3.19 | 2.42 | 0.07 |  |  | 1.69 | 0.47 | 0.01 | 0.09 | 0.05 |  | 0.03 | 6.50 | 70.70 | 1,091 | 1,179 | OK:30 | OK |
| lowsun_fp | delta | -0.54 | -2.31 |  | +1.04 | +0.17 | -3.54 |  |  |  |  |  |  |  |  |  |  | +3.55 |  | -38.6% | +0.0% |  |  |
| sunrise_ridge | classic | 14.28 | 14.17 |  | 0.10 | 12.74 | 1.34 |  |  |  |  |  |  |  |  |  |  | 13.72 | 41.10 | 11,301 | 7,680 | OK:30 | OK |
| sunrise_ridge | remastered | 5.22 | 3.58 | 0.15 | 1.38 | 1.81 | 0.38 | 0.10 |  |  | 1.35 | 0.35 | 0.01 | 0.10 | 0.06 |  | 0.03 | 3.92 | 74.40 | 1,660 | 7,680 | OK:30 | OK |
| sunrise_ridge | delta | -9.06 | -10.59 |  | +1.28 | -10.93 | -0.96 |  |  |  |  |  |  |  |  |  |  | -9.80 |  | -85.3% | +0.0% |  |  |
| noon_fields | classic | 7.29 | 7.20 |  | 0.08 | 5.46 | 1.66 |  |  |  |  |  |  |  |  |  |  | 6.93 | 80.60 | 5,878 | 3,920 | OK:30 | OK |
| noon_fields | remastered | 7.12 | 6.30 | 0.10 | 1.14 | 4.78 | 0.38 | 0.07 |  |  | 0.63 | 0.27 | 0.01 | 0.08 | 0.05 |  | 0.02 | 6.78 | 83.10 | 1,447 | 3,920 | OK:30 | OK |
| noon_fields | delta | -0.17 | -0.90 |  | +1.06 | -0.68 | -1.28 |  |  |  |  |  |  |  |  |  |  | -0.15 |  | -75.4% | +0.0% |  |  |
| forest_edge_golden | classic | 5.62 | 5.46 |  | 0.09 | 3.22 | 2.15 |  |  |  |  |  |  |  |  |  |  | 4.90 | 179.00 | 2,189 | 1,316 | OK:30 | OK |
| forest_edge_golden | remastered | 6.22 | 4.98 | 0.25 | 1.32 | 2.72 | 0.94 | 0.12 |  |  | 0.90 | 0.28 | 0.01 | 0.12 | 0.07 |  | 0.03 | 5.88 | 105.10 | 757 | 1,316 | OK:30 | OK |
| forest_edge_golden | delta | +0.60 | -0.48 |  | +1.23 | -0.50 | -1.21 |  |  |  |  |  |  |  |  |  |  | +0.98 |  | -65.4% | +0.0% |  |  |
| coastline_sunset | classic | 2.90 | 2.57 |  | 1.20 | 0.00 | 1.37 |  |  |  |  |  |  |  |  |  |  | 0.75 | 217.00 | 155 | 0 | OK:30 | OK |
| coastline_sunset | remastered | 3.97 | 1.83 | 0.00 | 1.60 | 0.00 | 0.23 | 0.11 |  |  | 1.88 | 0.10 | 0.01 | 0.10 | 0.07 |  | 0.04 | 3.56 | 184.20 | 155 | 0 | OK:30 | OK |
| coastline_sunset | delta | +1.07 | -0.74 |  | +0.40 | +0.00 | -1.14 |  |  |  |  |  |  |  |  |  |  | +2.81 |  | +0.0% |  |  |  |
| town_night | classic | 6.52 | 6.31 |  | 0.09 | 1.32 | 4.91 |  |  |  |  |  |  |  |  |  |  | 5.56 | 125.50 | 1,189 | 1,022 | OK:220 | OK |
| town_night | remastered | 8.37 | 7.31 | 0.14 | 0.91 | 2.68 | 3.73 | 0.04 |  |  | 0.73 | 0.32 | 0.01 | 0.10 | 0.05 |  | 0.03 | 7.78 | 88.70 | 1,135 | 1,032 | OK:40 | OK |
| town_night | delta | +1.85 | +1.00 |  | +0.82 | +1.36 | -1.18 |  |  |  |  |  |  |  |  |  |  | +2.22 |  | -4.5% | +1.0% |  |  |
| firefight_1 | classic | 4.58 | 4.12 |  | 0.24 | 0.59 | 3.28 |  |  |  |  |  |  |  |  |  |  | 0.79 | 184.20 | 464 | 448 | OK:48 | OK |
| firefight_1 | remastered | 6.63 | 4.67 |  | 0.82 | 1.91 | 1.94 | 0.14 |  |  | 1.41 | 0.58 | 0.01 | 0.13 | 0.08 |  | 0.09 | 4.68 | 142.40 | 446 | 483 | UNSETTLED:300 | OK |
| firefight_1 | delta | +2.05 | +0.55 |  | +0.58 | +1.32 | -1.34 |  |  |  |  |  |  |  |  |  |  | +3.89 |  | -3.9% | +7.2% |  |  |
| firefight_2 | classic | 4.58 | 4.12 |  | 0.24 | 0.59 | 3.28 |  |  |  |  |  |  |  |  |  |  | 0.79 | 184.20 | 532 | 526 | OK:48 | OK |
| firefight_2 | remastered | 6.63 | 4.67 |  | 0.82 | 1.91 | 1.94 | 0.14 |  |  | 1.41 | 0.58 | 0.01 | 0.13 | 0.08 |  | 0.09 | 4.68 | 142.40 | 484 | 476 | UNSETTLED:300 | OK |
| firefight_2 | delta | +2.05 | +0.55 |  | +0.58 | +1.32 | -1.34 |  |  |  |  |  |  |  |  |  |  | +3.89 |  | -9.0% | -9.5% |  |  |
| firefight_3 | classic | 4.58 | 4.12 |  | 0.24 | 0.59 | 3.28 |  |  |  |  |  |  |  |  |  |  | 0.79 | 184.20 | 483 | 432 | OK:48 | OK |
| firefight_3 | remastered | 6.63 | 4.67 |  | 0.82 | 1.91 | 1.94 | 0.14 |  |  | 1.41 | 0.58 | 0.01 | 0.13 | 0.08 |  | 0.09 | 4.68 | 142.40 | 576 | 579 | UNSETTLED:300 | OK |
| firefight_3 | delta | +2.05 | +0.55 |  | +0.58 | +1.32 | -1.34 |  |  |  |  |  |  |  |  |  |  | +3.89 |  | +16.1% | +25.4% |  |  |
| tank_gunner_interior | classic | 5.45 | 5.16 |  | 0.10 | 2.25 | 2.81 |  |  |  |  |  |  |  |  |  |  | 4.55 | 119.90 | 2,536 | 1,220 | OK:30 | OK |
| tank_gunner_interior | remastered | 6.55 | 5.80 | 0.06 | 0.58 | 2.49 | 2.72 | 0.06 |  |  | 0.48 | 0.37 |  |  | 0.05 |  | 0.02 | 6.24 | 90.20 | 1,534 | 1,232 | OK:30 | OK |
| tank_gunner_interior | delta | +1.10 | +0.64 |  | +0.48 | +0.24 | -0.09 |  |  |  |  |  |  |  |  |  |  | +1.69 |  | -39.5% | +1.0% |  |  |
| tank_gunner_optics | classic | 4.11 | 3.77 |  | 0.19 | 1.78 | 1.79 |  |  |  |  |  |  |  |  |  |  | 3.47 | 148.80 | 1,721 | 743 | OK:30 | OK |
| tank_gunner_optics | remastered | 7.44 | 6.58 | 0.08 | 0.84 | 3.53 | 2.20 | 0.10 |  |  | 0.55 | 0.31 |  | 0.11 |  |  | 0.03 | 7.10 | 110.90 | 1,143 | 742 | OK:30 | OK |
| tank_gunner_optics | delta | +3.33 | +2.81 |  | +0.65 | +1.75 | +0.41 |  |  |  |  |  |  |  |  |  |  | +3.63 |  | -33.6% | -0.1% |  |  |
| overcast_broken | classic | 10.13 | 10.03 |  | 0.11 | 7.47 | 2.45 |  |  |  |  |  |  |  |  |  |  | 9.71 | 57.00 | 7,001 | 5,083 | OK:30 | OK |
| overcast_broken | remastered | 6.84 | 5.63 | 0.15 | 1.67 | 3.54 | 0.42 | 0.07 |  |  | 1.01 | 0.44 | 0.01 | 0.08 | 0.05 |  | 0.03 | 6.37 | 74.30 | 1,863 | 5,083 | OK:30 | OK |
| overcast_broken | delta | -3.29 | -4.40 |  | +1.56 | -3.93 | -2.03 |  |  |  |  |  |  |  |  |  |  | -3.34 |  | -73.4% | +0.0% |  |  |
| overcast_heavy | classic | 9.20 | 9.10 |  | 0.09 | 7.23 | 1.77 |  |  |  |  |  |  |  |  |  |  | 8.59 | 66.70 | 6,757 | 5,083 | OK:30 | OK |
| overcast_heavy | remastered | 6.67 | 5.38 | 0.14 | 1.46 | 3.52 | 0.40 | 0.07 |  |  | 1.07 | 0.55 | 0.01 | 0.08 | 0.05 |  | 0.02 | 6.11 | 75.10 | 1,619 | 5,083 | OK:30 | OK |
| overcast_heavy | delta | -2.53 | -3.72 |  | +1.37 | -3.71 | -1.37 |  |  |  |  |  |  |  |  |  |  | -2.48 |  | -76.0% | +0.0% |  |  |
| sinai_lowsun_fp | classic | 2.43 | 2.32 |  | 0.08 | 1.27 | 0.96 |  |  |  |  |  |  |  |  |  |  | 2.06 | 197.90 | 603 | 314 | OK:30 | OK |
| sinai_lowsun_fp | remastered | 6.64 | 4.79 | 0.27 | 1.12 | 2.52 | 1.14 | 0.10 |  |  | 1.55 | 0.49 | 0.01 | 0.14 | 0.07 |  | 0.04 | 5.67 | 116.20 | 418 | 314 | UNSETTLED:300 | OK |
| sinai_lowsun_fp | delta | +4.21 | +2.47 |  | +1.04 | +1.25 | +0.18 |  |  |  |  |  |  |  |  |  |  | +3.61 |  | -30.7% | +0.0% |  |  |
| sinai_desert_noon | classic | 5.93 | 5.79 |  | 0.15 | 3.05 | 2.59 |  |  |  |  |  |  |  |  |  |  | 5.03 | 89.10 | 2,243 | 1,100 | OK:30 | OK |
| sinai_desert_noon | remastered | 6.80 | 5.28 | 0.11 | 0.94 | 2.68 | 1.66 | 0.09 |  |  | 1.26 | 0.56 | 0.01 | 0.10 | 0.06 |  | 0.03 | 6.26 | 87.30 | 1,181 | 1,100 | OK:30 | OK |
| sinai_desert_noon | delta | +0.87 | -0.51 |  | +0.79 | -0.37 | -0.93 |  |  |  |  |  |  |  |  |  |  | +1.23 |  | -47.3% | +0.0% |  |  |
