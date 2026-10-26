# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| bay_offshore | classic | 1.19 | 1.08 |  |  |  |  |  |  | 260.60 | OK |
| bay_offshore | remastered | 26.33 | 25.95 | 0.03 | 0.27 | 0.19 | 0.05 |  | 0.03 | 48.10 | OK |
| bay_offshore | delta | +25.14 | +24.87 |  |  |  |  |  |  |  |  |
| coastline_sunset | classic | 1.28 | 1.19 |  |  |  |  |  |  | 385.50 | OK |
| coastline_sunset | remastered | 2.65 | 2.10 | 0.00 | 0.44 | 0.20 | 0.05 | 0.16 | 0.03 | 280.60 | OK |
| coastline_sunset | delta | +1.37 | +0.91 |  |  |  |  |  |  |  |  |
| town_rain | classic | 3.65 | 3.54 |  |  |  |  |  |  | 162.90 | OK |
| town_rain | remastered | 4.10 | 3.67 | 0.04 | 0.32 | 0.25 | 0.05 |  | 0.03 | 172.50 | OK |
| town_rain | delta | +0.45 | +0.13 |  |  |  |  |  |  |  |  |
| town_wet_dusk | classic | 2.82 | 2.72 |  |  |  |  |  |  | 170.80 | OK |
| town_wet_dusk | remastered | 5.72 | 4.68 | 0.03 | 0.93 | 0.30 | 0.05 | 0.16 | 0.41 | 161.10 | OK |
| town_wet_dusk | delta | +2.90 | +1.96 |  |  |  |  |  |  |  |  |
| street_night | classic | 2.53 | 2.42 |  |  |  |  |  |  | 188.30 | OK |
| street_night | remastered | 4.48 | 3.86 |  | 0.50 | 0.43 | 0.05 |  | 0.03 | 144.40 | OK |
| street_night | delta | +1.95 | +1.44 |  |  |  |  |  |  |  |  |
| town_night | classic | 13.91 | 13.76 |  |  |  |  |  |  | 70.70 | OK |
| town_night | remastered | 19.22 | 18.57 |  | 0.31 | 0.23 | 0.05 |  | 0.03 | 50.70 | OK |
| town_night | delta | +5.31 | +4.81 |  |  |  |  |  |  |  |  |
| lowsun_fp | classic | 7.10 | 6.98 |  |  |  |  |  |  | 94.70 | OK |
| lowsun_fp | remastered | 11.44 | 10.02 | 0.25 | 1.30 | 0.17 | 0.05 | 0.22 | 0.87 | 75.30 | OK |
| lowsun_fp | delta | +4.34 | +3.04 |  |  |  |  |  |  |  |  |
| overcast_heavy_rain | classic | 2.30 | 2.19 |  |  |  |  |  |  | 217.60 | OK |
| overcast_heavy_rain | remastered | 1.37 | 0.68 | 0.03 | 0.57 | 0.50 | 0.05 |  | 0.03 | 275.30 | OK |
| overcast_heavy_rain | delta | -0.93 | -1.51 |  |  |  |  |  |  |  |  |
