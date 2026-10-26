# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| noon_fields | classic | 3.15 | 3.04 |  |  |  |  |  |  | 240.30 | OK |
| noon_fields | remastered | 20.38 | 19.95 | 0.18 | 0.32 | 0.24 | 0.05 |  | 0.03 | 49.30 | OK |
| noon_fields | delta | +17.23 | +16.91 |  |  |  |  |  |  |  |  |
| noon_bushes_zoom | classic | 10.05 | 9.99 |  |  |  |  |  |  | 83.30 | OK |
| noon_bushes_zoom | remastered | 12.80 | 12.45 | 0.00 | 0.29 | 0.22 | 0.05 |  | 0.03 | 63.30 | OK |
| noon_bushes_zoom | delta | +2.75 | +2.46 |  |  |  |  |  |  |  |  |
