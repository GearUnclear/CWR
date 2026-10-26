# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 3.59 | 3.49 |  |  |  |  |  |  | 153.50 | 1,778 | 1,179 | OK:30 | OK |
| lowsun_fp | remastered | 3.57 | 2.77 | 0.06 | 0.69 | 0.35 | 0.05 | 0.22 | 0.07 | 159.10 | 1,615 | 1,179 | OK:30 | OK |
| lowsun_fp | delta | -0.02 | -0.72 |  |  |  |  |  |  |  | -9.2% | +0.0% |  |  |
| sunrise_ridge | classic | 15.65 | 15.53 |  |  |  |  |  |  | 41.70 | 11,301 | 7,680 | OK:30 | OK |
| sunrise_ridge | remastered | 19.35 | 18.51 | 0.10 | 0.72 | 0.49 | 0.05 | 0.15 | 0.03 | 41.20 | 11,172 | 7,680 | OK:30 | OK |
| sunrise_ridge | delta | +3.70 | +2.98 |  |  |  |  |  |  |  | -1.1% | +0.0% |  |  |
| noon_fields | classic | 8.23 | 8.12 |  |  |  |  |  |  | 72.60 | 5,878 | 3,920 | OK:30 | OK |
| noon_fields | remastered | 11.96 | 11.43 | 0.07 | 0.42 | 0.34 | 0.05 |  | 0.03 | 62.00 | 5,575 | 3,920 | OK:30 | OK |
| noon_fields | delta | +3.73 | +3.31 |  |  |  |  |  |  |  | -5.2% | +0.0% |  |  |
| forest_edge_golden | classic | 3.69 | 3.56 |  |  |  |  |  |  | 185.00 | 2,189 | 1,316 | OK:30 | OK |
| forest_edge_golden | remastered | 4.87 | 4.34 | 0.19 | 0.39 | 0.31 | 0.05 |  | 0.03 | 140.30 | 1,847 | 1,316 | OK:30 | OK |
| forest_edge_golden | delta | +1.18 | +0.78 |  |  |  |  |  |  |  | -15.6% | +0.0% |  |  |
| coastline_sunset | classic | 1.69 | 1.37 |  |  |  |  |  |  | 354.20 | 155 | 0 | OK:30 | OK |
| coastline_sunset | remastered | 1.80 | 0.74 | 0.00 | 0.90 | 0.42 | 0.09 | 0.31 | 0.08 | 400.40 | 155 | 0 | OK:30 | OK |
| coastline_sunset | delta | +0.11 | -0.63 |  |  |  |  |  |  |  | +0.0% |  |  |  |
| town_night | classic | 8.89 | 8.74 |  |  |  |  |  |  | 115.10 | 1,207 | 1,057 | OK:225 | OK |
| town_night | remastered | 6.54 | 5.89 |  | 0.49 | 0.41 | 0.05 |  | 0.03 | 128.20 | 1,225 | 1,055 | OK:41 | OK |
| town_night | delta | -2.35 | -2.85 |  |  |  |  |  |  |  | +1.5% | -0.2% |  |  |
| firefight_1 | classic | 3.33 | 3.18 |  |  |  |  |  |  | 261.30 | 450 | 406 | OK:46 | OK |
| firefight_1 | remastered | 3.92 | 2.80 |  | 0.91 | 0.75 | 0.09 |  | 0.06 | 204.10 | 506 | 452 | OK:275 | OK |
| firefight_1 | delta | +0.59 | -0.38 |  |  |  |  |  |  |  | +11.1% | +10.2% |  |  |
| firefight_2 | classic | 3.33 | 3.18 |  |  |  |  |  |  | 261.30 | 495 | 445 | OK:46 | OK |
| firefight_2 | remastered | 3.92 | 2.80 |  | 0.91 | 0.75 | 0.09 |  | 0.06 | 204.10 | 544 | 526 | OK:275 | OK |
| firefight_2 | delta | +0.59 | -0.38 |  |  |  |  |  |  |  | +9.0% | +15.4% |  |  |
| firefight_3 | classic | 3.33 | 3.18 |  |  |  |  |  |  | 261.30 | 497 | 421 | OK:46 | OK |
| firefight_3 | remastered | 3.92 | 2.80 |  | 0.91 | 0.75 | 0.09 |  | 0.06 | 204.10 | 491 | 447 | OK:275 | OK |
| firefight_3 | delta | +0.59 | -0.38 |  |  |  |  |  |  |  | -1.2% | +5.8% |  |  |
| tank_gunner_interior | classic | 6.90 | 6.34 |  |  |  |  |  |  | 108.80 | 2,528 | 1,215 | OK:30 | OK |
| tank_gunner_interior | remastered | 5.68 | 4.67 | 0.06 | 0.42 |  | 0.39 |  | 0.02 | 115.80 | 2,309 | 1,230 | OK:30 | OK |
| tank_gunner_interior | delta | -1.22 | -1.67 |  |  |  |  |  |  |  | -8.7% | +1.2% |  |  |
| tank_gunner_optics | classic | 4.53 | 3.90 |  |  |  |  |  |  | 152.20 | 1,731 | 745 | OK:30 | OK |
| tank_gunner_optics | remastered | 6.00 | 4.97 | 0.27 | 0.30 | 0.28 |  |  | 0.02 | 147.10 | 1,521 | 745 | OK:30 | OK |
| tank_gunner_optics | delta | +1.47 | +1.07 |  |  |  |  |  |  |  | -12.1% | +0.0% |  |  |
| overcast_broken | classic | 11.36 | 11.25 |  |  |  |  |  |  | 61.60 | 7,001 | 5,083 | OK:30 | OK |
| overcast_broken | remastered | 10.74 | 10.03 | 0.08 | 0.61 | 0.54 | 0.05 |  | 0.03 | 58.90 | 6,843 | 5,083 | OK:30 | OK |
| overcast_broken | delta | -0.62 | -1.22 |  |  |  |  |  |  |  | -2.3% | +0.0% |  |  |
| overcast_heavy | classic | 9.92 | 9.80 |  |  |  |  |  |  | 65.20 | 6,757 | 5,083 | OK:30 | OK |
| overcast_heavy | remastered | 13.28 | 12.66 | 0.15 | 0.51 | 0.43 | 0.05 |  | 0.03 | 51.90 | 6,599 | 5,083 | OK:30 | OK |
| overcast_heavy | delta | +3.36 | +2.86 |  |  |  |  |  |  |  | -2.3% | +0.0% |  |  |
| sinai_lowsun_fp | classic | 4.41 | 3.71 |  |  |  |  |  |  | 138.50 | 603 | 314 | OK:30 | OK |
| sinai_lowsun_fp | remastered | 4.72 | 2.85 | 0.35 | 1.51 | 0.91 | 0.12 | 0.35 | 0.13 | 143.80 | 583 | 314 | OK:30 | OK |
| sinai_lowsun_fp | delta | +0.31 | -0.86 |  |  |  |  |  |  |  | -3.3% | +0.0% |  |  |
| sinai_desert_noon | classic | 5.98 | 5.83 |  |  |  |  |  |  | 97.50 | 2,241 | 1,100 | OK:30 | OK |
| sinai_desert_noon | remastered | 6.80 | 5.85 | 0.06 | 0.79 | 0.67 | 0.07 |  | 0.04 | 87.90 | 2,230 | 1,100 | OK:30 | OK |
| sinai_desert_noon | delta | +0.82 | +0.02 |  |  |  |  |  |  |  | -0.5% | +0.0% |  |  |
