# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| forest_edge_golden | classic | 1.71 | 1.62 |  |  |  |  |  |  | 322.60 | OK |
| forest_edge_golden | remastered | 3.78 | 3.57 | 0.03 | 0.12 | 0.07 | 0.04 |  | 0.01 | 165.50 | OK |
| forest_edge_golden | delta | +2.07 | +1.95 |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 3.46 | 3.37 |  |  |  |  |  |  | 148.80 | OK |
| sinai_lowsun_fp | remastered | 9.46 | 9.17 | 0.43 | 0.20 | 0.08 | 0.04 | 0.07 | 0.01 | 100.50 | OK |
| sinai_lowsun_fp | delta | +6.00 | +5.80 |  |  |  |  |  |  |  |  |
