# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| bay_offshore | classic | 2.36 | 2.24 |  |  |  |  |  |  | 207.90 | OK |
| bay_offshore | remastered | 21.72 | 21.35 | 0.03 | 0.26 | 0.18 | 0.05 |  | 0.03 | 51.90 | OK |
| bay_offshore | delta | +19.36 | +19.11 |  |  |  |  |  |  |  |  |
| coastline_sunset | classic | 1.16 | 1.07 |  |  |  |  |  |  | 462.00 | OK |
| coastline_sunset | remastered | 2.04 | 1.39 | 0.00 | 0.55 | 0.30 | 0.05 | 0.16 | 0.03 | 316.00 | OK |
| coastline_sunset | delta | +0.88 | +0.32 |  |  |  |  |  |  |  |  |
| town_rain | classic | 1.64 | 1.53 |  |  |  |  |  |  | 225.70 | OK |
| town_rain | remastered | 3.57 | 3.08 | 0.03 | 0.38 | 0.30 | 0.05 |  | 0.03 | 231.50 | OK |
| town_rain | delta | +1.93 | +1.55 |  |  |  |  |  |  |  |  |
| town_wet_dusk | classic | 2.40 | 2.30 |  |  |  |  |  |  | 176.70 | OK |
| town_wet_dusk | remastered | 4.67 | 3.57 | 0.05 | 1.00 | 0.43 | 0.05 | 0.16 | 0.36 | 180.90 | OK |
| town_wet_dusk | delta | +2.27 | +1.27 |  |  |  |  |  |  |  |  |
| street_night | classic | 3.33 | 3.21 |  |  |  |  |  |  | 201.90 | OK |
| street_night | remastered | 5.61 | 5.09 |  | 0.40 | 0.32 | 0.05 |  | 0.03 | 122.90 | OK |
| street_night | delta | +2.28 | +1.88 |  |  |  |  |  |  |  |  |
| town_night | classic | 6.71 | 6.56 |  |  |  |  |  |  | 135.90 | OK |
| town_night | remastered | 10.73 | 10.17 |  | 0.40 | 0.32 | 0.05 |  | 0.03 | 100.70 | OK |
| town_night | delta | +4.02 | +3.61 |  |  |  |  |  |  |  |  |
| lowsun_fp | classic | 3.21 | 3.10 |  |  |  |  |  |  | 121.00 | OK |
| lowsun_fp | remastered | 10.20 | 9.04 | 0.31 | 1.03 | 0.20 | 0.05 | 0.22 | 0.56 | 91.10 | OK |
| lowsun_fp | delta | +6.99 | +5.94 |  |  |  |  |  |  |  |  |
| overcast_heavy_rain | classic | 1.36 | 1.25 |  |  |  |  |  |  | 353.50 | OK |
| overcast_heavy_rain | remastered | 1.82 | 1.27 | 0.03 | 0.43 | 0.35 | 0.05 |  | 0.03 | 273.20 | OK |
| overcast_heavy_rain | delta | +0.46 | +0.02 |  |  |  |  |  |  |  |  |
