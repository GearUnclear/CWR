# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| bay_offshore | classic | 2.14 | 2.03 |  |  |  |  |  |  | 202.00 | OK |
| bay_offshore | remastered | 22.08 | 21.71 | 0.03 | 0.25 | 0.18 | 0.05 |  | 0.03 | 61.50 | OK |
| bay_offshore | delta | +19.94 | +19.68 |  |  |  |  |  |  |  |  |
| coastline_sunset | classic | 1.26 | 1.16 |  |  |  |  |  |  | 418.00 | OK |
| coastline_sunset | remastered | 3.46 | 2.90 | 0.00 | 0.46 | 0.18 | 0.05 | 0.16 | 0.07 | 325.50 | OK |
| coastline_sunset | delta | +2.20 | +1.74 |  |  |  |  |  |  |  |  |
| town_rain | classic | 2.61 | 2.51 |  |  |  |  |  |  | 164.10 | OK |
| town_rain | remastered | 3.19 | 2.65 | 0.04 | 0.43 | 0.35 | 0.05 |  | 0.03 | 169.70 | OK |
| town_rain | delta | +0.58 | +0.14 |  |  |  |  |  |  |  |  |
| town_wet_dusk | classic | 6.74 | 6.63 |  |  |  |  |  |  | 121.90 | OK |
| town_wet_dusk | remastered | 12.04 | 9.99 | 0.97 | 1.94 | 0.21 | 0.05 | 0.16 | 1.52 | 81.20 | OK |
| town_wet_dusk | delta | +5.30 | +3.36 |  |  |  |  |  |  |  |  |
| street_night | classic | 4.31 | 4.19 |  |  |  |  |  |  | 110.50 | OK |
| street_night | remastered | 3.56 | 3.03 |  | 0.41 | 0.33 | 0.05 |  | 0.03 | 127.50 | OK |
| street_night | delta | -0.75 | -1.16 |  |  |  |  |  |  |  |  |
| town_night | classic | 15.62 | 15.46 |  |  |  |  |  |  | 63.80 | OK |
| town_night | remastered | 22.57 | 22.03 |  | 0.27 | 0.19 | 0.05 |  | 0.03 | 44.20 | OK |
| town_night | delta | +6.95 | +6.57 |  |  |  |  |  |  |  |  |
| lowsun_fp | classic | 9.06 | 8.94 |  |  |  |  |  |  | 71.40 | OK |
| lowsun_fp | remastered | 16.19 | 14.19 | 0.20 | 1.88 | 0.18 | 0.05 | 0.22 | 1.43 | 60.70 | OK |
| lowsun_fp | delta | +7.13 | +5.25 |  |  |  |  |  |  |  |  |
