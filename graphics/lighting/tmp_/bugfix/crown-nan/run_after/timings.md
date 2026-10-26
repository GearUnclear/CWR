# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| forest_edge_golden | classic | missing |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | remastered | 3.61 | 2.72 | 0.03 | 0.74 | 0.67 | 0.05 |  | 0.03 | 97.70 | OK |
| woods_mixed_noon | classic | missing |  |  |  |  |  |  |  |  |  |
| woods_mixed_noon | remastered | 3.70 | 2.67 | 0.04 | 0.92 | 0.84 | 0.05 |  | 0.03 | 122.70 | OK |
| woods_pine2_evening | classic | missing |  |  |  |  |  |  |  |  |  |
| woods_pine2_evening | remastered | 8.07 | 6.37 | 0.03 | 1.59 | 0.69 | 0.05 | 0.29 | 0.55 | 74.00 | OK |
| town_day | classic | missing |  |  |  |  |  |  |  |  |  |
| town_day | remastered | 4.94 | 4.16 | 0.04 | 0.64 | 0.56 | 0.05 |  | 0.03 | 86.70 | OK |
