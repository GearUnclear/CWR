# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| coastline_sunset | classic | 2.28 | 1.90 |  |  |  |  |  |  | 231.30 | OK |
| coastline_sunset | remastered | 5.31 | 2.92 | 0.00 | 2.07 | 0.96 | 0.18 | 0.57 | 0.36 | 168.30 | OK |
| coastline_sunset | delta | +3.03 | +1.02 |  |  |  |  |  |  |  |  |
| sea_noon | classic | 2.59 | 2.26 |  |  |  |  |  |  | 213.50 | OK |
| sea_noon | remastered | 6.13 | 4.46 | 0.00 | 1.33 | 0.94 | 0.23 |  | 0.16 | 141.60 | OK |
| sea_noon | delta | +3.54 | +2.20 |  |  |  |  |  |  |  |  |
| sea_away_from_sun | classic | 2.94 | 2.48 |  |  |  |  |  |  | 208.50 | OK |
| sea_away_from_sun | remastered | 3.63 | 2.37 | 0.00 | 0.96 | 0.66 | 0.17 |  | 0.13 | 190.30 | OK |
| sea_away_from_sun | delta | +0.69 | -0.11 |  |  |  |  |  |  |  |  |
| coast_shore_midsun | classic | 2.25 | 2.10 |  |  |  |  |  |  | 256.10 | OK |
| coast_shore_midsun | remastered | 1.55 | 0.85 | 0.00 | 0.66 | 0.36 | 0.05 | 0.22 | 0.03 | 238.50 | OK |
| coast_shore_midsun | delta | -0.70 | -1.25 |  |  |  |  |  |  |  |  |
