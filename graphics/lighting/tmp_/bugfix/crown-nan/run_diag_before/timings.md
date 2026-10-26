# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| forest_edge_golden | classic | missing |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | remastered | 7.44 | 5.76 | 0.11 | 1.22 | 0.91 | 0.18 |  | 0.13 | 73.80 | OK |
| woods_mixed_noon | classic | missing |  |  |  |  |  |  |  |  |  |
| woods_mixed_noon | remastered | 7.21 | 5.54 | 0.08 | 1.42 | 1.18 | 0.13 |  | 0.11 | 78.30 | OK |
| woods_pine2_evening | classic | missing |  |  |  |  |  |  |  |  |  |
| woods_pine2_evening | remastered | 4.74 | 3.50 | 0.03 | 1.13 | 0.72 | 0.05 | 0.32 | 0.04 | 81.00 | OK |
| town_day | classic | missing |  |  |  |  |  |  |  |  |  |
| town_day | remastered | 4.52 | 3.37 | 0.12 | 0.94 | 0.74 | 0.09 |  | 0.11 | 111.30 | OK |
