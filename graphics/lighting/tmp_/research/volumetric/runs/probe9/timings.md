# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 1.93 | 1.82 |  |  |  |  |  |  | 246.30 | OK |
| lowsun_fp | remastered | 5.39 | 4.15 | 0.05 | 1.13 | 0.17 | 0.05 | 0.22 | 0.69 | 158.30 | OK |
| lowsun_fp | delta | +3.46 | +2.33 |  |  |  |  |  |  |  |  |
| pine_eve | classic | 8.01 | 7.89 |  |  |  |  |  |  | 87.10 | OK |
| pine_eve | remastered | 21.09 | 20.57 | 0.21 | 0.40 | 0.16 | 0.05 | 0.15 | 0.04 | 47.40 | OK |
| pine_eve | delta | +13.08 | +12.68 |  |  |  |  |  |  |  |  |
| pinewood_morn | classic | 5.17 | 5.03 |  |  |  |  |  |  | 110.90 | OK |
| pinewood_morn | remastered | 4.57 | 3.84 | 0.34 | 0.58 | 0.35 | 0.05 | 0.15 | 0.04 | 126.40 | OK |
| pinewood_morn | delta | -0.60 | -1.19 |  |  |  |  |  |  |  |  |
| town_eve | classic | 3.19 | 3.07 |  |  |  |  |  |  | 133.40 | OK |
| town_eve | remastered | 4.11 | 3.44 | 0.25 | 0.55 | 0.31 | 0.05 | 0.15 | 0.04 | 159.00 | OK |
| town_eve | delta | +0.92 | +0.37 |  |  |  |  |  |  |  |  |
