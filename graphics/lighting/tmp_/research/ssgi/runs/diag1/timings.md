# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 2.42 | 2.30 |  |  |  |  |  |  | 216.50 | OK |
| lowsun_fp | remastered | 4.70 | 3.90 | 0.04 | 0.69 | 0.39 | 0.05 | 0.22 | 0.04 | 125.10 | OK |
| lowsun_fp | delta | +2.28 | +1.60 |  |  |  |  |  |  |  |  |
