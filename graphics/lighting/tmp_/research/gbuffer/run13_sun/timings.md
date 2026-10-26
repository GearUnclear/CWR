# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| noon_fields | classic | missing |  |  |  |  |  |  |  |  |  |
| noon_fields | remastered | missing |  |  |  |  |  |  |  |  |  |
| lowsun_fp | classic | missing |  |  |  |  |  |  |  |  |  |
| lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |
| town_day | classic | missing |  |  |  |  |  |  |  |  |  |
| town_day | remastered | missing |  |  |  |  |  |  |  |  |  |
