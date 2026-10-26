# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| coastline_sunset | classic | 1.11 | 0.93 |  |  |  |  |  |  | 362.50 | OK |
| coastline_sunset | remastered | 2.85 | 1.37 | 0.00 | 1.28 | 0.46 | 0.15 | 0.49 | 0.18 | 249.90 | OK |
| coastline_sunset | delta | +1.74 | +0.44 |  |  |  |  |  |  |  |  |
| sea_noon | classic | 1.80 | 1.62 |  |  |  |  |  |  | 322.50 | OK |
| sea_noon | remastered | 2.43 | 1.39 | 0.00 | 0.78 | 0.52 | 0.10 |  | 0.16 | 218.40 | OK |
| sea_noon | delta | +0.63 | -0.23 |  |  |  |  |  |  |  |  |
| sea_away_from_sun | classic | 1.64 | 1.43 |  |  |  |  |  |  | 324.40 | OK |
| sea_away_from_sun | remastered | 2.58 | 1.50 | 0.00 | 0.86 | 0.58 | 0.15 |  | 0.13 | 276.20 | OK |
| sea_away_from_sun | delta | +0.94 | +0.07 |  |  |  |  |  |  |  |  |
| coast_shore_midsun | classic | 1.13 | 1.05 |  |  |  |  |  |  | 341.50 | OK |
| coast_shore_midsun | remastered | 2.84 | 1.45 | 0.00 | 1.32 | 0.48 | 0.13 | 0.57 | 0.14 | 266.00 | OK |
| coast_shore_midsun | delta | +1.71 | +0.40 |  |  |  |  |  |  |  |  |
