# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| sinai_lowsun_fp | classic | missing |  |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | remastered | 5.32 | 3.92 | 0.04 | 1.28 | 0.64 | 0.05 | 0.22 | 0.37 | 103.30 | OK |
