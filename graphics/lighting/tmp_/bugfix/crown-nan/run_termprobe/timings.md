# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| forest_edge_golden | classic | missing |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | remastered | 5.15 | 4.77 | 0.03 | 0.24 | 0.17 | 0.05 |  | 0.03 | 145.30 | OK |
| woods_mixed_noon | classic | missing |  |  |  |  |  |  |  |  |  |
| woods_mixed_noon | remastered | 4.40 | 3.93 | 0.03 | 0.37 | 0.30 | 0.05 |  | 0.03 | 147.50 | OK |
| woods_pine2_evening | classic | missing |  |  |  |  |  |  |  |  |  |
| woods_pine2_evening | remastered | 5.24 | 4.12 | 0.03 | 1.01 | 0.29 | 0.05 | 0.29 | 0.38 | 136.30 | OK |
| town_day | classic | missing |  |  |  |  |  |  |  |  |  |
| town_day | remastered | 4.50 | 4.08 | 0.04 | 0.28 | 0.21 | 0.05 |  | 0.03 | 157.30 | OK |
