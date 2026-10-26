# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| forest_edge_golden | classic | 2.83 | 2.71 |  |  |  |  |  |  | 233.80 | OK |
| forest_edge_golden | remastered | 8.28 | 7.86 | 0.03 | 0.28 | 0.20 | 0.05 |  | 0.03 | 94.10 | OK |
| forest_edge_golden | delta | +5.45 | +5.15 |  |  |  |  |  |  |  |  |
| forest_edge_noon | classic | 9.11 | 8.99 |  |  |  |  |  |  | 163.90 | OK |
| forest_edge_noon | remastered | 14.16 | 13.77 | 0.03 | 0.26 | 0.18 | 0.05 |  | 0.03 | 61.30 | OK |
| forest_edge_noon | delta | +5.05 | +4.78 |  |  |  |  |  |  |  |  |
| dawn_box | classic | 10.43 | 10.33 |  |  |  |  |  |  | 187.40 | OK |
| dawn_box | remastered | 19.68 | 19.29 | 0.04 | 0.28 | 0.20 | 0.05 |  | 0.03 | 47.10 | OK |
| dawn_box | delta | +9.25 | +8.96 |  |  |  |  |  |  |  |  |
| dawn_box5 | classic | 15.18 | 15.08 |  |  |  |  |  |  | 185.40 | OK |
| dawn_box5 | remastered | 20.36 | 20.00 | 0.04 | 0.26 | 0.18 | 0.05 |  | 0.03 | 43.20 | OK |
| dawn_box5 | delta | +5.18 | +4.92 |  |  |  |  |  |  |  |  |
| sinai_dusk_box | classic | 11.92 | 11.81 |  |  |  |  |  |  | 64.00 | OK |
| sinai_dusk_box | remastered | 15.75 | 15.37 | 0.05 | 0.25 | 0.18 | 0.05 |  | 0.03 | 47.00 | OK |
| sinai_dusk_box | delta | +3.83 | +3.56 |  |  |  |  |  |  |  |  |
