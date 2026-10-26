# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 5.41 | 5.14 |  |  |  |  |  |  | 113.00 | 1,622 | 1,179 | OK:30 | OK |
| lowsun_fp | remastered | 3.77 | 2.96 | 0.08 | 0.71 | 0.39 | 0.05 | 0.24 | 0.04 | 151.10 | 1,615 | 1,179 | OK:30 | OK |
| lowsun_fp | delta | -1.64 | -2.18 |  |  |  |  |  |  |  | -0.4% | +0.0% |  |  |
| sunrise_ridge | classic | 14.54 | 14.42 |  |  |  |  |  |  | 44.90 | 11,172 | 7,680 | OK:30 | OK |
| sunrise_ridge | remastered | 17.68 | 16.85 | 0.00 | 0.72 | 0.49 | 0.05 | 0.15 | 0.03 | 43.50 | 11,172 | 7,680 | OK:30 | OK |
| sunrise_ridge | delta | +3.14 | +2.43 |  |  |  |  |  |  |  | +0.0% | +0.0% |  |  |
| noon_fields | classic | 5.83 | 5.72 |  |  |  |  |  |  | 90.40 | 5,580 | 3,920 | OK:30 | OK |
| noon_fields | remastered | 7.66 | 7.16 | 0.04 | 0.40 | 0.33 | 0.05 |  | 0.03 | 82.90 | 5,575 | 3,920 | OK:30 | OK |
| noon_fields | delta | +1.83 | +1.44 |  |  |  |  |  |  |  | -0.1% | +0.0% |  |  |
| forest_edge_golden | classic | 5.08 | 4.74 |  |  |  |  |  |  | 212.50 | 1,850 | 1,316 | OK:30 | OK |
| forest_edge_golden | remastered | 4.82 | 4.21 | 0.05 | 0.44 | 0.33 | 0.07 |  | 0.04 | 132.10 | 1,847 | 1,316 | OK:30 | OK |
| forest_edge_golden | delta | -0.26 | -0.53 |  |  |  |  |  |  |  | -0.2% | +0.0% |  |  |
| coastline_sunset | classic | 1.16 | 0.99 |  |  |  |  |  |  | 161.70 | 155 | 0 | OK:30 | OK |
| coastline_sunset | remastered | 2.22 | 1.03 | 0.00 | 1.01 | 0.45 | 0.10 | 0.35 | 0.10 | 214.30 | 155 | 0 | OK:30 | OK |
| coastline_sunset | delta | +1.06 | +0.04 |  |  |  |  |  |  |  | +0.0% |  |  |  |
| town_night | classic | 7.94 | 7.78 |  |  |  |  |  |  | 95.60 | 1,230 | 1,073 | OK:254 | OK |
| town_night | remastered | 6.17 | 5.45 |  | 0.54 | 0.45 | 0.06 |  | 0.04 | 136.10 | 1,236 | 1,071 | OK:98 | OK |
| town_night | delta | -1.77 | -2.33 |  |  |  |  |  |  |  | +0.5% | -0.2% |  |  |
| firefight_1 | classic | 4.40 | 3.99 |  |  |  |  |  |  | 201.80 | 448 | 437 | OK:46 | OK |
| firefight_1 | remastered | 3.92 | 2.81 |  | 0.97 | 0.89 | 0.05 |  | 0.03 | 132.60 | 480 | 469 | OK:279 | OK |
| firefight_1 | delta | -0.48 | -1.18 |  |  |  |  |  |  |  | +6.7% | +6.8% |  |  |
| firefight_2 | classic | 4.40 | 3.99 |  |  |  |  |  |  | 201.80 | 537 | 540 | OK:46 | OK |
| firefight_2 | remastered | 3.92 | 2.81 |  | 0.97 | 0.89 | 0.05 |  | 0.03 | 132.60 | 578 | 570 | OK:279 | OK |
| firefight_2 | delta | -0.48 | -1.18 |  |  |  |  |  |  |  | +7.1% | +5.3% |  |  |
| firefight_3 | classic | 4.40 | 3.99 |  |  |  |  |  |  | 201.80 | 455 | 395 | OK:46 | OK |
| firefight_3 | remastered | 3.92 | 2.81 |  | 0.97 | 0.89 | 0.05 |  | 0.03 | 132.60 | 591 | 543 | OK:279 | OK |
| firefight_3 | delta | -0.48 | -1.18 |  |  |  |  |  |  |  | +23.0% | +27.3% |  |  |
| tank_gunner_interior | classic | 6.98 | 6.65 |  |  |  |  |  |  | 100.60 | 2,298 | 1,215 | OK:30 | OK |
| tank_gunner_interior | remastered | 9.62 | 8.85 | 0.47 | 0.50 |  | 0.48 |  | 0.02 | 74.20 | 2,311 | 1,232 | OK:30 | OK |
| tank_gunner_interior | delta | +2.64 | +2.20 |  |  |  |  |  |  |  | +0.6% | +1.4% |  |  |
| tank_gunner_optics | classic | 3.10 | 2.82 |  |  |  |  |  |  | 142.00 | 1,536 | 744 | OK:30 | OK |
| tank_gunner_optics | remastered | 5.20 | 4.31 | 0.04 | 0.68 | 0.66 |  |  | 0.02 | 113.60 | 1,519 | 743 | OK:30 | OK |
| tank_gunner_optics | delta | +2.10 | +1.49 |  |  |  |  |  |  |  | -1.1% | -0.1% |  |  |
| overcast_broken | classic | 13.81 | 13.70 |  |  |  |  |  |  | 49.40 | 6,845 | 5,083 | OK:30 | OK |
| overcast_broken | remastered | 15.73 | 14.59 | 0.07 | 1.04 | 0.96 | 0.05 |  | 0.03 | 38.20 | 6,843 | 5,083 | OK:30 | OK |
| overcast_broken | delta | +1.92 | +0.89 |  |  |  |  |  |  |  | -0.0% | +0.0% |  |  |
| overcast_heavy | classic | 12.26 | 12.14 |  |  |  |  |  |  | 55.10 | 6,601 | 5,083 | OK:30 | OK |
| overcast_heavy | remastered | 20.18 | 19.32 | 0.03 | 0.73 | 0.65 | 0.05 |  | 0.03 | 45.00 | 6,599 | 5,083 | OK:30 | OK |
| overcast_heavy | delta | +7.92 | +7.18 |  |  |  |  |  |  |  | -0.0% | +0.0% |  |  |
| sinai_lowsun_fp | classic | 2.63 | 2.52 |  |  |  |  |  |  | 137.80 | 586 | 314 | OK:30 | OK |
| sinai_lowsun_fp | remastered | 3.53 | 2.26 | 0.05 | 1.15 | 0.84 | 0.05 | 0.22 | 0.03 | 164.80 | 583 | 314 | OK:30 | OK |
| sinai_lowsun_fp | delta | +0.90 | -0.26 |  |  |  |  |  |  |  | -0.5% | +0.0% |  |  |
| sinai_desert_noon | classic | 6.75 | 6.64 |  |  |  |  |  |  | 81.80 | 2,235 | 1,100 | OK:30 | OK |
| sinai_desert_noon | remastered | 7.50 | 6.59 | 0.29 | 0.78 | 0.70 | 0.05 |  | 0.03 | 79.20 | 2,232 | 1,100 | OK:30 | OK |
| sinai_desert_noon | delta | +0.75 | -0.05 |  |  |  |  |  |  |  | -0.1% | +0.0% |  |  |
