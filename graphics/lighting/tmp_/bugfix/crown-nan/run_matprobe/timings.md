# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| forest_edge_golden | classic | missing |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | remastered | 4.82 | 4.34 | 0.03 | 0.34 | 0.27 | 0.05 |  | 0.03 | 123.80 | OK |
| woods_mixed_noon | classic | missing |  |  |  |  |  |  |  |  |  |
| woods_mixed_noon | remastered | 5.16 | 4.72 | 0.03 | 0.34 | 0.26 | 0.05 |  | 0.03 | 151.70 | OK |
| woods_pine2_evening | classic | missing |  |  |  |  |  |  |  |  |  |
| woods_pine2_evening | remastered | 9.53 | 7.60 | 0.03 | 1.82 | 0.23 | 0.05 | 0.29 | 1.25 | 101.70 | OK |
| town_day | classic | missing |  |  |  |  |  |  |  |  |  |
| town_day | remastered | 6.37 | 5.96 | 0.04 | 0.27 | 0.19 | 0.05 |  | 0.03 | 135.70 | OK |
