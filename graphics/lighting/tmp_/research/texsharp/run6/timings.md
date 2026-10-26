# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| noon_fields | classic | 2.65 | 2.55 |  |  |  |  |  |  | 191.90 | OK |
| noon_fields | remastered | 17.06 | 16.60 | 0.09 | 0.35 | 0.28 | 0.05 |  | 0.03 | 34.30 | OK |
| noon_fields | delta | +14.41 | +14.05 |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 9.56 | 9.43 |  |  |  |  |  |  | 181.10 | OK |
| forest_edge_golden | remastered | 14.36 | 13.97 | 0.03 | 0.26 | 0.19 | 0.05 |  | 0.03 | 60.60 | OK |
| forest_edge_golden | delta | +4.80 | +4.54 |  |  |  |  |  |  |  |  |
| lowsun_fp | classic | 11.16 | 11.05 |  |  |  |  |  |  | 69.40 | OK |
| lowsun_fp | remastered | 14.46 | 12.64 | 0.05 | 1.71 | 0.20 | 0.05 | 0.22 | 1.25 | 57.30 | OK |
| lowsun_fp | delta | +3.30 | +1.59 |  |  |  |  |  |  |  |  |
| overcast_broken | classic | 12.66 | 12.55 |  |  |  |  |  |  | 51.10 | OK |
| overcast_broken | remastered | 13.92 | 13.28 | 0.03 | 0.53 | 0.46 | 0.05 |  | 0.03 | 49.70 | OK |
| overcast_broken | delta | +1.26 | +0.73 |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 4.77 | 4.65 |  |  |  |  |  |  | 103.30 | OK |
| sinai_desert_noon | remastered | 18.41 | 17.50 | 0.08 | 0.79 | 0.71 | 0.05 |  | 0.03 | 36.80 | OK |
| sinai_desert_noon | delta | +13.64 | +12.85 |  |  |  |  |  |  |  |  |
