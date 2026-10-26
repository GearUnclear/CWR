# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| forest_edge_noon | classic | 2.49 | 2.38 |  |  |  |  |  |  | 318.80 | OK |
| forest_edge_noon | remastered | 4.20 | 3.77 | 0.03 | 0.29 | 0.22 | 0.05 |  | 0.03 | 135.20 | OK |
| forest_edge_noon | delta | +1.71 | +1.39 |  |  |  |  |  |  |  |  |
| dawn_box | classic | 9.26 | 9.16 |  |  |  |  |  |  | 200.60 | OK |
| dawn_box | remastered | 10.39 | 9.89 | 0.04 | 0.40 | 0.32 | 0.05 |  | 0.03 | 65.60 | OK |
| dawn_box | delta | +1.13 | +0.73 |  |  |  |  |  |  |  |  |
| dawn_box5 | classic | 7.08 | 6.98 |  |  |  |  |  |  | 199.40 | OK |
| dawn_box5 | remastered | 11.01 | 10.48 | 0.04 | 0.42 | 0.34 | 0.05 |  | 0.03 | 75.70 | OK |
| dawn_box5 | delta | +3.93 | +3.50 |  |  |  |  |  |  |  |  |
| sinai_dusk_box | classic | 8.30 | 8.15 |  |  |  |  |  |  | 84.10 | OK |
| sinai_dusk_box | remastered | 15.22 | 14.65 | 0.05 | 0.45 | 0.37 | 0.05 |  | 0.03 | 58.80 | OK |
| sinai_dusk_box | delta | +6.92 | +6.50 |  |  |  |  |  |  |  |  |
