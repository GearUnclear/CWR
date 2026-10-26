# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| bay_offshore | classic | 2.63 | 2.52 |  |  |  |  |  |  | 176.10 | OK |
| bay_offshore | remastered | 28.52 | 28.15 | 0.03 | 0.25 | 0.18 | 0.05 |  | 0.03 | 43.70 | OK |
| bay_offshore | delta | +25.89 | +25.63 |  |  |  |  |  |  |  |  |
| coastline_sunset | classic | 1.66 | 1.57 |  |  |  |  |  |  | 346.90 | OK |
| coastline_sunset | remastered | 3.45 | 2.97 | 0.00 | 0.39 | 0.14 | 0.05 | 0.16 | 0.03 | 218.50 | OK |
| coastline_sunset | delta | +1.79 | +1.40 |  |  |  |  |  |  |  |  |
| town_rain | classic | 5.53 | 5.42 |  |  |  |  |  |  | 123.70 | OK |
| town_rain | remastered | 8.36 | 7.97 | 0.39 | 0.28 | 0.20 | 0.05 |  | 0.03 | 98.60 | OK |
| town_rain | delta | +2.83 | +2.55 |  |  |  |  |  |  |  |  |
| town_wet_dusk | classic | 8.79 | 8.69 |  |  |  |  |  |  | 96.60 | OK |
| town_wet_dusk | remastered | 12.40 | 10.89 | 0.03 | 1.41 | 0.23 | 0.05 | 0.17 | 0.97 | 71.10 | OK |
| town_wet_dusk | delta | +3.61 | +2.20 |  |  |  |  |  |  |  |  |
| street_night | classic | 6.59 | 6.48 |  |  |  |  |  |  | 97.00 | OK |
| street_night | remastered | 9.19 | 8.66 |  | 0.42 | 0.35 | 0.05 |  | 0.03 | 82.30 | OK |
| street_night | delta | +2.60 | +2.18 |  |  |  |  |  |  |  |  |
| town_night | classic | 12.19 | 12.04 |  |  |  |  |  |  | 68.00 | OK |
| town_night | remastered | 20.26 | 19.73 |  | 0.34 | 0.26 | 0.05 |  | 0.03 | 49.40 | OK |
| town_night | delta | +8.07 | +7.69 |  |  |  |  |  |  |  |  |
| lowsun_fp | classic | 12.67 | 12.56 |  |  |  |  |  |  | 67.30 | OK |
| lowsun_fp | remastered | 16.62 | 14.60 | 0.47 | 1.91 | 0.17 | 0.05 | 0.22 | 1.47 | 58.10 | OK |
| lowsun_fp | delta | +3.95 | +2.04 |  |  |  |  |  |  |  |  |
| overcast_heavy_rain | classic | 2.20 | 2.08 |  |  |  |  |  |  | 246.30 | OK |
| overcast_heavy_rain | remastered | 3.19 | 2.71 | 0.03 | 0.36 | 0.29 | 0.05 |  | 0.03 | 205.90 | OK |
| overcast_heavy_rain | delta | +0.99 | +0.63 |  |  |  |  |  |  |  |  |
