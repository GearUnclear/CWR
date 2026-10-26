# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| town_night | classic | 5.45 | 5.30 |  |  |  |  |  |  | 101.80 | OK |
| town_night | remastered | 7.71 | 7.20 |  | 0.34 | 0.26 | 0.05 |  | 0.03 | 89.70 | OK |
| town_night | delta | +2.26 | +1.90 |  |  |  |  |  |  |  |  |
| town_lamps | classic | 2.24 | 2.09 |  |  |  |  |  |  | 171.90 | OK |
| town_lamps | remastered | 2.07 | 1.44 |  | 0.47 | 0.40 | 0.05 |  | 0.03 | 182.80 | OK |
| town_lamps | delta | -0.17 | -0.65 |  |  |  |  |  |  |  |  |
| dusk_town | classic | 4.23 | 4.09 |  |  |  |  |  |  | 134.60 | OK |
| dusk_town | remastered | 7.50 | 6.88 |  | 0.47 | 0.39 | 0.05 |  | 0.03 | 111.80 | OK |
| dusk_town | delta | +3.27 | +2.79 |  |  |  |  |  |  |  |  |
