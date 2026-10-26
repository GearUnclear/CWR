# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| sunrise_oc | classic | 1.15 | 1.02 |  |  |  |  |  |  | 183.90 | OK |
| sunrise_oc | remastered | missing |  |  |  |  |  |  |  |  |  |
| coast_oc | classic | 0.79 | 0.69 |  |  |  |  |  |  | 511.20 | OK |
| coast_oc | remastered | missing |  |  |  |  |  |  |  |  |  |
| coast_oc_up | classic | 0.94 | 0.81 |  |  |  |  |  |  | 495.40 | OK |
| coast_oc_up | remastered | missing |  |  |  |  |  |  |  |  |  |
| coastline_sunset | classic | 0.88 | 0.78 |  |  |  |  |  |  | 454.20 | OK |
| coastline_sunset | remastered | missing |  |  |  |  |  |  |  |  |  |
| sunrise_ridge | classic | 4.37 | 4.25 |  |  |  |  |  |  | 116.60 | OK |
| sunrise_ridge | remastered | missing |  |  |  |  |  |  |  |  |  |
