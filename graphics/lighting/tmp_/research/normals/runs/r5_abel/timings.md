# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 3.85 | 3.74 |  |  |  |  |  |  | 149.60 | OK |
| lowsun_fp | remastered | 11.13 | 9.56 | 0.04 | 1.46 | 0.17 | 0.05 | 0.22 | 1.03 | 76.00 | OK |
| lowsun_fp | delta | +7.28 | +5.82 |  |  |  |  |  |  |  |  |
| hull_close | classic | 14.07 | 13.98 |  |  |  |  |  |  | 58.90 | OK |
| hull_close | remastered | 21.36 | 19.39 | 0.24 | 1.88 | 0.19 | 0.05 | 0.23 | 1.41 | 46.10 | OK |
| hull_close | delta | +7.29 | +5.41 |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 8.71 | 8.59 |  |  |  |  |  |  | 223.10 | OK |
| forest_edge_golden | remastered | 14.00 | 13.63 | 0.03 | 0.25 | 0.17 | 0.05 |  | 0.03 | 61.50 | OK |
| forest_edge_golden | delta | +5.29 | +5.04 |  |  |  |  |  |  |  |  |
| noon_fields | classic | 13.46 | 13.35 |  |  |  |  |  |  | 54.20 | OK |
| noon_fields | remastered | 18.70 | 18.28 | 0.04 | 0.31 | 0.23 | 0.05 |  | 0.03 | 42.50 | OK |
| noon_fields | delta | +5.24 | +4.93 |  |  |  |  |  |  |  |  |
| tank_gunner_interior | classic | 11.98 | 11.03 |  |  |  |  |  |  | 66.60 | OK |
| tank_gunner_interior | remastered | 12.37 | 11.17 | 0.03 | 0.39 |  | 0.36 |  | 0.02 | 57.00 | OK |
| tank_gunner_interior | delta | +0.39 | +0.14 |  |  |  |  |  |  |  |  |
