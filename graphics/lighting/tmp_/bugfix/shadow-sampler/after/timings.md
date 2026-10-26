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
| smoke_1 | classic |  |  |  |  |  |  |  |  | 128.80 | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 7.09 | 6.05 | 0.03 | 0.98 | 0.51 | 0.05 | 0.22 | 0.20 | 108.60 | OK |
| smoke_3 | classic | 3.82 | 3.65 | 0.03 | 1.39 | 0.85 | 0.05 | 0.22 | 0.27 | 97.90 | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 5.95 | 4.70 | 0.03 | 1.21 | 0.38 | 0.05 | 0.22 | 0.56 | 92.70 | OK |
