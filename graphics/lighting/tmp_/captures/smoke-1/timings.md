# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| smoke_1 | classic |  |  |  |  |  |  |  |  | 142.20 | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 3.59 | 2.24 | 0.13 | 1.20 | 0.57 | 0.10 | 0.44 | 0.08 | 135.20 | OK |
| smoke_3 | classic | 2.36 | 2.07 | 0.10 | 1.39 | 0.63 | 0.10 | 0.45 | 0.21 | 129.40 | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 4.46 | 2.78 | 0.09 | 1.53 | 0.66 | 0.12 | 0.56 | 0.19 | 138.50 | OK |
