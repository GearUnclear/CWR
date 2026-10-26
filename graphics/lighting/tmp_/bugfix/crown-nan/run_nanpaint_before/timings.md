# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| forest_edge_golden | classic | missing |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | remastered | 4.74 | 4.31 | 0.03 | 0.29 | 0.21 | 0.05 |  | 0.03 | 146.40 | OK |
| woods_mixed_noon | classic | missing |  |  |  |  |  |  |  |  |  |
| woods_mixed_noon | remastered | 3.98 | 3.36 | 0.03 | 0.52 | 0.44 | 0.05 |  | 0.03 | 156.40 | OK |
| woods_pine2_evening | classic | missing |  |  |  |  |  |  |  |  |  |
| woods_pine2_evening | remastered | 8.81 | 6.78 | 0.03 | 1.92 | 0.29 | 0.05 | 0.28 | 1.30 | 82.60 | OK |
| town_day | classic | missing |  |  |  |  |  |  |  |  |  |
| town_day | remastered | 6.08 | 5.57 | 0.04 | 0.37 | 0.29 | 0.05 |  | 0.03 | 108.70 | OK |
