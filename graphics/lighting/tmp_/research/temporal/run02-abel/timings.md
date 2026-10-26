# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| forest_edge_golden | classic | 3.63 | 3.50 |  |  |  |  |  |  | 345.40 | OK |
| forest_edge_golden | remastered | 8.25 | 7.86 | 0.03 | 0.25 | 0.18 | 0.05 |  | 0.03 | 101.80 | OK |
| forest_edge_golden | delta | +4.62 | +4.36 |  |  |  |  |  |  |  |  |
| town_night | classic | 6.16 | 6.01 |  |  |  |  |  |  | 130.40 | OK |
| town_night | remastered | 10.22 | 9.41 |  | 0.62 | 0.55 | 0.05 |  | 0.03 | 89.60 | OK |
| town_night | delta | +4.06 | +3.40 |  |  |  |  |  |  |  |  |
