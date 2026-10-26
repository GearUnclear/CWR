# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| bay_offshore | classic | 2.14 | 2.03 |  |  |  |  |  |  | 210.50 | OK |
| bay_offshore | remastered | 24.06 | 23.60 | 0.05 | 0.34 | 0.27 | 0.05 |  | 0.03 | 51.30 | OK |
| bay_offshore | delta | +21.92 | +21.57 |  |  |  |  |  |  |  |  |
| coastline_sunset | classic | 1.20 | 1.10 |  |  |  |  |  |  | 436.80 | OK |
| coastline_sunset | remastered | 1.84 | 1.27 | 0.00 | 0.47 | 0.23 | 0.05 | 0.16 | 0.03 | 332.80 | OK |
| coastline_sunset | delta | +0.64 | +0.17 |  |  |  |  |  |  |  |  |
| town_rain | classic | 3.89 | 3.78 |  |  |  |  |  |  | 183.20 | OK |
| town_rain | remastered | 3.24 | 2.75 | 0.03 | 0.38 | 0.31 | 0.05 |  | 0.03 | 173.80 | OK |
| town_rain | delta | -0.65 | -1.03 |  |  |  |  |  |  |  |  |
| town_wet_dusk | classic | 2.59 | 2.49 |  |  |  |  |  |  | 198.20 | OK |
| town_wet_dusk | remastered | 5.23 | 4.62 | 0.03 | 0.50 | 0.25 | 0.05 | 0.16 | 0.04 | 121.20 | OK |
| town_wet_dusk | delta | +2.64 | +2.13 |  |  |  |  |  |  |  |  |
| street_night | classic | 3.69 | 3.57 |  |  |  |  |  |  | 181.80 | OK |
| street_night | remastered | 2.90 | 2.32 |  | 0.46 | 0.39 | 0.05 |  | 0.03 | 126.10 | OK |
| street_night | delta | -0.79 | -1.25 |  |  |  |  |  |  |  |  |
| town_night | classic | 10.89 | 10.74 |  |  |  |  |  |  | 101.80 | OK |
| town_night | remastered | 9.18 | 8.59 |  | 0.42 | 0.35 | 0.05 |  | 0.03 | 70.00 | OK |
| town_night | delta | -1.71 | -2.15 |  |  |  |  |  |  |  |  |
| lowsun_fp | classic | 7.44 | 7.32 |  |  |  |  |  |  | 94.10 | OK |
| lowsun_fp | remastered | 12.96 | 11.17 | 0.06 | 1.67 | 0.19 | 0.05 | 0.22 | 1.21 | 91.00 | OK |
| lowsun_fp | delta | +5.52 | +3.85 |  |  |  |  |  |  |  |  |
| overcast_heavy_rain | classic | 2.71 | 2.60 |  |  |  |  |  |  | 242.30 | OK |
| overcast_heavy_rain | remastered | 2.62 | 2.25 | 0.03 | 0.25 | 0.18 | 0.05 |  | 0.03 | 246.70 | OK |
| overcast_heavy_rain | delta | -0.09 | -0.35 |  |  |  |  |  |  |  |  |
