# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 3.19 | 3.09 |  |  |  |  |  |  | 125.70 | OK |
| lowsun_fp | remastered | 5.98 | 5.11 | 0.04 | 0.76 | 0.40 | 0.05 | 0.22 | 0.09 | 98.90 | OK |
| lowsun_fp | delta | +2.79 | +2.02 |  |  |  |  |  |  |  |  |
| hull_close | classic | 7.10 | 7.01 |  |  |  |  |  |  | 92.20 | OK |
| hull_close | remastered | 13.55 | 12.21 | 0.04 | 1.24 | 0.25 | 0.05 | 0.24 | 0.71 | 65.20 | OK |
| hull_close | delta | +6.45 | +5.20 |  |  |  |  |  |  |  |  |
| noon_fields | classic | 21.84 | 21.74 |  |  |  |  |  |  | 39.70 | OK |
| noon_fields | remastered | 25.13 | 24.73 | 0.04 | 0.29 | 0.21 | 0.05 |  | 0.03 | 30.10 | OK |
| noon_fields | delta | +3.29 | +2.99 |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 4.93 | 4.80 |  |  |  |  |  |  | 213.80 | OK |
| forest_edge_golden | remastered | 10.32 | 9.93 | 0.03 | 0.25 | 0.18 | 0.05 |  | 0.03 | 86.40 | OK |
| forest_edge_golden | delta | +5.39 | +5.13 |  |  |  |  |  |  |  |  |
| tank_gunner_interior | classic | 10.26 | 9.25 |  |  |  |  |  |  | 77.70 | OK |
| tank_gunner_interior | remastered | 16.14 | 14.82 | 0.11 | 0.17 |  | 0.15 |  | 0.02 | 55.50 | OK |
| tank_gunner_interior | delta | +5.88 | +5.57 |  |  |  |  |  |  |  |  |
