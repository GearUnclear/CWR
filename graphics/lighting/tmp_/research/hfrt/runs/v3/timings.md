# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| sunrise_ridge | classic | 3.53 | 3.41 |  |  |  |  |  |  | 119.70 | OK |
| sunrise_ridge | remastered | 11.96 | 11.36 | 0.00 | 0.47 | 0.25 | 0.05 | 0.15 | 0.03 | 57.60 | OK |
| sunrise_ridge | delta | +8.43 | +7.95 |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 9.29 | 9.16 |  |  |  |  |  |  | 154.10 | OK |
| forest_edge_golden | remastered | 13.72 | 13.30 | 0.03 | 0.29 | 0.21 | 0.05 |  | 0.03 | 56.20 | OK |
| forest_edge_golden | delta | +4.43 | +4.14 |  |  |  |  |  |  |  |  |
| hill_1730 | classic | 24.90 | 24.82 |  |  |  |  |  |  | 31.00 | OK |
| hill_1730 | remastered | 46.90 | 45.34 | 0.00 | 1.47 | 1.39 | 0.06 |  | 0.03 | 19.60 | OK |
| hill_1730 | delta | +22.00 | +20.52 |  |  |  |  |  |  |  |  |
| overcast_broken | classic | 34.11 | 34.01 |  |  |  |  |  |  | 23.20 | OK |
| overcast_broken | remastered | 41.30 | 40.31 | 0.03 | 0.89 | 0.81 | 0.05 |  | 0.03 | 19.40 | OK |
| overcast_broken | delta | +7.19 | +6.30 |  |  |  |  |  |  |  |  |
