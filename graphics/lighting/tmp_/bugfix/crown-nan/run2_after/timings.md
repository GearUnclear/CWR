# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| forest_edge_golden | classic | missing |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | remastered | 3.29 | 2.84 | 0.03 | 0.32 | 0.24 | 0.05 |  | 0.03 | 179.00 | OK |
| woods_mixed_noon | classic | missing |  |  |  |  |  |  |  |  |  |
| woods_mixed_noon | remastered | 3.51 | 2.90 | 0.03 | 0.52 | 0.44 | 0.05 |  | 0.03 | 191.30 | OK |
| woods_pine2_evening | classic | missing |  |  |  |  |  |  |  |  |  |
| woods_pine2_evening | remastered | 5.11 | 4.00 | 0.03 | 1.01 | 0.26 | 0.05 | 0.29 | 0.41 | 140.40 | OK |
| town_day | classic | missing |  |  |  |  |  |  |  |  |  |
| town_day | remastered | 3.28 | 2.82 | 0.04 | 0.33 | 0.25 | 0.05 |  | 0.03 | 173.80 | OK |
