# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| town_night | classic | 4.88 | 4.73 |  |  |  |  |  |  | 135.00 | OK |
| town_night | remastered | 8.36 | 7.71 |  | 0.47 | 0.40 | 0.05 |  | 0.03 | 103.90 | OK |
| town_night | delta | +3.48 | +2.98 |  |  |  |  |  |  |  |  |
| lowsun_fp | classic | 8.65 | 8.54 |  |  |  |  |  |  | 89.10 | OK |
| lowsun_fp | remastered | 13.28 | 11.71 | 0.11 | 1.46 | 0.18 | 0.05 | 0.22 | 1.01 | 67.10 | OK |
| lowsun_fp | delta | +4.63 | +3.17 |  |  |  |  |  |  |  |  |
| noon_fields | classic | 6.53 | 6.42 |  |  |  |  |  |  | 75.20 | OK |
| noon_fields | remastered | 21.43 | 21.07 | 0.20 | 0.25 | 0.17 | 0.05 |  | 0.03 | 43.30 | OK |
| noon_fields | delta | +14.90 | +14.65 |  |  |  |  |  |  |  |  |
| town_night_rain | classic | 4.54 | 4.42 |  |  |  |  |  |  | 135.70 | OK |
| town_night_rain | remastered | 6.79 | 6.36 |  | 0.30 | 0.22 | 0.05 |  | 0.03 | 137.90 | OK |
| town_night_rain | delta | +2.25 | +1.94 |  |  |  |  |  |  |  |  |
| abel_rain | classic | 4.57 | 4.45 |  |  |  |  |  |  | 173.20 | OK |
| abel_rain | remastered | 5.37 | 3.62 | 0.04 | 1.62 | 0.16 | 0.05 | 0.22 | 1.20 | 179.70 | OK |
| abel_rain | delta | +0.80 | -0.83 |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 5.71 | 5.60 |  |  |  |  |  |  | 110.40 | OK |
| sinai_lowsun_fp | remastered | 10.45 | 8.97 | 0.28 | 1.36 | 0.21 | 0.05 | 0.22 | 0.88 | 85.00 | OK |
| sinai_lowsun_fp | delta | +4.74 | +3.37 |  |  |  |  |  |  |  |  |
