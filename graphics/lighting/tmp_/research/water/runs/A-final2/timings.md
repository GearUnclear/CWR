# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| coastline_sunset | classic | 1.36 | 1.26 |  |  |  |  |  |  | 419.80 | OK |
| coastline_sunset | remastered | 3.78 | 3.25 | 0.00 | 0.43 | 0.19 | 0.05 | 0.16 | 0.03 | 214.50 | OK |
| coastline_sunset | delta | +2.42 | +1.99 |  |  |  |  |  |  |  |  |
| sea_noon | classic | 1.71 | 1.62 |  |  |  |  |  |  | 369.60 | OK |
| sea_noon | remastered | 3.50 | 3.16 | 0.00 | 0.26 | 0.19 | 0.05 |  | 0.03 | 217.60 | OK |
| sea_noon | delta | +1.79 | +1.54 |  |  |  |  |  |  |  |  |
| sea_away_from_sun | classic | 1.18 | 1.08 |  |  |  |  |  |  | 384.90 | OK |
| sea_away_from_sun | remastered | 2.30 | 1.86 | 0.00 | 0.34 | 0.27 | 0.05 |  | 0.03 | 255.40 | OK |
| sea_away_from_sun | delta | +1.12 | +0.78 |  |  |  |  |  |  |  |  |
| coast_shore_midsun | classic | 0.79 | 0.74 |  |  |  |  |  |  | 430.10 | OK |
| coast_shore_midsun | remastered | 4.02 | 2.79 | 0.00 | 1.20 | 0.17 | 0.05 | 0.20 | 0.77 | 252.80 | OK |
| coast_shore_midsun | delta | +3.23 | +2.05 |  |  |  |  |  |  |  |  |
