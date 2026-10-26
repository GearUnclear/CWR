# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| coastline_sunset | classic | 0.81 | 0.71 |  |  |  |  |  |  | 430.90 | OK |
| coastline_sunset | remastered | 2.77 | 2.22 | 0.00 | 0.45 | 0.20 | 0.05 | 0.16 | 0.03 | 304.20 | OK |
| coastline_sunset | delta | +1.96 | +1.51 |  |  |  |  |  |  |  |  |
| sea_noon | classic | 0.85 | 0.77 |  |  |  |  |  |  | 455.60 | OK |
| sea_noon | remastered | 1.72 | 1.30 | 0.00 | 0.34 | 0.26 | 0.05 |  | 0.03 | 335.70 | OK |
| sea_noon | delta | +0.87 | +0.53 |  |  |  |  |  |  |  |  |
| sea_away_from_sun | classic | 0.77 | 0.68 |  |  |  |  |  |  | 447.10 | OK |
| sea_away_from_sun | remastered | 1.00 | 0.51 | 0.00 | 0.39 | 0.32 | 0.05 |  | 0.03 | 427.00 | OK |
| sea_away_from_sun | delta | +0.23 | -0.17 |  |  |  |  |  |  |  |  |
| coast_shore_midsun | classic | 0.88 | 0.83 |  |  |  |  |  |  | 443.20 | OK |
| coast_shore_midsun | remastered | 3.50 | 2.28 | 0.00 | 1.18 | 0.18 | 0.05 | 0.20 | 0.75 | 331.90 | OK |
| coast_shore_midsun | delta | +2.62 | +1.45 |  |  |  |  |  |  |  |  |
