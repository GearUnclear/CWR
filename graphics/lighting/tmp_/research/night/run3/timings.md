# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| town_night | classic | 13.17 | 13.01 |  |  |  |  |  |  | 58.80 | OK |
| town_night | remastered | 19.99 | 19.15 |  | 0.66 | 0.58 | 0.05 |  | 0.03 | 46.10 | OK |
| town_night | delta | +6.82 | +6.14 |  |  |  |  |  |  |  |  |
