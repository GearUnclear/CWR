# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| forest_edge_noon | classic | 2.77 | 2.65 |  |  |  |  |  |  | 307.10 | OK |
| forest_edge_noon | remastered | 7.27 | 6.89 | 0.03 | 0.25 | 0.18 | 0.05 |  | 0.03 | 115.30 | OK |
| forest_edge_noon | delta | +4.50 | +4.24 |  |  |  |  |  |  |  |  |
| noon_fields | classic | 25.37 | 25.26 |  |  |  |  |  |  | 32.00 | OK |
| noon_fields | remastered | 32.62 | 32.27 | 0.04 | 0.24 | 0.16 | 0.05 |  | 0.03 | 28.70 | OK |
| noon_fields | delta | +7.25 | +7.01 |  |  |  |  |  |  |  |  |
