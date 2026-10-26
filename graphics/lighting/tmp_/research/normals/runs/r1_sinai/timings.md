# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| sinai_lowsun_fp | classic | 7.19 | 7.07 |  |  |  |  |  |  | 90.70 | OK |
| sinai_lowsun_fp | remastered | 13.91 | 11.77 | 0.07 | 2.02 | 0.20 | 0.05 | 0.22 | 1.55 | 66.80 | OK |
| sinai_lowsun_fp | delta | +6.72 | +4.70 |  |  |  |  |  |  |  |  |
| sinai_hull_close | classic | 7.84 | 7.73 |  |  |  |  |  |  | 90.70 | OK |
| sinai_hull_close | remastered | 11.38 | 9.05 | 0.08 | 2.22 | 0.27 | 0.05 | 0.24 | 1.66 | 76.10 | OK |
| sinai_hull_close | delta | +3.54 | +1.32 |  |  |  |  |  |  |  |  |
