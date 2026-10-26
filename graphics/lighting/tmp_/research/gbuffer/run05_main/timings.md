# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 4.81 | 4.70 |  |  |  |  |  |  | 150.00 | OK |
| lowsun_fp | remastered | 10.13 | 8.59 | 0.31 | 1.43 | 0.23 | 0.05 | 0.22 | 0.93 | 88.50 | OK |
| lowsun_fp | delta | +5.32 | +3.89 |  |  |  |  |  |  |  |  |
| noon_fields | classic | 11.12 | 11.01 |  |  |  |  |  |  | 56.40 | OK |
| noon_fields | remastered | 13.46 | 12.83 | 0.04 | 0.52 | 0.44 | 0.05 |  | 0.03 | 37.60 | OK |
| noon_fields | delta | +2.34 | +1.82 |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 5.26 | 5.13 |  |  |  |  |  |  | 154.90 | OK |
| forest_edge_golden | remastered | 10.90 | 10.42 | 0.03 | 0.35 | 0.26 | 0.05 |  | 0.03 | 71.40 | OK |
| forest_edge_golden | delta | +5.64 | +5.29 |  |  |  |  |  |  |  |  |
| town_day | classic | 3.07 | 2.95 |  |  |  |  |  |  | 138.40 | OK |
| town_day | remastered | 3.29 | 2.58 | 0.04 | 0.58 | 0.50 | 0.05 |  | 0.03 | 127.30 | OK |
| town_day | delta | +0.22 | -0.37 |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 7.25 | 7.14 |  |  |  |  |  |  | 72.60 | OK |
| sinai_desert_noon | remastered | 29.90 | 28.35 | 0.05 | 1.42 | 1.34 | 0.05 |  | 0.03 | 26.20 | OK |
| sinai_desert_noon | delta | +22.65 | +21.21 |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 7.04 | 6.92 |  |  |  |  |  |  | 87.70 | OK |
| sinai_lowsun_fp | remastered | 7.28 | 5.86 | 0.19 | 1.30 | 0.40 | 0.05 | 0.22 | 0.62 | 83.60 | OK |
| sinai_lowsun_fp | delta | +0.24 | -1.06 |  |  |  |  |  |  |  |  |
