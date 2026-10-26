# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| hill_1730 | classic | 1.23 | 1.13 |  |  |  |  |  |  | 141.80 | OK |
| hill_1730 | remastered | 5.19 | 4.41 | 0.00 | 0.70 | 0.63 | 0.05 |  | 0.03 | 94.60 | OK |
| hill_1730 | delta | +3.96 | +3.28 |  |  |  |  |  |  |  |  |
| slope_tele_1730 | classic | 7.19 | 7.15 |  |  |  |  |  |  | 75.40 | OK |
| slope_tele_1730 | remastered | 14.49 | 13.93 | 0.00 | 0.53 | 0.45 | 0.05 |  | 0.03 | 52.60 | OK |
| slope_tele_1730 | delta | +7.30 | +6.78 |  |  |  |  |  |  |  |  |
| ground_b_1730 | classic | 38.17 | 38.07 |  |  |  |  |  |  | 23.20 | OK |
| ground_b_1730 | remastered | 46.37 | 45.25 | 0.00 | 1.02 | 0.94 | 0.05 |  | 0.03 | 20.40 | OK |
| ground_b_1730 | delta | +8.20 | +7.18 |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 7.80 | 7.67 |  |  |  |  |  |  | 229.20 | OK |
| forest_edge_golden | remastered | 15.49 | 15.01 | 0.04 | 0.35 | 0.27 | 0.05 |  | 0.03 | 53.20 | OK |
| forest_edge_golden | delta | +7.69 | +7.34 |  |  |  |  |  |  |  |  |
