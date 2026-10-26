# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| noon_fields | classic | 1.86 | 1.76 |  |  |  |  |  |  | 256.30 | OK |
| noon_fields | remastered | missing |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 6.59 | 6.46 |  |  |  |  |  |  | 221.90 | OK |
| forest_edge_golden | remastered | missing |  |  |  |  |  |  |  |  |  |
| lowsun_fp | classic | 14.01 | 13.90 |  |  |  |  |  |  | 62.30 | OK |
| lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |
| high_fields_noon | classic | 30.10 | 30.01 |  |  |  |  |  |  | 28.10 | OK |
| high_fields_noon | remastered | missing |  |  |  |  |  |  |  |  |  |
| high_fields_eve | classic | 23.59 | 23.51 |  |  |  |  |  |  | 31.20 | OK |
| high_fields_eve | remastered | missing |  |  |  |  |  |  |  |  |  |
| overcast_broken | classic | 19.17 | 19.06 |  |  |  |  |  |  | 35.00 | OK |
| overcast_broken | remastered | missing |  |  |  |  |  |  |  |  |  |
| high_valley_eden | classic | 24.63 | 24.54 |  |  |  |  |  |  | 31.50 | OK |
| high_valley_eden | remastered | missing |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 4.02 | 3.91 |  |  |  |  |  |  | 120.70 | OK |
| sinai_desert_noon | remastered | missing |  |  |  |  |  |  |  |  |  |
