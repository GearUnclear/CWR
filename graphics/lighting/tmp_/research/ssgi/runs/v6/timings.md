# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 5.24 | 5.14 |  |  |  |  |  |  | 148.30 | OK |
| lowsun_fp | remastered | 9.09 | 7.69 | 0.10 | 1.29 | 0.21 | 0.05 | 0.22 | 0.82 | 102.10 | OK |
| lowsun_fp | delta | +3.85 | +2.55 |  |  |  |  |  |  |  |  |
| noon_fields | classic | 14.61 | 14.50 |  |  |  |  |  |  | 61.90 | OK |
| noon_fields | remastered | 11.65 | 11.09 | 0.05 | 0.45 | 0.37 | 0.05 |  | 0.03 | 45.00 | OK |
| noon_fields | delta | -2.96 | -3.41 |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 6.13 | 6.00 |  |  |  |  |  |  | 136.80 | OK |
| forest_edge_golden | remastered | 9.02 | 8.49 | 0.03 | 0.39 | 0.31 | 0.05 |  | 0.03 | 72.20 | OK |
| forest_edge_golden | delta | +2.89 | +2.49 |  |  |  |  |  |  |  |  |
