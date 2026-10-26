# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| coastline_sunset | classic | 1.06 | 0.97 |  |  |  |  |  |  | 390.50 | OK |
| coastline_sunset | remastered | 3.20 | 2.70 | 0.00 | 0.40 | 0.16 | 0.05 | 0.16 | 0.03 | 247.80 | OK |
| coastline_sunset | delta | +2.14 | +1.73 |  |  |  |  |  |  |  |  |
| sea_noon | classic | 1.60 | 1.52 |  |  |  |  |  |  | 341.50 | OK |
| sea_noon | remastered | 3.41 | 3.09 | 0.00 | 0.24 | 0.16 | 0.05 |  | 0.03 | 210.20 | OK |
| sea_noon | delta | +1.81 | +1.57 |  |  |  |  |  |  |  |  |
| sea_away_from_sun | classic | 1.63 | 1.54 |  |  |  |  |  |  | 313.80 | OK |
| sea_away_from_sun | remastered | 8.21 | 6.30 | 0.00 | 1.81 | 1.74 | 0.05 |  | 0.03 | 53.90 | OK |
| sea_away_from_sun | delta | +6.58 | +4.76 |  |  |  |  |  |  |  |  |
| coast_shore_midsun | classic | 1.08 | 1.04 |  |  |  |  |  |  | 378.70 | OK |
| coast_shore_midsun | remastered | 2.35 | 1.54 | 0.00 | 0.77 | 0.30 | 0.05 | 0.20 | 0.22 | 258.90 | OK |
| coast_shore_midsun | delta | +1.27 | +0.50 |  |  |  |  |  |  |  |  |
