# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| overcast_broken | classic | 9.94 | 9.83 |  |  |  |  |  |  | 56.10 | OK |
| overcast_broken | remastered | missing |  |  |  |  |  |  |  |  |  |
| broken_up | classic | 11.80 | 11.64 |  |  |  |  |  |  | 63.50 | OK |
| broken_up | remastered | missing |  |  |  |  |  |  |  |  |  |
| broken_high | classic | 51.01 | 50.93 |  |  |  |  |  |  | 30.90 | OK |
| broken_high | remastered | missing |  |  |  |  |  |  |  |  |  |
| overcast_heavy | classic | 31.10 | 30.99 |  |  |  |  |  |  | 27.00 | OK |
| overcast_heavy | remastered | missing |  |  |  |  |  |  |  |  |  |
| heavy_up | classic | 6.46 | 6.30 |  |  |  |  |  |  | 94.50 | OK |
| heavy_up | remastered | missing |  |  |  |  |  |  |  |  |  |
