# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 8.47 | 8.36 |  |  |  |  |  |  | 104.60 | OK |
| lowsun_fp | remastered | 17.95 | 15.69 | 0.44 | 2.16 | 0.18 | 0.05 | 0.22 | 1.71 | 60.50 | OK |
| lowsun_fp | delta | +9.48 | +7.33 |  |  |  |  |  |  |  |  |
| noon_fields | classic | 8.07 | 7.94 |  |  |  |  |  |  | 65.20 | OK |
| noon_fields | remastered | 24.89 | 24.54 | 0.04 | 0.24 | 0.16 | 0.05 |  | 0.03 | 34.80 | OK |
| noon_fields | delta | +16.82 | +16.60 |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 5.69 | 5.56 |  |  |  |  |  |  | 206.80 | OK |
| forest_edge_golden | remastered | 8.56 | 7.99 | 0.03 | 0.43 | 0.36 | 0.05 |  | 0.03 | 86.40 | OK |
| forest_edge_golden | delta | +2.87 | +2.43 |  |  |  |  |  |  |  |  |
| town_night | classic | 6.18 | 6.01 |  |  |  |  |  |  | 128.70 | OK |
| town_night | remastered | 7.48 | 6.83 |  | 0.45 | 0.37 | 0.05 |  | 0.03 | 98.50 | OK |
| town_night | delta | +1.30 | +0.82 |  |  |  |  |  |  |  |  |
