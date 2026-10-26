# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 3.74 | 3.63 |  |  |  |  |  |  | 155.10 | OK |
| lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |
| noon_fields | classic | 22.46 | 22.35 |  |  |  |  |  |  | 26.00 | OK |
| noon_fields | remastered | missing |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 6.98 | 6.82 |  |  |  |  |  |  | 195.60 | OK |
| forest_edge_golden | remastered | missing |  |  |  |  |  |  |  |  |  |
