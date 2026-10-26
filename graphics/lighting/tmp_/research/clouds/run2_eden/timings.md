# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| overcast_broken | classic | 19.96 | 19.85 |  |  |  |  |  |  | 43.60 | OK |
| overcast_broken | remastered | missing |  |  |  |  |  |  |  |  |  |
| broken_high | classic | 32.88 | 32.79 |  |  |  |  |  |  | 23.90 | OK |
| broken_high | remastered | missing |  |  |  |  |  |  |  |  |  |
| overcast_heavy | classic | 11.29 | 11.17 |  |  |  |  |  |  | 62.60 | OK |
| overcast_heavy | remastered | missing |  |  |  |  |  |  |  |  |  |
