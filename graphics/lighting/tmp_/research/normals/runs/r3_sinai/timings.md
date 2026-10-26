# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| sinai_lowsun_fp | classic | 5.65 | 5.53 |  |  |  |  |  |  | 110.60 | OK |
| sinai_lowsun_fp | remastered | 9.37 | 7.78 | 0.69 | 1.47 | 0.24 | 0.05 | 0.22 | 0.96 | 118.70 | OK |
| sinai_lowsun_fp | delta | +3.72 | +2.25 |  |  |  |  |  |  |  |  |
| sinai_hull_close | classic | 5.14 | 5.03 |  |  |  |  |  |  | 121.50 | OK |
| sinai_hull_close | remastered | 6.81 | 5.39 | 0.04 | 1.31 | 0.42 | 0.05 | 0.24 | 0.60 | 111.30 | OK |
| sinai_hull_close | delta | +1.67 | +0.36 |  |  |  |  |  |  |  |  |
