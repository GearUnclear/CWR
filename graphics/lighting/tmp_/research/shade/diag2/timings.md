# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 5.96 | 5.85 |  |  |  |  |  |  | 129.30 | OK |
| lowsun_fp | remastered | 13.12 | 11.52 | 0.17 | 1.49 | 0.24 | 0.05 | 0.22 | 0.98 | 84.40 | OK |
| lowsun_fp | delta | +7.16 | +5.67 |  |  |  |  |  |  |  |  |
| lowsun_box | classic | 7.16 | 7.05 |  |  |  |  |  |  | 146.40 | OK |
| lowsun_box | remastered | 11.76 | 11.33 | 0.04 | 0.31 | 0.24 | 0.05 |  | 0.03 | 69.90 | OK |
| lowsun_box | delta | +4.60 | +4.28 |  |  |  |  |  |  |  |  |
