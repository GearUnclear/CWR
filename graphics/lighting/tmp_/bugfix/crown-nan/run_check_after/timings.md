# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| forest_edge_golden | classic | missing |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | remastered | 4.06 | 3.12 | 0.03 | 0.79 | 0.71 | 0.05 |  | 0.03 | 99.10 | OK |
| woods_mixed_noon | classic | missing |  |  |  |  |  |  |  |  |  |
| woods_mixed_noon | remastered | 4.96 | 3.79 | 0.03 | 1.07 | 0.99 | 0.05 |  | 0.03 | 131.70 | OK |
| woods_pine2_evening | classic | missing |  |  |  |  |  |  |  |  |  |
| woods_pine2_evening | remastered | 5.64 | 4.10 | 0.03 | 1.43 | 0.75 | 0.05 | 0.29 | 0.35 | 97.00 | OK |
| town_day | classic | missing |  |  |  |  |  |  |  |  |  |
| town_day | remastered | 4.57 | 3.81 | 0.04 | 0.62 | 0.54 | 0.05 |  | 0.03 | 102.70 | OK |
