# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| grove_sun | classic | 3.97 | 3.85 |  |  |  |  |  |  | 129.30 | OK |
| grove_sun | remastered | 8.75 | 7.92 | 0.00 | 0.70 | 0.47 | 0.05 | 0.15 | 0.03 | 80.40 | OK |
| grove_sun | delta | +4.78 | +4.07 |  |  |  |  |  |  |  |  |
| sunrise_ridge | classic | 46.01 | 45.89 |  |  |  |  |  |  | 24.80 | OK |
| sunrise_ridge | remastered | 40.83 | 39.80 | 0.00 | 0.90 | 0.67 | 0.05 | 0.15 | 0.03 | 21.20 | OK |
| sunrise_ridge | delta | -5.18 | -6.09 |  |  |  |  |  |  |  |  |
| grove_mist | classic | 6.55 | 6.43 |  |  |  |  |  |  | 101.90 | OK |
| grove_mist | remastered | 7.79 | 7.20 | 0.00 | 0.46 | 0.23 | 0.05 | 0.15 | 0.03 | 89.90 | OK |
| grove_mist | delta | +1.24 | +0.77 |  |  |  |  |  |  |  |  |
| town_night | classic | 10.50 | 10.35 |  |  |  |  |  |  | 79.00 | OK |
| town_night | remastered | 12.84 | 12.26 |  | 0.41 | 0.34 | 0.05 |  | 0.03 | 66.20 | OK |
| town_night | delta | +2.34 | +1.91 |  |  |  |  |  |  |  |  |
