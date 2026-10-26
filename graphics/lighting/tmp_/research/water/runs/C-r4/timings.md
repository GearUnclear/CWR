# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| coastline_sunset | classic | 1.55 | 1.46 |  |  |  |  |  |  | 321.70 | OK |
| coastline_sunset | remastered | 3.60 | 3.03 | 0.00 | 0.46 | 0.22 | 0.05 | 0.16 | 0.03 | 194.40 | OK |
| coastline_sunset | delta | +2.05 | +1.57 |  |  |  |  |  |  |  |  |
| sea_noon | classic | 1.62 | 1.54 |  |  |  |  |  |  | 312.50 | OK |
| sea_noon | remastered | 4.00 | 3.67 | 0.00 | 0.25 | 0.17 | 0.05 |  | 0.03 | 178.30 | OK |
| sea_noon | delta | +2.38 | +2.13 |  |  |  |  |  |  |  |  |
| sea_away_from_sun | classic | 1.78 | 1.68 |  |  |  |  |  |  | 287.40 | OK |
| sea_away_from_sun | remastered | 3.27 | 2.89 | 0.00 | 0.27 | 0.20 | 0.05 |  | 0.03 | 175.80 | OK |
| sea_away_from_sun | delta | +1.49 | +1.21 |  |  |  |  |  |  |  |  |
| coast_shore_midsun | classic | 1.62 | 1.57 |  |  |  |  |  |  | 288.80 | OK |
| coast_shore_midsun | remastered | 5.02 | 3.31 | 0.00 | 1.59 | 0.28 | 0.05 | 0.20 | 1.06 | 190.50 | OK |
| coast_shore_midsun | delta | +3.40 | +1.74 |  |  |  |  |  |  |  |  |
