# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 5.70 | 5.60 |  |  |  |  |  |  | 116.40 | OK |
| lowsun_fp | remastered | 14.60 | 12.92 | 0.06 | 1.58 | 0.19 | 0.05 | 0.22 | 1.13 | 71.80 | OK |
| lowsun_fp | delta | +8.90 | +7.32 |  |  |  |  |  |  |  |  |
