# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 3.10 | 2.99 |  |  |  |  |  |  | 204.20 | OK |
| lowsun_fp | remastered | 3.73 | 2.95 | 0.04 | 0.67 | 0.37 | 0.05 | 0.22 | 0.04 | 131.40 | OK |
| lowsun_fp | delta | +0.63 | -0.04 |  |  |  |  |  |  |  |  |
| noon_fields | classic | 17.12 | 17.01 |  |  |  |  |  |  | 51.50 | OK |
| noon_fields | remastered | 29.59 | 29.23 | 0.04 | 0.24 | 0.17 | 0.05 |  | 0.03 | 33.40 | OK |
| noon_fields | delta | +12.47 | +12.22 |  |  |  |  |  |  |  |  |
