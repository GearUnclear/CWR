# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 5.34 | 5.24 |  |  |  |  |  |  | 131.30 | OK |
| lowsun_fp | remastered | 6.21 | 5.05 | 0.04 | 1.05 | 0.28 | 0.05 | 0.22 | 0.49 | 116.30 | OK |
| lowsun_fp | delta | +0.87 | -0.19 |  |  |  |  |  |  |  |  |
