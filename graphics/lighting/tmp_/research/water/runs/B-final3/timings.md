# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| coastline_sunset | classic | 2.48 | 2.00 |  |  |  |  |  |  | 230.80 | OK |
| coastline_sunset | remastered | 5.72 | 3.05 | 0.00 | 2.30 | 0.96 | 0.25 | 0.89 | 0.19 | 162.40 | OK |
| coastline_sunset | delta | +3.24 | +1.05 |  |  |  |  |  |  |  |  |
| sea_noon | classic | 2.99 | 2.58 |  |  |  |  |  |  | 203.90 | OK |
| sea_noon | remastered | 5.59 | 3.97 | 0.00 | 1.32 | 0.86 | 0.28 |  | 0.18 | 143.80 | OK |
| sea_noon | delta | +2.60 | +1.39 |  |  |  |  |  |  |  |  |
| sea_away_from_sun | classic | 2.93 | 2.39 |  |  |  |  |  |  | 207.40 | OK |
| sea_away_from_sun | remastered | 5.44 | 3.49 | 0.00 | 1.51 | 0.92 | 0.38 |  | 0.20 | 146.70 | OK |
| sea_away_from_sun | delta | +2.51 | +1.10 |  |  |  |  |  |  |  |  |
| coast_shore_midsun | classic | 2.61 | 2.46 |  |  |  |  |  |  | 233.90 | OK |
| coast_shore_midsun | remastered | 5.64 | 3.31 | 0.00 | 2.22 | 0.90 | 0.26 | 0.87 | 0.18 | 164.20 | OK |
| coast_shore_midsun | delta | +3.03 | +0.85 |  |  |  |  |  |  |  |  |
