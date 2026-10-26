# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 5.03 | 4.92 |  |  |  |  |  |  | 129.00 | OK |
| lowsun_fp | remastered | 5.77 | 4.73 | 0.12 | 0.93 | 0.22 | 0.05 | 0.22 | 0.44 | 111.00 | OK |
| lowsun_fp | delta | +0.74 | -0.19 |  |  |  |  |  |  |  |  |
| pine_eve | classic | 23.62 | 23.50 |  |  |  |  |  |  | 36.40 | OK |
| pine_eve | remastered | 26.14 | 25.58 | 0.38 | 0.43 | 0.19 | 0.05 | 0.15 | 0.04 | 33.10 | OK |
| pine_eve | delta | +2.52 | +2.08 |  |  |  |  |  |  |  |  |
| pinewood_morn | classic | 15.05 | 14.92 |  |  |  |  |  |  | 54.50 | OK |
| pinewood_morn | remastered | 13.35 | 12.74 | 1.43 | 0.46 | 0.20 | 0.05 | 0.15 | 0.06 | 60.50 | OK |
| pinewood_morn | delta | -1.70 | -2.18 |  |  |  |  |  |  |  |  |
| pine_mist | classic | 5.29 | 5.16 |  |  |  |  |  |  | 112.20 | OK |
| pine_mist | remastered | 10.85 | 10.29 | 0.19 | 0.43 | 0.19 | 0.05 | 0.15 | 0.04 | 79.80 | OK |
| pine_mist | delta | +5.56 | +5.13 |  |  |  |  |  |  |  |  |
| sunrise_ridge | classic | 13.01 | 12.88 |  |  |  |  |  |  | 47.70 | OK |
| sunrise_ridge | remastered | 33.00 | 32.25 | 0.18 | 0.61 | 0.37 | 0.05 | 0.15 | 0.04 | 22.20 | OK |
| sunrise_ridge | delta | +19.99 | +19.37 |  |  |  |  |  |  |  |  |
| town_night | classic | 16.22 | 16.07 |  |  |  |  |  |  | 54.90 | OK |
| town_night | remastered | 17.35 | 16.88 |  | 0.31 | 0.23 | 0.05 |  | 0.03 | 51.00 | OK |
| town_night | delta | +1.13 | +0.81 |  |  |  |  |  |  |  |  |
