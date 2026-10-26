# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| overcast_broken | classic | 12.00 | 11.89 |  |  |  |  |  |  | 59.70 | OK |
| overcast_broken | remastered | missing |  |  |  |  |  |  |  |  |  |
| broken_up | classic | 7.88 | 7.73 |  |  |  |  |  |  | 70.30 | OK |
| broken_up | remastered | missing |  |  |  |  |  |  |  |  |  |
| broken_high | classic | 30.58 | 30.50 |  |  |  |  |  |  | 25.80 | OK |
| broken_high | remastered | missing |  |  |  |  |  |  |  |  |  |
| overcast_heavy | classic | 21.47 | 21.36 |  |  |  |  |  |  | 34.70 | OK |
| overcast_heavy | remastered | missing |  |  |  |  |  |  |  |  |  |
| heavy_up | classic | 9.00 | 8.84 |  |  |  |  |  |  | 65.00 | OK |
| heavy_up | remastered | missing |  |  |  |  |  |  |  |  |  |
