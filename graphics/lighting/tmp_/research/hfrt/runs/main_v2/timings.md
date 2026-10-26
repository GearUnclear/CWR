# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| sunrise_ridge | classic | 3.09 | 2.97 |  |  |  |  |  |  | 118.90 | OK |
| sunrise_ridge | remastered | 9.10 | 8.27 | 0.00 | 0.70 | 0.47 | 0.05 | 0.15 | 0.03 | 66.60 | OK |
| sunrise_ridge | delta | +6.01 | +5.30 |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 7.87 | 7.75 |  |  |  |  |  |  | 191.50 | OK |
| forest_edge_golden | remastered | 12.23 | 11.79 | 0.12 | 0.31 | 0.24 | 0.05 |  | 0.03 | 69.90 | OK |
| forest_edge_golden | delta | +4.36 | +4.04 |  |  |  |  |  |  |  |  |
| hill_0700 | classic | 43.01 | 42.94 |  |  |  |  |  |  | 45.20 | OK |
| hill_0700 | remastered | 44.54 | 43.31 | 0.00 | 1.16 | 1.08 | 0.05 |  | 0.03 | 20.10 | OK |
| hill_0700 | delta | +1.53 | +0.37 |  |  |  |  |  |  |  |  |
| hill_1730 | classic | 30.06 | 29.98 |  |  |  |  |  |  | 40.10 | OK |
| hill_1730 | remastered | 55.64 | 54.13 | 0.00 | 1.43 | 1.35 | 0.05 |  | 0.03 | 18.70 | OK |
| hill_1730 | delta | +25.58 | +24.15 |  |  |  |  |  |  |  |  |
| overcast_broken | classic | 18.29 | 18.18 |  |  |  |  |  |  | 39.80 | OK |
| overcast_broken | remastered | 29.23 | 28.51 | 0.03 | 0.61 | 0.53 | 0.05 |  | 0.03 | 26.00 | OK |
| overcast_broken | delta | +10.94 | +10.33 |  |  |  |  |  |  |  |  |
