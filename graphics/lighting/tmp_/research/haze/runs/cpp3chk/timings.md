# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| sunrise_ridge | classic | 2.93 | 2.81 |  |  |  |  |  |  | 105.60 | OK |
| sunrise_ridge | remastered | missing |  |  |  |  |  |  |  |  |  |
