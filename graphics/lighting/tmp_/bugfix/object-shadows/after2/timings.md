# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 5.37 | 5.27 |  |  |  |  |  |  | 122.30 | 1,778 | 1,179 | OK:30 | OK |
| lowsun_fp | remastered | 5.39 | 4.34 | 0.07 | 0.94 | 0.29 | 0.05 | 0.22 | 0.38 | 115.40 | 1,615 | 1,179 | OK:30 | OK |
| lowsun_fp | delta | +0.02 | -0.93 |  |  |  |  |  |  |  | -9.2% | +0.0% |  |  |
| sunrise_ridge | classic | 16.45 | 16.34 |  |  |  |  |  |  | 39.20 | 11,301 | 7,680 | OK:30 | OK |
| sunrise_ridge | remastered | 17.74 | 16.95 | 0.11 | 0.68 | 0.46 | 0.05 | 0.15 | 0.03 | 43.00 | 11,172 | 7,680 | OK:30 | OK |
| sunrise_ridge | delta | +1.29 | +0.61 |  |  |  |  |  |  |  | -1.1% | +0.0% |  |  |
| noon_fields | classic | 7.01 | 6.91 |  |  |  |  |  |  | 81.30 | 5,878 | 3,920 | OK:30 | OK |
| noon_fields | remastered | 7.68 | 7.18 | 0.07 | 0.40 | 0.32 | 0.05 |  | 0.03 | 81.10 | 5,575 | 3,920 | OK:30 | OK |
| noon_fields | delta | +0.67 | +0.27 |  |  |  |  |  |  |  | -5.2% | +0.0% |  |  |
| forest_edge_golden | classic | 5.97 | 5.80 |  |  |  |  |  |  | 167.70 | 2,189 | 1,316 | OK:30 | OK |
| forest_edge_golden | remastered | 5.63 | 4.85 | 0.30 | 0.60 | 0.46 | 0.08 |  | 0.06 | 114.80 | 1,847 | 1,316 | OK:30 | OK |
| forest_edge_golden | delta | -0.34 | -0.95 |  |  |  |  |  |  |  | -15.6% | +0.0% |  |  |
| coastline_sunset | classic | 3.40 | 2.84 |  |  |  |  |  |  | 230.80 | 155 | 0 | OK:30 | OK |
| coastline_sunset | remastered | 4.81 | 2.35 | 0.00 | 2.07 | 0.85 | 0.26 | 0.73 | 0.22 | 201.20 | 155 | 0 | OK:30 | OK |
| coastline_sunset | delta | +1.41 | -0.49 |  |  |  |  |  |  |  | +0.0% |  |  |  |
| town_night | classic | 6.68 | 6.40 |  |  |  |  |  |  | 115.10 | 1,214 | 1,040 | OK:266 | OK |
| town_night | remastered | 5.98 | 5.20 |  | 0.57 | 0.48 | 0.05 |  | 0.03 | 123.70 | 1,199 | 1,036 | OK:30 | OK |
| town_night | delta | -0.70 | -1.20 |  |  |  |  |  |  |  | -1.2% | -0.4% |  |  |
| firefight_1 | classic | 4.14 | 3.72 |  |  |  |  |  |  | 177.10 | 465 | 403 | OK:47 | OK |
| firefight_1 | remastered | 5.09 | 3.23 |  | 1.22 | 1.00 | 0.11 |  | 0.11 | 130.80 | 472 | 429 | OK:66 | OK |
| firefight_1 | delta | +0.95 | -0.49 |  |  |  |  |  |  |  | +1.5% | +6.1% |  |  |
| firefight_2 | classic | 4.14 | 3.72 |  |  |  |  |  |  | 177.10 | 539 | 484 | OK:47 | OK |
| firefight_2 | remastered | 5.09 | 3.23 |  | 1.22 | 1.00 | 0.11 |  | 0.11 | 130.80 | 550 | 513 | OK:66 | OK |
| firefight_2 | delta | +0.95 | -0.49 |  |  |  |  |  |  |  | +2.0% | +5.7% |  |  |
| firefight_3 | classic | 4.14 | 3.72 |  |  |  |  |  |  | 177.10 | 480 | 422 | OK:47 | OK |
| firefight_3 | remastered | 5.09 | 3.23 |  | 1.22 | 1.00 | 0.11 |  | 0.11 | 130.80 | 494 | 436 | OK:66 | OK |
| firefight_3 | delta | +0.95 | -0.49 |  |  |  |  |  |  |  | +2.8% | +3.2% |  |  |
| tank_gunner_interior | classic | 5.54 | 5.25 |  |  |  |  |  |  | 114.90 | 2,541 | 1,229 | OK:30 | OK |
| tank_gunner_interior | remastered | 6.40 | 5.61 | 0.07 | 0.50 |  | 0.47 |  | 0.03 | 104.50 | 2,323 | 1,244 | OK:30 | OK |
| tank_gunner_interior | delta | +0.86 | +0.36 |  |  |  |  |  |  |  | -8.6% | +1.2% |  |  |
| tank_gunner_optics | classic | 5.39 | 4.93 |  |  |  |  |  |  | 134.30 | 1,731 | 745 | OK:30 | OK |
| tank_gunner_optics | remastered | 4.80 | 4.00 | 0.09 | 0.52 | 0.48 |  |  | 0.03 | 119.10 | 1,524 | 745 | OK:30 | OK |
| tank_gunner_optics | delta | -0.59 | -0.93 |  |  |  |  |  |  |  | -12.0% | +0.0% |  |  |
