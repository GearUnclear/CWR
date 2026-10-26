# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| coastline_sunset | classic | 0.92 | 0.82 |  |  |  |  |  |  | 325.90 | OK |
| coastline_sunset | remastered | 1.78 | 1.05 | 0.00 | 0.63 | 0.39 | 0.05 | 0.16 | 0.03 | 284.50 | OK |
| coastline_sunset | delta | +0.86 | +0.23 |  |  |  |  |  |  |  |  |
| sea_noon | classic | 1.23 | 1.15 |  |  |  |  |  |  | 309.80 | OK |
| sea_noon | remastered | 2.03 | 1.55 | 0.00 | 0.40 | 0.32 | 0.05 |  | 0.03 | 252.40 | OK |
| sea_noon | delta | +0.80 | +0.40 |  |  |  |  |  |  |  |  |
| sea_away_from_sun | classic | 1.50 | 1.40 |  |  |  |  |  |  | 317.20 | OK |
| sea_away_from_sun | remastered | 2.56 | 2.19 | 0.00 | 0.27 | 0.19 | 0.05 |  | 0.03 | 246.30 | OK |
| sea_away_from_sun | delta | +1.06 | +0.79 |  |  |  |  |  |  |  |  |
| coast_shore_midsun | classic | 1.57 | 1.52 |  |  |  |  |  |  | 320.80 | OK |
| coast_shore_midsun | remastered | 5.03 | 3.19 | 0.00 | 1.79 | 1.09 | 0.05 | 0.20 | 0.45 | 135.50 | OK |
| coast_shore_midsun | delta | +3.46 | +1.67 |  |  |  |  |  |  |  |  |
