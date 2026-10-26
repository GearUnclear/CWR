# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| town_night | classic | 7.15 | 7.00 |  |  |  |  |  |  | 108.80 | OK |
| town_night | remastered | 9.05 | 8.54 |  | 0.35 | 0.28 | 0.05 |  | 0.03 | 92.20 | OK |
| town_night | delta | +1.90 | +1.54 |  |  |  |  |  |  |  |  |
| town_lamps | classic | 3.44 | 3.29 |  |  |  |  |  |  | 163.00 | OK |
| town_lamps | remastered | 2.81 | 2.33 |  | 0.31 | 0.24 | 0.05 |  | 0.03 | 192.10 | OK |
| town_lamps | delta | -0.63 | -0.96 |  |  |  |  |  |  |  |  |
| road_moonless | classic | 5.50 | 5.38 |  |  |  |  |  |  | 120.70 | OK |
| road_moonless | remastered | 6.49 | 6.09 |  | 0.28 | 0.20 | 0.05 |  | 0.03 | 114.10 | OK |
| road_moonless | delta | +0.99 | +0.71 |  |  |  |  |  |  |  |  |
| road_moonlit | classic | 5.19 | 5.07 |  |  |  |  |  |  | 125.60 | OK |
| road_moonlit | remastered | 6.50 | 6.07 |  | 0.31 | 0.23 | 0.05 |  | 0.03 | 112.70 | OK |
| road_moonlit | delta | +1.31 | +1.00 |  |  |  |  |  |  |  |  |
| nv_town | classic | 6.19 |  |  |  |  |  |  |  | 133.80 | OK |
| nv_town | remastered | 9.17 |  |  |  |  |  |  |  | 98.60 | OK |
| nv_town | delta | +2.98 |  |  |  |  |  |  |  |  |  |
| nv_road | classic | 5.20 |  |  |  |  |  |  |  | 126.40 | OK |
| nv_road | remastered | 6.10 |  |  |  |  |  |  |  | 116.10 | OK |
| nv_road | delta | +0.90 |  |  |  |  |  |  |  |  |  |
| dusk_town | classic | 6.15 | 6.01 |  |  |  |  |  |  | 134.00 | OK |
| dusk_town | remastered | 8.85 | 8.37 |  | 0.33 | 0.25 | 0.05 |  | 0.03 | 100.30 | OK |
| dusk_town | delta | +2.70 | +2.36 |  |  |  |  |  |  |  |  |
