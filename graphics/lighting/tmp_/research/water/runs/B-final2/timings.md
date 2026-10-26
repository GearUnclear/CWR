# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| coastline_sunset | classic | 2.31 | 1.96 |  |  |  |  |  |  | 239.00 | OK |
| coastline_sunset | remastered | 1.24 | 0.52 | 0.00 | 0.63 | 0.37 | 0.05 | 0.18 | 0.03 | 275.50 | OK |
| coastline_sunset | delta | -1.07 | -1.44 |  |  |  |  |  |  |  |  |
| sea_noon | classic | 2.46 | 2.15 |  |  |  |  |  |  | 238.00 | OK |
| sea_noon | remastered | 4.70 | 3.34 | 0.00 | 1.10 | 0.76 | 0.20 |  | 0.14 | 158.70 | OK |
| sea_noon | delta | +2.24 | +1.19 |  |  |  |  |  |  |  |  |
| sea_away_from_sun | classic | 3.72 | 2.81 |  |  |  |  |  |  | 208.10 | OK |
| sea_away_from_sun | remastered | 4.74 | 2.90 | 0.00 | 1.44 | 0.91 | 0.35 |  | 0.18 | 166.10 | OK |
| sea_away_from_sun | delta | +1.02 | +0.09 |  |  |  |  |  |  |  |  |
| coast_shore_midsun | classic | 2.74 | 2.62 |  |  |  |  |  |  | 220.10 | OK |
| coast_shore_midsun | remastered | 5.88 | 3.64 | 0.00 | 2.12 | 1.20 | 0.16 | 0.60 | 0.16 | 166.70 | OK |
| coast_shore_midsun | delta | +3.14 | +1.02 |  |  |  |  |  |  |  |  |
