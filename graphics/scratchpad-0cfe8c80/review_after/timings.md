# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| forest_edge_golden | classic | missing |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | remastered | 5.94 | 4.42 | 0.11 | 1.07 | 0.76 | 0.17 |  | 0.14 | 119.80 | OK |
| woods_mixed_noon | classic | missing |  |  |  |  |  |  |  |  |  |
| woods_mixed_noon | remastered | 3.81 | 3.00 | 0.06 | 0.66 | 0.51 | 0.09 |  | 0.05 | 173.20 | OK |
| woods_pine2_evening | classic | missing |  |  |  |  |  |  |  |  |  |
| woods_pine2_evening | remastered | 4.59 | 3.30 | 0.03 | 1.17 | 0.34 | 0.06 | 0.34 | 0.43 | 162.20 | OK |
| town_day | classic | missing |  |  |  |  |  |  |  |  |  |
| town_day | remastered | 3.44 | 2.95 | 0.04 | 0.36 | 0.29 | 0.05 |  | 0.03 | 175.50 | OK |
