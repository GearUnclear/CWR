# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| sunrise_ridge | classic | 4.33 | 4.21 |  |  |  |  |  |  | 111.80 | OK |
| sunrise_ridge | remastered | 13.25 | 12.67 | 0.00 | 0.45 | 0.22 | 0.05 | 0.15 | 0.03 | 62.90 | OK |
| sunrise_ridge | delta | +8.92 | +8.46 |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 7.69 | 7.57 |  |  |  |  |  |  | 224.70 | OK |
| forest_edge_golden | remastered | 10.12 | 9.69 | 0.03 | 0.30 | 0.22 | 0.05 |  | 0.03 | 74.70 | OK |
| forest_edge_golden | delta | +2.43 | +2.12 |  |  |  |  |  |  |  |  |
| hill_0700 | classic | 31.38 | 31.31 |  |  |  |  |  |  | 47.80 | OK |
| hill_0700 | remastered | 33.05 | 31.85 | 0.00 | 1.13 | 1.04 | 0.05 |  | 0.03 | 29.80 | OK |
| hill_0700 | delta | +1.67 | +0.54 |  |  |  |  |  |  |  |  |
| hill_1730 | classic | 39.11 | 39.02 |  |  |  |  |  |  | 49.00 | OK |
| hill_1730 | remastered | 40.15 | 39.41 | 0.00 | 0.66 | 0.58 | 0.05 |  | 0.03 | 21.50 | OK |
| hill_1730 | delta | +1.04 | +0.39 |  |  |  |  |  |  |  |  |
| overcast_broken | classic | 24.08 | 23.98 |  |  |  |  |  |  | 31.50 | OK |
| overcast_broken | remastered | 27.85 | 27.04 | 0.04 | 0.71 | 0.63 | 0.05 |  | 0.03 | 25.50 | OK |
| overcast_broken | delta | +3.77 | +3.06 |  |  |  |  |  |  |  |  |
