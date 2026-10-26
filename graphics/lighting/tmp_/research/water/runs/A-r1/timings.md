# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| coastline_sunset | classic | 1.19 | 1.04 |  |  |  |  |  |  | 253.80 | OK |
| coastline_sunset | remastered | 2.25 | 0.96 | 0.00 | 1.13 | 0.66 | 0.09 | 0.29 | 0.09 | 224.70 | OK |
| coastline_sunset | delta | +1.06 | -0.08 |  |  |  |  |  |  |  |  |
| sea_noon | classic | 4.67 | 4.54 |  |  |  |  |  |  | 117.50 | OK |
| sea_noon | remastered | 3.55 | 2.42 | 0.00 | 0.99 | 0.82 | 0.10 |  | 0.07 | 115.50 | OK |
| sea_noon | delta | -1.12 | -2.12 |  |  |  |  |  |  |  |  |
| sea_away_from_sun | classic | 3.51 | 3.35 |  |  |  |  |  |  | 154.00 | OK |
| sea_away_from_sun | remastered | 2.25 | 1.15 | 0.00 | 0.94 | 0.78 | 0.10 |  | 0.06 | 166.50 | OK |
| sea_away_from_sun | delta | -1.26 | -2.20 |  |  |  |  |  |  |  |  |
| coast_shore_midsun | classic | 1.92 | 1.85 |  |  |  |  |  |  | 187.30 | OK |
| coast_shore_midsun | remastered | 3.19 | 1.74 | 0.00 | 1.38 | 0.80 | 0.08 | 0.32 | 0.18 | 201.30 | OK |
| coast_shore_midsun | delta | +1.27 | -0.11 |  |  |  |  |  |  |  |  |
