# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| noon_fields | classic | 4.34 | 4.23 |  |  |  |  |  |  | 253.80 | OK |
| noon_fields | remastered | 34.23 | 33.88 | 0.04 | 0.24 | 0.17 | 0.05 |  | 0.03 | 36.50 | OK |
| noon_fields | delta | +29.89 | +29.65 |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 8.28 | 8.16 |  |  |  |  |  |  | 203.20 | OK |
| forest_edge_golden | remastered | 15.56 | 15.17 | 0.03 | 0.26 | 0.18 | 0.05 |  | 0.03 | 56.50 | OK |
| forest_edge_golden | delta | +7.28 | +7.01 |  |  |  |  |  |  |  |  |
| lowsun_fp | classic | 11.71 | 11.59 |  |  |  |  |  |  | 78.20 | OK |
| lowsun_fp | remastered | 14.85 | 13.19 | 0.17 | 1.55 | 0.17 | 0.05 | 0.22 | 1.11 | 59.60 | OK |
| lowsun_fp | delta | +3.14 | +1.60 |  |  |  |  |  |  |  |  |
