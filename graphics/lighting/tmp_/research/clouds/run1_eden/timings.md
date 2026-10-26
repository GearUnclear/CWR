# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| overcast_broken | classic | 15.01 | 14.90 |  |  |  |  |  |  | 47.20 | OK |
| overcast_broken | remastered | missing |  |  |  |  |  |  |  |  |  |
| broken_high | classic | 23.59 | 23.50 |  |  |  |  |  |  | 33.40 | OK |
| broken_high | remastered | missing |  |  |  |  |  |  |  |  |  |
| overcast_heavy | classic | 17.94 | 17.82 |  |  |  |  |  |  | 51.10 | OK |
| overcast_heavy | remastered | missing |  |  |  |  |  |  |  |  |  |
