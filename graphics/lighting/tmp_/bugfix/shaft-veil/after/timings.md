# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 5.94 | 5.83 |  |  |  |  |  |  | 108.80 | OK |
| lowsun_fp | remastered | 9.63 | 8.29 | 0.38 | 1.22 | 0.36 | 0.05 | 0.22 | 0.59 | 76.30 | OK |
| lowsun_fp | delta | +3.69 | +2.46 |  |  |  |  |  |  |  |  |
| lowsun_fp_noshafts | classic | missing |  |  |  |  |  |  |  |  |  |
| lowsun_fp_noshafts | remastered | 10.53 | 9.87 | 0.25 | 0.55 | 0.47 | 0.05 |  | 0.03 | 68.30 | OK |
| sunrise_ridge | classic | 29.79 | 29.67 |  |  |  |  |  |  | 24.60 | OK |
| sunrise_ridge | remastered | 27.45 | 26.25 | 0.00 | 1.07 | 0.84 | 0.05 | 0.15 | 0.03 | 21.70 | OK |
| sunrise_ridge | delta | -2.34 | -3.42 |  |  |  |  |  |  |  |  |
| sunrise_ridge_noshafts | classic | missing |  |  |  |  |  |  |  |  |  |
| sunrise_ridge_noshafts | remastered | 25.85 | 24.97 | 0.00 | 0.74 | 0.66 | 0.05 |  | 0.03 | 26.80 | OK |
| sinai_lowsun_fp | classic | 3.38 | 3.26 |  |  |  |  |  |  | 81.50 | OK |
| sinai_lowsun_fp | remastered | 7.32 | 5.58 | 0.09 | 1.62 | 1.30 | 0.05 | 0.22 | 0.04 | 69.70 | OK |
| sinai_lowsun_fp | delta | +3.94 | +2.32 |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp_noshafts | classic | missing |  |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp_noshafts | remastered | 14.37 | 13.09 | 0.16 | 1.16 | 1.08 | 0.05 |  | 0.03 | 41.30 | OK |
