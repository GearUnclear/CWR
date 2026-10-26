# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 3.73 | 3.63 |  |  |  |  |  |  | 145.00 | OK |
| lowsun_fp | remastered | 13.37 | 11.70 | 0.35 | 1.56 | 0.41 | 0.05 | 0.22 | 0.88 | 73.50 | OK |
| lowsun_fp | delta | +9.64 | +8.07 |  |  |  |  |  |  |  |  |
| noon_fields | classic | 18.40 | 18.29 |  |  |  |  |  |  | 38.20 | OK |
| noon_fields | remastered | 25.36 | 24.98 | 0.04 | 0.27 | 0.19 | 0.05 |  | 0.03 | 31.50 | OK |
| noon_fields | delta | +6.96 | +6.69 |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 5.15 | 5.02 |  |  |  |  |  |  | 213.20 | OK |
| forest_edge_golden | remastered | 10.61 | 10.20 | 0.03 | 0.27 | 0.20 | 0.05 |  | 0.03 | 66.40 | OK |
| forest_edge_golden | delta | +5.46 | +5.18 |  |  |  |  |  |  |  |  |
| town_night | classic | 6.15 | 5.98 |  |  |  |  |  |  | 109.10 | OK |
| town_night | remastered | 6.66 | 5.91 |  | 0.59 | 0.51 | 0.05 |  | 0.03 | 117.70 | OK |
| town_night | delta | +0.51 | -0.07 |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 2.99 | 2.88 |  |  |  |  |  |  | 131.30 | OK |
| sinai_lowsun_fp | remastered | 4.16 | 3.05 | 0.09 | 0.99 | 0.64 | 0.05 | 0.22 | 0.08 | 112.30 | OK |
| sinai_lowsun_fp | delta | +1.17 | +0.17 |  |  |  |  |  |  |  |  |
