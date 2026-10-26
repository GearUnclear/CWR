# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 3.26 | 3.23 |  |  |  |  |  |  | 175.30 | OK |
| lowsun_fp | remastered | 12.29 | 10.78 | 0.07 | 1.48 | 0.23 | 0.05 | 0.22 | 0.98 | 77.20 | OK |
| lowsun_fp | delta | +9.03 | +7.55 |  |  |  |  |  |  |  |  |
| lowsun_box | classic | 5.44 | 5.41 |  |  |  |  |  |  | 112.00 | OK |
| lowsun_box | remastered | 10.48 | 10.15 | 0.12 | 0.30 | 0.22 | 0.05 |  | 0.03 | 89.70 | OK |
| lowsun_box | delta | +5.04 | +4.74 |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 3.66 | 3.63 |  |  |  |  |  |  | 204.30 | OK |
| forest_edge_golden | remastered | 6.07 | 5.73 | 0.03 | 0.30 | 0.22 | 0.05 |  | 0.03 | 110.90 | OK |
| forest_edge_golden | delta | +2.41 | +2.10 |  |  |  |  |  |  |  |  |
| coast_box | classic | 3.42 | 3.40 |  |  |  |  |  |  | 151.60 | OK |
| coast_box | remastered | 7.28 | 6.98 | 0.04 | 0.26 | 0.19 | 0.05 |  | 0.03 | 106.90 | OK |
| coast_box | delta | +3.86 | +3.58 |  |  |  |  |  |  |  |  |
| noon_fields | classic | 17.89 | 17.86 |  |  |  |  |  |  | 47.70 | OK |
| noon_fields | remastered | 25.76 | 25.48 | 0.24 | 0.25 | 0.18 | 0.05 |  | 0.03 | 36.00 | OK |
| noon_fields | delta | +7.87 | +7.62 |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 2.01 | 1.98 |  |  |  |  |  |  | 148.30 | OK |
| sinai_lowsun_fp | remastered | 6.83 | 5.58 | 0.06 | 1.22 | 0.53 | 0.05 | 0.22 | 0.42 | 124.60 | OK |
| sinai_lowsun_fp | delta | +4.82 | +3.60 |  |  |  |  |  |  |  |  |
| sinai_box | classic | 13.05 | 13.02 |  |  |  |  |  |  | 61.50 | OK |
| sinai_box | remastered | 12.30 | 11.99 | 0.53 | 0.27 | 0.20 | 0.05 |  | 0.03 | 58.60 | OK |
| sinai_box | delta | -0.75 | -1.03 |  |  |  |  |  |  |  |  |
