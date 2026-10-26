# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| sinai_lowsun_fp | classic | 5.93 | 5.82 |  |  |  |  |  |  | 79.40 | OK |
| sinai_lowsun_fp | remastered | 7.88 | 6.42 | 0.07 | 1.34 | 0.75 | 0.05 | 0.23 | 0.32 | 73.30 | OK |
| sinai_lowsun_fp | delta | +1.95 | +0.60 |  |  |  |  |  |  |  |  |
