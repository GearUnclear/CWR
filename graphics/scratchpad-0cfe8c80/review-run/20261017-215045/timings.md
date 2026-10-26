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
| smoke_1 | classic |  |  |  |  |  |  |  |  | 217.50 | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 4.40 | 3.36 | 0.06 | 0.92 | 0.42 | 0.09 | 0.36 | 0.06 | 181.00 | OK |
| smoke_3 | classic | 2.65 | 2.43 | 0.06 | 0.91 | 0.40 | 0.09 | 0.37 | 0.06 | 159.80 | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 4.55 | 3.44 | 0.05 | 0.88 | 0.44 | 0.08 | 0.31 | 0.05 | 144.80 | OK |
