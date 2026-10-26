# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| coastline_sunset | classic | 1.99 | 1.79 |  |  |  |  |  |  | 189.90 | OK |
| coastline_sunset | remastered | 2.14 | 1.12 | 0.00 | 0.91 | 0.64 | 0.05 | 0.18 | 0.03 | 160.00 | OK |
| coastline_sunset | delta | +0.15 | -0.67 |  |  |  |  |  |  |  |  |
| sea_noon | classic | 2.06 | 1.91 |  |  |  |  |  |  | 249.50 | OK |
| sea_noon | remastered | 3.34 | 2.31 | 0.00 | 0.90 | 0.60 | 0.10 |  | 0.20 | 160.20 | OK |
| sea_noon | delta | +1.28 | +0.40 |  |  |  |  |  |  |  |  |
| sea_away_from_sun | classic | 1.41 | 1.21 |  |  |  |  |  |  | 229.30 | OK |
| sea_away_from_sun | remastered | 4.17 | 3.03 | 0.00 | 0.97 | 0.54 | 0.11 |  | 0.32 | 158.90 | OK |
| sea_away_from_sun | delta | +2.76 | +1.82 |  |  |  |  |  |  |  |  |
| coast_shore_midsun | classic | 1.57 | 1.49 |  |  |  |  |  |  | 245.70 | OK |
| coast_shore_midsun | remastered | 5.34 | 3.92 | 0.00 | 1.35 | 0.55 | 0.09 | 0.37 | 0.35 | 151.90 | OK |
| coast_shore_midsun | delta | +3.77 | +2.43 |  |  |  |  |  |  |  |  |
