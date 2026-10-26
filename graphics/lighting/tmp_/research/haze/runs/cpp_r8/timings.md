# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| valley_t0515 | classic | 3.22 | 3.11 |  |  |  |  |  |  | 149.80 | OK |
| valley_t0515 | remastered | missing |  |  |  |  |  |  |  |  |  |
| valley_morning | classic | 7.23 | 7.12 |  |  |  |  |  |  | 92.30 | OK |
| valley_morning | remastered | missing |  |  |  |  |  |  |  |  |  |
| sunrise_ridge | classic | 37.18 | 37.06 |  |  |  |  |  |  | 24.90 | OK |
| sunrise_ridge | remastered | missing |  |  |  |  |  |  |  |  |  |
