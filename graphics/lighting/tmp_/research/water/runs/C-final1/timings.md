# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| coastline_sunset | classic | 1.33 | 1.23 |  |  |  |  |  |  | 225.10 | OK |
| coastline_sunset | remastered | 3.47 | 2.95 | 0.00 | 0.41 | 0.17 | 0.05 | 0.16 | 0.03 | 181.70 | OK |
| coastline_sunset | delta | +2.14 | +1.72 |  |  |  |  |  |  |  |  |
| sea_noon | classic | 1.65 | 1.57 |  |  |  |  |  |  | 283.30 | OK |
| sea_noon | remastered | 3.52 | 3.15 | 0.00 | 0.29 | 0.22 | 0.05 |  | 0.03 | 193.50 | OK |
| sea_noon | delta | +1.87 | +1.58 |  |  |  |  |  |  |  |  |
| sea_away_from_sun | classic | 1.58 | 1.48 |  |  |  |  |  |  | 303.40 | OK |
| sea_away_from_sun | remastered | 2.89 | 2.48 | 0.00 | 0.30 | 0.23 | 0.05 |  | 0.03 | 194.50 | OK |
| sea_away_from_sun | delta | +1.31 | +1.00 |  |  |  |  |  |  |  |  |
| coast_shore_midsun | classic | 1.29 | 1.24 |  |  |  |  |  |  | 288.60 | OK |
| coast_shore_midsun | remastered | 4.92 | 3.41 | 0.00 | 1.46 | 0.16 | 0.05 | 0.20 | 1.05 | 196.70 | OK |
| coast_shore_midsun | delta | +3.63 | +2.17 |  |  |  |  |  |  |  |  |
