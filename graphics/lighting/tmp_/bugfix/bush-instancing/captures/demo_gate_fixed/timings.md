# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| noon_fields | classic | 1.45 | 1.35 |  |  |  |  |  |  | 298.40 | OK |
| noon_fields | remastered | 24.86 | 24.49 | 0.27 | 0.26 | 0.18 | 0.05 |  | 0.03 | 47.20 | OK |
| noon_fields | delta | +23.41 | +23.14 |  |  |  |  |  |  |  |  |
| noon_bushes_zoom | classic | 10.98 | 10.92 |  |  |  |  |  |  | 73.20 | OK |
| noon_bushes_zoom | remastered | 14.42 | 14.01 | 0.00 | 0.33 | 0.26 | 0.05 |  | 0.03 | 59.50 | OK |
| noon_bushes_zoom | delta | +3.44 | +3.09 |  |  |  |  |  |  |  |  |
