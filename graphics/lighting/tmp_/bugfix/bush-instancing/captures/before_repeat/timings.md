# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| noon_fields | classic | 2.66 | 2.56 |  |  |  |  |  |  | 212.00 | OK |
| noon_fields | remastered | 28.43 | 27.98 | 0.04 | 0.33 | 0.25 | 0.05 |  | 0.03 | 40.40 | OK |
| noon_fields | delta | +25.77 | +25.42 |  |  |  |  |  |  |  |  |
| noon_bushes_zoom | classic | 14.56 | 14.49 |  |  |  |  |  |  | 49.20 | OK |
| noon_bushes_zoom | remastered | 9.53 | 9.08 | 0.00 | 0.38 | 0.30 | 0.05 |  | 0.03 | 60.50 | OK |
| noon_bushes_zoom | delta | -5.03 | -5.41 |  |  |  |  |  |  |  |  |
