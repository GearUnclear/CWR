# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| noon_fields | classic | 1.33 | 1.23 |  |  |  |  |  |  | 280.50 | OK |
| noon_fields | remastered | 20.72 | 20.29 | 0.14 | 0.32 | 0.24 | 0.05 |  | 0.03 | 55.80 | OK |
| noon_fields | delta | +19.39 | +19.06 |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 4.74 | 4.62 |  |  |  |  |  |  | 232.80 | OK |
| forest_edge_golden | remastered | 7.44 | 6.90 | 0.03 | 0.40 | 0.33 | 0.05 |  | 0.03 | 99.70 | OK |
| forest_edge_golden | delta | +2.70 | +2.28 |  |  |  |  |  |  |  |  |
| lowsun_fp | classic | 7.57 | 7.37 |  |  |  |  |  |  | 118.50 | OK |
| lowsun_fp | remastered | 8.31 | 7.18 | 0.19 | 1.00 | 0.25 | 0.05 | 0.22 | 0.48 | 106.50 | OK |
| lowsun_fp | delta | +0.74 | -0.19 |  |  |  |  |  |  |  |  |
