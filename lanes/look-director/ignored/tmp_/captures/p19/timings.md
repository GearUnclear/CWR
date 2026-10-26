# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 4.22 | 4.06 |  | 0.12 | 1.93 | 2.01 |  |  |  |  |  |  |  |  |  |  | 2.89 | 157.30 | 1,778 | 1,179 | OK:30 | OK |
| lowsun_fp | remastered | 8.53 | 6.78 | 0.14 | 1.27 | 3.39 | 2.12 | 0.12 |  |  | 1.46 | 0.27 | 0.01 | 0.18 | 0.06 |  | 0.11 | 8.20 | 95.20 | 1,090 | 1,179 | OK:30 | OK |
| lowsun_fp | delta | +4.31 | +2.72 |  | +1.15 | +1.46 | +0.11 |  |  |  |  |  |  |  |  |  |  | +5.31 |  | -38.7% | +0.0% |  |  |
| sunrise_ridge | classic | 14.73 | 14.61 |  | 0.10 | 13.07 | 1.44 |  |  |  |  |  |  |  |  |  |  | 13.67 | 39.50 | 11,301 | 7,680 | OK:30 | OK |
| sunrise_ridge | remastered | 9.10 | 7.47 | 0.09 | 5.11 | 2.01 | 0.36 | 0.15 |  |  | 1.27 | 0.27 | 0.01 | 0.18 | 0.06 |  | 0.11 | 4.95 | 69.30 | 1,659 | 7,680 | OK:30 | OK |
| sunrise_ridge | delta | -5.63 | -7.14 |  | +5.01 | -11.06 | -1.08 |  |  |  |  |  |  |  |  |  |  | -8.72 |  | -85.3% | +0.0% |  |  |
| noon_fields | classic | 7.28 | 7.18 |  | 0.08 | 5.44 | 1.66 |  |  |  |  |  |  |  |  |  |  | 6.99 | 78.60 | 5,878 | 3,920 | OK:30 | OK |
| noon_fields | remastered | 9.68 | 8.29 | 0.10 | 1.68 | 6.10 | 0.51 | 0.11 |  |  | 1.12 | 0.26 | 0.01 | 0.16 | 0.05 |  | 0.11 | 9.31 | 68.70 | 1,447 | 3,920 | OK:30 | OK |
| noon_fields | delta | +2.40 | +1.11 |  | +1.60 | +0.66 | -1.15 |  |  |  |  |  |  |  |  |  |  | +2.32 |  | -75.4% | +0.0% |  |  |
| forest_edge_golden | classic | 5.26 | 5.11 |  | 0.07 | 2.87 | 2.17 |  |  |  |  |  |  |  |  |  |  | 3.84 | 168.40 | 2,189 | 1,316 | OK:30 | OK |
| forest_edge_golden | remastered | 6.71 | 5.02 | 0.19 | 1.52 | 2.67 | 0.83 | 0.14 |  |  | 1.32 | 0.29 | 0.01 | 0.22 | 0.05 |  | 0.12 | 6.22 | 98.50 | 757 | 1,316 | OK:30 | OK |
| forest_edge_golden | delta | +1.45 | -0.09 |  | +1.45 | -0.20 | -1.34 |  |  |  |  |  |  |  |  |  |  | +2.38 |  | -65.4% | +0.0% |  |  |
| coastline_sunset | classic | 1.03 | 0.84 |  | 0.49 | 0.00 | 0.35 |  |  |  |  |  |  |  |  |  |  | 0.62 | 397.60 | 155 | 0 | OK:30 | OK |
| coastline_sunset | remastered | 5.70 | 2.90 | 0.00 | 2.61 | 0.00 | 0.29 | 0.20 |  |  | 2.32 | 0.19 | 0.01 | 0.26 | 0.09 |  | 0.19 | 4.76 | 168.30 | 154 | 0 | OK:30 | OK |
| coastline_sunset | delta | +4.67 | +2.06 |  | +2.12 | +0.00 | -0.06 |  |  |  |  |  |  |  |  |  |  | +4.14 |  | -0.6% |  |  |  |
| town_night | classic | 6.64 | 6.45 |  | 0.08 | 1.39 | 4.98 |  |  |  |  |  |  |  |  |  |  | 5.77 | 121.10 | 1,236 | 1,071 | OK:236 | OK |
| town_night | remastered | 11.10 | 9.49 | 0.16 | 1.10 | 3.64 | 4.75 | 0.05 |  |  | 1.28 | 0.30 | 0.01 | 0.15 | 0.05 |  | 0.10 | 10.43 | 72.60 | 1,187 | 1,074 | OK:62 | OK |
| town_night | delta | +4.46 | +3.04 |  | +1.02 | +2.25 | -0.23 |  |  |  |  |  |  |  |  |  |  | +4.66 |  | -4.0% | +0.3% |  |  |
| firefight_1 | classic | 5.41 | 4.49 |  | 0.40 | 0.90 | 3.19 |  |  |  |  |  |  |  |  |  |  | 0.84 | 195.70 | 418 | 358 | OK:47 | OK |
| firefight_1 | remastered | 6.18 | 4.23 |  | 1.01 | 1.62 | 1.60 | 0.20 |  |  | 1.33 | 0.28 | 0.01 | 0.21 | 0.07 |  | 0.17 | 5.47 | 130.40 | 417 | 389 | UNSETTLED:300 | OK |
| firefight_1 | delta | +0.77 | -0.26 |  | +0.61 | +0.72 | -1.59 |  |  |  |  |  |  |  |  |  |  | +4.63 |  | -0.2% | +8.0% |  |  |
| firefight_2 | classic | 5.41 | 4.49 |  | 0.40 | 0.90 | 3.19 |  |  |  |  |  |  |  |  |  |  | 0.84 | 195.70 | 494 | 526 | OK:47 | OK |
| firefight_2 | remastered | 6.18 | 4.23 |  | 1.01 | 1.62 | 1.60 | 0.20 |  |  | 1.33 | 0.28 | 0.01 | 0.21 | 0.07 |  | 0.17 | 5.47 | 130.40 | 467 | 506 | UNSETTLED:300 | OK |
| firefight_2 | delta | +0.77 | -0.26 |  | +0.61 | +0.72 | -1.59 |  |  |  |  |  |  |  |  |  |  | +4.63 |  | -5.5% | -3.8% |  |  |
| firefight_3 | classic | 5.41 | 4.49 |  | 0.40 | 0.90 | 3.19 |  |  |  |  |  |  |  |  |  |  | 0.84 | 195.70 | 500 | 443 | OK:47 | OK |
| firefight_3 | remastered | 6.18 | 4.23 |  | 1.01 | 1.62 | 1.60 | 0.20 |  |  | 1.33 | 0.28 | 0.01 | 0.21 | 0.07 |  | 0.17 | 5.47 | 130.40 | 543 | 591 | UNSETTLED:300 | OK |
| firefight_3 | delta | +0.77 | -0.26 |  | +0.61 | +0.72 | -1.59 |  |  |  |  |  |  |  |  |  |  | +4.63 |  | +7.9% | +25.0% |  |  |
| tank_gunner_interior | classic | 5.10 | 4.83 |  | 0.09 | 1.93 | 2.81 |  |  |  |  |  |  |  |  |  |  | 4.33 | 125.70 | 2,532 | 1,219 | OK:30 | OK |
| tank_gunner_interior | remastered | 8.35 | 7.39 | 0.06 | 0.88 | 2.91 | 3.59 | 0.09 |  |  | 0.62 | 0.14 |  | 0.28 | 0.06 |  | 0.11 | 8.05 | 77.70 | 1,537 | 1,234 | OK:30 | OK |
| tank_gunner_interior | delta | +3.25 | +2.56 |  | +0.79 | +0.98 | +0.78 |  |  |  |  |  |  |  |  |  |  | +3.72 |  | -39.3% | +1.2% |  |  |
| tank_gunner_optics | classic | 3.67 | 3.40 |  | 0.12 | 1.68 | 1.61 |  |  |  |  |  |  |  |  |  |  | 3.22 | 161.30 | 1,717 | 743 | OK:30 | OK |
| tank_gunner_optics | remastered | 7.61 | 6.51 | 0.06 | 0.87 | 3.46 | 2.17 | 0.10 |  |  | 0.86 | 0.21 | 0.01 | 0.15 |  |  | 0.10 | 7.09 | 110.50 | 1,132 | 740 | OK:30 | OK |
| tank_gunner_optics | delta | +3.94 | +3.11 |  | +0.75 | +1.78 | +0.56 |  |  |  |  |  |  |  |  |  |  | +3.87 |  | -34.1% | -0.4% |  |  |
