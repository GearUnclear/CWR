# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| noon_fields | classic | 1.53 | 1.42 |  |  |  |  |  |  | 286.80 | OK |
| noon_fields | remastered | 12.49 | 12.13 | 0.04 | 0.25 | 0.17 | 0.05 |  | 0.03 | 81.00 | OK |
| noon_fields | delta | +10.96 | +10.71 |  |  |  |  |  |  |  |  |
