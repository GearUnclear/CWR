# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| forest_edge_golden | classic | 11.02 | 10.89 |  |  |  |  |  |  | 120.80 | OK |
| forest_edge_golden | remastered | 7.87 | 7.31 | 0.44 | 0.42 | 0.34 | 0.05 |  | 0.03 | 87.40 | OK |
| forest_edge_golden | delta | -3.15 | -3.58 |  |  |  |  |  |  |  |  |
| noon_fields | classic | 22.66 | 22.54 |  |  |  |  |  |  | 39.80 | OK |
| noon_fields | remastered | 18.64 | 18.12 | 0.09 | 0.41 | 0.34 | 0.05 |  | 0.03 | 38.60 | OK |
| noon_fields | delta | -4.02 | -4.42 |  |  |  |  |  |  |  |  |
| town_night | classic | 9.40 | 9.25 |  |  |  |  |  |  | 72.40 | OK |
| town_night | remastered | 10.34 | 9.81 |  | 0.36 | 0.28 | 0.05 |  | 0.03 | 87.60 | OK |
| town_night | delta | +0.94 | +0.56 |  |  |  |  |  |  |  |  |
| town_day | classic | 3.33 | 3.20 |  |  |  |  |  |  | 144.80 | OK |
| town_day | remastered | 4.95 | 4.45 | 0.14 | 0.36 | 0.28 | 0.05 |  | 0.03 | 100.90 | OK |
| town_day | delta | +1.62 | +1.25 |  |  |  |  |  |  |  |  |
| backlit_hornbeam | classic | missing |  |  |  |  |  |  |  |  |  |
| backlit_hornbeam | remastered | 12.68 | 12.11 | 0.79 | 0.45 | 0.22 | 0.05 | 0.15 | 0.03 | 60.30 | OK |
| pine_stand_noon | classic | missing |  |  |  |  |  |  |  |  |  |
| pine_stand_noon | remastered | 10.93 | 9.96 | 0.18 | 0.81 | 0.73 | 0.05 |  | 0.03 | 55.90 | OK |
