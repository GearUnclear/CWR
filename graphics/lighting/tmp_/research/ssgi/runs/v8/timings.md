# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| headlights_bounce | classic | 6.35 | 6.22 |  |  |  |  |  |  | 252.40 | OK |
| headlights_bounce | remastered | 9.51 | 8.95 |  | 0.43 | 0.35 | 0.05 |  | 0.03 | 257.00 | OK |
| headlights_bounce | delta | +3.16 | +2.73 |  |  |  |  |  |  |  |  |
| town_dark_fire | classic | 11.47 | 11.27 |  |  |  |  |  |  | 83.20 | OK |
| town_dark_fire | remastered | 16.83 | 16.41 |  | 0.26 | 0.18 | 0.05 |  | 0.03 | 54.30 | OK |
| town_dark_fire | delta | +5.36 | +5.14 |  |  |  |  |  |  |  |  |
| noon_fields | classic | 23.78 | 23.67 |  |  |  |  |  |  | 30.80 | OK |
| noon_fields | remastered | 26.35 | 26.00 | 0.09 | 0.24 | 0.16 | 0.05 |  | 0.03 | 31.10 | OK |
| noon_fields | delta | +2.57 | +2.33 |  |  |  |  |  |  |  |  |
