# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 2.84 | 2.73 |  |  |  |  |  |  | 170.40 | OK |
| lowsun_fp | remastered | 6.01 | 5.01 | 0.05 | 0.89 | 0.58 | 0.05 | 0.22 | 0.04 | 95.60 | OK |
| lowsun_fp | delta | +3.17 | +2.28 |  |  |  |  |  |  |  |  |
