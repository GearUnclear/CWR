# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| coastline_sunset | classic | 2.12 | 1.74 |  |  |  |  |  |  | 235.80 | OK |
| coastline_sunset | remastered | 5.37 | 2.70 | 0.00 | 2.25 | 0.94 | 0.24 | 0.89 | 0.18 | 179.10 | OK |
| coastline_sunset | delta | +3.25 | +0.96 |  |  |  |  |  |  |  |  |
| sea_noon | classic | 3.21 | 2.81 |  |  |  |  |  |  | 206.90 | OK |
| sea_noon | remastered | 5.49 | 3.70 | 0.00 | 1.47 | 0.97 | 0.29 |  | 0.21 | 150.60 | OK |
| sea_noon | delta | +2.28 | +0.89 |  |  |  |  |  |  |  |  |
| sea_away_from_sun | classic | 2.57 | 1.89 |  |  |  |  |  |  | 221.50 | OK |
| sea_away_from_sun | remastered | 5.15 | 3.03 | 0.00 | 1.55 | 1.03 | 0.31 |  | 0.22 | 159.60 | OK |
| sea_away_from_sun | delta | +2.58 | +1.14 |  |  |  |  |  |  |  |  |
| coast_shore_midsun | classic | 2.79 | 2.62 |  |  |  |  |  |  | 229.00 | OK |
| coast_shore_midsun | remastered | 5.90 | 3.13 | 0.00 | 2.64 | 1.08 | 0.25 | 0.96 | 0.35 | 168.90 | OK |
| coast_shore_midsun | delta | +3.11 | +0.51 |  |  |  |  |  |  |  |  |
