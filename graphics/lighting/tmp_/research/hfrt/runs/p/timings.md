# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| sunrise_ridge | classic | 4.05 | 3.92 |  |  |  |  |  |  | 122.50 | OK |
| sunrise_ridge | remastered | missing |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 5.27 | 5.14 |  |  |  |  |  |  | 217.50 | OK |
| forest_edge_golden | remastered | missing |  |  |  |  |  |  |  |  |  |
| hill_0700 | classic | 35.44 | 35.37 |  |  |  |  |  |  | 21.90 | OK |
| hill_0700 | remastered | missing |  |  |  |  |  |  |  |  |  |
| hill_1730 | classic | 15.37 | 15.29 |  |  |  |  |  |  | 60.50 | OK |
| hill_1730 | remastered | missing |  |  |  |  |  |  |  |  |  |
| overcast_broken | classic | 24.82 | 24.71 |  |  |  |  |  |  | 30.40 | OK |
| overcast_broken | remastered | missing |  |  |  |  |  |  |  |  |  |
