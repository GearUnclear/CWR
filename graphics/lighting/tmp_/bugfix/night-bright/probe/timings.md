# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| town_night | classic | missing |  |  |  |  |  |  |  |  |  |
| town_night | remastered | 12.89 | 11.72 |  | 1.00 | 0.92 | 0.05 |  | 0.03 | 64.00 | OK |
| town_lamps | classic | missing |  |  |  |  |  |  |  |  |  |
| town_lamps | remastered | 7.24 | 6.08 |  | 0.93 | 0.84 | 0.06 |  | 0.03 | 90.90 | OK |
| road_moonless | classic | missing |  |  |  |  |  |  |  |  |  |
| road_moonless | remastered | 9.41 | 8.44 |  | 0.85 | 0.77 | 0.05 |  | 0.03 | 67.40 | OK |
| road_moonlit | classic | missing |  |  |  |  |  |  |  |  |  |
| road_moonlit | remastered | 10.31 | 9.11 |  | 1.07 | 0.99 | 0.05 |  | 0.03 | 70.30 | OK |
| nv_town | classic | missing |  |  |  |  |  |  |  |  |  |
| nv_town | remastered | 11.24 |  |  |  |  |  |  |  | 69.70 | OK |
