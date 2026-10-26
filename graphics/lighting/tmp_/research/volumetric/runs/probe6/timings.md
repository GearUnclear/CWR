# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| hl_side | classic | 4.22 | 4.07 |  |  |  |  |  |  | 296.70 | OK |
| hl_side | remastered | 6.71 | 6.31 |  | 0.28 | 0.20 | 0.05 |  | 0.03 | 306.60 | OK |
| hl_side | delta | +2.49 | +2.24 |  |  |  |  |  |  |  |  |
| hl_front | classic | 7.70 | 7.55 |  |  |  |  |  |  | 78.10 | OK |
| hl_front | remastered | 7.33 | 6.75 |  | 0.44 | 0.36 | 0.05 |  | 0.03 | 81.50 | OK |
| hl_front | delta | -0.37 | -0.80 |  |  |  |  |  |  |  |  |
| pinewood_morn | classic | 20.07 | 19.93 |  |  |  |  |  |  | 40.70 | OK |
| pinewood_morn | remastered | 14.70 | 14.03 | 1.61 | 0.53 | 0.27 | 0.05 | 0.15 | 0.05 | 47.90 | OK |
| pinewood_morn | delta | -5.37 | -5.90 |  |  |  |  |  |  |  |  |
| pinewood_mist | classic | 13.73 | 13.59 |  |  |  |  |  |  | 61.00 | OK |
| pinewood_mist | remastered | 7.70 | 6.99 | 1.09 | 0.56 | 0.31 | 0.05 | 0.15 | 0.05 | 73.10 | OK |
| pinewood_mist | delta | -6.03 | -6.60 |  |  |  |  |  |  |  |  |
| pine_eve | classic | 34.85 | 34.73 |  |  |  |  |  |  | 27.70 | OK |
| pine_eve | remastered | 27.55 | 26.99 | 0.51 | 0.45 | 0.15 | 0.05 | 0.15 | 0.09 | 27.70 | OK |
| pine_eve | delta | -7.30 | -7.74 |  |  |  |  |  |  |  |  |
| lowsun_fp | classic | 14.84 | 14.73 |  |  |  |  |  |  | 55.40 | OK |
| lowsun_fp | remastered | 19.26 | 17.11 | 0.50 | 2.04 | 0.18 | 0.05 | 0.22 | 1.59 | 52.90 | OK |
| lowsun_fp | delta | +4.42 | +2.38 |  |  |  |  |  |  |  |  |
| sunrise_ridge | classic | 40.01 | 39.88 |  |  |  |  |  |  | 23.80 | OK |
| sunrise_ridge | remastered | 26.72 | 26.06 | 0.26 | 0.54 | 0.30 | 0.05 | 0.15 | 0.04 | 28.20 | OK |
| sunrise_ridge | delta | -13.29 | -13.82 |  |  |  |  |  |  |  |  |
| pinewood_pan | classic | missing |  |  |  |  |  |  |  |  |  |
| pinewood_pan | remastered | missing |  |  |  |  |  |  |  |  |  |
