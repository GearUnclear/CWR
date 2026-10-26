# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 4.10 | 4.00 |  |  |  |  |  |  | 162.00 | OK |
| lowsun_fp | remastered | 8.24 | 7.12 | 0.04 | 1.01 | 0.26 | 0.05 | 0.22 | 0.47 | 91.10 | OK |
| lowsun_fp | delta | +4.14 | +3.12 |  |  |  |  |  |  |  |  |
| hull_close | classic | 11.82 | 11.72 |  |  |  |  |  |  | 63.40 | OK |
| hull_close | remastered | 13.05 | 11.90 | 0.04 | 1.05 | 0.25 | 0.05 | 0.24 | 0.51 | 63.90 | OK |
| hull_close | delta | +1.23 | +0.18 |  |  |  |  |  |  |  |  |
| noon_fields | classic | 10.27 | 10.17 |  |  |  |  |  |  | 66.00 | OK |
| noon_fields | remastered | 26.48 | 26.12 | 0.08 | 0.25 | 0.17 | 0.05 |  | 0.03 | 36.00 | OK |
| noon_fields | delta | +16.21 | +15.95 |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 6.03 | 5.90 |  |  |  |  |  |  | 212.20 | OK |
| forest_edge_golden | remastered | 11.47 | 11.08 | 0.03 | 0.26 | 0.18 | 0.05 |  | 0.03 | 73.20 | OK |
| forest_edge_golden | delta | +5.44 | +5.18 |  |  |  |  |  |  |  |  |
| tank_gunner_interior | classic | 12.34 | 11.22 |  |  |  |  |  |  | 70.30 | OK |
| tank_gunner_interior | remastered | 15.43 | 14.09 | 0.04 | 0.22 |  | 0.19 |  | 0.02 | 53.30 | OK |
| tank_gunner_interior | delta | +3.09 | +2.87 |  |  |  |  |  |  |  |  |
