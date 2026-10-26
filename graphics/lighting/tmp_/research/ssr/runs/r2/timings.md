# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| bay_morning | classic | 1.70 | 1.58 |  |  |  |  |  |  | 208.40 | OK |
| bay_morning | remastered | 32.37 | 31.82 | 0.03 | 0.43 | 0.35 | 0.05 |  | 0.03 | 42.20 | OK |
| bay_morning | delta | +30.67 | +30.24 |  |  |  |  |  |  |  |  |
| coastline_sunset | classic | 0.73 | 0.64 |  |  |  |  |  |  | 398.70 | OK |
| coastline_sunset | remastered | 3.02 | 2.41 | 0.00 | 0.50 | 0.25 | 0.05 | 0.16 | 0.03 | 229.10 | OK |
| coastline_sunset | delta | +2.29 | +1.77 |  |  |  |  |  |  |  |  |
| lowsun_fp | classic | 7.01 | 6.91 |  |  |  |  |  |  | 93.20 | OK |
| lowsun_fp | remastered | 7.01 | 5.99 | 0.04 | 0.91 | 0.41 | 0.05 | 0.22 | 0.23 | 92.60 | OK |
| lowsun_fp | delta | +0.00 | -0.92 |  |  |  |  |  |  |  |  |
| town_night | classic | 10.43 | 10.28 |  |  |  |  |  |  | 83.20 | OK |
| town_night | remastered | 15.72 | 15.16 |  | 0.37 | 0.29 | 0.05 |  | 0.03 | 61.20 | OK |
| town_night | delta | +5.29 | +4.88 |  |  |  |  |  |  |  |  |
| overcast_heavy_rain | classic | 4.29 | 4.17 |  |  |  |  |  |  | 158.00 | OK |
| overcast_heavy_rain | remastered | 3.75 | 3.38 | 0.03 | 0.26 | 0.18 | 0.05 |  | 0.03 | 198.50 | OK |
| overcast_heavy_rain | delta | -0.54 | -0.79 |  |  |  |  |  |  |  |  |
