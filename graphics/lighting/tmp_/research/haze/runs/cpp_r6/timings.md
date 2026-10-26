# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| sunrise_ridge | classic | 3.43 | 3.31 |  |  |  |  |  |  | 110.60 | OK |
| sunrise_ridge | remastered | missing |  |  |  |  |  |  |  |  |  |
| valley_morning | classic | 7.17 | 7.05 |  |  |  |  |  |  | 81.70 | OK |
| valley_morning | remastered | missing |  |  |  |  |  |  |  |  |  |
| noon_fields | classic | 16.68 | 16.57 |  |  |  |  |  |  | 43.20 | OK |
| noon_fields | remastered | missing |  |  |  |  |  |  |  |  |  |
| overcast_broken | classic | 35.79 | 35.68 |  |  |  |  |  |  | 24.80 | OK |
| overcast_broken | remastered | missing |  |  |  |  |  |  |  |  |  |
| valley_t0515 | classic | missing |  |  |  |  |  |  |  |  |  |
| valley_t0515 | remastered | missing |  |  |  |  |  |  |  |  |  |
| valley_t0600 | classic | missing |  |  |  |  |  |  |  |  |  |
| valley_t0600 | remastered | missing |  |  |  |  |  |  |  |  |  |
| valley_t0630 | classic | missing |  |  |  |  |  |  |  |  |  |
| valley_t0630 | remastered | missing |  |  |  |  |  |  |  |  |  |
| valley_t0715 | classic | missing |  |  |  |  |  |  |  |  |  |
| valley_t0715 | remastered | missing |  |  |  |  |  |  |  |  |  |
| valley_t0800 | classic | missing |  |  |  |  |  |  |  |  |  |
| valley_t0800 | remastered | missing |  |  |  |  |  |  |  |  |  |
