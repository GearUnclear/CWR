# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 4.49 | 4.39 |  |  |  |  |  |  | 126.50 | OK |
| lowsun_fp | remastered | 13.21 | 11.90 | 0.04 | 1.20 | 0.26 | 0.05 | 0.22 | 0.68 | 74.80 | OK |
| lowsun_fp | delta | +8.72 | +7.51 |  |  |  |  |  |  |  |  |
