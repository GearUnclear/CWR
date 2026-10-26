# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 2.82 | 2.68 |  |  |  |  |  |  | 199.80 | OK |
| lowsun_fp | remastered | 6.39 | 5.54 | 0.04 | 0.74 | 0.27 | 0.05 | 0.23 | 0.19 | 135.60 | OK |
| lowsun_fp | delta | +3.57 | +2.86 |  |  |  |  |  |  |  |  |
| town_day | classic | 2.71 | 2.58 |  |  |  |  |  |  | 200.20 | OK |
| town_day | remastered | 2.33 | 1.80 | 0.04 | 0.39 | 0.32 | 0.05 |  | 0.03 | 198.90 | OK |
| town_day | delta | -0.38 | -0.78 |  |  |  |  |  |  |  |  |
