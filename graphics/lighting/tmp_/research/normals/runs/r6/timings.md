# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 2.65 | 2.55 |  |  |  |  |  |  | 217.80 | OK |
| lowsun_fp | remastered | 8.44 | 7.43 | 0.04 | 0.90 | 0.28 | 0.05 | 0.22 | 0.35 | 106.60 | OK |
| lowsun_fp | delta | +5.79 | +4.88 |  |  |  |  |  |  |  |  |
| hull_close | classic | 13.76 | 13.67 |  |  |  |  |  |  | 61.70 | OK |
| hull_close | remastered | 16.56 | 14.88 | 0.13 | 1.58 | 0.22 | 0.05 | 0.24 | 1.08 | 50.10 | OK |
| hull_close | delta | +2.80 | +1.21 |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 8.28 | 8.15 |  |  |  |  |  |  | 226.50 | OK |
| forest_edge_golden | remastered | 13.69 | 13.31 | 0.11 | 0.26 | 0.18 | 0.05 |  | 0.03 | 72.20 | OK |
| forest_edge_golden | delta | +5.41 | +5.16 |  |  |  |  |  |  |  |  |
| noon_fields | classic | 22.10 | 21.98 |  |  |  |  |  |  | 35.70 | OK |
| noon_fields | remastered | 25.76 | 25.36 | 0.04 | 0.29 | 0.21 | 0.05 |  | 0.03 | 31.40 | OK |
| noon_fields | delta | +3.66 | +3.38 |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 3.14 | 3.02 |  |  |  |  |  |  | 161.80 | OK |
| sinai_lowsun_fp | remastered | 8.60 | 6.87 | 0.51 | 1.60 | 0.23 | 0.05 | 0.22 | 1.10 | 125.30 | OK |
| sinai_lowsun_fp | delta | +5.46 | +3.85 |  |  |  |  |  |  |  |  |
| sinai_hull_close | classic | 4.85 | 4.75 |  |  |  |  |  |  | 123.10 | OK |
| sinai_hull_close | remastered | 8.42 | 6.70 | 0.04 | 1.62 | 0.20 | 0.05 | 0.24 | 1.13 | 108.60 | OK |
| sinai_hull_close | delta | +3.57 | +1.95 |  |  |  |  |  |  |  |  |
