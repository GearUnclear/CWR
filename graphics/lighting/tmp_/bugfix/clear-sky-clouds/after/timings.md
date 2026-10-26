# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| clear_high_n | classic | missing |  |  |  |  |  |  |  |  |  |
| clear_high_n | remastered | 4.60 | 4.11 | 0.00 | 0.46 | 0.38 | 0.05 |  | 0.03 | 78.40 | OK |
| clear_high_s | classic | missing |  |  |  |  |  |  |  |  |  |
| clear_high_s | remastered | 16.74 | 16.31 | 0.00 | 0.41 | 0.33 | 0.05 |  | 0.03 | 48.80 | OK |
| forest_edge_noon | classic | 2.47 | 2.35 |  |  |  |  |  |  | 198.30 | OK |
| forest_edge_noon | remastered | 2.18 | 1.71 | 0.00 | 0.34 | 0.27 | 0.05 |  | 0.03 | 184.10 | OK |
| forest_edge_noon | delta | -0.29 | -0.64 |  |  |  |  |  |  |  |  |
| coastline_sunset | classic | 1.71 | 1.41 |  |  |  |  |  |  | 250.80 | OK |
| coastline_sunset | remastered | 2.06 | 0.96 | 0.00 | 0.96 | 0.30 | 0.08 | 0.25 | 0.33 | 321.90 | OK |
| coastline_sunset | delta | +0.35 | -0.45 |  |  |  |  |  |  |  |  |
| light_high_n | classic | missing |  |  |  |  |  |  |  |  |  |
| light_high_n | remastered | 3.41 | 2.75 | 0.00 | 0.61 | 0.50 | 0.07 |  | 0.04 | 111.70 | OK |
| broken_high | classic | 15.50 | 15.40 |  |  |  |  |  |  | 61.00 | OK |
| broken_high | remastered | 8.60 | 7.86 | 0.03 | 0.62 | 0.51 | 0.07 |  | 0.04 | 69.30 | OK |
| broken_high | delta | -6.90 | -7.54 |  |  |  |  |  |  |  |  |
| overcast_broken | classic | 4.75 | 4.63 |  |  |  |  |  |  | 95.80 | OK |
| overcast_broken | remastered | 5.53 | 4.86 | 0.03 | 0.55 | 0.46 | 0.06 |  | 0.03 | 86.10 | OK |
| overcast_broken | delta | +0.78 | +0.23 |  |  |  |  |  |  |  |  |
| overcast_heavy | classic | 4.67 | 4.56 |  |  |  |  |  |  | 99.60 | OK |
| overcast_heavy | remastered | 11.22 | 9.60 | 0.03 | 1.51 | 1.43 | 0.05 |  | 0.03 | 77.40 | OK |
| overcast_heavy | delta | +6.55 | +5.04 |  |  |  |  |  |  |  |  |
