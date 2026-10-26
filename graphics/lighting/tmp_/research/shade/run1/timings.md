# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 4.00 | 3.89 |  |  |  |  |  |  | 86.90 | OK |
| lowsun_fp | remastered | 9.24 | 7.92 | 0.11 | 1.21 | 0.68 | 0.05 | 0.22 | 0.26 | 61.20 | OK |
| lowsun_fp | delta | +5.24 | +4.03 |  |  |  |  |  |  |  |  |
| lowsun_box | classic | 8.67 | 8.56 |  |  |  |  |  |  | 75.70 | OK |
| lowsun_box | remastered | 11.92 | 10.69 | 0.04 | 1.11 | 1.03 | 0.05 |  | 0.03 | 45.60 | OK |
| lowsun_box | delta | +3.25 | +2.13 |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 5.54 | 5.40 |  |  |  |  |  |  | 64.20 | OK |
| forest_edge_golden | remastered | 9.15 | 8.53 | 0.12 | 0.48 | 0.41 | 0.05 |  | 0.03 | 50.30 | OK |
| forest_edge_golden | delta | +3.61 | +3.13 |  |  |  |  |  |  |  |  |
| evening_box | classic | 4.31 | 4.20 |  |  |  |  |  |  | 100.20 | OK |
| evening_box | remastered | 8.32 | 7.48 | 0.12 | 0.72 | 0.64 | 0.05 |  | 0.03 | 68.70 | OK |
| evening_box | delta | +4.01 | +3.28 |  |  |  |  |  |  |  |  |
| noon_fields | classic | 22.20 | 22.09 |  |  |  |  |  |  | 38.70 | OK |
| noon_fields | remastered | 19.05 | 18.12 | 0.04 | 0.82 | 0.74 | 0.05 |  | 0.03 | 29.30 | OK |
| noon_fields | delta | -3.15 | -3.97 |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 4.69 | 4.57 |  |  |  |  |  |  | 77.80 | OK |
| sinai_lowsun_fp | remastered | 7.62 | 5.84 | 0.10 | 1.53 | 1.19 | 0.05 | 0.22 | 0.07 | 54.80 | OK |
| sinai_lowsun_fp | delta | +2.93 | +1.27 |  |  |  |  |  |  |  |  |
| sinai_box | classic | 18.78 | 18.66 |  |  |  |  |  |  | 33.70 | OK |
| sinai_box | remastered | 18.87 | 17.35 | 0.06 | 1.39 | 1.31 | 0.05 |  | 0.03 | 33.20 | OK |
| sinai_box | delta | +0.09 | -1.31 |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 14.24 | 14.00 |  |  |  |  |  |  | 44.00 | OK |
| sinai_desert_noon | remastered | 13.91 | 12.06 | 0.56 | 1.70 | 1.59 | 0.07 |  | 0.04 | 34.40 | OK |
| sinai_desert_noon | delta | -0.33 | -1.94 |  |  |  |  |  |  |  |  |
