# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| overcast_broken | classic | 29.96 | 29.85 |  |  |  |  |  |  | 31.90 | OK |
| overcast_broken | remastered | 39.85 | 39.15 | 0.04 | 0.60 | 0.52 | 0.05 |  | 0.03 | 22.10 | OK |
| overcast_broken | delta | +9.89 | +9.30 |  |  |  |  |  |  |  |  |
