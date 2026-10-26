# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| coastline_sunset | classic | 4.69 | 3.83 |  |  |  |  |  |  | 177.90 | OK |
| coastline_sunset | remastered | 4.59 | 2.50 | 0.00 | 1.79 | 0.85 | 0.20 | 0.55 | 0.18 | 171.60 | OK |
| coastline_sunset | delta | -0.10 | -1.33 |  |  |  |  |  |  |  |  |
| sea_noon | classic | 4.35 | 3.88 |  |  |  |  |  |  | 174.00 | OK |
| sea_noon | remastered | 4.59 | 3.28 | 0.00 | 1.08 | 0.77 | 0.19 |  | 0.13 | 155.60 | OK |
| sea_noon | delta | +0.24 | -0.60 |  |  |  |  |  |  |  |  |
| sea_away_from_sun | classic | 2.42 | 1.87 |  |  |  |  |  |  | 236.80 | OK |
| sea_away_from_sun | remastered | 1.11 | 0.57 | 0.00 | 0.45 | 0.37 | 0.05 |  | 0.03 | 286.80 | OK |
| sea_away_from_sun | delta | -1.31 | -1.30 |  |  |  |  |  |  |  |  |
| coast_shore_midsun | classic | 2.23 | 2.09 |  |  |  |  |  |  | 296.10 | OK |
| coast_shore_midsun | remastered | 5.81 | 3.65 | 0.00 | 2.04 | 0.94 | 0.18 | 0.73 | 0.20 | 150.30 | OK |
| coast_shore_midsun | delta | +3.58 | +1.56 |  |  |  |  |  |  |  |  |
