# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| town_night | classic | 7.32 | 6.99 |  |  |  |  |  |  | 120.00 | OK |
| town_night | remastered | 6.40 | 5.65 |  | 0.58 | 0.49 | 0.06 |  | 0.03 | 117.80 | OK |
| town_night | delta | -0.92 | -1.34 |  |  |  |  |  |  |  |  |
| town_lamps | classic | 3.65 | 3.28 |  |  |  |  |  |  | 146.60 | OK |
| town_lamps | remastered | 4.35 | 3.05 |  | 0.87 | 0.61 | 0.14 |  | 0.13 | 148.40 | OK |
| town_lamps | delta | +0.70 | -0.23 |  |  |  |  |  |  |  |  |
| road_moonless | classic | 5.71 | 5.44 |  |  |  |  |  |  | 125.90 | OK |
| road_moonless | remastered | 3.96 | 3.41 |  | 0.44 | 0.36 | 0.05 |  | 0.03 | 149.30 | OK |
| road_moonless | delta | -1.75 | -2.03 |  |  |  |  |  |  |  |  |
| road_moonlit | classic | 5.18 | 5.02 |  |  |  |  |  |  | 131.40 | OK |
| road_moonlit | remastered | 4.44 | 3.84 |  | 0.46 | 0.37 | 0.06 |  | 0.03 | 138.20 | OK |
| road_moonlit | delta | -0.74 | -1.18 |  |  |  |  |  |  |  |  |
| nv_town | classic | 6.70 |  |  |  |  |  |  |  | 133.90 | OK |
| nv_town | remastered | 6.10 |  |  |  |  |  |  |  | 128.10 | OK |
| nv_town | delta | -0.60 |  |  |  |  |  |  |  |  |  |
| nv_road | classic | 3.98 |  |  |  |  |  |  |  | 124.40 | OK |
| nv_road | remastered | 4.41 |  |  |  |  |  |  |  | 103.80 | OK |
| nv_road | delta | +0.43 |  |  |  |  |  |  |  |  |  |
| dusk_town | classic | 5.50 | 5.36 |  |  |  |  |  |  | 119.70 | OK |
| dusk_town | remastered | 6.88 | 6.22 |  | 0.52 | 0.44 | 0.05 |  | 0.03 | 115.20 | OK |
| dusk_town | delta | +1.38 | +0.86 |  |  |  |  |  |  |  |  |
