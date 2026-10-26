# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| town_night | classic | 6.94 | 6.79 |  |  |  |  |  |  | 114.00 | OK |
| town_night | remastered | 9.12 | 8.61 |  | 0.35 | 0.28 | 0.05 |  | 0.03 | 96.50 | OK |
| town_night | delta | +2.18 | +1.82 |  |  |  |  |  |  |  |  |
| town_lamps | classic | 3.56 | 3.41 |  |  |  |  |  |  | 162.30 | OK |
| town_lamps | remastered | 2.78 | 2.25 |  | 0.33 | 0.25 | 0.05 |  | 0.03 | 192.40 | OK |
| town_lamps | delta | -0.78 | -1.16 |  |  |  |  |  |  |  |  |
| road_moonless | classic | 5.37 | 5.25 |  |  |  |  |  |  | 123.40 | OK |
| road_moonless | remastered | 6.73 | 6.28 |  | 0.33 | 0.26 | 0.05 |  | 0.03 | 111.10 | OK |
| road_moonless | delta | +1.36 | +1.03 |  |  |  |  |  |  |  |  |
| road_moonlit | classic | 5.53 | 5.40 |  |  |  |  |  |  | 126.50 | OK |
| road_moonlit | remastered | 6.29 | 5.87 |  | 0.30 | 0.23 | 0.05 |  | 0.03 | 114.00 | OK |
| road_moonlit | delta | +0.76 | +0.47 |  |  |  |  |  |  |  |  |
| nv_town | classic | 6.01 |  |  |  |  |  |  |  | 132.50 | OK |
| nv_town | remastered | 8.72 |  |  |  |  |  |  |  | 96.80 | OK |
| nv_town | delta | +2.71 |  |  |  |  |  |  |  |  |  |
| nv_road | classic | 5.68 |  |  |  |  |  |  |  | 112.70 | OK |
| nv_road | remastered | 6.19 |  |  |  |  |  |  |  | 111.40 | OK |
| nv_road | delta | +0.51 |  |  |  |  |  |  |  |  |  |
| dusk_town | classic | 5.91 | 5.77 |  |  |  |  |  |  | 121.40 | OK |
| dusk_town | remastered | 9.01 | 8.36 |  | 0.50 | 0.42 | 0.05 |  | 0.03 | 93.60 | OK |
| dusk_town | delta | +3.10 | +2.59 |  |  |  |  |  |  |  |  |
