# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| sinai_lowsun_fp | classic | 2.55 | 2.44 |  |  |  |  |  |  | 134.00 | OK |
| sinai_lowsun_fp | remastered | 7.02 | 5.37 | 0.19 | 1.53 | 0.51 | 0.05 | 0.22 | 0.75 | 121.20 | OK |
| sinai_lowsun_fp | delta | +4.47 | +2.93 |  |  |  |  |  |  |  |  |
| sinai_hull_close | classic | 5.42 | 5.31 |  |  |  |  |  |  | 113.80 | OK |
| sinai_hull_close | remastered | 10.75 | 8.84 | 0.04 | 1.80 | 0.20 | 0.05 | 0.24 | 1.31 | 89.50 | OK |
| sinai_hull_close | delta | +5.33 | +3.53 |  |  |  |  |  |  |  |  |
