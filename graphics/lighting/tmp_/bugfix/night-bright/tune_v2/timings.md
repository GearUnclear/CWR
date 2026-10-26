# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| town_night | classic | 13.80 | 13.58 |  |  |  |  |  |  | 64.80 | OK |
| town_night | remastered | 14.69 | 14.19 |  | 0.34 | 0.26 | 0.05 |  | 0.03 | 59.90 | OK |
| town_night | delta | +0.89 | +0.61 |  |  |  |  |  |  |  |  |
| town_lamps | classic | 3.49 | 3.33 |  |  |  |  |  |  | 148.40 | OK |
| town_lamps | remastered | 4.23 | 3.73 |  | 0.29 | 0.22 | 0.05 |  | 0.03 | 162.70 | OK |
| town_lamps | delta | +0.74 | +0.40 |  |  |  |  |  |  |  |  |
| road_moonless | classic | 8.82 | 8.70 |  |  |  |  |  |  | 73.30 | OK |
| road_moonless | remastered | 14.45 | 13.85 |  | 0.46 | 0.39 | 0.05 |  | 0.03 | 59.30 | OK |
| road_moonless | delta | +5.63 | +5.15 |  |  |  |  |  |  |  |  |
| road_moonlit | classic | 5.79 | 5.67 |  |  |  |  |  |  | 103.60 | OK |
| road_moonlit | remastered | 10.37 | 9.96 |  | 0.27 | 0.20 | 0.05 |  | 0.03 | 80.50 | OK |
| road_moonlit | delta | +4.58 | +4.29 |  |  |  |  |  |  |  |  |
| nv_town | classic | 11.03 |  |  |  |  |  |  |  | 82.00 | OK |
| nv_town | remastered | 14.98 |  |  |  |  |  |  |  | 52.60 | OK |
| nv_town | delta | +3.95 |  |  |  |  |  |  |  |  |  |
