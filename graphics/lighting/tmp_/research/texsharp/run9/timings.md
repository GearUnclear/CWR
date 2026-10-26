# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| noon_fields | classic | 3.19 | 3.09 |  |  |  |  |  |  | 245.20 | OK |
| noon_fields | remastered | 31.81 | 31.45 | 0.04 | 0.24 | 0.17 | 0.05 |  | 0.03 | 38.40 | OK |
| noon_fields | delta | +28.62 | +28.36 |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 8.17 | 8.05 |  |  |  |  |  |  | 213.30 | OK |
| forest_edge_golden | remastered | 13.01 | 12.63 | 0.03 | 0.25 | 0.18 | 0.05 |  | 0.03 | 66.50 | OK |
| forest_edge_golden | delta | +4.84 | +4.58 |  |  |  |  |  |  |  |  |
| lowsun_fp | classic | 5.55 | 5.44 |  |  |  |  |  |  | 116.40 | OK |
| lowsun_fp | remastered | 9.38 | 7.93 | 0.06 | 1.33 | 0.20 | 0.05 | 0.22 | 0.86 | 88.90 | OK |
| lowsun_fp | delta | +3.83 | +2.49 |  |  |  |  |  |  |  |  |
