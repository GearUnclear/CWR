# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| ob | classic | missing |  |  |  |  |  |  |  |  |  |
| ob | remastered | 12.74 | 12.33 | 0.03 | 0.30 | 0.23 | 0.05 |  | 0.03 | 50.80 | OK |
| ob_dbgsun | classic | missing |  |  |  |  |  |  |  |  |  |
| ob_dbgsun | remastered | 12.74 | 12.33 | 0.03 | 0.30 | 0.23 | 0.05 |  | 0.03 | 50.80 | OK |
| high | classic | 18.91 | 18.82 |  |  |  |  |  |  | 42.50 | OK |
| high | remastered | 20.64 | 19.91 | 0.09 | 0.65 | 0.57 | 0.05 |  | 0.03 | 36.30 | OK |
| high | delta | +1.73 | +1.09 |  |  |  |  |  |  |  |  |
| high_dbgsun | classic | missing |  |  |  |  |  |  |  |  |  |
| high_dbgsun | remastered | 20.64 | 19.91 | 0.09 | 0.65 | 0.57 | 0.05 |  | 0.03 | 36.30 | OK |
