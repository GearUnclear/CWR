# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 5.56 | 5.46 |  |  |  |  |  |  | 130.80 | OK |
| lowsun_fp | remastered | 13.52 | 11.71 | 0.12 | 1.71 | 0.24 | 0.05 | 0.22 | 1.20 | 80.00 | OK |
| lowsun_fp | delta | +7.96 | +6.25 |  |  |  |  |  |  |  |  |
| lowsun_box | classic | 7.56 | 7.45 |  |  |  |  |  |  | 200.90 | OK |
| lowsun_box | remastered | 14.76 | 14.29 | 0.13 | 0.35 | 0.28 | 0.05 |  | 0.03 | 57.90 | OK |
| lowsun_box | delta | +7.20 | +6.84 |  |  |  |  |  |  |  |  |
| dawn_box | classic | 9.47 | 9.37 |  |  |  |  |  |  | 194.80 | OK |
| dawn_box | remastered | 5.65 | 4.93 | 0.04 | 0.61 | 0.53 | 0.05 |  | 0.03 | 97.30 | OK |
| dawn_box | delta | -3.82 | -4.44 |  |  |  |  |  |  |  |  |
| dawn_box5 | classic | 5.21 | 5.11 |  |  |  |  |  |  | 199.80 | OK |
| dawn_box5 | remastered | 8.72 | 8.30 | 0.11 | 0.32 | 0.24 | 0.05 |  | 0.03 | 84.10 | OK |
| dawn_box5 | delta | +3.51 | +3.19 |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 4.54 | 4.42 |  |  |  |  |  |  | 226.20 | OK |
| forest_edge_golden | remastered | 5.54 | 5.08 | 0.03 | 0.33 | 0.25 | 0.05 |  | 0.03 | 101.70 | OK |
| forest_edge_golden | delta | +1.00 | +0.66 |  |  |  |  |  |  |  |  |
| forest_edge_noon | classic | 2.36 | 2.25 |  |  |  |  |  |  | 220.10 | OK |
| forest_edge_noon | remastered | 4.03 | 3.51 | 0.03 | 0.39 | 0.32 | 0.05 |  | 0.03 | 142.30 | OK |
| forest_edge_noon | delta | +1.67 | +1.26 |  |  |  |  |  |  |  |  |
| coastline_sunset | classic | 0.56 | 0.46 |  |  |  |  |  |  | 504.30 | OK |
| coastline_sunset | remastered | 1.48 | 0.84 | 0.00 | 0.55 | 0.30 | 0.05 | 0.16 | 0.03 | 362.10 | OK |
| coastline_sunset | delta | +0.92 | +0.38 |  |  |  |  |  |  |  |  |
| sunrise_ridge | classic | 3.89 | 3.77 |  |  |  |  |  |  | 122.50 | OK |
| sunrise_ridge | remastered | 39.72 | 38.99 | 0.00 | 0.60 | 0.37 | 0.05 | 0.15 | 0.03 | 28.50 | OK |
| sunrise_ridge | delta | +35.83 | +35.22 |  |  |  |  |  |  |  |  |
| noon_fields | classic | 20.95 | 20.67 |  |  |  |  |  |  | 44.80 | OK |
| noon_fields | remastered | 28.12 | 27.68 | 0.24 | 0.33 | 0.25 | 0.05 |  | 0.03 | 29.80 | OK |
| noon_fields | delta | +7.17 | +7.01 |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 5.11 | 5.00 |  |  |  |  |  |  | 133.30 | OK |
| sinai_lowsun_fp | remastered | 3.30 | 2.21 | 0.06 | 0.95 | 0.55 | 0.05 | 0.22 | 0.14 | 177.10 | OK |
| sinai_lowsun_fp | delta | -1.81 | -2.79 |  |  |  |  |  |  |  |  |
| sinai_box | classic | 7.22 | 7.10 |  |  |  |  |  |  | 81.50 | OK |
| sinai_box | remastered | 7.83 | 7.10 | 0.05 | 0.62 | 0.54 | 0.05 |  | 0.03 | 84.10 | OK |
| sinai_box | delta | +0.61 | +0.00 |  |  |  |  |  |  |  |  |
| sinai_dusk_box | classic | 8.33 | 8.21 |  |  |  |  |  |  | 71.20 | OK |
| sinai_dusk_box | remastered | 6.92 | 6.28 | 0.16 | 0.52 | 0.44 | 0.05 |  | 0.03 | 85.90 | OK |
| sinai_dusk_box | delta | -1.41 | -1.93 |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 9.10 | 8.99 |  |  |  |  |  |  | 75.00 | OK |
| sinai_desert_noon | remastered | 9.23 | 8.37 | 0.22 | 0.74 | 0.66 | 0.05 |  | 0.03 | 70.80 | OK |
| sinai_desert_noon | delta | +0.13 | -0.62 |  |  |  |  |  |  |  |  |
