# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| noon_fields | classic | 1.86 | 1.75 |  |  |  |  |  |  | 237.10 | OK |
| noon_fields | remastered | missing |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 8.42 | 8.29 |  |  |  |  |  |  | 222.60 | OK |
| forest_edge_golden | remastered | missing |  |  |  |  |  |  |  |  |  |
| lowsun_fp | classic | 11.42 | 11.31 |  |  |  |  |  |  | 67.20 | OK |
| lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |
| high_fields_noon | classic | 18.22 | 18.13 |  |  |  |  |  |  | 42.90 | OK |
| high_fields_noon | remastered | missing |  |  |  |  |  |  |  |  |  |
| overcast_broken | classic | 10.96 | 10.85 |  |  |  |  |  |  | 55.40 | OK |
| overcast_broken | remastered | missing |  |  |  |  |  |  |  |  |  |
| high_valley_eden | classic | 10.97 | 10.89 |  |  |  |  |  |  | 59.90 | OK |
| high_valley_eden | remastered | missing |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 2.26 | 2.15 |  |  |  |  |  |  | 163.00 | OK |
| sinai_desert_noon | remastered | missing |  |  |  |  |  |  |  |  |  |
