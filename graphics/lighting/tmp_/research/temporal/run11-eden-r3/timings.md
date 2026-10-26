# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| overcast_broken | classic | missing |  |  |  |  |  |  |  |  |  |
| overcast_broken | remastered | 38.01 | 37.07 | 0.32 | 0.83 | 0.75 | 0.05 |  | 0.03 | 21.40 | OK |
