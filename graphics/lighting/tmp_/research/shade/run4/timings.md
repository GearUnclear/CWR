# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| forest_edge_golden | classic | 3.34 | 3.21 |  |  |  |  |  |  | 243.90 | OK |
| forest_edge_golden | remastered | 7.71 | 7.32 | 0.03 | 0.25 | 0.18 | 0.05 |  | 0.03 | 102.20 | OK |
| forest_edge_golden | delta | +4.37 | +4.11 |  |  |  |  |  |  |  |  |
| lowsun_fp | classic | 9.57 | 9.47 |  |  |  |  |  |  | 62.90 | OK |
| lowsun_fp | remastered | 15.83 | 13.95 | 0.21 | 1.76 | 0.78 | 0.05 | 0.22 | 0.71 | 59.80 | OK |
| lowsun_fp | delta | +6.26 | +4.48 |  |  |  |  |  |  |  |  |
| lowsun_box | classic | 12.36 | 12.17 |  |  |  |  |  |  | 115.40 | OK |
| lowsun_box | remastered | 21.56 | 21.18 | 0.04 | 0.27 | 0.19 | 0.05 |  | 0.03 | 38.00 | OK |
| lowsun_box | delta | +9.20 | +9.01 |  |  |  |  |  |  |  |  |
| coastline_sunset | classic | 1.84 | 1.74 |  |  |  |  |  |  | 227.80 | OK |
| coastline_sunset | remastered | 3.38 | 2.78 | 0.00 | 0.50 | 0.25 | 0.05 | 0.16 | 0.03 | 169.30 | OK |
| coastline_sunset | delta | +1.54 | +1.04 |  |  |  |  |  |  |  |  |
