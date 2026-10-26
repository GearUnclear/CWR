# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| forest_edge_golden | classic | missing |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | remastered | 3.45 | 3.06 | 0.03 | 0.26 | 0.18 | 0.05 |  | 0.03 | 165.20 | OK |
| woods_mixed_noon | classic | missing |  |  |  |  |  |  |  |  |  |
| woods_mixed_noon | remastered | 4.78 | 4.35 | 0.03 | 0.33 | 0.25 | 0.05 |  | 0.03 | 166.70 | OK |
| woods_pine2_evening | classic | missing |  |  |  |  |  |  |  |  |  |
| woods_pine2_evening | remastered | 5.77 | 4.45 | 0.03 | 1.22 | 0.30 | 0.05 | 0.29 | 0.58 | 112.80 | OK |
| town_day | classic | missing |  |  |  |  |  |  |  |  |  |
| town_day | remastered | 3.45 | 2.93 | 0.04 | 0.39 | 0.31 | 0.05 |  | 0.03 | 163.10 | OK |
