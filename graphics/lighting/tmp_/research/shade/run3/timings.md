# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 7.23 | 7.13 |  |  |  |  |  |  | 100.30 | OK |
| lowsun_fp | remastered | 13.84 | 12.24 | 0.04 | 1.48 | 0.28 | 0.05 | 0.22 | 0.94 | 68.20 | OK |
| lowsun_fp | delta | +6.61 | +5.11 |  |  |  |  |  |  |  |  |
| lowsun_box | classic | 9.32 | 9.21 |  |  |  |  |  |  | 129.50 | OK |
| lowsun_box | remastered | 12.68 | 12.17 | 0.04 | 0.40 | 0.32 | 0.05 |  | 0.03 | 62.60 | OK |
| lowsun_box | delta | +3.36 | +2.96 |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 7.22 | 7.09 |  |  |  |  |  |  | 164.60 | OK |
| forest_edge_golden | remastered | 9.74 | 9.22 | 0.03 | 0.38 | 0.30 | 0.05 |  | 0.03 | 72.20 | OK |
| forest_edge_golden | delta | +2.52 | +2.13 |  |  |  |  |  |  |  |  |
| sunrise_ridge | classic | 32.77 | 32.65 |  |  |  |  |  |  | 35.70 | OK |
| sunrise_ridge | remastered | 47.63 | 46.66 | 0.00 | 0.84 | 0.61 | 0.05 | 0.15 | 0.03 | 21.50 | OK |
| sunrise_ridge | delta | +14.86 | +14.01 |  |  |  |  |  |  |  |  |
| coastline_sunset | classic | 1.46 | 1.36 |  |  |  |  |  |  | 348.50 | OK |
| coastline_sunset | remastered | 3.71 | 3.19 | 0.00 | 0.41 | 0.16 | 0.05 | 0.16 | 0.03 | 219.60 | OK |
| coastline_sunset | delta | +2.25 | +1.83 |  |  |  |  |  |  |  |  |
| noon_fields | classic | 5.60 | 5.49 |  |  |  |  |  |  | 144.90 | OK |
| noon_fields | remastered | 22.49 | 22.03 | 0.04 | 0.34 | 0.26 | 0.05 |  | 0.03 | 37.40 | OK |
| noon_fields | delta | +16.89 | +16.54 |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 4.86 | 4.75 |  |  |  |  |  |  | 115.80 | OK |
| sinai_lowsun_fp | remastered | 3.96 | 2.97 | 0.05 | 0.87 | 0.51 | 0.05 | 0.22 | 0.09 | 144.30 | OK |
| sinai_lowsun_fp | delta | -0.90 | -1.78 |  |  |  |  |  |  |  |  |
| sinai_box | classic | 10.34 | 10.22 |  |  |  |  |  |  | 64.40 | OK |
| sinai_box | remastered | 11.13 | 10.69 | 0.09 | 0.31 | 0.24 | 0.05 |  | 0.03 | 56.50 | OK |
| sinai_box | delta | +0.79 | +0.47 |  |  |  |  |  |  |  |  |
