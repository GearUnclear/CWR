# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| hull_close | classic | 4.15 | 4.06 |  |  |  |  |  |  | 176.60 | OK |
| hull_close | remastered | 4.12 | 3.05 | 0.04 | 0.97 | 0.24 | 0.05 | 0.24 | 0.45 | 189.50 | OK |
| hull_close | delta | -0.03 | -1.01 |  |  |  |  |  |  |  |  |
| tank_gunner_interior | classic | 7.45 | 7.08 |  |  |  |  |  |  | 79.80 | OK |
| tank_gunner_interior | remastered | 11.82 | 10.60 | 0.06 | 0.21 |  | 0.18 |  | 0.02 | 72.80 | OK |
| tank_gunner_interior | delta | +4.37 | +3.52 |  |  |  |  |  |  |  |  |
