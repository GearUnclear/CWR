# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| forest_edge_golden | classic | 8.96 | 8.83 |  |  |  |  |  |  | 141.60 | OK |
| forest_edge_golden | remastered | 8.85 | 8.41 | 0.19 | 0.29 | 0.21 | 0.05 |  | 0.03 | 95.30 | OK |
| forest_edge_golden | delta | -0.11 | -0.42 |  |  |  |  |  |  |  |  |
| noon_fields | classic | 18.92 | 18.81 |  |  |  |  |  |  | 43.30 | OK |
| noon_fields | remastered | 25.25 | 24.86 | 0.60 | 0.28 | 0.20 | 0.05 |  | 0.03 | 36.30 | OK |
| noon_fields | delta | +6.33 | +6.05 |  |  |  |  |  |  |  |  |
| town_night | classic | 11.08 | 10.92 |  |  |  |  |  |  | 83.10 | OK |
| town_night | remastered | 9.10 | 8.67 |  | 0.27 | 0.20 | 0.05 |  | 0.03 | 90.30 | OK |
| town_night | delta | -1.98 | -2.25 |  |  |  |  |  |  |  |  |
| town_day | classic | 4.35 | 4.22 |  |  |  |  |  |  | 137.40 | OK |
| town_day | remastered | 7.91 | 7.50 | 0.14 | 0.28 | 0.20 | 0.05 |  | 0.03 | 84.90 | OK |
| town_day | delta | +3.56 | +3.28 |  |  |  |  |  |  |  |  |
| backlit_hornbeam | classic | missing |  |  |  |  |  |  |  |  |  |
| backlit_hornbeam | remastered | 8.60 | 8.02 | 0.49 | 0.46 | 0.23 | 0.05 | 0.15 | 0.03 | 77.30 | OK |
| pine_stand_noon | classic | missing |  |  |  |  |  |  |  |  |  |
| pine_stand_noon | remastered | 8.04 | 7.58 | 0.25 | 0.36 | 0.28 | 0.05 |  | 0.03 | 94.70 | OK |
