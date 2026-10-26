# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| bay_offshore | classic | 1.53 | 1.40 |  |  |  |  |  |  | 205.00 | OK |
| bay_offshore | remastered | 8.60 | 8.01 | 0.03 | 0.48 | 0.40 | 0.05 |  | 0.03 | 85.20 | OK |
| bay_offshore | delta | +7.07 | +6.61 |  |  |  |  |  |  |  |  |
| coastline_sunset | classic | 1.09 | 0.97 |  |  |  |  |  |  | 368.20 | OK |
| coastline_sunset | remastered | 1.77 | 0.91 | 0.00 | 0.74 | 0.42 | 0.07 | 0.22 | 0.04 | 264.00 | OK |
| coastline_sunset | delta | +0.68 | -0.06 |  |  |  |  |  |  |  |  |
| town_rain | classic | 1.70 | 1.59 |  |  |  |  |  |  | 178.00 | OK |
| town_rain | remastered | 2.86 | 2.11 | 0.04 | 0.63 | 0.55 | 0.05 |  | 0.03 | 147.10 | OK |
| town_rain | delta | +1.16 | +0.52 |  |  |  |  |  |  |  |  |
| town_wet_dusk | classic | 3.20 | 3.10 |  |  |  |  |  |  | 154.80 | OK |
| town_wet_dusk | remastered | 6.29 | 5.44 | 0.50 | 0.74 | 0.39 | 0.05 | 0.16 | 0.14 | 121.10 | OK |
| town_wet_dusk | delta | +3.09 | +2.34 |  |  |  |  |  |  |  |  |
| town_night | classic | 13.12 | 12.96 |  |  |  |  |  |  | 70.80 | OK |
| town_night | remastered | 19.80 | 19.08 |  | 0.29 | 0.22 | 0.05 |  | 0.03 | 48.30 | OK |
| town_night | delta | +6.68 | +6.12 |  |  |  |  |  |  |  |  |
| lowsun_fp | classic | 10.96 | 10.85 |  |  |  |  |  |  | 67.70 | OK |
| lowsun_fp | remastered | 9.64 | 8.02 | 0.14 | 1.49 | 0.21 | 0.05 | 0.22 | 1.01 | 68.50 | OK |
| lowsun_fp | delta | -1.32 | -2.83 |  |  |  |  |  |  |  |  |
| overcast_heavy_rain | classic | 2.02 | 1.91 |  |  |  |  |  |  | 207.00 | OK |
| overcast_heavy_rain | remastered | 2.41 | 1.69 | 0.03 | 0.59 | 0.52 | 0.05 |  | 0.03 | 207.10 | OK |
| overcast_heavy_rain | delta | +0.39 | -0.22 |  |  |  |  |  |  |  |  |
