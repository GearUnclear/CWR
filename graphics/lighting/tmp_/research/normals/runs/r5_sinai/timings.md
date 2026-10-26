# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| sinai_lowsun_fp | classic | 2.45 | 2.34 |  |  |  |  |  |  | 175.70 | OK |
| sinai_lowsun_fp | remastered | 9.65 | 8.20 | 1.11 | 1.34 | 0.26 | 0.05 | 0.22 | 0.81 | 133.90 | OK |
| sinai_lowsun_fp | delta | +7.20 | +5.86 |  |  |  |  |  |  |  |  |
