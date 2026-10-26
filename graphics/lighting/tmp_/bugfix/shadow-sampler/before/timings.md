# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| paths_1 | classic | missing |  |  |  |  |  |  |  |  |  |
| paths_1 | remastered | missing |  |  |  |  |  |  |  |  |  |
| paths_2 | classic | missing |  |  |  |  |  |  |  |  |  |
| paths_2 | remastered | missing |  |  |  |  |  |  |  |  |  |
| paths_3 | classic | missing |  |  |  |  |  |  |  |  |  |
| paths_3 | remastered | missing |  |  |  |  |  |  |  |  |  |
| paths_4 | classic | missing |  |  |  |  |  |  |  |  |  |
| paths_4 | remastered | missing |  |  |  |  |  |  |  |  |  |
| smoke_1 | classic |  |  |  |  |  |  |  |  | 113.50 | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 5.48 | 4.48 | 0.03 | 0.94 | 0.50 | 0.05 | 0.22 | 0.17 | 99.60 | OK |
| smoke_3 | classic | 13.83 | 13.67 | 0.03 | 1.04 | 0.54 | 0.05 | 0.22 | 0.23 | 74.70 | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 11.56 | 10.43 | 0.04 | 1.07 | 0.57 | 0.05 | 0.22 | 0.23 | 66.40 | OK |
