# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| forest_edge_golden | classic | 2.60 | 2.50 |  |  |  |  |  |  | 269.80 | OK |
| forest_edge_golden | remastered | 4.67 | 4.38 | 0.03 | 0.19 | 0.13 | 0.04 |  | 0.01 | 136.30 | OK |
| forest_edge_golden | delta | +2.07 | +1.88 |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 2.94 | 2.85 |  |  |  |  |  |  | 151.70 | OK |
| sinai_lowsun_fp | remastered | 7.38 | 6.66 | 0.05 | 0.62 | 0.50 | 0.04 | 0.07 | 0.01 | 105.80 | OK |
| sinai_lowsun_fp | delta | +4.44 | +3.81 |  |  |  |  |  |  |  |  |
