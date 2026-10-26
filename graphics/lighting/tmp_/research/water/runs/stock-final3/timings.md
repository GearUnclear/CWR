# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| coastline_sunset | classic | 0.55 | 0.46 |  |  |  |  |  |  | 433.20 | OK |
| coastline_sunset | remastered | 2.57 | 1.11 | 0.00 | 1.26 | 0.48 | 0.14 | 0.53 | 0.11 | 364.20 | OK |
| coastline_sunset | delta | +2.02 | +0.65 |  |  |  |  |  |  |  |  |
| sea_noon | classic | 2.86 | 2.49 |  |  |  |  |  |  | 220.10 | OK |
| sea_noon | remastered | 5.11 | 3.27 | 0.00 | 1.45 | 0.92 | 0.32 |  | 0.21 | 156.00 | OK |
| sea_noon | delta | +2.25 | +0.78 |  |  |  |  |  |  |  |  |
| sea_away_from_sun | classic | 3.36 | 2.69 |  |  |  |  |  |  | 201.50 | OK |
| sea_away_from_sun | remastered | 5.12 | 2.87 | 0.00 | 1.69 | 1.06 | 0.39 |  | 0.25 | 161.20 | OK |
| sea_away_from_sun | delta | +1.76 | +0.18 |  |  |  |  |  |  |  |  |
| coast_shore_midsun | classic | 2.42 | 2.27 |  |  |  |  |  |  | 231.90 | OK |
| coast_shore_midsun | remastered | 5.49 | 2.93 | 0.00 | 2.44 | 0.88 | 0.24 | 1.11 | 0.21 | 172.00 | OK |
| coast_shore_midsun | delta | +3.07 | +0.66 |  |  |  |  |  |  |  |  |
