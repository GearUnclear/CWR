# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| bay_offshore | classic | 1.21 | 1.10 |  |  |  |  |  |  | 211.80 | OK |
| bay_offshore | remastered | 7.75 | 7.21 | 0.03 | 0.42 | 0.35 | 0.05 |  | 0.03 | 97.40 | OK |
| bay_offshore | delta | +6.54 | +6.11 |  |  |  |  |  |  |  |  |
| coastline_sunset | classic | 1.19 | 1.10 |  |  |  |  |  |  | 420.80 | OK |
| coastline_sunset | remastered | 2.26 | 1.65 | 0.00 | 0.50 | 0.26 | 0.05 | 0.16 | 0.03 | 304.50 | OK |
| coastline_sunset | delta | +1.07 | +0.55 |  |  |  |  |  |  |  |  |
| town_rain | classic | 6.16 | 6.06 |  |  |  |  |  |  | 124.70 | OK |
| town_rain | remastered | 7.45 | 7.05 | 0.47 | 0.29 | 0.21 | 0.05 |  | 0.03 | 114.40 | OK |
| town_rain | delta | +1.29 | +0.99 |  |  |  |  |  |  |  |  |
| town_wet_dusk | classic | 6.96 | 6.85 |  |  |  |  |  |  | 111.50 | OK |
| town_wet_dusk | remastered | 10.77 | 9.25 | 0.68 | 1.42 | 0.23 | 0.05 | 0.16 | 0.97 | 82.90 | OK |
| town_wet_dusk | delta | +3.81 | +2.40 |  |  |  |  |  |  |  |  |
| town_night | classic | 8.57 | 8.42 |  |  |  |  |  |  | 89.30 | OK |
| town_night | remastered | 15.22 | 14.75 |  | 0.30 | 0.23 | 0.05 |  | 0.03 | 62.80 | OK |
| town_night | delta | +6.65 | +6.33 |  |  |  |  |  |  |  |  |
| lowsun_fp | classic | 6.49 | 6.37 |  |  |  |  |  |  | 110.00 | OK |
| lowsun_fp | remastered | 10.84 | 9.07 | 0.37 | 1.65 | 0.19 | 0.05 | 0.22 | 1.18 | 79.50 | OK |
| lowsun_fp | delta | +4.35 | +2.70 |  |  |  |  |  |  |  |  |
| overcast_heavy_rain | classic | 4.24 | 4.13 |  |  |  |  |  |  | 175.40 | OK |
| overcast_heavy_rain | remastered | 3.28 | 2.88 | 0.03 | 0.28 | 0.21 | 0.05 |  | 0.03 | 190.80 | OK |
| overcast_heavy_rain | delta | -0.96 | -1.25 |  |  |  |  |  |  |  |  |
