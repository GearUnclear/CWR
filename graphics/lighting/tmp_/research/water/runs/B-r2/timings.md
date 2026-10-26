# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| coastline_sunset | classic | 1.88 | 1.69 |  |  |  |  |  |  | 200.50 | OK |
| coastline_sunset | remastered | 1.81 | 0.83 | 0.00 | 0.86 | 0.60 | 0.05 | 0.18 | 0.03 | 145.80 | OK |
| coastline_sunset | delta | -0.07 | -0.86 |  |  |  |  |  |  |  |  |
| sea_noon | classic | 2.02 | 1.87 |  |  |  |  |  |  | 248.40 | OK |
| sea_noon | remastered | 3.11 | 2.07 | 0.00 | 0.90 | 0.66 | 0.10 |  | 0.14 | 164.90 | OK |
| sea_noon | delta | +1.09 | +0.20 |  |  |  |  |  |  |  |  |
| sea_away_from_sun | classic | 1.45 | 1.25 |  |  |  |  |  |  | 225.20 | OK |
| sea_away_from_sun | remastered | 3.97 | 2.96 | 0.00 | 0.84 | 0.54 | 0.11 |  | 0.19 | 174.00 | OK |
| sea_away_from_sun | delta | +2.52 | +1.71 |  |  |  |  |  |  |  |  |
| coast_shore_midsun | classic | 1.39 | 1.30 |  |  |  |  |  |  | 237.10 | OK |
| coast_shore_midsun | remastered | 4.81 | 2.94 | 0.00 | 1.80 | 0.56 | 0.10 | 0.37 | 0.77 | 180.40 | OK |
| coast_shore_midsun | delta | +3.42 | +1.64 |  |  |  |  |  |  |  |  |
