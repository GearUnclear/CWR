# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| noon_fields | classic | 0.99 | 0.89 |  |  |  |  |  |  | 276.20 | OK |
| noon_fields | remastered | missing |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 1.36 | 1.23 |  |  |  |  |  |  | 183.20 | OK |
| forest_edge_golden | remastered | missing |  |  |  |  |  |  |  |  |  |
| lowsun_fp | classic | 5.85 | 5.73 |  |  |  |  |  |  | 125.90 | OK |
| lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |
| high_fields_noon | classic | 2.45 | 2.37 |  |  |  |  |  |  | 120.10 | OK |
| high_fields_noon | remastered | missing |  |  |  |  |  |  |  |  |  |
| overcast_broken | classic | 21.32 | 21.22 |  |  |  |  |  |  | 36.10 | OK |
| overcast_broken | remastered | missing |  |  |  |  |  |  |  |  |  |
| high_valley_eden | classic | 26.63 | 26.54 |  |  |  |  |  |  | 29.80 | OK |
| high_valley_eden | remastered | missing |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 2.99 | 2.88 |  |  |  |  |  |  | 149.00 | OK |
| sinai_desert_noon | remastered | missing |  |  |  |  |  |  |  |  |  |
| high_sinai_noon | classic | 7.75 | 7.66 |  |  |  |  |  |  | 77.20 | OK |
| high_sinai_noon | remastered | missing |  |  |  |  |  |  |  |  |  |
| noon_fields__r0 | classic | missing |  |  |  |  |  |  |  |  |  |
| noon_fields__r0 | remastered | missing |  |  |  |  |  |  |  |  |  |
| noon_fields__r1 | classic | missing |  |  |  |  |  |  |  |  |  |
| noon_fields__r1 | remastered | missing |  |  |  |  |  |  |  |  |  |
| noon_fields__r2 | classic | missing |  |  |  |  |  |  |  |  |  |
| noon_fields__r2 | remastered | missing |  |  |  |  |  |  |  |  |  |
| noon_fields__r3 | classic | missing |  |  |  |  |  |  |  |  |  |
| noon_fields__r3 | remastered | missing |  |  |  |  |  |  |  |  |  |
| high_fields_noon__r0 | classic | missing |  |  |  |  |  |  |  |  |  |
| high_fields_noon__r0 | remastered | missing |  |  |  |  |  |  |  |  |  |
| high_fields_noon__r1 | classic | missing |  |  |  |  |  |  |  |  |  |
| high_fields_noon__r1 | remastered | missing |  |  |  |  |  |  |  |  |  |
| high_fields_noon__r2 | classic | missing |  |  |  |  |  |  |  |  |  |
| high_fields_noon__r2 | remastered | missing |  |  |  |  |  |  |  |  |  |
| high_fields_noon__r3 | classic | missing |  |  |  |  |  |  |  |  |  |
| high_fields_noon__r3 | remastered | missing |  |  |  |  |  |  |  |  |  |
| forest_edge_golden__r0 | classic | missing |  |  |  |  |  |  |  |  |  |
| forest_edge_golden__r0 | remastered | missing |  |  |  |  |  |  |  |  |  |
| forest_edge_golden__r1 | classic | missing |  |  |  |  |  |  |  |  |  |
| forest_edge_golden__r1 | remastered | missing |  |  |  |  |  |  |  |  |  |
| forest_edge_golden__r2 | classic | missing |  |  |  |  |  |  |  |  |  |
| forest_edge_golden__r2 | remastered | missing |  |  |  |  |  |  |  |  |  |
| forest_edge_golden__r3 | classic | missing |  |  |  |  |  |  |  |  |  |
| forest_edge_golden__r3 | remastered | missing |  |  |  |  |  |  |  |  |  |
