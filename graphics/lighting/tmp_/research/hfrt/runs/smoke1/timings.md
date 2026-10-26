# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| hill_0700 | classic | 3.22 | 3.12 |  |  |  |  |  |  | 70.50 | OK |
| hill_0700 | remastered | 32.96 | 31.08 | 0.00 | 1.78 | 1.70 | 0.05 |  | 0.03 | 36.30 | OK |
| hill_0700 | delta | +29.74 | +27.96 |  |  |  |  |  |  |  |  |
