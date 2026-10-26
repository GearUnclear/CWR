# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| coastline_sunset | classic | 1.16 | 1.00 |  |  |  |  |  |  | 266.90 | OK |
| coastline_sunset | remastered | 2.55 | 1.27 | 0.00 | 1.15 | 0.62 | 0.07 | 0.24 | 0.21 | 197.50 | OK |
| coastline_sunset | delta | +1.39 | +0.27 |  |  |  |  |  |  |  |  |
| sea_noon | classic | 3.90 | 3.76 |  |  |  |  |  |  | 128.60 | OK |
| sea_noon | remastered | 3.34 | 2.15 | 0.00 | 1.07 | 0.93 | 0.09 |  | 0.05 | 145.30 | OK |
| sea_noon | delta | -0.56 | -1.61 |  |  |  |  |  |  |  |  |
| sea_away_from_sun | classic | 1.62 | 1.45 |  |  |  |  |  |  | 183.90 | OK |
| sea_away_from_sun | remastered | 3.45 | 2.69 | 0.00 | 0.64 | 0.53 | 0.07 |  | 0.04 | 156.20 | OK |
| sea_away_from_sun | delta | +1.83 | +1.24 |  |  |  |  |  |  |  |  |
| coast_shore_midsun | classic | 1.22 | 1.15 |  |  |  |  |  |  | 250.00 | OK |
| coast_shore_midsun | remastered | 3.71 | 2.34 | 0.00 | 1.31 | 0.79 | 0.09 | 0.36 | 0.07 | 169.50 | OK |
| coast_shore_midsun | delta | +2.49 | +1.19 |  |  |  |  |  |  |  |  |
