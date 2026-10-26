# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| forest_edge_golden | classic | missing |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | remastered | 3.08 | 2.59 | 0.03 | 0.36 | 0.28 | 0.05 |  | 0.03 | 148.90 | OK |
| woods_mixed_noon | classic | missing |  |  |  |  |  |  |  |  |  |
| woods_mixed_noon | remastered | 4.19 | 3.53 | 0.03 | 0.56 | 0.48 | 0.05 |  | 0.03 | 161.60 | OK |
| woods_pine2_evening | classic | missing |  |  |  |  |  |  |  |  |  |
| woods_pine2_evening | remastered | 4.85 | 3.72 | 0.03 | 1.01 | 0.30 | 0.05 | 0.29 | 0.38 | 129.60 | OK |
| town_day | classic | missing |  |  |  |  |  |  |  |  |  |
| town_day | remastered | 3.41 | 2.90 | 0.04 | 0.38 | 0.30 | 0.05 |  | 0.03 | 157.90 | OK |
