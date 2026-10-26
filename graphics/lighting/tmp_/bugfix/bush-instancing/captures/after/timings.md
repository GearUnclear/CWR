# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| noon_fields | classic | 3.08 | 2.98 |  |  |  |  |  |  | 258.40 | OK |
| noon_fields | remastered | 30.64 | 30.30 | 0.34 | 0.24 | 0.16 | 0.05 |  | 0.03 | 39.90 | OK |
| noon_fields | delta | +27.56 | +27.32 |  |  |  |  |  |  |  |  |
| noon_bushes_zoom | classic | 9.92 | 9.85 |  |  |  |  |  |  | 69.90 | OK |
| noon_bushes_zoom | remastered | 10.95 | 10.49 | 0.00 | 0.39 | 0.31 | 0.05 |  | 0.03 | 67.20 | OK |
| noon_bushes_zoom | delta | +1.03 | +0.64 |  |  |  |  |  |  |  |  |
