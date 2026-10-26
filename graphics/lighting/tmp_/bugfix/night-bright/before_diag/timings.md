# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| town_night | classic | 7.83 | 7.40 |  |  |  |  |  |  | 87.30 | OK |
| town_night | remastered | 8.01 | 6.92 |  | 0.88 | 0.77 | 0.07 |  | 0.04 | 87.90 | OK |
| town_night | delta | +0.18 | -0.48 |  |  |  |  |  |  |  |  |
| town_lamps | classic | 8.27 | 7.76 |  |  |  |  |  |  | 71.00 | OK |
| town_lamps | remastered | 8.64 | 6.98 |  | 1.21 | 0.92 | 0.16 |  | 0.13 | 81.60 | OK |
| town_lamps | delta | +0.37 | -0.78 |  |  |  |  |  |  |  |  |
| road_moonless | classic | 10.07 | 9.78 |  |  |  |  |  |  | 64.50 | OK |
| road_moonless | remastered | 8.98 | 7.89 |  | 0.93 | 0.82 | 0.07 |  | 0.04 | 66.90 | OK |
| road_moonless | delta | -1.09 | -1.89 |  |  |  |  |  |  |  |  |
| road_moonlit | classic | 8.14 | 7.84 |  |  |  |  |  |  | 59.90 | OK |
| road_moonlit | remastered | 13.53 | 12.31 |  | 0.93 | 0.71 | 0.10 |  | 0.12 | 53.10 | OK |
| road_moonlit | delta | +5.39 | +4.47 |  |  |  |  |  |  |  |  |
| nv_town | classic | 11.02 |  |  |  |  |  |  |  | 58.90 | OK |
| nv_town | remastered | 11.39 |  |  |  |  |  |  |  | 62.00 | OK |
| nv_town | delta | +0.37 |  |  |  |  |  |  |  |  |  |
