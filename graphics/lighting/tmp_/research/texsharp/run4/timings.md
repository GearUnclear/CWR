# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| noon_fields | classic | 3.06 | 2.95 |  |  |  |  |  |  | 163.60 | OK |
| noon_fields | remastered | 31.21 | 30.85 | 0.04 | 0.25 | 0.17 | 0.05 |  | 0.03 | 36.80 | OK |
| noon_fields | delta | +28.15 | +27.90 |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 6.25 | 6.12 |  |  |  |  |  |  | 153.80 | OK |
| forest_edge_golden | remastered | 10.77 | 10.33 | 0.03 | 0.31 | 0.23 | 0.05 |  | 0.03 | 66.00 | OK |
| forest_edge_golden | delta | +4.52 | +4.21 |  |  |  |  |  |  |  |  |
| lowsun_fp | classic | 10.41 | 10.08 |  |  |  |  |  |  | 72.30 | OK |
| lowsun_fp | remastered | 10.66 | 9.29 | 0.04 | 1.26 | 0.22 | 0.05 | 0.22 | 0.77 | 73.00 | OK |
| lowsun_fp | delta | +0.25 | -0.79 |  |  |  |  |  |  |  |  |
| overcast_broken | classic | 17.38 | 17.28 |  |  |  |  |  |  | 36.90 | OK |
| overcast_broken | remastered | 22.40 | 21.34 | 0.09 | 0.96 | 0.88 | 0.05 |  | 0.03 | 32.40 | OK |
| overcast_broken | delta | +5.02 | +4.06 |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 3.94 | 3.83 |  |  |  |  |  |  | 88.10 | OK |
| sinai_desert_noon | remastered | 12.99 | 11.62 | 0.15 | 1.25 | 1.17 | 0.05 |  | 0.03 | 48.70 | OK |
| sinai_desert_noon | delta | +9.05 | +7.79 |  |  |  |  |  |  |  |  |
