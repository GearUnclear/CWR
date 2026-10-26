# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| olive_morn | classic | 3.07 | 2.94 |  |  |  |  |  |  | 177.80 | OK |
| olive_morn | remastered | 5.95 | 5.01 | 1.16 | 0.82 | 0.34 | 0.05 | 0.15 | 0.27 | 125.70 | OK |
| olive_morn | delta | +2.88 | +2.07 |  |  |  |  |  |  |  |  |
| town_eve | classic | 5.51 | 5.39 |  |  |  |  |  |  | 111.80 | OK |
| town_eve | remastered | 7.51 | 6.84 | 0.31 | 0.54 | 0.30 | 0.05 | 0.15 | 0.04 | 102.00 | OK |
| town_eve | delta | +2.00 | +1.45 |  |  |  |  |  |  |  |  |
| hl_front | classic | 9.69 | 9.55 |  |  |  |  |  |  | 80.30 | OK |
| hl_front | remastered | 5.75 | 5.26 |  | 0.34 | 0.26 | 0.05 |  | 0.03 | 97.80 | OK |
| hl_front | delta | -3.94 | -4.29 |  |  |  |  |  |  |  |  |
| hl_side | classic | missing |  |  |  |  |  |  |  |  |  |
| hl_side | remastered | 1.96 | 1.58 |  | 0.26 | 0.18 | 0.05 |  | 0.03 | 288.90 | OK |
| pinewood_morn | classic | missing |  |  |  |  |  |  |  |  |  |
| pinewood_morn | remastered | 10.46 | 9.88 | 1.21 | 0.43 | 0.19 | 0.05 | 0.15 | 0.04 | 86.90 | OK |
| forest_edge_golden | classic | 16.60 | 15.96 |  |  |  |  |  |  | 138.40 | OK |
| forest_edge_golden | remastered | 15.01 | 14.59 | 0.92 | 0.29 | 0.20 | 0.05 |  | 0.03 | 54.50 | OK |
| forest_edge_golden | delta | -1.59 | -1.37 |  |  |  |  |  |  |  |  |
