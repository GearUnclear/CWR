# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| hill_1730 | classic | 2.43 | 2.35 |  |  |  |  |  |  | 166.20 | OK |
| hill_1730 | remastered | 10.21 | 9.81 | 0.00 | 0.32 | 0.25 | 0.05 |  | 0.03 | 78.60 | OK |
| hill_1730 | delta | +7.78 | +7.46 |  |  |  |  |  |  |  |  |
| slope_tele_1730 | classic | 15.57 | 15.54 |  |  |  |  |  |  | 48.30 | OK |
| slope_tele_1730 | remastered | 31.63 | 30.78 | 0.00 | 0.82 | 0.74 | 0.05 |  | 0.03 | 30.00 | OK |
| slope_tele_1730 | delta | +16.06 | +15.24 |  |  |  |  |  |  |  |  |
| ground_b_1730 | classic | 35.29 | 35.19 |  |  |  |  |  |  | 32.30 | OK |
| ground_b_1730 | remastered | 38.49 | 37.41 | 0.00 | 0.98 | 0.89 | 0.05 |  | 0.03 | 22.40 | OK |
| ground_b_1730 | delta | +3.20 | +2.22 |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 7.99 | 7.86 |  |  |  |  |  |  | 232.40 | OK |
| forest_edge_golden | remastered | 16.64 | 16.25 | 0.03 | 0.26 | 0.19 | 0.05 |  | 0.03 | 52.20 | OK |
| forest_edge_golden | delta | +8.65 | +8.39 |  |  |  |  |  |  |  |  |
