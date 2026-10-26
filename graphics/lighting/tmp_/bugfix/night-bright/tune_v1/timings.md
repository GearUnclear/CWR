# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| town_night | classic | 14.78 | 14.62 |  |  |  |  |  |  | 57.00 | OK |
| town_night | remastered | 18.03 | 16.83 |  | 1.03 | 0.95 | 0.05 |  | 0.03 | 47.40 | OK |
| town_night | delta | +3.25 | +2.21 |  |  |  |  |  |  |  |  |
| town_lamps | classic | 3.66 | 3.50 |  |  |  |  |  |  | 101.80 | OK |
| town_lamps | remastered | 3.38 | 2.92 |  | 0.29 | 0.22 | 0.05 |  | 0.03 | 152.30 | OK |
| town_lamps | delta | -0.28 | -0.58 |  |  |  |  |  |  |  |  |
| road_moonless | classic | 9.25 | 9.13 |  |  |  |  |  |  | 85.30 | OK |
| road_moonless | remastered | 9.34 | 8.96 |  | 0.25 | 0.17 | 0.05 |  | 0.03 | 81.40 | OK |
| road_moonless | delta | +0.09 | -0.17 |  |  |  |  |  |  |  |  |
| road_moonlit | classic | 6.94 | 6.81 |  |  |  |  |  |  | 88.60 | OK |
| road_moonlit | remastered | 9.14 | 8.74 |  | 0.27 | 0.19 | 0.05 |  | 0.03 | 78.40 | OK |
| road_moonlit | delta | +2.20 | +1.93 |  |  |  |  |  |  |  |  |
| nv_town | classic | 6.77 |  |  |  |  |  |  |  | 117.80 | OK |
| nv_town | remastered | 12.84 |  |  |  |  |  |  |  | 71.90 | OK |
| nv_town | delta | +6.07 |  |  |  |  |  |  |  |  |  |
