# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| noon_fields | classic | 4.14 | 4.03 |  |  |  |  |  |  | 214.80 | OK |
| noon_fields | remastered | 38.84 | 38.48 | 0.04 | 0.24 | 0.17 | 0.05 |  | 0.03 | 30.80 | OK |
| noon_fields | delta | +34.70 | +34.45 |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 9.71 | 9.58 |  |  |  |  |  |  | 158.50 | OK |
| forest_edge_golden | remastered | 15.60 | 15.19 | 0.03 | 0.27 | 0.20 | 0.05 |  | 0.03 | 52.70 | OK |
| forest_edge_golden | delta | +5.89 | +5.61 |  |  |  |  |  |  |  |  |
| lowsun_fp | classic | 10.66 | 10.54 |  |  |  |  |  |  | 61.20 | OK |
| lowsun_fp | remastered | 16.25 | 14.06 | 0.52 | 2.07 | 0.18 | 0.05 | 0.22 | 1.62 | 53.30 | OK |
| lowsun_fp | delta | +5.59 | +3.52 |  |  |  |  |  |  |  |  |
| overcast_broken | classic | 36.40 | 36.29 |  |  |  |  |  |  | 22.60 | OK |
| overcast_broken | remastered | 43.56 | 42.91 | 0.13 | 0.54 | 0.47 | 0.05 |  | 0.03 | 19.10 | OK |
| overcast_broken | delta | +7.16 | +6.62 |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 19.03 | 18.92 |  |  |  |  |  |  | 48.20 | OK |
| sinai_desert_noon | remastered | 27.22 | 25.08 | 0.11 | 2.02 | 1.93 | 0.05 |  | 0.03 | 23.70 | OK |
| sinai_desert_noon | delta | +8.19 | +6.16 |  |  |  |  |  |  |  |  |
