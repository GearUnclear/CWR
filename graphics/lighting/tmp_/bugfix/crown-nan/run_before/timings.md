# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| forest_edge_golden | classic | missing |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | remastered | 3.13 | 2.62 | 0.03 | 0.37 | 0.29 | 0.05 |  | 0.03 | 119.20 | OK |
| woods_mixed_noon | classic | missing |  |  |  |  |  |  |  |  |  |
| woods_mixed_noon | remastered | 4.28 | 3.51 | 0.03 | 0.67 | 0.59 | 0.05 |  | 0.03 | 123.30 | OK |
| woods_pine2_evening | classic | missing |  |  |  |  |  |  |  |  |  |
| woods_pine2_evening | remastered | 8.31 | 6.74 | 0.03 | 1.47 | 0.37 | 0.05 | 0.29 | 0.76 | 81.60 | OK |
| town_day | classic | missing |  |  |  |  |  |  |  |  |  |
| town_day | remastered | 6.69 | 6.28 | 0.04 | 0.27 | 0.19 | 0.05 |  | 0.03 | 102.60 | OK |
