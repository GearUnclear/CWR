# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| pinewood_morn | classic | 6.11 | 5.97 |  |  |  |  |  |  | 121.10 | OK |
| pinewood_morn | remastered | 6.16 | 5.44 | 0.26 | 0.57 | 0.25 | 0.05 | 0.15 | 0.12 | 111.10 | OK |
| pinewood_morn | delta | +0.05 | -0.53 |  |  |  |  |  |  |  |  |
| pine_eve | classic | 22.28 | 22.16 |  |  |  |  |  |  | 35.80 | OK |
| pine_eve | remastered | 15.43 | 14.80 | 0.29 | 0.51 | 0.28 | 0.05 | 0.15 | 0.03 | 46.90 | OK |
| pine_eve | delta | -6.85 | -7.36 |  |  |  |  |  |  |  |  |
| pine_mist | classic | 17.21 | 17.09 |  |  |  |  |  |  | 54.80 | OK |
| pine_mist | remastered | 16.27 | 15.75 | 0.19 | 0.38 | 0.15 | 0.05 | 0.15 | 0.03 | 55.70 | OK |
| pine_mist | delta | -0.94 | -1.34 |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 15.96 | 15.37 |  |  |  |  |  |  | 80.70 | OK |
| forest_edge_golden | remastered | 14.10 | 13.66 | 0.29 | 0.30 | 0.23 | 0.05 |  | 0.03 | 52.50 | OK |
| forest_edge_golden | delta | -1.86 | -1.71 |  |  |  |  |  |  |  |  |
| lowsun_fp | classic | 13.71 | 13.59 |  |  |  |  |  |  | 59.10 | OK |
| lowsun_fp | remastered | 19.36 | 16.99 | 0.48 | 2.26 | 0.17 | 0.05 | 0.22 | 1.82 | 50.40 | OK |
| lowsun_fp | delta | +5.65 | +3.40 |  |  |  |  |  |  |  |  |
| noon_fields | classic | 33.14 | 33.03 |  |  |  |  |  |  | 25.20 | OK |
| noon_fields | remastered | 44.59 | 44.23 | 0.07 | 0.25 | 0.17 | 0.05 |  | 0.03 | 20.30 | OK |
| noon_fields | delta | +11.45 | +11.20 |  |  |  |  |  |  |  |  |
| town_night | classic | 12.37 | 12.20 |  |  |  |  |  |  | 67.70 | OK |
| town_night | remastered | 15.58 | 15.14 |  | 0.27 | 0.20 | 0.05 |  | 0.03 | 61.50 | OK |
| town_night | delta | +3.21 | +2.94 |  |  |  |  |  |  |  |  |
