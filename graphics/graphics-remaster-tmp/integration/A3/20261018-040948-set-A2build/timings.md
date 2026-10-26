# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | resolve | lindepth | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 6.66 | 6.21 |  |  |  |  |  |  |  |  | 117.90 | 1,778 | 1,179 | OK:30 | OK |
| lowsun_fp | remastered | 3.93 | 3.12 | 0.07 | 0.70 |  |  | 0.34 | 0.06 | 0.26 | 0.05 | 154.50 | 1,615 | 1,179 | OK:30 | OK |
| lowsun_fp | delta | -2.73 | -3.09 |  |  |  |  |  |  |  |  |  | -9.2% | +0.0% |  |  |
| sunrise_ridge | classic | 14.33 | 14.22 |  |  |  |  |  |  |  |  | 43.80 | 11,301 | 7,680 | OK:30 | OK |
| sunrise_ridge | remastered | 18.19 | 17.32 | 0.09 | 0.76 |  |  | 0.53 | 0.05 | 0.15 | 0.03 | 43.30 | 11,172 | 7,680 | OK:30 | OK |
| sunrise_ridge | delta | +3.86 | +3.10 |  |  |  |  |  |  |  |  |  | -1.1% | +0.0% |  |  |
| noon_fields | classic | 7.07 | 6.97 |  |  |  |  |  |  |  |  | 82.50 | 5,878 | 3,920 | OK:30 | OK |
| noon_fields | remastered | 7.91 | 7.43 | 0.07 | 0.38 |  |  | 0.30 | 0.05 |  | 0.03 | 80.10 | 5,575 | 3,920 | OK:30 | OK |
| noon_fields | delta | +0.84 | +0.46 |  |  |  |  |  |  |  |  |  | -5.2% | +0.0% |  |  |
| forest_edge_golden | classic | 5.76 | 5.56 |  |  |  |  |  |  |  |  | 184.00 | 2,189 | 1,316 | OK:30 | OK |
| forest_edge_golden | remastered | 5.63 | 4.98 | 0.27 | 0.46 |  |  | 0.35 | 0.07 |  | 0.04 | 129.80 | 1,847 | 1,316 | OK:30 | OK |
| forest_edge_golden | delta | -0.13 | -0.58 |  |  |  |  |  |  |  |  |  | -15.6% | +0.0% |  |  |
| coastline_sunset | classic | 2.53 | 2.02 |  |  |  |  |  |  |  |  | 224.10 | 155 | 0 | OK:30 | OK |
| coastline_sunset | remastered | 4.67 | 1.86 | 0.00 | 2.37 |  |  | 0.94 | 0.39 | 0.71 | 0.33 | 210.10 | 155 | 0 | OK:30 | OK |
| coastline_sunset | delta | +2.14 | -0.16 |  |  |  |  |  |  |  |  |  | +0.0% |  |  |  |
| town_night | classic | 6.11 | 5.91 |  |  |  |  |  |  |  |  | 123.20 | 1,200 | 1,035 | OK:222 | OK |
| town_night | remastered | 6.45 | 5.70 |  | 0.56 |  |  | 0.45 | 0.07 |  | 0.04 | 122.90 | 1,181 | 1,014 | OK:94 | OK |
| town_night | delta | +0.34 | -0.21 |  |  |  |  |  |  |  |  |  | -1.6% | -2.0% |  |  |
| firefight_1 | classic | 4.93 | 4.47 |  |  |  |  |  |  |  |  | 170.80 | 429 | 402 | OK:45 | OK |
| firefight_1 | remastered | 6.04 | 3.95 |  | 1.41 |  |  | 1.12 | 0.16 |  | 0.13 | 143.50 | 515 | 446 | OK:282 | OK |
| firefight_1 | delta | +1.11 | -0.52 |  |  |  |  |  |  |  |  |  | +16.7% | +9.9% |  |  |
| firefight_2 | classic | 4.93 | 4.47 |  |  |  |  |  |  |  |  | 170.80 | 538 | 520 | OK:45 | OK |
| firefight_2 | remastered | 6.04 | 3.95 |  | 1.41 |  |  | 1.12 | 0.16 |  | 0.13 | 143.50 | 563 | 528 | OK:282 | OK |
| firefight_2 | delta | +1.11 | -0.52 |  |  |  |  |  |  |  |  |  | +4.4% | +1.5% |  |  |
| firefight_3 | classic | 4.93 | 4.47 |  |  |  |  |  |  |  |  | 170.80 | 448 | 361 | OK:45 | OK |
| firefight_3 | remastered | 6.04 | 3.95 |  | 1.41 |  |  | 1.12 | 0.16 |  | 0.13 | 143.50 | 482 | 404 | OK:282 | OK |
| firefight_3 | delta | +1.11 | -0.52 |  |  |  |  |  |  |  |  |  | +7.1% | +10.6% |  |  |
| overcast_broken | classic | 9.95 | 9.84 |  |  |  |  |  |  |  |  | 64.50 | 7,001 | 5,083 | OK:30 | OK |
| overcast_broken | remastered | 10.20 | 9.52 | 0.08 | 0.58 |  |  | 0.50 | 0.05 |  | 0.03 | 62.40 | 6,843 | 5,083 | OK:30 | OK |
| overcast_broken | delta | +0.25 | -0.32 |  |  |  |  |  |  |  |  |  | -2.3% | +0.0% |  |  |
| overcast_heavy | classic | 9.17 | 9.06 |  |  |  |  |  |  |  |  | 68.60 | 6,757 | 5,083 | OK:30 | OK |
| overcast_heavy | remastered | 10.77 | 10.10 | 0.08 | 0.55 |  |  | 0.48 | 0.05 |  | 0.03 | 62.20 | 6,599 | 5,083 | OK:30 | OK |
| overcast_heavy | delta | +1.60 | +1.04 |  |  |  |  |  |  |  |  |  | -2.3% | +0.0% |  |  |
| tank_gunner_interior | classic | 5.58 | 5.25 |  |  |  |  |  |  |  |  | 118.80 | 2,530 | 1,217 | OK:30 | OK |
| tank_gunner_interior | remastered | 6.36 | 5.61 | 0.07 | 0.44 |  |  |  | 0.41 |  | 0.03 | 107.60 | 2,308 | 1,230 | OK:30 | OK |
| tank_gunner_interior | delta | +0.78 | +0.36 |  |  |  |  |  |  |  |  |  | -8.8% | +1.1% |  |  |
| tank_gunner_optics | classic | 6.04 | 5.30 |  |  |  |  |  |  |  |  | 126.00 | 1,728 | 742 | OK:30 | OK |
| tank_gunner_optics | remastered | 3.95 | 3.28 | 0.07 | 0.43 |  |  | 0.41 |  |  | 0.03 | 161.30 | 1,520 | 741 | OK:30 | OK |
| tank_gunner_optics | delta | -2.09 | -2.02 |  |  |  |  |  |  |  |  |  | -12.0% | -0.1% |  |  |
| sinai_lowsun_fp | classic | 4.35 | 3.89 |  |  |  |  |  |  |  |  | 139.10 | 603 | 314 | OK:30 | OK |
| sinai_lowsun_fp | remastered | 5.15 | 3.05 | 0.35 | 1.73 |  |  | 0.90 | 0.19 | 0.40 | 0.24 | 132.40 | 583 | 314 | OK:30 | OK |
| sinai_lowsun_fp | delta | +0.80 | -0.84 |  |  |  |  |  |  |  |  |  | -3.3% | +0.0% |  |  |
| sinai_desert_noon | classic | 6.04 | 5.87 |  |  |  |  |  |  |  |  | 96.80 | 2,243 | 1,100 | OK:30 | OK |
| sinai_desert_noon | remastered | 6.47 | 5.48 | 0.05 | 0.85 |  |  | 0.74 | 0.07 |  | 0.04 | 88.00 | 2,232 | 1,100 | OK:30 | OK |
| sinai_desert_noon | delta | +0.43 | -0.39 |  |  |  |  |  |  |  |  |  | -0.5% | +0.0% |  |  |
