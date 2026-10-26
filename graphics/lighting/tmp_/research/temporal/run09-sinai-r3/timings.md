# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| sinai_lowsun_fp | classic | missing |  |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | remastered | 7.65 | 6.20 | 0.04 | 1.33 | 0.39 | 0.05 | 0.22 | 0.66 | 104.50 | OK |
