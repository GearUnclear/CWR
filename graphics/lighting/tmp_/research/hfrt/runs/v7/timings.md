# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| hill_1730 | classic | 2.35 | 2.27 |  |  |  |  |  |  | 121.80 | OK |
| hill_1730 | remastered | 9.54 | 8.88 | 0.00 | 0.58 | 0.50 | 0.05 |  | 0.03 | 72.80 | OK |
| hill_1730 | delta | +7.19 | +6.61 |  |  |  |  |  |  |  |  |
| slope_tele_1730 | classic | 14.31 | 14.28 |  |  |  |  |  |  | 47.00 | OK |
| slope_tele_1730 | remastered | 16.86 | 15.87 | 0.00 | 0.96 | 0.88 | 0.05 |  | 0.03 | 43.20 | OK |
| slope_tele_1730 | delta | +2.55 | +1.59 |  |  |  |  |  |  |  |  |
| ground_a_1730 | classic | 8.79 | 8.68 |  |  |  |  |  |  | 67.20 | OK |
| ground_a_1730 | remastered | 14.03 | 13.30 | 0.00 | 0.62 | 0.54 | 0.05 |  | 0.03 | 49.90 | OK |
| ground_a_1730 | delta | +5.24 | +4.62 |  |  |  |  |  |  |  |  |
| ground_b_1730 | classic | 15.01 | 14.91 |  |  |  |  |  |  | 43.50 | OK |
| ground_b_1730 | remastered | 26.80 | 25.58 | 0.00 | 1.12 | 1.04 | 0.05 |  | 0.03 | 29.90 | OK |
| ground_b_1730 | delta | +11.79 | +10.67 |  |  |  |  |  |  |  |  |
| hill_0700 | classic | 9.86 | 9.78 |  |  |  |  |  |  | 60.70 | OK |
| hill_0700 | remastered | 38.41 | 37.53 | 0.00 | 0.80 | 0.72 | 0.05 |  | 0.03 | 26.40 | OK |
| hill_0700 | delta | +28.55 | +27.75 |  |  |  |  |  |  |  |  |
| sunrise_ridge | classic | 9.78 | 9.66 |  |  |  |  |  |  | 57.30 | OK |
| sunrise_ridge | remastered | 9.83 | 9.05 | 0.00 | 0.65 | 0.42 | 0.05 | 0.15 | 0.03 | 58.60 | OK |
| sunrise_ridge | delta | +0.05 | -0.61 |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 3.60 | 3.49 |  |  |  |  |  |  | 207.90 | OK |
| forest_edge_golden | remastered | 3.85 | 3.35 | 0.03 | 0.37 | 0.30 | 0.05 |  | 0.03 | 152.40 | OK |
| forest_edge_golden | delta | +0.25 | -0.14 |  |  |  |  |  |  |  |  |
