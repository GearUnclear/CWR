# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_shadow | classic | 4.72 | 4.62 |  |  |  |  |  |  | 119.40 | OK |
| lowsun_shadow | remastered | 25.56 | 23.47 | 0.04 | 2.00 | 0.18 | 0.05 | 0.22 | 1.55 | 50.60 | OK |
| lowsun_shadow | delta | +20.84 | +18.85 |  |  |  |  |  |  |  |  |
