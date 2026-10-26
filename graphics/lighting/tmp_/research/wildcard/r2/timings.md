# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | missing |  |  |  |  |  |  |  |  |  |
| lowsun_fp | remastered | 6.18 | 4.94 | 0.05 | 1.10 | 0.24 | 0.07 | 0.31 | 0.47 | 172.90 | OK |
| noon_fields | classic | missing |  |  |  |  |  |  |  |  |  |
| noon_fields | remastered | 12.36 | 11.90 | 0.04 | 0.35 | 0.27 | 0.05 |  | 0.03 | 62.30 | OK |
