# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| coastline_sunset | classic | 1.54 | 1.38 |  |  |  |  |  |  | 236.20 | OK |
| coastline_sunset | remastered | 2.44 | 1.06 | 0.00 | 1.23 | 0.64 | 0.09 | 0.29 | 0.20 | 223.60 | OK |
| coastline_sunset | delta | +0.90 | -0.32 |  |  |  |  |  |  |  |  |
| sea_noon | classic | 4.33 | 4.20 |  |  |  |  |  |  | 113.40 | OK |
| sea_noon | remastered | 3.11 | 1.84 | 0.00 | 1.13 | 0.95 | 0.10 |  | 0.09 | 137.60 | OK |
| sea_noon | delta | -1.22 | -2.36 |  |  |  |  |  |  |  |  |
| sea_away_from_sun | classic | 1.59 | 1.43 |  |  |  |  |  |  | 185.50 | OK |
| sea_away_from_sun | remastered | 2.39 | 1.25 | 0.00 | 0.95 | 0.79 | 0.10 |  | 0.06 | 164.00 | OK |
| sea_away_from_sun | delta | +0.80 | -0.18 |  |  |  |  |  |  |  |  |
| coast_shore_midsun | classic | 1.06 | 0.98 |  |  |  |  |  |  | 197.90 | OK |
| coast_shore_midsun | remastered | 2.69 | 1.36 | 0.00 | 1.27 | 0.69 | 0.08 | 0.32 | 0.18 | 193.30 | OK |
| coast_shore_midsun | delta | +1.63 | +0.38 |  |  |  |  |  |  |  |  |
