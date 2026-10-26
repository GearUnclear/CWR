# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| sunrise_oc | classic | 1.76 | 1.64 |  |  |  |  |  |  | 154.30 | OK |
| sunrise_oc | remastered | missing |  |  |  |  |  |  |  |  |  |
| coast_oc | classic | 1.15 | 1.05 |  |  |  |  |  |  | 450.90 | OK |
| coast_oc | remastered | missing |  |  |  |  |  |  |  |  |  |
| coast_oc_up | classic | 1.18 | 1.05 |  |  |  |  |  |  | 456.00 | OK |
| coast_oc_up | remastered | missing |  |  |  |  |  |  |  |  |  |
| coastline_sunset | classic | 0.87 | 0.77 |  |  |  |  |  |  | 449.60 | OK |
| coastline_sunset | remastered | missing |  |  |  |  |  |  |  |  |  |
| sunrise_ridge | classic | 4.17 | 4.05 |  |  |  |  |  |  | 112.30 | OK |
| sunrise_ridge | remastered | missing |  |  |  |  |  |  |  |  |  |
