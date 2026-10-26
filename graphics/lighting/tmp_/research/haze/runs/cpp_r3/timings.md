# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| sunrise_ridge | classic | 1.37 | 1.23 |  |  |  |  |  |  | 137.50 | OK |
| sunrise_ridge | remastered | missing |  |  |  |  |  |  |  |  |  |
| valley_morning | classic | 34.35 | 34.23 |  |  |  |  |  |  | 23.80 | OK |
| valley_morning | remastered | missing |  |  |  |  |  |  |  |  |  |
| noon_fields | classic | 10.52 | 10.42 |  |  |  |  |  |  | 51.30 | OK |
| noon_fields | remastered | missing |  |  |  |  |  |  |  |  |  |
| overcast_broken | classic | 31.57 | 31.46 |  |  |  |  |  |  | 25.30 | OK |
| overcast_broken | remastered | missing |  |  |  |  |  |  |  |  |  |
