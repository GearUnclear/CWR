# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| coastline_sunset | classic | 9.48 | 9.33 |  |  |  |  |  |  | 53.90 | OK |
| coastline_sunset | remastered | 3.39 | 2.06 | 0.00 | 1.23 | 0.96 | 0.05 | 0.17 | 0.05 | 75.80 | OK |
| coastline_sunset | delta | -6.09 | -7.27 |  |  |  |  |  |  |  |  |
| sea_noon | classic | 5.15 | 5.06 |  |  |  |  |  |  | 96.60 | OK |
| sea_noon | remastered | 3.32 | 2.10 | 0.00 | 1.14 | 1.07 | 0.05 |  | 0.03 | 120.10 | OK |
| sea_noon | delta | -1.83 | -2.96 |  |  |  |  |  |  |  |  |
| sea_away_from_sun | classic | 2.42 | 2.33 |  |  |  |  |  |  | 151.50 | OK |
| sea_away_from_sun | remastered | 6.41 | 5.13 | 0.00 | 1.17 | 1.10 | 0.05 |  | 0.03 | 118.60 | OK |
| sea_away_from_sun | delta | +3.99 | +2.80 |  |  |  |  |  |  |  |  |
| coast_shore_midsun | classic | 2.45 | 2.40 |  |  |  |  |  |  | 132.20 | OK |
| coast_shore_midsun | remastered | 3.77 | 2.41 | 0.00 | 1.32 | 0.92 | 0.05 | 0.20 | 0.14 | 145.50 | OK |
| coast_shore_midsun | delta | +1.32 | +0.01 |  |  |  |  |  |  |  |  |
