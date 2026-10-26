# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| town_night | classic | 14.08 | 13.93 |  |  |  |  |  |  | 65.50 | OK |
| town_night | remastered | 16.21 | 15.79 |  | 0.26 | 0.18 | 0.05 |  | 0.03 | 52.60 | OK |
| town_night | delta | +2.13 | +1.86 |  |  |  |  |  |  |  |  |
