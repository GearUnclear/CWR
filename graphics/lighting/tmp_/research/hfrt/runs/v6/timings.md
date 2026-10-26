# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| hill_1730 | classic | 2.95 | 2.87 |  |  |  |  |  |  | 143.50 | OK |
| hill_1730 | remastered | 13.05 | 12.67 | 0.00 | 0.30 | 0.22 | 0.05 |  | 0.03 | 67.40 | OK |
| hill_1730 | delta | +10.10 | +9.80 |  |  |  |  |  |  |  |  |
| slope_tele_1730 | classic | 19.35 | 19.31 |  |  |  |  |  |  | 50.10 | OK |
| slope_tele_1730 | remastered | 23.87 | 22.97 | 0.00 | 0.87 | 0.78 | 0.05 |  | 0.03 | 28.00 | OK |
| slope_tele_1730 | delta | +4.52 | +3.66 |  |  |  |  |  |  |  |  |
| forest_side_1815 | classic | 8.19 | 8.07 |  |  |  |  |  |  | 119.80 | OK |
| forest_side_1815 | remastered | 12.98 | 12.61 | 0.00 | 0.24 | 0.16 | 0.05 |  | 0.03 | 64.80 | OK |
| forest_side_1815 | delta | +4.79 | +4.54 |  |  |  |  |  |  |  |  |
| forest_back_1815 | classic | 0.46 | 0.36 |  |  |  |  |  |  | 724.00 | OK |
| forest_back_1815 | remastered | 1.22 | 0.61 | 0.00 | 0.49 | 0.26 | 0.05 | 0.15 | 0.03 | 564.10 | OK |
| forest_back_1815 | delta | +0.76 | +0.25 |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 2.37 | 2.25 |  |  |  |  |  |  | 348.90 | OK |
| forest_edge_golden | remastered | 12.85 | 12.47 | 0.03 | 0.26 | 0.18 | 0.05 |  | 0.03 | 78.10 | OK |
| forest_edge_golden | delta | +10.48 | +10.22 |  |  |  |  |  |  |  |  |
