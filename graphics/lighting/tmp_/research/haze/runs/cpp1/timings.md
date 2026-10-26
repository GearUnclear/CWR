# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| sunrise_ridge | classic | 2.07 | 1.94 |  |  |  |  |  |  | 111.20 | OK |
| sunrise_ridge | remastered | missing |  |  |  |  |  |  |  |  |  |
| valley_morning | classic | 31.15 | 31.04 |  |  |  |  |  |  | 23.50 | OK |
| valley_morning | remastered | missing |  |  |  |  |  |  |  |  |  |
| noon_fields | classic | 23.77 | 23.66 |  |  |  |  |  |  | 35.00 | OK |
| noon_fields | remastered | missing |  |  |  |  |  |  |  |  |  |
