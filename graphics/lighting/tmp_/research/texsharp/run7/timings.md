# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| noon_fields | classic | 2.52 | 2.42 |  |  |  |  |  |  | 210.50 | OK |
| noon_fields | remastered | 21.64 | 21.18 | 0.04 | 0.35 | 0.27 | 0.05 |  | 0.03 | 45.50 | OK |
| noon_fields | delta | +19.12 | +18.76 |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 6.59 | 6.37 |  |  |  |  |  |  | 229.90 | OK |
| forest_edge_golden | remastered | 16.78 | 16.35 | 0.07 | 0.30 | 0.22 | 0.05 |  | 0.03 | 56.70 | OK |
| forest_edge_golden | delta | +10.19 | +9.98 |  |  |  |  |  |  |  |  |
| lowsun_fp | classic | 5.83 | 5.71 |  |  |  |  |  |  | 104.60 | OK |
| lowsun_fp | remastered | 8.38 | 7.27 | 0.06 | 0.99 | 0.19 | 0.05 | 0.22 | 0.53 | 70.50 | OK |
| lowsun_fp | delta | +2.55 | +1.56 |  |  |  |  |  |  |  |  |
