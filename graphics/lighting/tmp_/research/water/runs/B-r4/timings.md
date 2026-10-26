# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| coastline_sunset | classic | 1.57 | 1.48 |  |  |  |  |  |  | 318.90 | OK |
| coastline_sunset | remastered | 4.05 | 3.53 | 0.00 | 0.42 | 0.18 | 0.05 | 0.16 | 0.03 | 207.50 | OK |
| coastline_sunset | delta | +2.48 | +2.05 |  |  |  |  |  |  |  |  |
| sea_noon | classic | 1.85 | 1.77 |  |  |  |  |  |  | 317.80 | OK |
| sea_noon | remastered | 3.86 | 3.54 | 0.00 | 0.23 | 0.16 | 0.05 |  | 0.03 | 195.20 | OK |
| sea_noon | delta | +2.01 | +1.77 |  |  |  |  |  |  |  |  |
| sea_away_from_sun | classic | 1.73 | 1.64 |  |  |  |  |  |  | 341.90 | OK |
| sea_away_from_sun | remastered | 3.31 | 2.79 | 0.00 | 0.42 | 0.34 | 0.05 |  | 0.03 | 227.00 | OK |
| sea_away_from_sun | delta | +1.58 | +1.15 |  |  |  |  |  |  |  |  |
| coast_shore_midsun | classic | 1.92 | 1.88 |  |  |  |  |  |  | 328.20 | OK |
| coast_shore_midsun | remastered | 6.89 | 4.72 | 0.00 | 2.10 | 0.19 | 0.05 | 0.20 | 1.66 | 186.70 | OK |
| coast_shore_midsun | delta | +4.97 | +2.84 |  |  |  |  |  |  |  |  |
