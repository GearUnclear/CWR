# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| clear_high_n | classic | missing |  |  |  |  |  |  |  |  |  |
| clear_high_n | remastered | 32.94 | 30.95 | 0.00 | 1.94 | 1.83 | 0.08 |  | 0.04 | 21.60 | OK |
| clear_high_s | classic | missing |  |  |  |  |  |  |  |  |  |
| clear_high_s | remastered | 42.45 | 40.80 | 0.00 | 1.60 | 1.48 | 0.08 |  | 0.04 | 19.40 | OK |
| forest_edge_noon | classic | 9.06 | 8.29 |  |  |  |  |  |  | 48.20 | OK |
| forest_edge_noon | remastered | 5.04 | 4.01 | 0.00 | 0.85 | 0.74 | 0.07 |  | 0.04 | 72.40 | OK |
| forest_edge_noon | delta | -4.02 | -4.28 |  |  |  |  |  |  |  |  |
| coastline_sunset | classic | 5.15 | 5.01 |  |  |  |  |  |  | 133.30 | OK |
| coastline_sunset | remastered | 4.71 | 2.83 | 0.00 | 1.70 | 1.02 | 0.09 | 0.31 | 0.28 | 129.70 | OK |
| coastline_sunset | delta | -0.44 | -2.18 |  |  |  |  |  |  |  |  |
| light_high_n | classic | missing |  |  |  |  |  |  |  |  |  |
| light_high_n | remastered | 8.92 | 7.64 | 0.00 | 1.23 | 1.06 | 0.12 |  | 0.05 | 46.30 | OK |
