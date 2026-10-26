# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| coastline_sunset | classic | 3.00 | 2.79 |  |  |  |  |  |  | 276.50 | OK |
| coastline_sunset | remastered | 2.56 | 1.19 | 0.00 | 1.19 | 0.47 | 0.13 | 0.45 | 0.14 | 300.50 | OK |
| coastline_sunset | delta | -0.44 | -1.60 |  |  |  |  |  |  |  |  |
| sea_noon | classic | 2.17 | 1.91 |  |  |  |  |  |  | 265.70 | OK |
| sea_noon | remastered | 1.16 | 0.57 | 0.00 | 0.48 | 0.38 | 0.07 |  | 0.04 | 348.30 | OK |
| sea_noon | delta | -1.01 | -1.34 |  |  |  |  |  |  |  |  |
| sea_away_from_sun | classic | 1.45 | 1.22 |  |  |  |  |  |  | 320.40 | OK |
| sea_away_from_sun | remastered | 2.14 | 1.19 | 0.00 | 0.78 | 0.50 | 0.17 |  | 0.11 | 245.40 | OK |
| sea_away_from_sun | delta | +0.69 | -0.03 |  |  |  |  |  |  |  |  |
| coast_shore_midsun | classic | 2.14 | 2.04 |  |  |  |  |  |  | 344.50 | OK |
| coast_shore_midsun | remastered | 2.96 | 1.65 | 0.00 | 1.24 | 0.49 | 0.12 | 0.54 | 0.09 | 270.10 | OK |
| coast_shore_midsun | delta | +0.82 | -0.39 |  |  |  |  |  |  |  |  |
