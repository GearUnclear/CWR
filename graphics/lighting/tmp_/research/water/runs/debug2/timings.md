# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| coastline_sunset | classic | 2.62 | 2.15 |  |  |  |  |  |  | 228.50 | OK |
| coastline_sunset | remastered | 4.50 | 2.20 | 0.00 | 1.83 | 0.81 | 0.20 | 0.57 | 0.25 | 176.30 | OK |
| coastline_sunset | delta | +1.88 | +0.05 |  |  |  |  |  |  |  |  |
| sea_noon | classic | 2.56 | 2.26 |  |  |  |  |  |  | 219.10 | OK |
| sea_noon | remastered | 4.50 | 2.96 | 0.00 | 1.25 | 0.80 | 0.22 |  | 0.23 | 174.00 | OK |
| sea_noon | delta | +1.94 | +0.70 |  |  |  |  |  |  |  |  |
| sea_away_from_sun | classic | 2.59 | 2.07 |  |  |  |  |  |  | 223.20 | OK |
| sea_away_from_sun | remastered | 4.12 | 2.41 | 0.00 | 1.35 | 0.82 | 0.36 |  | 0.16 | 185.40 | OK |
| sea_away_from_sun | delta | +1.53 | +0.34 |  |  |  |  |  |  |  |  |
| coast_shore_midsun | classic | 2.49 | 2.36 |  |  |  |  |  |  | 230.70 | OK |
| coast_shore_midsun | remastered | 4.99 | 2.84 | 0.00 | 2.03 | 0.88 | 0.16 | 0.58 | 0.42 | 189.80 | OK |
| coast_shore_midsun | delta | +2.50 | +0.48 |  |  |  |  |  |  |  |  |
