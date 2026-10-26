# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| clear_high_n | classic | missing |  |  |  |  |  |  |  |  |  |
| clear_high_n | remastered | 4.03 | 3.37 | 0.00 | 0.62 | 0.52 | 0.06 |  | 0.04 | 78.50 | OK |
| clear_high_s | classic | missing |  |  |  |  |  |  |  |  |  |
| clear_high_s | remastered | 17.49 | 17.16 | 0.00 | 0.29 | 0.21 | 0.05 |  | 0.03 | 47.70 | OK |
| forest_edge_noon | classic | 2.84 | 2.72 |  |  |  |  |  |  | 189.70 | OK |
| forest_edge_noon | remastered | 3.30 | 2.87 | 0.00 | 0.31 | 0.23 | 0.05 |  | 0.03 | 152.10 | OK |
| forest_edge_noon | delta | +0.46 | +0.15 |  |  |  |  |  |  |  |  |
| coastline_sunset | classic | 1.03 | 0.94 |  |  |  |  |  |  | 452.10 | OK |
| coastline_sunset | remastered | 2.20 | 1.63 | 0.00 | 0.47 | 0.23 | 0.05 | 0.16 | 0.03 | 326.70 | OK |
| coastline_sunset | delta | +1.17 | +0.69 |  |  |  |  |  |  |  |  |
| light_high_n | classic | missing |  |  |  |  |  |  |  |  |  |
| light_high_n | remastered | 8.22 | 7.76 | 0.00 | 0.42 | 0.35 | 0.05 |  | 0.03 | 91.20 | OK |
| broken_high | classic | 9.43 | 9.33 |  |  |  |  |  |  | 73.30 | OK |
| broken_high | remastered | 9.67 | 9.12 | 0.02 | 0.45 | 0.38 | 0.05 |  | 0.03 | 67.80 | OK |
| broken_high | delta | +0.24 | -0.21 |  |  |  |  |  |  |  |  |
| overcast_broken | classic | 4.28 | 4.17 |  |  |  |  |  |  | 94.80 | OK |
| overcast_broken | remastered | 4.86 | 4.19 | 0.03 | 0.56 | 0.47 | 0.05 |  | 0.03 | 95.40 | OK |
| overcast_broken | delta | +0.58 | +0.02 |  |  |  |  |  |  |  |  |
| overcast_heavy | classic | 4.56 | 4.45 |  |  |  |  |  |  | 92.80 | OK |
| overcast_heavy | remastered | 4.96 | 4.29 | 0.03 | 0.55 | 0.48 | 0.05 |  | 0.03 | 90.80 | OK |
| overcast_heavy | delta | +0.40 | -0.16 |  |  |  |  |  |  |  |  |
