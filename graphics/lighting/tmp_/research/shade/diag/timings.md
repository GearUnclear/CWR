# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 4.32 | 4.22 |  |  |  |  |  |  | 116.60 | OK |
| lowsun_fp | remastered | 10.52 | 8.68 | 0.42 | 1.74 | 0.19 | 0.05 | 0.22 | 1.28 | 106.00 | OK |
| lowsun_fp | delta | +6.20 | +4.46 |  |  |  |  |  |  |  |  |
| lowsun_box | classic | 11.39 | 11.29 |  |  |  |  |  |  | 186.20 | OK |
| lowsun_box | remastered | 19.98 | 19.61 | 0.04 | 0.26 | 0.18 | 0.05 |  | 0.03 | 44.20 | OK |
| lowsun_box | delta | +8.59 | +8.32 |  |  |  |  |  |  |  |  |
