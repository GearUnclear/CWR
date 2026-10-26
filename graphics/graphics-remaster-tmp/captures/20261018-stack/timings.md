# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 4.51 | 4.34 |  | 0.14 | 1.99 | 2.20 |  |  |  |  |  |  |  |  |  |  | 3.98 | 136.50 | 1,778 | 1,179 | OK:30 | OK |
| lowsun_fp | remastered | 5.89 | 4.87 | 0.16 | 0.82 | 2.48 | 1.57 | 0.06 |  |  | 0.85 | 0.30 |  | 0.09 | 0.05 | 0.22 | 0.03 | 5.52 | 115.00 | 1,091 | 1,179 | OK:30 | OK |
| lowsun_fp | delta | +1.38 | +0.53 |  | +0.68 | +0.49 | -0.63 |  |  |  |  |  |  |  |  |  |  | +1.54 |  | -38.6% | +0.0% |  |  |
| sunrise_ridge | classic | 14.54 | 14.43 |  | 0.10 | 12.92 | 1.41 |  |  |  |  |  |  |  |  |  |  | 13.95 | 41.10 | 11,301 | 7,680 | OK:30 | OK |
| sunrise_ridge | remastered | 4.90 | 3.66 | 0.13 | 1.51 | 1.81 | 0.34 | 0.10 |  |  | 0.98 | 0.38 |  | 0.12 | 0.07 | 0.20 | 0.04 | 3.47 | 75.00 | 1,661 | 7,680 | OK:30 | OK |
| sunrise_ridge | delta | -9.64 | -10.77 |  | +1.41 | -11.11 | -1.07 |  |  |  |  |  |  |  |  |  |  | -10.48 |  | -85.3% | +0.0% |  |  |
| noon_fields | classic | 7.23 | 7.13 |  | 0.08 | 5.37 | 1.69 |  |  |  |  |  |  |  |  |  |  | 7.00 | 79.40 | 5,878 | 3,920 | OK:30 | OK |
| noon_fields | remastered | 6.77 | 6.03 | 0.10 | 1.04 | 4.56 | 0.42 | 0.07 |  |  | 0.57 | 0.29 |  | 0.08 | 0.05 |  | 0.02 | 6.37 | 86.50 | 1,447 | 3,920 | OK:30 | OK |
| noon_fields | delta | -0.46 | -1.10 |  | +0.96 | -0.81 | -1.27 |  |  |  |  |  |  |  |  |  |  | -0.63 |  | -75.4% | +0.0% |  |  |
| forest_edge_golden | classic | 5.88 | 5.69 |  | 0.10 | 3.25 | 2.34 |  |  |  |  |  |  |  |  |  |  | 5.25 | 177.80 | 2,189 | 1,316 | OK:30 | OK |
| forest_edge_golden | remastered | 4.30 | 3.49 | 0.24 | 0.92 | 1.85 | 0.73 | 0.08 |  |  | 0.58 | 0.23 |  | 0.10 | 0.05 |  | 0.03 | 4.09 | 128.90 | 757 | 1,316 | OK:30 | OK |
| forest_edge_golden | delta | -1.58 | -2.20 |  | +0.82 | -1.40 | -1.61 |  |  |  |  |  |  |  |  |  |  | -1.16 |  | -65.4% | +0.0% |  |  |
| coastline_sunset | classic | 1.90 | 1.54 |  | 1.07 | 0.00 | 0.47 |  |  |  |  |  |  |  |  |  |  | 0.84 | 308.50 | 155 | 0 | OK:30 | OK |
| coastline_sunset | remastered | 3.02 | 1.76 | 0.00 | 1.52 | 0.00 | 0.23 | 0.10 |  |  | 1.04 | 0.34 |  | 0.10 | 0.07 | 0.24 | 0.07 | 2.63 | 208.30 | 155 | 0 | OK:30 | OK |
| coastline_sunset | delta | +1.12 | +0.22 |  | +0.45 | +0.00 | -0.24 |  |  |  |  |  |  |  |  |  |  | +1.79 |  | +0.0% |  |  |  |
| town_night | classic | 7.11 | 6.86 |  | 0.12 | 1.26 | 5.49 |  |  |  |  |  |  |  |  |  |  | 6.15 | 115.70 | 1,236 | 1,079 | UNSETTLED:300 | OK |
| town_night | remastered | 6.66 | 5.86 |  | 0.64 | 1.73 | 3.48 | 0.05 |  |  | 0.65 | 0.33 |  | 0.11 | 0.05 |  | 0.02 | 6.37 | 104.10 | 1,193 | 1,078 | UNSETTLED:300 | OK |
| town_night | delta | -0.45 | -1.00 |  | +0.52 | +0.47 | -2.01 |  |  |  |  |  |  |  |  |  |  | +0.22 |  | -3.5% | -0.1% |  |  |
| firefight_1 | classic | 4.91 | 4.32 |  | 0.28 | 0.77 | 3.27 |  |  |  |  |  |  |  |  |  |  | 0.79 | 187.50 | 418 | 364 | OK:46 | OK |
| firefight_1 | remastered | 5.61 | 3.98 |  | 0.73 | 1.38 | 1.86 | 0.13 |  |  | 1.28 | 0.74 |  | 0.13 | 0.09 |  | 0.04 | 3.89 | 152.70 | 490 | 490 | UNSETTLED:300 | OK |
| firefight_1 | delta | +0.70 | -0.34 |  | +0.45 | +0.61 | -1.41 |  |  |  |  |  |  |  |  |  |  | +3.10 |  | +14.7% | +25.7% |  |  |
| firefight_2 | classic | 4.91 | 4.32 |  | 0.28 | 0.77 | 3.27 |  |  |  |  |  |  |  |  |  |  | 0.79 | 187.50 | 541 | 505 | OK:46 | OK |
| firefight_2 | remastered | 5.61 | 3.98 |  | 0.73 | 1.38 | 1.86 | 0.13 |  |  | 1.28 | 0.74 |  | 0.13 | 0.09 |  | 0.04 | 3.89 | 152.70 | 588 | 582 | UNSETTLED:300 | OK |
| firefight_2 | delta | +0.70 | -0.34 |  | +0.45 | +0.61 | -1.41 |  |  |  |  |  |  |  |  |  |  | +3.10 |  | +8.0% | +13.2% |  |  |
| firefight_3 | classic | 4.91 | 4.32 |  | 0.28 | 0.77 | 3.27 |  |  |  |  |  |  |  |  |  |  | 0.79 | 187.50 | 494 | 408 | OK:46 | OK |
| firefight_3 | remastered | 5.61 | 3.98 |  | 0.73 | 1.38 | 1.86 | 0.13 |  |  | 1.28 | 0.74 |  | 0.13 | 0.09 |  | 0.04 | 3.89 | 152.70 | 566 | 551 | UNSETTLED:300 | OK |
| firefight_3 | delta | +0.70 | -0.34 |  | +0.45 | +0.61 | -1.41 |  |  |  |  |  |  |  |  |  |  | +3.10 |  | +12.7% | +26.0% |  |  |
| tank_gunner_interior | classic | 5.56 | 5.27 |  | 0.10 | 2.31 | 2.86 |  |  |  |  |  |  |  |  |  |  | 4.68 | 117.80 | 2,529 | 1,216 | OK:30 | OK |
| tank_gunner_interior | remastered | 6.63 | 5.71 | 0.06 | 0.55 | 2.42 | 2.74 | 0.06 |  |  | 0.62 | 0.51 |  |  | 0.06 |  | 0.02 | 6.14 | 91.20 | 1,533 | 1,230 | OK:30 | OK |
| tank_gunner_interior | delta | +1.07 | +0.44 |  | +0.45 | +0.11 | -0.12 |  |  |  |  |  |  |  |  |  |  | +1.46 |  | -39.4% | +1.1% |  |  |
| tank_gunner_optics | classic | 4.52 | 4.18 |  | 0.21 | 1.92 | 2.05 |  |  |  |  |  |  |  |  |  |  | 3.15 | 134.70 | 1,730 | 744 | OK:30 | OK |
| tank_gunner_optics | remastered | 7.44 | 6.62 | 0.09 | 0.71 | 3.66 | 2.25 | 0.10 |  |  | 0.53 | 0.34 |  | 0.11 |  |  | 0.03 | 6.87 | 109.40 | 1,142 | 744 | OK:30 | OK |
| tank_gunner_optics | delta | +2.92 | +2.44 |  | +0.50 | +1.74 | +0.20 |  |  |  |  |  |  |  |  |  |  | +3.72 |  | -34.0% | +0.0% |  |  |
| overcast_broken | classic | 10.11 | 10.01 |  | 0.11 | 7.49 | 2.42 |  |  |  |  |  |  |  |  |  |  | 9.75 | 58.30 | 7,001 | 5,083 | OK:30 | OK |
| overcast_broken | remastered | 6.52 | 5.58 | 0.14 | 1.66 | 3.53 | 0.39 | 0.07 |  |  | 0.76 | 0.45 |  | 0.08 | 0.05 |  | 0.03 | 5.77 | 76.50 | 1,863 | 5,083 | OK:30 | OK |
| overcast_broken | delta | -3.59 | -4.43 |  | +1.55 | -3.96 | -2.03 |  |  |  |  |  |  |  |  |  |  | -3.98 |  | -73.4% | +0.0% |  |  |
| overcast_heavy | classic | 8.89 | 8.78 |  | 0.09 | 7.27 | 1.42 |  |  |  |  |  |  |  |  |  |  | 8.23 | 67.70 | 6,757 | 5,083 | OK:30 | OK |
| overcast_heavy | remastered | 6.40 | 5.44 | 0.14 | 1.36 | 3.66 | 0.41 | 0.07 |  |  | 0.77 | 0.46 |  | 0.09 | 0.06 |  | 0.03 | 5.41 | 79.80 | 1,619 | 5,083 | OK:30 | OK |
| overcast_heavy | delta | -2.49 | -3.34 |  | +1.27 | -3.61 | -1.01 |  |  |  |  |  |  |  |  |  |  | -2.82 |  | -76.0% | +0.0% |  |  |
| sinai_lowsun_fp | classic | 2.58 | 2.39 |  | 0.14 | 0.93 | 1.32 |  |  |  |  |  |  |  |  |  |  | 2.13 | 192.20 | 603 | 314 | OK:30 | OK |
| sinai_lowsun_fp | remastered | 6.33 | 4.69 | 0.29 | 0.99 | 2.55 | 1.15 | 0.10 |  |  | 1.37 | 0.46 |  | 0.16 | 0.08 | 0.33 | 0.13 | 5.73 | 123.30 | 418 | 314 | UNSETTLED:300 | OK |
| sinai_lowsun_fp | delta | +3.75 | +2.30 |  | +0.85 | +1.62 | -0.17 |  |  |  |  |  |  |  |  |  |  | +3.60 |  | -30.7% | +0.0% |  |  |
| sinai_desert_noon | classic | 5.77 | 5.63 |  | 0.15 | 2.85 | 2.63 |  |  |  |  |  |  |  |  |  |  | 4.86 | 91.90 | 2,242 | 1,100 | OK:30 | OK |
| sinai_desert_noon | remastered | 6.69 | 5.39 | 0.12 | 0.86 | 2.77 | 1.76 | 0.09 |  |  | 1.07 | 0.71 |  | 0.10 | 0.07 |  | 0.03 | 6.02 | 89.20 | 1,180 | 1,100 | OK:30 | OK |
| sinai_desert_noon | delta | +0.92 | -0.24 |  | +0.71 | -0.08 | -0.87 |  |  |  |  |  |  |  |  |  |  | +1.16 |  | -47.4% | +0.0% |  |  |
