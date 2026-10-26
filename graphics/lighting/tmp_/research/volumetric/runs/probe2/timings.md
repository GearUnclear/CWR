# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| pine_eve | classic | 6.03 | 5.91 |  |  |  |  |  |  | 123.90 | OK |
| pine_eve | remastered | 7.19 | 6.68 | 1.44 | 0.38 | 0.15 | 0.05 | 0.15 | 0.03 | 98.30 | OK |
| pine_eve | delta | +1.16 | +0.77 |  |  |  |  |  |  |  |  |
| pine_low | classic | 27.26 | 27.15 |  |  |  |  |  |  | 33.00 | OK |
| pine_low | remastered | 31.74 | 31.22 | 0.23 | 0.40 | 0.17 | 0.05 | 0.15 | 0.03 | 29.10 | OK |
| pine_low | delta | +4.48 | +4.07 |  |  |  |  |  |  |  |  |
| pine_mist | classic | 10.76 | 10.64 |  |  |  |  |  |  | 72.40 | OK |
| pine_mist | remastered | 8.06 | 7.42 | 0.23 | 0.50 | 0.27 | 0.05 | 0.15 | 0.03 | 82.00 | OK |
| pine_mist | delta | -2.70 | -3.22 |  |  |  |  |  |  |  |  |
| sunrise_ridge | classic | 26.83 | 26.70 |  |  |  |  |  |  | 35.80 | OK |
| sunrise_ridge | remastered | 40.97 | 40.22 | 0.38 | 0.62 | 0.39 | 0.05 | 0.15 | 0.03 | 21.30 | OK |
| sunrise_ridge | delta | +14.14 | +13.52 |  |  |  |  |  |  |  |  |
| town_night | classic | 10.81 | 10.66 |  |  |  |  |  |  | 70.10 | OK |
| town_night | remastered | 12.07 | 11.40 |  | 0.51 | 0.43 | 0.05 |  | 0.03 | 73.00 | OK |
| town_night | delta | +1.26 | +0.74 |  |  |  |  |  |  |  |  |
