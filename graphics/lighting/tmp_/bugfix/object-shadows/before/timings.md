# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| forest_edge_golden | classic | 5.59 | 5.47 |  |  |  |  |  |  | 179.60 | OK |
| forest_edge_golden | remastered | 6.15 | 5.72 | 0.03 | 0.29 | 0.22 | 0.05 |  | 0.03 | 111.60 | OK |
| forest_edge_golden | delta | +0.56 | +0.25 |  |  |  |  |  |  |  |  |
| noon_fields | classic | 21.18 | 21.08 |  |  |  |  |  |  | 37.30 | OK |
| noon_fields | remastered | 21.38 | 20.98 | 0.11 | 0.28 | 0.21 | 0.05 |  | 0.03 | 37.90 | OK |
| noon_fields | delta | +0.20 | -0.10 |  |  |  |  |  |  |  |  |
| town_night | classic | 13.81 | 13.62 |  |  |  |  |  |  | 73.40 | OK |
| town_night | remastered | 7.06 | 6.14 |  | 0.61 | 0.54 | 0.05 |  | 0.03 | 95.80 | OK |
| town_night | delta | -6.75 | -7.48 |  |  |  |  |  |  |  |  |
| town_day | classic | 3.48 | 3.35 |  |  |  |  |  |  | 155.90 | OK |
| town_day | remastered | 4.45 | 3.85 | 0.03 | 0.45 | 0.37 | 0.05 |  | 0.03 | 110.30 | OK |
| town_day | delta | +0.97 | +0.50 |  |  |  |  |  |  |  |  |
| backlit_hornbeam | classic | missing |  |  |  |  |  |  |  |  |  |
| backlit_hornbeam | remastered | 6.72 | 6.02 | 0.00 | 0.59 | 0.36 | 0.05 | 0.15 | 0.03 | 85.30 | OK |
| pine_stand_noon | classic | missing |  |  |  |  |  |  |  |  |  |
| pine_stand_noon | remastered | 6.37 | 5.84 | 0.00 | 0.40 | 0.32 | 0.05 |  | 0.03 | 93.20 | OK |
