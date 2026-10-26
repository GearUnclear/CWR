# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| sunrise_oc | classic | missing |  |  |  |  |  |  |  |  |  |
| sunrise_oc | remastered | missing |  |  |  |  |  |  |  |  |  |
| overcast_broken | classic | missing |  |  |  |  |  |  |  |  |  |
| overcast_broken | remastered | missing |  |  |  |  |  |  |  |  |  |
| broken_high | classic | missing |  |  |  |  |  |  |  |  |  |
| broken_high | remastered | missing |  |  |  |  |  |  |  |  |  |
