# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| smoke_1 | classic |  |  |  |  |  |  |  |  | 170.60 | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 4.50 | 3.42 | 0.06 | 0.96 | 0.38 | 0.08 | 0.35 | 0.16 | 160.50 | OK |
| smoke_3 | classic | 2.98 | 2.77 | 0.06 | 0.92 | 0.39 | 0.08 | 0.35 | 0.10 | 154.30 | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 5.40 | 4.22 | 0.06 | 1.05 | 0.39 | 0.10 | 0.42 | 0.15 | 154.50 | OK |
