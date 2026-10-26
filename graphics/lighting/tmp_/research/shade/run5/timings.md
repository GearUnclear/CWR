# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 2.64 | 2.54 |  |  |  |  |  |  | 208.70 | OK |
| lowsun_fp | remastered | 8.34 | 7.23 | 0.14 | 1.01 | 0.22 | 0.05 | 0.22 | 0.52 | 108.70 | OK |
| lowsun_fp | delta | +5.70 | +4.69 |  |  |  |  |  |  |  |  |
| lowsun_box | classic | 7.48 | 7.37 |  |  |  |  |  |  | 186.10 | OK |
| lowsun_box | remastered | 12.21 | 11.74 | 0.11 | 0.37 | 0.29 | 0.05 |  | 0.03 | 68.00 | OK |
| lowsun_box | delta | +4.73 | +4.37 |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 7.07 | 6.95 |  |  |  |  |  |  | 214.30 | OK |
| forest_edge_golden | remastered | 10.95 | 10.51 | 0.03 | 0.30 | 0.23 | 0.05 |  | 0.03 | 75.50 | OK |
| forest_edge_golden | delta | +3.88 | +3.56 |  |  |  |  |  |  |  |  |
| coastline_sunset | classic | 1.20 | 1.11 |  |  |  |  |  |  | 407.10 | OK |
| coastline_sunset | remastered | 2.55 | 2.05 | 0.00 | 0.41 | 0.17 | 0.05 | 0.16 | 0.03 | 268.40 | OK |
| coastline_sunset | delta | +1.35 | +0.94 |  |  |  |  |  |  |  |  |
