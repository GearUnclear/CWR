# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 6.12 | 4.80 | 0.62 |  |  |  |  |  | 40.10 | OK |
| lowsun_fp | remastered | 7.50 | 5.10 | 0.90 | 0.71 | 0.30 | 0.21 | 0.05 | 0.15 | 33.00 | OK |
| lowsun_fp | delta | +1.38 | +0.30 | +0.28 |  |  |  |  |  |  |  |
| town_night | classic |  |  |  |  |  |  |  |  |  | FAIL:remastered shaders unavailable |
| town_night | remastered | missing |  |  |  |  |  |  |  |  |  |
