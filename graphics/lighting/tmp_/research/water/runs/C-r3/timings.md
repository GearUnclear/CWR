# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| coastline_sunset | classic | 2.16 | 2.06 |  |  |  |  |  |  | 153.30 | OK |
| coastline_sunset | remastered | 5.18 | 3.88 | 0.00 | 1.11 | 0.87 | 0.05 | 0.16 | 0.03 | 111.70 | OK |
| coastline_sunset | delta | +3.02 | +1.82 |  |  |  |  |  |  |  |  |
| sea_noon | classic | 4.86 | 4.77 |  |  |  |  |  |  | 87.60 | OK |
| sea_noon | remastered | 6.21 | 4.30 | 0.00 | 1.83 | 1.75 | 0.05 |  | 0.03 | 102.70 | OK |
| sea_noon | delta | +1.35 | -0.47 |  |  |  |  |  |  |  |  |
| sea_away_from_sun | classic | 4.57 | 4.47 |  |  |  |  |  |  | 87.60 | OK |
| sea_away_from_sun | remastered | 5.78 | 4.75 | 0.00 | 0.93 | 0.86 | 0.05 |  | 0.03 | 72.70 | OK |
| sea_away_from_sun | delta | +1.21 | +0.28 |  |  |  |  |  |  |  |  |
| coast_shore_midsun | classic | 8.14 | 8.09 |  |  |  |  |  |  | 113.40 | OK |
| coast_shore_midsun | remastered | 8.19 | 6.23 | 0.00 | 1.90 | 1.57 | 0.05 | 0.20 | 0.07 | 96.70 | OK |
| coast_shore_midsun | delta | +0.05 | -1.86 |  |  |  |  |  |  |  |  |
