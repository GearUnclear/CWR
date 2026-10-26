# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| town_night | classic | 9.55 | 9.37 |  |  |  |  |  |  | 101.20 | OK |
| town_night | remastered | 10.31 | 9.80 |  | 0.35 | 0.28 | 0.05 |  | 0.03 | 78.40 | OK |
| town_night | delta | +0.76 | +0.43 |  |  |  |  |  |  |  |  |
