# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 4.35 | 4.24 |  |  |  |  |  |  | 140.70 | OK |
| lowsun_fp | remastered | 8.26 | 7.19 | 0.07 | 0.95 | 0.29 | 0.05 | 0.22 | 0.39 | 98.50 | OK |
| lowsun_fp | delta | +3.91 | +2.95 |  |  |  |  |  |  |  |  |
| noon_fields | classic | 20.22 | 20.11 |  |  |  |  |  |  | 39.90 | OK |
| noon_fields | remastered | 11.78 | 11.18 | 0.04 | 0.49 | 0.41 | 0.05 |  | 0.03 | 58.70 | OK |
| noon_fields | delta | -8.44 | -8.93 |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 4.46 | 4.33 |  |  |  |  |  |  | 152.80 | OK |
| forest_edge_golden | remastered | 11.26 | 10.84 | 0.03 | 0.29 | 0.21 | 0.05 |  | 0.03 | 80.30 | OK |
| forest_edge_golden | delta | +6.80 | +6.51 |  |  |  |  |  |  |  |  |
| town_day | classic | 3.55 | 3.43 |  |  |  |  |  |  | 130.90 | OK |
| town_day | remastered | 4.87 | 4.34 | 0.04 | 0.39 | 0.31 | 0.05 |  | 0.03 | 112.60 | OK |
| town_day | delta | +1.32 | +0.91 |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 6.73 | 6.61 |  |  |  |  |  |  | 89.70 | OK |
| sinai_desert_noon | remastered | 29.89 | 29.28 | 0.05 | 0.48 | 0.40 | 0.05 |  | 0.03 | 31.40 | OK |
| sinai_desert_noon | delta | +23.16 | +22.67 |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 6.34 | 6.22 |  |  |  |  |  |  | 100.80 | OK |
| sinai_lowsun_fp | remastered | 8.11 | 6.54 | 0.11 | 1.45 | 0.27 | 0.05 | 0.22 | 0.91 | 84.30 | OK |
| sinai_lowsun_fp | delta | +1.77 | +0.32 |  |  |  |  |  |  |  |  |
