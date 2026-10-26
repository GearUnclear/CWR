# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| overcast_broken | classic | 17.40 | 17.29 |  |  |  |  |  |  | 32.20 | OK |
| overcast_broken | remastered | 30.54 | 29.25 | 0.15 | 1.19 | 1.11 | 0.05 |  | 0.03 | 28.40 | OK |
| overcast_broken | delta | +13.14 | +11.96 |  |  |  |  |  |  |  |  |
