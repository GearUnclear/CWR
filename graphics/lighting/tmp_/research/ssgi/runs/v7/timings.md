# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| headlights_bounce | classic | 3.40 | 3.28 |  |  |  |  |  |  | 307.90 | OK |
| headlights_bounce | remastered | 3.81 | 3.37 |  | 0.30 | 0.22 | 0.05 |  | 0.03 | 290.10 | OK |
| headlights_bounce | delta | +0.41 | +0.09 |  |  |  |  |  |  |  |  |
| town_dark_fire | classic | 5.70 | 5.54 |  |  |  |  |  |  | 99.30 | OK |
| town_dark_fire | remastered | 13.94 | 13.36 |  | 0.38 | 0.30 | 0.05 |  | 0.03 | 59.90 | OK |
| town_dark_fire | delta | +8.24 | +7.82 |  |  |  |  |  |  |  |  |
| lowsun_fp | classic | 4.32 | 4.21 |  |  |  |  |  |  | 124.20 | OK |
| lowsun_fp | remastered | 9.21 | 7.71 | 0.20 | 1.38 | 0.19 | 0.05 | 0.22 | 0.92 | 82.90 | OK |
| lowsun_fp | delta | +4.89 | +3.50 |  |  |  |  |  |  |  |  |
| noon_fields | classic | 20.51 | 20.14 |  |  |  |  |  |  | 37.50 | OK |
| noon_fields | remastered | 32.14 | 31.78 | 0.04 | 0.24 | 0.17 | 0.05 |  | 0.03 | 25.20 | OK |
| noon_fields | delta | +11.63 | +11.64 |  |  |  |  |  |  |  |  |
