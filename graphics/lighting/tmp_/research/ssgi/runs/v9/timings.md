# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| headlights_bounce | classic | 6.02 | 5.89 |  |  |  |  |  |  | 128.10 | OK |
| headlights_bounce | remastered | 10.08 | 9.57 |  | 0.38 | 0.30 | 0.05 |  | 0.03 | 89.40 | OK |
| headlights_bounce | delta | +4.06 | +3.68 |  |  |  |  |  |  |  |  |
| town_dark_fire | classic | 11.21 | 11.04 |  |  |  |  |  |  | 82.00 | OK |
| town_dark_fire | remastered | 12.74 | 12.32 |  | 0.26 | 0.18 | 0.05 |  | 0.03 | 70.60 | OK |
| town_dark_fire | delta | +1.53 | +1.28 |  |  |  |  |  |  |  |  |
| lowsun_fp | classic | 12.50 | 12.39 |  |  |  |  |  |  | 67.50 | OK |
| lowsun_fp | remastered | 16.23 | 14.26 | 0.39 | 1.86 | 0.18 | 0.05 | 0.22 | 1.40 | 59.50 | OK |
| lowsun_fp | delta | +3.73 | +1.87 |  |  |  |  |  |  |  |  |
| noon_fields | classic | 16.59 | 16.48 |  |  |  |  |  |  | 46.00 | OK |
| noon_fields | remastered | 25.21 | 24.82 | 0.14 | 0.29 | 0.20 | 0.05 |  | 0.03 | 35.60 | OK |
| noon_fields | delta | +8.62 | +8.34 |  |  |  |  |  |  |  |  |
