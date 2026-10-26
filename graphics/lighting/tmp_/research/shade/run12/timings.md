# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| forest_edge_noon | classic | 1.98 | 1.86 |  |  |  |  |  |  | 316.50 | OK |
| forest_edge_noon | remastered | 6.81 | 6.45 | 0.03 | 0.24 | 0.16 | 0.05 |  | 0.03 | 113.40 | OK |
| forest_edge_noon | delta | +4.83 | +4.59 |  |  |  |  |  |  |  |  |
| lowsun_fp | classic | 8.64 | 8.53 |  |  |  |  |  |  | 85.30 | OK |
| lowsun_fp | remastered | 12.42 | 10.94 | 0.12 | 1.38 | 0.17 | 0.05 | 0.22 | 0.94 | 74.00 | OK |
| lowsun_fp | delta | +3.78 | +2.41 |  |  |  |  |  |  |  |  |
| sunrise_ridge | classic | 35.09 | 34.97 |  |  |  |  |  |  | 40.60 | OK |
| sunrise_ridge | remastered | 42.69 | 42.12 | 0.00 | 0.44 | 0.21 | 0.05 | 0.15 | 0.03 | 19.30 | OK |
| sunrise_ridge | delta | +7.60 | +7.15 |  |  |  |  |  |  |  |  |
| noon_fields | classic | 26.12 | 26.02 |  |  |  |  |  |  | 32.10 | OK |
| noon_fields | remastered | 35.18 | 34.84 | 0.09 | 0.24 | 0.16 | 0.05 |  | 0.03 | 25.60 | OK |
| noon_fields | delta | +9.06 | +8.82 |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 6.77 | 6.65 |  |  |  |  |  |  | 229.60 | OK |
| forest_edge_golden | remastered | 14.43 | 14.06 | 0.03 | 0.24 | 0.17 | 0.05 |  | 0.03 | 64.30 | OK |
| forest_edge_golden | delta | +7.66 | +7.41 |  |  |  |  |  |  |  |  |
| coastline_sunset | classic | 1.73 | 1.63 |  |  |  |  |  |  | 376.90 | OK |
| coastline_sunset | remastered | 3.78 | 3.30 | 0.00 | 0.38 | 0.14 | 0.05 | 0.16 | 0.03 | 228.90 | OK |
| coastline_sunset | delta | +2.05 | +1.67 |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 6.16 | 6.04 |  |  |  |  |  |  | 131.70 | OK |
| sinai_lowsun_fp | remastered | 9.60 | 8.04 | 0.11 | 1.44 | 0.30 | 0.05 | 0.22 | 0.88 | 119.10 | OK |
| sinai_lowsun_fp | delta | +3.44 | +2.00 |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 21.62 | 21.50 |  |  |  |  |  |  | 51.30 | OK |
| sinai_desert_noon | remastered | 21.36 | 20.62 | 0.04 | 0.62 | 0.54 | 0.05 |  | 0.03 | 36.50 | OK |
| sinai_desert_noon | delta | -0.26 | -0.88 |  |  |  |  |  |  |  |  |
