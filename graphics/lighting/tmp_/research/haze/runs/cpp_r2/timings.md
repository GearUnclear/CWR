# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| sunrise_ridge | classic | 2.03 | 1.91 |  |  |  |  |  |  | 105.80 | OK |
| sunrise_ridge | remastered | missing |  |  |  |  |  |  |  |  |  |
| valley_morning | classic | 25.92 | 25.80 |  |  |  |  |  |  | 28.40 | OK |
| valley_morning | remastered | missing |  |  |  |  |  |  |  |  |  |
| noon_fields | classic | 21.56 | 21.45 |  |  |  |  |  |  | 40.90 | OK |
| noon_fields | remastered | missing |  |  |  |  |  |  |  |  |  |
| overcast_broken | classic | 24.90 | 24.79 |  |  |  |  |  |  | 31.50 | OK |
| overcast_broken | remastered | missing |  |  |  |  |  |  |  |  |  |
