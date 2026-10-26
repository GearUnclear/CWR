# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| hill_1730 | classic | 1.75 | 1.67 |  |  |  |  |  |  | 162.40 | OK |
| hill_1730 | remastered | 9.62 | 9.11 | 0.00 | 0.43 | 0.35 | 0.05 |  | 0.03 | 79.80 | OK |
| hill_1730 | delta | +7.87 | +7.44 |  |  |  |  |  |  |  |  |
| slope_tele_1730 | classic | 21.95 | 21.91 |  |  |  |  |  |  | 40.20 | OK |
| slope_tele_1730 | remastered | 30.86 | 29.97 | 0.00 | 0.85 | 0.77 | 0.05 |  | 0.03 | 27.70 | OK |
| slope_tele_1730 | delta | +8.91 | +8.06 |  |  |  |  |  |  |  |  |
| ground_b_1730 | classic | 44.95 | 44.85 |  |  |  |  |  |  | 20.50 | OK |
| ground_b_1730 | remastered | 38.30 | 37.70 | 0.00 | 0.49 | 0.41 | 0.05 |  | 0.03 | 22.60 | OK |
| ground_b_1730 | delta | -6.65 | -7.15 |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 9.57 | 9.44 |  |  |  |  |  |  | 233.40 | OK |
| forest_edge_golden | remastered | 11.23 | 10.80 | 0.10 | 0.30 | 0.22 | 0.05 |  | 0.03 | 69.90 | OK |
| forest_edge_golden | delta | +1.66 | +1.36 |  |  |  |  |  |  |  |  |
