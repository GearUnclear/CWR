# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| forest_edge_golden | classic | missing |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | remastered | 5.17 | 4.30 | 0.03 | 0.74 | 0.66 | 0.05 |  | 0.03 | 90.30 | OK |
| woods_mixed_noon | classic | missing |  |  |  |  |  |  |  |  |  |
| woods_mixed_noon | remastered | 4.88 | 4.32 | 0.03 | 0.46 | 0.38 | 0.05 |  | 0.03 | 138.60 | OK |
| woods_pine2_evening | classic | missing |  |  |  |  |  |  |  |  |  |
| woods_pine2_evening | remastered | 8.89 | 7.18 | 0.03 | 1.60 | 0.32 | 0.05 | 0.29 | 0.94 | 87.30 | OK |
| town_day | classic | missing |  |  |  |  |  |  |  |  |  |
| town_day | remastered | 6.47 | 6.02 | 0.04 | 0.31 | 0.23 | 0.05 |  | 0.03 | 100.90 | OK |
