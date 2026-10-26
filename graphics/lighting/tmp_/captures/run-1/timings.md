# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 2.81 | 2.61 |  |  |  |  |  |  | 200.10 | OK |
| lowsun_fp | remastered | 4.87 | 3.89 | 0.05 | 0.86 | 0.43 | 0.06 | 0.26 | 0.11 | 128.60 | OK |
| lowsun_fp | delta | +2.06 | +1.28 |  |  |  |  |  |  |  |  |
| sunrise_ridge | classic | 7.62 | 7.50 |  |  |  |  |  |  | 72.00 | OK |
| sunrise_ridge | remastered | 18.69 | 17.76 | 0.00 | 0.81 | 0.59 | 0.05 | 0.15 | 0.03 | 48.30 | OK |
| sunrise_ridge | delta | +11.07 | +10.26 |  |  |  |  |  |  |  |  |
| noon_fields | classic | 6.24 | 6.14 |  |  |  |  |  |  | 89.40 | OK |
| noon_fields | remastered | 8.24 | 7.76 | 0.04 | 0.38 | 0.31 | 0.05 |  | 0.03 | 78.00 | OK |
| noon_fields | delta | +2.00 | +1.62 |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 3.79 | 3.61 |  |  |  |  |  |  | 224.90 | OK |
| forest_edge_golden | remastered | 4.56 | 3.94 | 0.04 | 0.46 | 0.37 | 0.06 |  | 0.04 | 130.00 | OK |
| forest_edge_golden | delta | +0.77 | +0.33 |  |  |  |  |  |  |  |  |
| coastline_sunset | classic | 2.29 | 1.88 |  |  |  |  |  |  | 276.40 | OK |
| coastline_sunset | remastered | 4.38 | 2.06 | 0.00 | 1.83 | 0.69 | 0.17 | 0.58 | 0.39 | 201.30 | OK |
| coastline_sunset | delta | +2.09 | +0.18 |  |  |  |  |  |  |  |  |
| town_night | classic | 5.82 | 5.58 |  |  |  |  |  |  | 137.60 | OK |
| town_night | remastered | 5.43 | 4.72 |  | 0.55 | 0.47 | 0.05 |  | 0.03 | 126.30 | OK |
| town_night | delta | -0.39 | -0.86 |  |  |  |  |  |  |  |  |
| firefight_1 | classic | 4.73 | 4.02 |  |  |  |  |  |  | 135.80 | OK |
| firefight_1 | remastered | 4.93 | 3.27 |  | 1.07 | 0.84 | 0.13 |  | 0.10 | 150.20 | OK |
| firefight_1 | delta | +0.20 | -0.75 |  |  |  |  |  |  |  |  |
| firefight_2 | classic | 4.73 | 4.02 |  |  |  |  |  |  | 135.80 | OK |
| firefight_2 | remastered | 4.93 | 3.27 |  | 1.07 | 0.84 | 0.13 |  | 0.10 | 150.20 | OK |
| firefight_2 | delta | +0.20 | -0.75 |  |  |  |  |  |  |  |  |
| firefight_3 | classic | 4.73 | 4.02 |  |  |  |  |  |  | 135.80 | OK |
| firefight_3 | remastered | 4.93 | 3.27 |  | 1.07 | 0.84 | 0.13 |  | 0.10 | 150.20 | OK |
| firefight_3 | delta | +0.20 | -0.75 |  |  |  |  |  |  |  |  |
| tank_gunner_interior | classic | 4.59 | 4.32 |  |  |  |  |  |  | 133.40 | OK |
| tank_gunner_interior | remastered | 6.57 | 5.79 | 0.06 | 0.49 |  | 0.46 |  | 0.03 | 103.90 | OK |
| tank_gunner_interior | delta | +1.98 | +1.47 |  |  |  |  |  |  |  |  |
| tank_gunner_optics | classic | 3.31 | 3.07 |  |  |  |  |  |  | 147.10 | OK |
| tank_gunner_optics | remastered | 4.18 | 3.44 | 0.04 | 0.43 | 0.41 |  |  | 0.02 | 159.90 | OK |
| tank_gunner_optics | delta | +0.87 | +0.37 |  |  |  |  |  |  |  |  |
| overcast_broken | classic | 8.88 | 8.77 |  |  |  |  |  |  | 68.50 | OK |
| overcast_broken | remastered | 10.45 | 9.77 | 0.03 | 0.58 | 0.51 | 0.05 |  | 0.03 | 61.80 | OK |
| overcast_broken | delta | +1.57 | +1.00 |  |  |  |  |  |  |  |  |
| overcast_heavy | classic | 8.63 | 8.52 |  |  |  |  |  |  | 70.00 | OK |
| overcast_heavy | remastered | 10.51 | 9.82 | 0.03 | 0.59 | 0.51 | 0.05 |  | 0.03 | 62.40 | OK |
| overcast_heavy | delta | +1.88 | +1.30 |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 3.97 | 3.65 |  |  |  |  |  |  | 128.00 | OK |
| sinai_lowsun_fp | remastered | 4.27 | 3.05 | 0.07 | 1.08 | 0.61 | 0.07 | 0.31 | 0.09 | 121.20 | OK |
| sinai_lowsun_fp | delta | +0.30 | -0.60 |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 6.52 | 6.39 |  |  |  |  |  |  | 91.90 | OK |
| sinai_desert_noon | remastered | 7.43 | 6.45 | 0.06 | 0.84 | 0.74 | 0.06 |  | 0.04 | 83.10 | OK |
| sinai_desert_noon | delta | +0.91 | +0.06 |  |  |  |  |  |  |  |  |
