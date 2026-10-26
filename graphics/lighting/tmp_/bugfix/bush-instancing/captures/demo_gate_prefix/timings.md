# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| noon_fields | classic | 1.62 | 1.52 |  |  |  |  |  |  | 324.40 | OK |
| noon_fields | remastered | 6.19 | 5.82 | 0.33 | 0.26 | 0.19 | 0.05 |  | 0.03 | 110.50 | OK |
| noon_fields | delta | +4.57 | +4.30 |  |  |  |  |  |  |  |  |
| noon_bushes_zoom | classic | 2.43 | 2.36 |  |  |  |  |  |  | 161.90 | OK |
| noon_bushes_zoom | remastered | 4.20 | 3.85 | 0.00 | 0.28 | 0.20 | 0.05 |  | 0.03 | 117.90 | OK |
| noon_bushes_zoom | delta | +1.77 | +1.49 |  |  |  |  |  |  |  |  |
