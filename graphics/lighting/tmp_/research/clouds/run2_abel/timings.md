# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| sunrise_oc | classic | 1.85 | 1.72 |  |  |  |  |  |  | 182.30 | OK |
| sunrise_oc | remastered | missing |  |  |  |  |  |  |  |  |  |
| coast_oc | classic | 1.73 | 1.64 |  |  |  |  |  |  | 382.40 | OK |
| coast_oc | remastered | missing |  |  |  |  |  |  |  |  |  |
| coast_oc_up | classic | 1.31 | 1.18 |  |  |  |  |  |  | 413.90 | OK |
| coast_oc_up | remastered | missing |  |  |  |  |  |  |  |  |  |
| coastline_sunset | classic | 1.66 | 1.56 |  |  |  |  |  |  | 386.00 | OK |
| coastline_sunset | remastered | missing |  |  |  |  |  |  |  |  |  |
