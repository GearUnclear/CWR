# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 5.48 | 5.37 |  |  |  |  |  |  | 110.60 | OK |
| lowsun_fp | remastered | 11.29 | 9.94 | 0.36 | 1.24 | 0.39 | 0.05 | 0.22 | 0.58 | 78.40 | OK |
| lowsun_fp | delta | +5.81 | +4.57 |  |  |  |  |  |  |  |  |
| town_golden | classic | 4.37 | 4.25 |  |  |  |  |  |  | 130.30 | OK |
| town_golden | remastered | 6.80 | 5.29 | 0.04 | 1.38 | 0.29 | 0.05 | 0.25 | 0.79 | 118.90 | OK |
| town_golden | delta | +2.43 | +1.04 |  |  |  |  |  |  |  |  |
| noon_fields | classic | 26.61 | 26.50 |  |  |  |  |  |  | 32.30 | OK |
| noon_fields | remastered | 31.93 | 31.58 | 0.21 | 0.24 | 0.16 | 0.05 |  | 0.03 | 26.30 | OK |
| noon_fields | delta | +5.32 | +5.08 |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 9.15 | 9.03 |  |  |  |  |  |  | 197.70 | OK |
| forest_edge_golden | remastered | 13.69 | 13.31 | 0.03 | 0.25 | 0.17 | 0.05 |  | 0.03 | 60.30 | OK |
| forest_edge_golden | delta | +4.54 | +4.28 |  |  |  |  |  |  |  |  |
| town_night | classic | 12.36 | 12.22 |  |  |  |  |  |  | 71.30 | OK |
| town_night | remastered | 13.71 | 13.25 |  | 0.31 | 0.23 | 0.05 |  | 0.03 | 59.60 | OK |
| town_night | delta | +1.35 | +1.03 |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 5.37 | 5.26 |  |  |  |  |  |  | 95.00 | OK |
| sinai_lowsun_fp | remastered | 9.56 | 7.96 | 0.20 | 1.49 | 0.68 | 0.05 | 0.23 | 0.53 | 81.00 | OK |
| sinai_lowsun_fp | delta | +4.19 | +2.70 |  |  |  |  |  |  |  |  |
