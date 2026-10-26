# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| forest_edge_noon | classic | 1.52 | 1.40 |  |  |  |  |  |  | 345.50 | OK |
| forest_edge_noon | remastered | 5.01 | 4.48 | 0.03 | 0.40 | 0.33 | 0.05 |  | 0.03 | 158.10 | OK |
| forest_edge_noon | delta | +3.49 | +3.08 |  |  |  |  |  |  |  |  |
| coastline_sunset | classic | 1.23 | 1.13 |  |  |  |  |  |  | 491.70 | OK |
| coastline_sunset | remastered | 1.25 | 0.54 | 0.00 | 0.61 | 0.37 | 0.05 | 0.16 | 0.03 | 401.10 | OK |
| coastline_sunset | delta | +0.02 | -0.59 |  |  |  |  |  |  |  |  |
