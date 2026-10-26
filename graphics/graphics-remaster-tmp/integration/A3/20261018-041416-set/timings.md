# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | resolve | lindepth | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 6.33 | 5.91 |  |  |  |  |  |  |  |  | 116.20 | 1,778 | 1,179 | OK:30 | OK |
| lowsun_fp | remastered | 4.13 | 3.27 | 0.07 | 0.74 | 0.25 |  | 0.11 | 0.06 | 0.26 | 0.07 | 153.80 | 1,615 | 1,179 | OK:30 | OK |
| lowsun_fp | delta | -2.20 | -2.64 |  |  |  |  |  |  |  |  |  | -9.2% | +0.0% |  |  |
| sunrise_ridge | classic | 14.45 | 14.33 |  |  |  |  |  |  |  |  | 43.80 | 11,301 | 7,680 | OK:30 | OK |
| sunrise_ridge | remastered | 17.97 | 17.17 | 0.09 | 0.68 | 0.37 |  | 0.09 | 0.05 | 0.15 | 0.03 | 43.50 | 11,172 | 7,680 | OK:30 | OK |
| sunrise_ridge | delta | +3.52 | +2.84 |  |  |  |  |  |  |  |  |  | -1.1% | +0.0% |  |  |
| noon_fields | classic | 6.93 | 6.83 |  |  |  |  |  |  |  |  | 83.70 | 5,878 | 3,920 | OK:30 | OK |
| noon_fields | remastered | 7.86 | 7.37 | 0.07 | 0.38 | 0.23 |  | 0.08 | 0.05 |  | 0.03 | 80.40 | 5,575 | 3,920 | OK:30 | OK |
| noon_fields | delta | +0.93 | +0.54 |  |  |  |  |  |  |  |  |  | -5.2% | +0.0% |  |  |
| forest_edge_golden | classic | 5.54 | 5.35 |  |  |  |  |  |  |  |  | 182.10 | 2,189 | 1,316 | OK:30 | OK |
| forest_edge_golden | remastered | 4.51 | 3.92 | 0.21 | 0.44 | 0.24 |  | 0.11 | 0.06 |  | 0.03 | 139.80 | 1,847 | 1,316 | OK:30 | OK |
| forest_edge_golden | delta | -1.03 | -1.43 |  |  |  |  |  |  |  |  |  | -15.6% | +0.0% |  |  |
| coastline_sunset | classic | 2.72 | 2.20 |  |  |  |  |  |  |  |  | 249.70 | 155 | 0 | OK:30 | OK |
| coastline_sunset | remastered | 4.28 | 1.88 | 0.00 | 1.87 | 0.55 |  | 0.25 | 0.22 | 0.67 | 0.19 | 197.10 | 155 | 0 | OK:30 | OK |
| coastline_sunset | delta | +1.56 | -0.32 |  |  |  |  |  |  |  |  |  | +0.0% |  |  |  |
| town_night | classic | 6.57 | 6.36 |  |  |  |  |  |  |  |  | 118.60 | 1,203 | 1,057 | OK:247 | OK |
| town_night | remastered | 6.72 | 5.93 |  | 0.60 | 0.36 |  | 0.14 | 0.06 |  | 0.04 | 120.70 | 1,217 | 1,053 | OK:91 | OK |
| town_night | delta | +0.15 | -0.43 |  |  |  |  |  |  |  |  |  | +1.2% | -0.4% |  |  |
| firefight_1 | classic | 4.54 | 3.77 |  |  |  |  |  |  |  |  | 177.00 | 469 | 438 | OK:56 | OK |
| firefight_1 | remastered | 5.18 | 3.42 |  | 1.22 | 0.83 |  | 0.16 | 0.13 |  | 0.10 | 145.40 | 477 | 408 | OK:288 | OK |
| firefight_1 | delta | +0.64 | -0.35 |  |  |  |  |  |  |  |  |  | +1.7% | -6.8% |  |  |
| firefight_2 | classic | 4.54 | 3.77 |  |  |  |  |  |  |  |  | 177.00 | 514 | 523 | OK:56 | OK |
| firefight_2 | remastered | 5.18 | 3.42 |  | 1.22 | 0.83 |  | 0.16 | 0.13 |  | 0.10 | 145.40 | 492 | 450 | OK:288 | OK |
| firefight_2 | delta | +0.64 | -0.35 |  |  |  |  |  |  |  |  |  | -4.3% | -14.0% |  |  |
| firefight_3 | classic | 4.54 | 3.77 |  |  |  |  |  |  |  |  | 177.00 | 462 | 425 | OK:56 | OK |
| firefight_3 | remastered | 5.18 | 3.42 |  | 1.22 | 0.83 |  | 0.16 | 0.13 |  | 0.10 | 145.40 | 521 | 454 | OK:288 | OK |
| firefight_3 | delta | +0.64 | -0.35 |  |  |  |  |  |  |  |  |  | +11.3% | +6.4% |  |  |
| tank_gunner_interior | classic | 5.86 | 5.51 |  |  |  |  |  |  |  |  | 112.90 | 2,533 | 1,220 | OK:30 | OK |
| tank_gunner_interior | remastered | 6.32 | 5.60 | 0.07 | 0.43 | 0.34 |  |  | 0.06 |  | 0.03 | 108.20 | 2,308 | 1,229 | OK:30 | OK |
| tank_gunner_interior | delta | +0.46 | +0.09 |  |  |  |  |  |  |  |  |  | -8.9% | +0.7% |  |  |
| tank_gunner_optics | classic | 4.06 | 3.74 |  |  |  |  |  |  |  |  | 136.30 | 1,729 | 743 | OK:30 | OK |
| tank_gunner_optics | remastered | 4.52 | 3.82 | 0.08 | 0.43 | 0.30 |  | 0.10 |  |  | 0.03 | 138.50 | 1,516 | 743 | OK:30 | OK |
| tank_gunner_optics | delta | +0.46 | +0.08 |  |  |  |  |  |  |  |  |  | -12.3% | +0.0% |  |  |
| overcast_broken | classic | 9.43 | 9.32 |  |  |  |  |  |  |  |  | 64.50 | 7,001 | 5,083 | OK:30 | OK |
| overcast_broken | remastered | 10.12 | 9.43 | 0.08 | 0.59 | 0.43 |  | 0.08 | 0.05 |  | 0.03 | 62.80 | 6,843 | 5,083 | OK:30 | OK |
| overcast_broken | delta | +0.69 | +0.11 |  |  |  |  |  |  |  |  |  | -2.3% | +0.0% |  |  |
| overcast_heavy | classic | 9.01 | 8.89 |  |  |  |  |  |  |  |  | 68.80 | 6,757 | 5,083 | OK:30 | OK |
| overcast_heavy | remastered | 10.16 | 9.48 | 0.11 | 0.57 | 0.41 |  | 0.08 | 0.05 |  | 0.03 | 64.10 | 6,599 | 5,083 | OK:30 | OK |
| overcast_heavy | delta | +1.15 | +0.59 |  |  |  |  |  |  |  |  |  | -2.3% | +0.0% |  |  |
| sinai_lowsun_fp | classic | 4.29 | 3.86 |  |  |  |  |  |  |  |  | 140.60 | 602 | 314 | OK:30 | OK |
| sinai_lowsun_fp | remastered | 5.63 | 3.40 | 0.51 | 1.73 | 0.86 |  | 0.19 | 0.17 | 0.36 | 0.14 | 136.60 | 582 | 314 | OK:30 | OK |
| sinai_lowsun_fp | delta | +1.34 | -0.46 |  |  |  |  |  |  |  |  |  | -3.3% | +0.0% |  |  |
| sinai_desert_noon | classic | 5.77 | 5.62 |  |  |  |  |  |  |  |  | 97.40 | 2,242 | 1,100 | OK:30 | OK |
| sinai_desert_noon | remastered | 6.67 | 5.71 | 0.05 | 0.81 | 0.57 |  | 0.12 | 0.07 |  | 0.04 | 87.80 | 2,231 | 1,100 | OK:30 | OK |
| sinai_desert_noon | delta | +0.90 | +0.09 |  |  |  |  |  |  |  |  |  | -0.5% | +0.0% |  |  |
