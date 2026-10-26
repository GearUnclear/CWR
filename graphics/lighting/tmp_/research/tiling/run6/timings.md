# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| noon_fields | classic | 3.05 | 2.94 |  |  |  |  |  |  | 268.00 | OK |
| noon_fields | remastered | missing |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 9.86 | 9.73 |  |  |  |  |  |  | 228.30 | OK |
| forest_edge_golden | remastered | missing |  |  |  |  |  |  |  |  |  |
| high_fields_noon | classic | 8.22 | 8.14 |  |  |  |  |  |  | 67.80 | OK |
| high_fields_noon | remastered | missing |  |  |  |  |  |  |  |  |  |
| overcast_broken | classic | 32.73 | 32.62 |  |  |  |  |  |  | 26.60 | OK |
| overcast_broken | remastered | missing |  |  |  |  |  |  |  |  |  |
| high_valley_eden | classic | 10.36 | 10.28 |  |  |  |  |  |  | 54.80 | OK |
| high_valley_eden | remastered | missing |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 3.15 | 3.04 |  |  |  |  |  |  | 138.10 | OK |
| sinai_desert_noon | remastered | missing |  |  |  |  |  |  |  |  |  |
| high_sinai_noon | classic | 10.88 | 10.78 |  |  |  |  |  |  | 65.10 | OK |
| high_sinai_noon | remastered | missing |  |  |  |  |  |  |  |  |  |
