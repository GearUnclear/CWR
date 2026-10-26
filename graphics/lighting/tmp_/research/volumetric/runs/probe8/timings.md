# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| town_eve | classic | 4.26 | 4.14 |  |  |  |  |  |  | 141.40 | OK |
| town_eve | remastered | 4.66 | 4.01 | 0.13 | 0.53 | 0.29 | 0.05 | 0.15 | 0.04 | 144.30 | OK |
| town_eve | delta | +0.40 | -0.13 |  |  |  |  |  |  |  |  |
| pinewood_morn | classic | 14.68 | 14.55 |  |  |  |  |  |  | 48.00 | OK |
| pinewood_morn | remastered | 6.65 | 5.91 | 0.63 | 0.60 | 0.36 | 0.05 | 0.15 | 0.04 | 99.00 | OK |
| pinewood_morn | delta | -8.03 | -8.64 |  |  |  |  |  |  |  |  |
| pine_eve | classic | 17.23 | 17.12 |  |  |  |  |  |  | 50.60 | OK |
| pine_eve | remastered | 29.56 | 29.04 | 0.38 | 0.40 | 0.16 | 0.05 | 0.15 | 0.04 | 33.30 | OK |
| pine_eve | delta | +12.33 | +11.92 |  |  |  |  |  |  |  |  |
| pinewood_mist | classic | 5.64 | 5.50 |  |  |  |  |  |  | 126.70 | OK |
| pinewood_mist | remastered | 5.23 | 4.63 | 1.05 | 0.45 | 0.21 | 0.05 | 0.15 | 0.04 | 155.70 | OK |
| pinewood_mist | delta | -0.41 | -0.87 |  |  |  |  |  |  |  |  |
| sunrise_ridge | classic | 40.58 | 40.46 |  |  |  |  |  |  | 32.50 | OK |
| sunrise_ridge | remastered | 33.62 | 32.86 | 0.17 | 0.63 | 0.40 | 0.05 | 0.15 | 0.04 | 24.90 | OK |
| sunrise_ridge | delta | -6.96 | -7.60 |  |  |  |  |  |  |  |  |
| lowsun_fp | classic | 12.97 | 12.86 |  |  |  |  |  |  | 63.00 | OK |
| lowsun_fp | remastered | 12.95 | 11.34 | 0.32 | 1.50 | 0.30 | 0.05 | 0.22 | 0.93 | 60.90 | OK |
| lowsun_fp | delta | -0.02 | -1.52 |  |  |  |  |  |  |  |  |
| town_night_fog | classic | 16.72 | 16.57 |  |  |  |  |  |  | 54.30 | OK |
| town_night_fog | remastered | 17.41 | 16.98 |  | 0.28 | 0.20 | 0.05 |  | 0.03 | 51.70 | OK |
| town_night_fog | delta | +0.69 | +0.41 |  |  |  |  |  |  |  |  |
