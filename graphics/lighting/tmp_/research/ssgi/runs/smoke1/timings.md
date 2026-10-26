# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 4.34 | 4.17 |  |  |  |  |  |  | 159.40 | OK |
| lowsun_fp | remastered | 6.59 | 5.55 | 0.09 | 0.91 | 0.52 | 0.06 | 0.27 | 0.06 | 104.20 | OK |
| lowsun_fp | delta | +2.25 | +1.38 |  |  |  |  |  |  |  |  |
