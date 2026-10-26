# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| sunrise_oc | classic | 1.29 | 1.17 |  |  |  |  |  |  | 166.10 | OK |
| sunrise_oc | remastered | missing |  |  |  |  |  |  |  |  |  |
| coast_oc | classic | 1.02 | 0.93 |  |  |  |  |  |  | 456.10 | OK |
| coast_oc | remastered | missing |  |  |  |  |  |  |  |  |  |
| coast_oc_up | classic | 1.40 | 1.27 |  |  |  |  |  |  | 403.50 | OK |
| coast_oc_up | remastered | missing |  |  |  |  |  |  |  |  |  |
| coastline_sunset | classic | 0.64 | 0.54 |  |  |  |  |  |  | 465.90 | OK |
| coastline_sunset | remastered | missing |  |  |  |  |  |  |  |  |  |
