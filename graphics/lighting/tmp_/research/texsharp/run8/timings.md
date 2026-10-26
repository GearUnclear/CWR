# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| noon_fields | classic | 3.36 | 3.26 |  |  |  |  |  |  | 265.70 | OK |
| noon_fields | remastered | 35.94 | 35.56 | 0.05 | 0.27 | 0.20 | 0.05 |  | 0.03 | 36.00 | OK |
| noon_fields | delta | +32.58 | +32.30 |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 6.29 | 6.17 |  |  |  |  |  |  | 192.10 | OK |
| forest_edge_golden | remastered | 11.98 | 11.59 | 0.03 | 0.26 | 0.19 | 0.05 |  | 0.03 | 93.20 | OK |
| forest_edge_golden | delta | +5.69 | +5.42 |  |  |  |  |  |  |  |  |
| lowsun_fp | classic | 8.84 | 8.73 |  |  |  |  |  |  | 79.60 | OK |
| lowsun_fp | remastered | 12.87 | 11.48 | 0.55 | 1.27 | 0.27 | 0.05 | 0.22 | 0.73 | 66.30 | OK |
| lowsun_fp | delta | +4.03 | +2.75 |  |  |  |  |  |  |  |  |
| overcast_broken | classic | 28.80 | 28.69 |  |  |  |  |  |  | 26.40 | OK |
| overcast_broken | remastered | 29.27 | 28.59 | 0.03 | 0.58 | 0.50 | 0.05 |  | 0.03 | 25.10 | OK |
| overcast_broken | delta | +0.47 | -0.10 |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 5.62 | 5.51 |  |  |  |  |  |  | 92.70 | OK |
| sinai_desert_noon | remastered | 27.00 | 26.29 | 0.04 | 0.59 | 0.51 | 0.05 |  | 0.03 | 33.60 | OK |
| sinai_desert_noon | delta | +21.38 | +20.78 |  |  |  |  |  |  |  |  |
