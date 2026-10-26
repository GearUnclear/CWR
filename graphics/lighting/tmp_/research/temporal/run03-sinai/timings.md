# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| sinai_lowsun_fp | classic | 3.40 | 3.28 |  |  |  |  |  |  | 121.30 | OK |
| sinai_lowsun_fp | remastered | 7.07 | 5.54 | 0.33 | 1.41 | 0.54 | 0.05 | 0.22 | 0.60 | 113.50 | OK |
| sinai_lowsun_fp | delta | +3.67 | +2.26 |  |  |  |  |  |  |  |  |
