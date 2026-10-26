# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| town_night | classic | 6.93 | 6.77 |  |  |  |  |  |  | 133.10 | OK |
| town_night | remastered | 7.94 | 7.27 |  | 0.50 | 0.42 | 0.05 |  | 0.03 | 89.60 | OK |
| town_night | delta | +1.01 | +0.50 |  |  |  |  |  |  |  |  |
| town_night_rain | classic | 3.16 | 3.04 |  |  |  |  |  |  | 185.40 | OK |
| town_night_rain | remastered | 4.34 | 3.71 |  | 0.49 | 0.41 | 0.05 |  | 0.03 | 139.00 | OK |
| town_night_rain | delta | +1.18 | +0.67 |  |  |  |  |  |  |  |  |
