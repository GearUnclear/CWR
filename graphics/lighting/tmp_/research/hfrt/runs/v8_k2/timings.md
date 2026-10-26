# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| hill_1730 | classic | 1.75 | 1.67 |  |  |  |  |  |  | 163.30 | OK |
| hill_1730 | remastered | 10.14 | 9.49 | 0.00 | 0.56 | 0.49 | 0.05 |  | 0.03 | 82.00 | OK |
| hill_1730 | delta | +8.39 | +7.82 |  |  |  |  |  |  |  |  |
| slope_tele_1730 | classic | 14.77 | 14.74 |  |  |  |  |  |  | 51.10 | OK |
| slope_tele_1730 | remastered | 20.47 | 19.75 | 0.00 | 0.69 | 0.61 | 0.05 |  | 0.03 | 40.50 | OK |
| slope_tele_1730 | delta | +5.70 | +5.01 |  |  |  |  |  |  |  |  |
| ground_b_1730 | classic | 8.58 | 8.48 |  |  |  |  |  |  | 75.00 | OK |
| ground_b_1730 | remastered | 42.21 | 41.44 | 0.00 | 0.67 | 0.59 | 0.05 |  | 0.03 | 22.40 | OK |
| ground_b_1730 | delta | +33.63 | +32.96 |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 2.86 | 2.72 |  |  |  |  |  |  | 234.00 | OK |
| forest_edge_golden | remastered | 6.21 | 5.60 | 0.25 | 0.47 | 0.37 | 0.06 |  | 0.03 | 121.80 | OK |
| forest_edge_golden | delta | +3.35 | +2.88 |  |  |  |  |  |  |  |  |
