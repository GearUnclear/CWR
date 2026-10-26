# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 9.08 | 8.97 |  |  |  |  |  |  | 83.30 | OK |
| lowsun_fp | remastered | 21.55 | 19.29 | 0.46 | 2.15 | 0.18 | 0.05 | 0.22 | 1.70 | 49.80 | OK |
| lowsun_fp | delta | +12.47 | +10.32 |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 4.62 | 4.49 |  |  |  |  |  |  | 112.20 | OK |
| forest_edge_golden | remastered | 12.05 | 11.36 | 0.03 | 0.35 | 0.27 | 0.05 |  | 0.03 | 73.80 | OK |
| forest_edge_golden | delta | +7.43 | +6.87 |  |  |  |  |  |  |  |  |
| town_day | classic | 5.78 | 5.66 |  |  |  |  |  |  | 112.90 | OK |
| town_day | remastered | 8.76 | 8.34 | 0.04 | 0.28 | 0.20 | 0.05 |  | 0.03 | 90.60 | OK |
| town_day | delta | +2.98 | +2.68 |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 4.54 | 4.42 |  |  |  |  |  |  | 99.20 | OK |
| sinai_lowsun_fp | remastered | 9.29 | 7.85 | 0.13 | 1.32 | 0.92 | 0.05 | 0.22 | 0.12 | 75.60 | OK |
| sinai_lowsun_fp | delta | +4.75 | +3.43 |  |  |  |  |  |  |  |  |
