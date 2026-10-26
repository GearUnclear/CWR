# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| bay_offshore | classic | 1.20 | 1.09 |  |  |  |  |  |  | 238.10 | OK |
| bay_offshore | remastered | 7.29 | 6.74 | 0.03 | 0.43 | 0.36 | 0.05 |  | 0.03 | 105.50 | OK |
| bay_offshore | delta | +6.09 | +5.65 |  |  |  |  |  |  |  |  |
| coastline_sunset | classic | 0.75 | 0.66 |  |  |  |  |  |  | 506.30 | OK |
| coastline_sunset | remastered | 1.94 | 1.37 | 0.00 | 0.47 | 0.23 | 0.05 | 0.16 | 0.03 | 338.30 | OK |
| coastline_sunset | delta | +1.19 | +0.71 |  |  |  |  |  |  |  |  |
| town_rain | classic | 3.34 | 3.23 |  |  |  |  |  |  | 212.70 | OK |
| town_rain | remastered | 2.51 | 2.00 | 0.03 | 0.40 | 0.33 | 0.05 |  | 0.03 | 195.50 | OK |
| town_rain | delta | -0.83 | -1.23 |  |  |  |  |  |  |  |  |
| town_wet_dusk | classic | 6.52 | 6.42 |  |  |  |  |  |  | 118.80 | OK |
| town_wet_dusk | remastered | 12.15 | 10.43 | 0.04 | 1.61 | 0.20 | 0.05 | 0.16 | 1.20 | 82.40 | OK |
| town_wet_dusk | delta | +5.63 | +4.01 |  |  |  |  |  |  |  |  |
| street_night | classic | 4.62 | 4.50 |  |  |  |  |  |  | 131.00 | OK |
| street_night | remastered | 9.92 | 9.53 |  | 0.27 | 0.20 | 0.05 |  | 0.03 | 88.40 | OK |
| street_night | delta | +5.30 | +5.03 |  |  |  |  |  |  |  |  |
| town_night | classic | 10.57 | 10.42 |  |  |  |  |  |  | 84.70 | OK |
| town_night | remastered | 6.07 | 5.40 |  | 0.51 | 0.43 | 0.05 |  | 0.03 | 91.10 | OK |
| town_night | delta | -4.50 | -5.02 |  |  |  |  |  |  |  |  |
| lowsun_fp | classic | 4.45 | 4.33 |  |  |  |  |  |  | 148.00 | OK |
| lowsun_fp | remastered | 4.75 | 3.94 | 0.04 | 0.69 | 0.38 | 0.05 | 0.22 | 0.05 | 125.10 | OK |
| lowsun_fp | delta | +0.30 | -0.39 |  |  |  |  |  |  |  |  |
| overcast_heavy_rain | classic | 1.79 | 1.68 |  |  |  |  |  |  | 301.40 | OK |
| overcast_heavy_rain | remastered | 1.90 | 1.21 | 0.03 | 0.57 | 0.49 | 0.05 |  | 0.03 | 265.90 | OK |
| overcast_heavy_rain | delta | +0.11 | -0.47 |  |  |  |  |  |  |  |  |
