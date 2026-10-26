# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| forest_edge_golden | classic | 1.44 | 1.32 |  |  |  |  |  |  | 263.90 | OK |
| forest_edge_golden | remastered | 2.66 | 2.11 | 0.03 | 0.42 | 0.35 | 0.05 |  | 0.03 | 150.70 | OK |
| forest_edge_golden | delta | +1.22 | +0.79 |  |  |  |  |  |  |  |  |
| hill_0700 | classic | 26.32 | 26.24 |  |  |  |  |  |  | 28.30 | OK |
| hill_0700 | remastered | 45.40 | 44.01 | 0.00 | 1.32 | 1.23 | 0.05 |  | 0.03 | 21.80 | OK |
| hill_0700 | delta | +19.08 | +17.77 |  |  |  |  |  |  |  |  |
| hill_1730 | classic | 4.50 | 4.42 |  |  |  |  |  |  | 88.50 | OK |
| hill_1730 | remastered | 51.29 | 49.59 | 0.00 | 1.61 | 1.53 | 0.05 |  | 0.03 | 22.00 | OK |
| hill_1730 | delta | +46.79 | +45.17 |  |  |  |  |  |  |  |  |
