# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 5.97 | 5.87 |  |  |  |  |  |  | 109.00 | OK |
| lowsun_fp | remastered | 11.47 | 9.31 | 0.23 | 2.06 | 0.17 | 0.05 | 0.22 | 1.62 | 80.30 | OK |
| lowsun_fp | delta | +5.50 | +3.44 |  |  |  |  |  |  |  |  |
| pine_eve | classic | 29.19 | 29.08 |  |  |  |  |  |  | 34.50 | OK |
| pine_eve | remastered | 31.97 | 31.45 | 0.37 | 0.39 | 0.15 | 0.05 | 0.15 | 0.04 | 26.90 | OK |
| pine_eve | delta | +2.78 | +2.37 |  |  |  |  |  |  |  |  |
| pinewood_morn | classic | 15.29 | 15.15 |  |  |  |  |  |  | 53.20 | OK |
| pinewood_morn | remastered | 9.17 | 8.41 | 0.88 | 0.61 | 0.37 | 0.05 | 0.15 | 0.04 | 74.80 | OK |
| pinewood_morn | delta | -6.12 | -6.74 |  |  |  |  |  |  |  |  |
| pine_mist | classic | 11.56 | 11.43 |  |  |  |  |  |  | 73.20 | OK |
| pine_mist | remastered | 6.78 | 6.00 | 0.25 | 0.65 | 0.41 | 0.05 | 0.15 | 0.04 | 106.50 | OK |
| pine_mist | delta | -4.78 | -5.43 |  |  |  |  |  |  |  |  |
| sunrise_ridge | classic | 11.69 | 11.56 |  |  |  |  |  |  | 47.50 | OK |
| sunrise_ridge | remastered | 26.05 | 24.95 | 0.07 | 0.97 | 0.74 | 0.05 | 0.15 | 0.04 | 24.60 | OK |
| sunrise_ridge | delta | +14.36 | +13.39 |  |  |  |  |  |  |  |  |
| town_night | classic | 16.33 | 16.18 |  |  |  |  |  |  | 51.10 | OK |
| town_night | remastered | 18.17 | 17.40 |  | 0.60 | 0.52 | 0.05 |  | 0.03 | 47.70 | OK |
| town_night | delta | +1.84 | +1.22 |  |  |  |  |  |  |  |  |
