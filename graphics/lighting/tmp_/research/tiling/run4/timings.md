# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| noon_fields | classic | 2.42 | 2.31 |  |  |  |  |  |  | 278.90 | OK |
| noon_fields | remastered | missing |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 8.18 | 7.99 |  |  |  |  |  |  | 227.70 | OK |
| forest_edge_golden | remastered | missing |  |  |  |  |  |  |  |  |  |
| lowsun_fp | classic | 14.25 | 14.14 |  |  |  |  |  |  | 58.80 | OK |
| lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |
| overcast_broken | classic | 11.26 | 11.15 |  |  |  |  |  |  | 43.10 | OK |
| overcast_broken | remastered | missing |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 3.00 | 2.88 |  |  |  |  |  |  | 143.60 | OK |
| sinai_desert_noon | remastered | missing |  |  |  |  |  |  |  |  |  |
| high_sinai_noon | classic | 13.97 | 13.88 |  |  |  |  |  |  | 56.10 | OK |
| high_sinai_noon | remastered | missing |  |  |  |  |  |  |  |  |  |
