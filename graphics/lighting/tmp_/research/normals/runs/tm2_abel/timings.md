# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 4.38 | 4.27 |  |  |  |  |  |  | 144.00 | OK |
| lowsun_fp | remastered | 8.58 | 7.49 | 0.04 | 0.98 | 0.22 | 0.05 | 0.22 | 0.50 | 91.40 | OK |
| lowsun_fp | delta | +4.20 | +3.22 |  |  |  |  |  |  |  |  |
| hull_close | classic | 8.17 | 8.06 |  |  |  |  |  |  | 86.40 | OK |
| hull_close | remastered | 13.34 | 11.79 | 0.04 | 1.44 | 0.25 | 0.05 | 0.24 | 0.91 | 65.20 | OK |
| hull_close | delta | +5.17 | +3.73 |  |  |  |  |  |  |  |  |
| noon_fields | classic | 26.87 | 26.76 |  |  |  |  |  |  | 32.30 | OK |
| noon_fields | remastered | 34.75 | 34.40 | 0.09 | 0.24 | 0.17 | 0.05 |  | 0.03 | 26.00 | OK |
| noon_fields | delta | +7.88 | +7.64 |  |  |  |  |  |  |  |  |
