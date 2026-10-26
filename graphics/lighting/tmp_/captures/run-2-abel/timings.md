# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 2.92 | 2.73 |  |  |  |  |  |  | 200.70 | OK |
| lowsun_fp | remastered | 4.94 | 3.97 | 0.05 | 0.85 | 0.41 | 0.06 | 0.27 | 0.12 | 127.90 | OK |
| lowsun_fp | delta | +2.02 | +1.24 |  |  |  |  |  |  |  |  |
| sunrise_ridge | classic | 6.82 | 6.69 |  |  |  |  |  |  | 75.40 | OK |
| sunrise_ridge | remastered | 18.64 | 17.78 | 0.00 | 0.74 | 0.51 | 0.05 | 0.15 | 0.03 | 49.10 | OK |
| sunrise_ridge | delta | +11.82 | +11.09 |  |  |  |  |  |  |  |  |
| noon_fields | classic | 6.04 | 5.94 |  |  |  |  |  |  | 86.70 | OK |
| noon_fields | remastered | 8.22 | 7.71 | 0.04 | 0.41 | 0.34 | 0.05 |  | 0.03 | 77.70 | OK |
| noon_fields | delta | +2.18 | +1.77 |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 3.68 | 3.50 |  |  |  |  |  |  | 225.30 | OK |
| forest_edge_golden | remastered | 4.86 | 4.22 | 0.04 | 0.48 | 0.38 | 0.06 |  | 0.04 | 126.60 | OK |
| forest_edge_golden | delta | +1.18 | +0.72 |  |  |  |  |  |  |  |  |
| coastline_sunset | classic | 2.04 | 1.70 |  |  |  |  |  |  | 277.20 | OK |
| coastline_sunset | remastered | 4.34 | 2.01 | 0.00 | 1.97 | 0.88 | 0.17 | 0.70 | 0.22 | 203.20 | OK |
| coastline_sunset | delta | +2.30 | +0.31 |  |  |  |  |  |  |  |  |
| town_night | classic | 5.66 | 5.43 |  |  |  |  |  |  | 145.20 | OK |
| town_night | remastered | 6.87 | 6.08 |  | 0.59 | 0.48 | 0.07 |  | 0.04 | 116.00 | OK |
| town_night | delta | +1.21 | +0.65 |  |  |  |  |  |  |  |  |
| firefight_1 | classic | 5.19 | 4.74 |  |  |  |  |  |  | 185.70 | OK |
| firefight_1 | remastered | 3.79 | 2.74 |  | 0.88 | 0.77 | 0.07 |  | 0.04 | 150.30 | OK |
| firefight_1 | delta | -1.40 | -2.00 |  |  |  |  |  |  |  |  |
| firefight_2 | classic | 5.19 | 4.74 |  |  |  |  |  |  | 185.70 | OK |
| firefight_2 | remastered | 3.79 | 2.74 |  | 0.88 | 0.77 | 0.07 |  | 0.04 | 150.30 | OK |
| firefight_2 | delta | -1.40 | -2.00 |  |  |  |  |  |  |  |  |
| firefight_3 | classic | 5.19 | 4.74 |  |  |  |  |  |  | 185.70 | OK |
| firefight_3 | remastered | 3.79 | 2.74 |  | 0.88 | 0.77 | 0.07 |  | 0.04 | 150.30 | OK |
| firefight_3 | delta | -1.40 | -2.00 |  |  |  |  |  |  |  |  |
| tank_gunner_interior | classic | 4.68 | 4.35 |  |  |  |  |  |  | 127.50 | OK |
| tank_gunner_interior | remastered | 6.47 | 5.68 | 0.04 | 0.48 |  | 0.45 |  | 0.03 | 105.10 | OK |
| tank_gunner_interior | delta | +1.79 | +1.33 |  |  |  |  |  |  |  |  |
| tank_gunner_optics | classic | 4.95 | 4.57 |  |  |  |  |  |  | 149.70 | OK |
| tank_gunner_optics | remastered | 4.99 | 4.13 | 0.06 | 0.56 | 0.53 |  |  | 0.03 | 134.20 | OK |
| tank_gunner_optics | delta | +0.04 | -0.44 |  |  |  |  |  |  |  |  |
