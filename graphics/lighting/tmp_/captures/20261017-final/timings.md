# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 3.80 | 3.48 |  |  |  |  |  |  | 168.10 | OK |
| lowsun_fp | remastered | 4.90 | 3.96 | 0.05 | 0.83 | 0.42 | 0.06 | 0.27 | 0.08 | 122.40 | OK |
| lowsun_fp | delta | +1.10 | +0.48 |  |  |  |  |  |  |  |  |
| sunrise_ridge | classic | 7.17 | 6.93 |  |  |  |  |  |  | 73.40 | OK |
| sunrise_ridge | remastered | 18.44 | 17.57 | 0.00 | 0.75 | 0.52 | 0.05 | 0.15 | 0.03 | 49.00 | OK |
| sunrise_ridge | delta | +11.27 | +10.64 |  |  |  |  |  |  |  |  |
| noon_fields | classic | 6.04 | 5.94 |  |  |  |  |  |  | 89.30 | OK |
| noon_fields | remastered | 8.23 | 7.74 | 0.04 | 0.39 | 0.32 | 0.05 |  | 0.03 | 78.30 | OK |
| noon_fields | delta | +2.19 | +1.80 |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 3.56 | 3.38 |  |  |  |  |  |  | 219.80 | OK |
| forest_edge_golden | remastered | 4.73 | 4.12 | 0.04 | 0.46 | 0.36 | 0.07 |  | 0.04 | 130.10 | OK |
| forest_edge_golden | delta | +1.17 | +0.74 |  |  |  |  |  |  |  |  |
| coastline_sunset | classic | 2.93 | 2.52 |  |  |  |  |  |  | 275.80 | OK |
| coastline_sunset | remastered | 4.44 | 2.11 | 0.00 | 1.79 | 0.85 | 0.23 | 0.54 | 0.18 | 196.20 | OK |
| coastline_sunset | delta | +1.51 | -0.41 |  |  |  |  |  |  |  |  |
| town_night | classic | 5.63 | 5.38 |  |  |  |  |  |  | 143.10 | OK |
| town_night | remastered | 5.57 | 4.85 |  | 0.54 | 0.45 | 0.06 |  | 0.03 | 121.80 | OK |
| town_night | delta | -0.06 | -0.53 |  |  |  |  |  |  |  |  |
| firefight_1 | classic | 4.45 | 3.97 |  |  |  |  |  |  | 201.00 | OK |
| firefight_1 | remastered | 3.85 | 2.75 |  | 0.93 | 0.82 | 0.07 |  | 0.04 | 150.50 | OK |
| firefight_1 | delta | -0.60 | -1.22 |  |  |  |  |  |  |  |  |
| firefight_2 | classic | 4.45 | 3.97 |  |  |  |  |  |  | 201.00 | OK |
| firefight_2 | remastered | 3.85 | 2.75 |  | 0.93 | 0.82 | 0.07 |  | 0.04 | 150.50 | OK |
| firefight_2 | delta | -0.60 | -1.22 |  |  |  |  |  |  |  |  |
| firefight_3 | classic | 4.45 | 3.97 |  |  |  |  |  |  | 201.00 | OK |
| firefight_3 | remastered | 3.85 | 2.75 |  | 0.93 | 0.82 | 0.07 |  | 0.04 | 150.50 | OK |
| firefight_3 | delta | -0.60 | -1.22 |  |  |  |  |  |  |  |  |
| tank_gunner_interior | classic | 4.55 | 4.25 |  |  |  |  |  |  | 130.20 | OK |
| tank_gunner_interior | remastered | 6.42 | 5.63 | 0.04 | 0.48 |  | 0.45 |  | 0.03 | 103.90 | OK |
| tank_gunner_interior | delta | +1.87 | +1.38 |  |  |  |  |  |  |  |  |
| tank_gunner_optics | classic | 5.64 | 5.10 |  |  |  |  |  |  | 142.70 | OK |
| tank_gunner_optics | remastered | 5.41 | 4.56 | 0.07 | 0.51 | 0.48 |  |  | 0.03 | 129.40 | OK |
| tank_gunner_optics | delta | -0.23 | -0.54 |  |  |  |  |  |  |  |  |
| overcast_broken | classic | 8.88 | 8.77 |  |  |  |  |  |  | 67.80 | OK |
| overcast_broken | remastered | 10.49 | 9.85 | 0.03 | 0.54 | 0.47 | 0.05 |  | 0.03 | 61.80 | OK |
| overcast_broken | delta | +1.61 | +1.08 |  |  |  |  |  |  |  |  |
| overcast_heavy | classic | 10.07 | 9.96 |  |  |  |  |  |  | 65.20 | OK |
| overcast_heavy | remastered | 10.43 | 9.81 | 0.03 | 0.51 | 0.43 | 0.05 |  | 0.03 | 62.40 | OK |
| overcast_heavy | delta | +0.36 | -0.15 |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 1.92 | 1.76 |  |  |  |  |  |  | 176.30 | OK |
| sinai_lowsun_fp | remastered | 4.47 | 3.18 | 0.09 | 1.13 | 0.65 | 0.07 | 0.32 | 0.09 | 115.50 | OK |
| sinai_lowsun_fp | delta | +2.55 | +1.42 |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 6.50 | 6.36 |  |  |  |  |  |  | 91.30 | OK |
| sinai_desert_noon | remastered | 7.34 | 6.39 | 0.07 | 0.82 | 0.72 | 0.06 |  | 0.04 | 80.90 | OK |
| sinai_desert_noon | delta | +0.84 | +0.03 |  |  |  |  |  |  |  |  |
