# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| forest_edge_golden | classic | 2.63 | 2.51 |  |  |  |  |  |  | 226.80 | OK |
| forest_edge_golden | remastered | 7.07 | 6.43 | 0.04 | 0.35 | 0.27 | 0.05 |  | 0.03 | 93.40 | OK |
| forest_edge_golden | delta | +4.44 | +3.92 |  |  |  |  |  |  |  |  |
| town_night | classic | 9.42 | 9.27 |  |  |  |  |  |  | 83.90 | OK |
| town_night | remastered | 13.60 | 12.99 |  | 0.45 | 0.37 | 0.05 |  | 0.03 | 64.50 | OK |
| town_night | delta | +4.18 | +3.72 |  |  |  |  |  |  |  |  |
