# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 4.64 | 4.53 |  |  |  |  |  |  | 163.40 | OK |
| lowsun_fp | remastered | 7.84 | 6.46 | 0.10 | 1.27 | 0.34 | 0.05 | 0.22 | 0.66 | 99.90 | OK |
| lowsun_fp | delta | +3.20 | +1.93 |  |  |  |  |  |  |  |  |
| hull_close | classic | 12.55 | 12.45 |  |  |  |  |  |  | 64.90 | OK |
| hull_close | remastered | 16.84 | 15.25 | 0.05 | 1.49 | 0.26 | 0.05 | 0.23 | 0.95 | 46.50 | OK |
| hull_close | delta | +4.29 | +2.80 |  |  |  |  |  |  |  |  |
| noon_fields | classic | 9.18 | 9.08 |  |  |  |  |  |  | 61.10 | OK |
| noon_fields | remastered | 16.89 | 16.53 | 0.04 | 0.25 | 0.17 | 0.05 |  | 0.03 | 46.20 | OK |
| noon_fields | delta | +7.71 | +7.45 |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 6.94 | 6.81 |  |  |  |  |  |  | 183.60 | OK |
| forest_edge_golden | remastered | 13.17 | 12.76 | 0.03 | 0.27 | 0.19 | 0.05 |  | 0.03 | 58.30 | OK |
| forest_edge_golden | delta | +6.23 | +5.95 |  |  |  |  |  |  |  |  |
| tank_gunner_interior | classic | 15.41 | 13.74 |  |  |  |  |  |  | 55.70 | OK |
| tank_gunner_interior | remastered | 7.49 | 6.50 | 0.03 | 0.58 |  | 0.56 |  | 0.02 | 57.30 | OK |
| tank_gunner_interior | delta | -7.92 | -7.24 |  |  |  |  |  |  |  |  |
