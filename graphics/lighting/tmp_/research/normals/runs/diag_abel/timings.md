# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| hull_close | classic | 6.34 | 6.24 |  |  |  |  |  |  | 110.20 | OK |
| hull_close | remastered | 10.42 | 8.71 | 0.04 | 1.61 | 0.19 | 0.05 | 0.23 | 1.14 | 82.00 | OK |
| hull_close | delta | +4.08 | +2.47 |  |  |  |  |  |  |  |  |
| tank_gunner_interior | classic | 12.92 | 11.54 |  |  |  |  |  |  | 62.60 | OK |
| tank_gunner_interior | remastered | 12.16 | 10.71 | 0.03 | 0.93 |  | 0.91 |  | 0.02 | 50.50 | OK |
| tank_gunner_interior | delta | -0.76 | -0.83 |  |  |  |  |  |  |  |  |
