# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| coastline_sunset | classic | 0.67 | 0.57 |  |  |  |  |  |  | 440.20 | OK |
| coastline_sunset | remastered | 2.69 | 2.05 | 0.00 | 0.54 | 0.30 | 0.05 | 0.16 | 0.03 | 341.60 | OK |
| coastline_sunset | delta | +2.02 | +1.48 |  |  |  |  |  |  |  |  |
| sea_noon | classic | 1.00 | 0.92 |  |  |  |  |  |  | 429.30 | OK |
| sea_noon | remastered | 2.12 | 1.73 | 0.00 | 0.31 | 0.23 | 0.05 |  | 0.03 | 265.60 | OK |
| sea_noon | delta | +1.12 | +0.81 |  |  |  |  |  |  |  |  |
| sea_away_from_sun | classic | 1.33 | 1.23 |  |  |  |  |  |  | 381.60 | OK |
| sea_away_from_sun | remastered | 3.23 | 2.91 | 0.00 | 0.22 | 0.14 | 0.05 |  | 0.03 | 232.40 | OK |
| sea_away_from_sun | delta | +1.90 | +1.68 |  |  |  |  |  |  |  |  |
| coast_shore_midsun | classic | 1.14 | 1.10 |  |  |  |  |  |  | 411.00 | OK |
| coast_shore_midsun | remastered | 3.41 | 2.37 | 0.00 | 0.99 | 0.24 | 0.05 | 0.20 | 0.50 | 262.70 | OK |
| coast_shore_midsun | delta | +2.27 | +1.27 |  |  |  |  |  |  |  |  |
