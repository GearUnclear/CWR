# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| noon_fields | classic | 23.49 | 23.38 |  |  |  |  |  |  | 42.20 | OK |
| noon_fields | remastered | 23.70 | 23.24 | 0.03 | 0.35 | 0.27 | 0.05 |  | 0.03 | 29.60 | OK |
| noon_fields | delta | +0.21 | -0.14 |  |  |  |  |  |  |  |  |
| noon_bushes_zoom | classic | 12.86 | 12.79 |  |  |  |  |  |  | 52.90 | OK |
| noon_bushes_zoom | remastered | 13.90 | 13.56 | 0.00 | 0.27 | 0.20 | 0.05 |  | 0.03 | 52.90 | OK |
| noon_bushes_zoom | delta | +1.04 | +0.77 |  |  |  |  |  |  |  |  |
