# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| noon_fields | classic | 3.09 | 2.98 |  |  |  |  |  |  | 261.00 | OK |
| noon_fields | remastered | 24.77 | 24.35 | 0.06 | 0.31 | 0.24 | 0.05 |  | 0.03 | 51.30 | OK |
| noon_fields | delta | +21.68 | +21.37 |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 5.93 | 5.80 |  |  |  |  |  |  | 194.00 | OK |
| forest_edge_golden | remastered | 8.43 | 7.87 | 0.03 | 0.42 | 0.34 | 0.05 |  | 0.03 | 76.20 | OK |
| forest_edge_golden | delta | +2.50 | +2.07 |  |  |  |  |  |  |  |  |
| lowsun_fp | classic | 10.67 | 10.56 |  |  |  |  |  |  | 74.90 | OK |
| lowsun_fp | remastered | 14.36 | 12.50 | 0.05 | 1.74 | 0.18 | 0.05 | 0.22 | 1.29 | 62.50 | OK |
| lowsun_fp | delta | +3.69 | +1.94 |  |  |  |  |  |  |  |  |
