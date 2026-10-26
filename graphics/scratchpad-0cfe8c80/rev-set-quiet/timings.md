# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 5.11 | 4.91 |  |  |  |  |  |  | 127.30 | 1,778 | 1,179 | OK:30 | OK |
| lowsun_fp | remastered | 4.49 | 3.58 | 0.07 | 0.78 | 0.38 | 0.06 | 0.28 | 0.07 | 134.20 | 1,615 | 1,179 | OK:30 | OK |
| lowsun_fp | delta | -0.62 | -1.33 |  |  |  |  |  |  |  | -9.2% | +0.0% |  |  |
| sunrise_ridge | classic | 14.42 | 14.30 |  |  |  |  |  |  | 43.90 | 11,301 | 7,680 | OK:30 | OK |
| sunrise_ridge | remastered | 18.13 | 17.25 | 0.09 | 0.76 | 0.54 | 0.05 | 0.15 | 0.03 | 43.30 | 11,172 | 7,680 | OK:30 | OK |
| sunrise_ridge | delta | +3.71 | +2.95 |  |  |  |  |  |  |  | -1.1% | +0.0% |  |  |
| noon_fields | classic | 7.09 | 6.99 |  |  |  |  |  |  | 83.30 | 5,878 | 3,920 | OK:30 | OK |
| noon_fields | remastered | 7.88 | 7.40 | 0.07 | 0.38 | 0.30 | 0.05 |  | 0.03 | 79.80 | 5,575 | 3,920 | OK:30 | OK |
| noon_fields | delta | +0.79 | +0.41 |  |  |  |  |  |  |  | -5.2% | +0.0% |  |  |
| forest_edge_golden | classic | 5.57 | 5.38 |  |  |  |  |  |  | 184.90 | 2,189 | 1,316 | OK:30 | OK |
| forest_edge_golden | remastered | 4.53 | 3.92 | 0.22 | 0.46 | 0.36 | 0.06 |  | 0.03 | 139.10 | 1,847 | 1,316 | OK:30 | OK |
| forest_edge_golden | delta | -1.04 | -1.46 |  |  |  |  |  |  |  | -15.6% | +0.0% |  |  |
| coastline_sunset | classic | 3.15 | 2.59 |  |  |  |  |  |  | 231.00 | 155 | 0 | OK:30 | OK |
| coastline_sunset | remastered | 4.48 | 2.03 | 0.00 | 1.91 | 0.79 | 0.21 | 0.59 | 0.32 | 198.30 | 155 | 0 | OK:30 | OK |
| coastline_sunset | delta | +1.33 | -0.56 |  |  |  |  |  |  |  | +0.0% |  |  |  |
| town_night | classic | 6.32 | 6.12 |  |  |  |  |  |  | 123.10 | 1,217 | 1,050 | UNSETTLED:300 | OK |
| town_night | remastered | 6.47 | 5.74 |  | 0.55 | 0.44 | 0.06 |  | 0.04 | 121.20 | 1,214 | 1,065 | OK:30 | OK |
| town_night | delta | +0.15 | -0.38 |  |  |  |  |  |  |  | -0.2% | +1.4% |  |  |
| firefight_1 | classic | 4.98 | 4.44 |  |  |  |  |  |  | 174.30 | 455 | 400 | OK:42 | OK |
| firefight_1 | remastered | 5.35 | 3.26 |  | 1.35 | 1.10 | 0.14 |  | 0.11 | 140.10 | 460 | 379 | OK:58 | OK |
| firefight_1 | delta | +0.37 | -1.18 |  |  |  |  |  |  |  | +1.1% | -5.2% |  |  |
| firefight_2 | classic | 4.98 | 4.44 |  |  |  |  |  |  | 174.30 | 562 | 522 | OK:42 | OK |
| firefight_2 | remastered | 5.35 | 3.26 |  | 1.35 | 1.10 | 0.14 |  | 0.11 | 140.10 | 570 | 554 | OK:58 | OK |
| firefight_2 | delta | +0.37 | -1.18 |  |  |  |  |  |  |  | +1.4% | +5.8% |  |  |
| firefight_3 | classic | 4.98 | 4.44 |  |  |  |  |  |  | 174.30 | 480 | 441 | OK:42 | OK |
| firefight_3 | remastered | 5.35 | 3.26 |  | 1.35 | 1.10 | 0.14 |  | 0.11 | 140.10 | 567 | 531 | OK:58 | OK |
| firefight_3 | delta | +0.37 | -1.18 |  |  |  |  |  |  |  | +15.3% | +16.9% |  |  |
| tank_gunner_interior | classic | 5.43 | 5.15 |  |  |  |  |  |  | 118.30 | 2,529 | 1,219 | OK:30 | OK |
| tank_gunner_interior | remastered | 6.29 | 5.55 | 0.07 | 0.45 |  | 0.42 |  | 0.03 | 107.00 | 2,314 | 1,235 | OK:30 | OK |
| tank_gunner_interior | delta | +0.86 | +0.40 |  |  |  |  |  |  |  | -8.5% | +1.3% |  |  |
| tank_gunner_optics | classic | 4.05 | 3.75 |  |  |  |  |  |  | 141.30 | 1,733 | 746 | OK:30 | OK |
| tank_gunner_optics | remastered | 5.69 | 4.87 | 0.10 | 0.50 | 0.46 |  |  | 0.04 | 130.30 | 1,524 | 744 | OK:30 | OK |
| tank_gunner_optics | delta | +1.64 | +1.12 |  |  |  |  |  |  |  | -12.1% | -0.3% |  |  |
| overcast_broken | classic | 9.58 | 9.48 |  |  |  |  |  |  | 63.90 | 7,001 | 5,083 | OK:30 | OK |
| overcast_broken | remastered | 10.09 | 9.48 | 0.08 | 0.52 | 0.44 | 0.05 |  | 0.03 | 62.40 | 6,843 | 5,083 | OK:30 | OK |
| overcast_broken | delta | +0.51 | +0.00 |  |  |  |  |  |  |  | -2.3% | +0.0% |  |  |
| overcast_heavy | classic | 9.28 | 9.17 |  |  |  |  |  |  | 67.40 | 6,757 | 5,083 | OK:30 | OK |
| overcast_heavy | remastered | 10.27 | 9.58 | 0.08 | 0.57 | 0.50 | 0.05 |  | 0.03 | 63.40 | 6,599 | 5,083 | OK:30 | OK |
| overcast_heavy | delta | +0.99 | +0.41 |  |  |  |  |  |  |  | -2.3% | +0.0% |  |  |
