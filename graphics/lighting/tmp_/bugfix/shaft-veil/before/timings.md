# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 2.93 | 2.83 |  |  |  |  |  |  | 124.70 | OK |
| lowsun_fp | remastered | 7.10 | 5.83 | 0.05 | 1.16 | 0.40 | 0.05 | 0.22 | 0.49 | 83.10 | OK |
| lowsun_fp | delta | +4.17 | +3.00 |  |  |  |  |  |  |  |  |
| lowsun_fp_noshafts | classic | missing |  |  |  |  |  |  |  |  |  |
| lowsun_fp_noshafts | remastered | 5.07 | 4.32 | 0.04 | 0.63 | 0.56 | 0.05 |  | 0.03 | 99.70 | OK |
| sunrise_ridge | classic | 28.44 | 28.32 |  |  |  |  |  |  | 32.50 | OK |
| sunrise_ridge | remastered | 31.18 | 29.87 | 0.00 | 1.19 | 0.96 | 0.05 | 0.15 | 0.03 | 25.40 | OK |
| sunrise_ridge | delta | +2.74 | +1.55 |  |  |  |  |  |  |  |  |
| sunrise_ridge_noshafts | classic | missing |  |  |  |  |  |  |  |  |  |
| sunrise_ridge_noshafts | remastered | 28.59 | 27.47 | 0.00 | 0.99 | 0.92 | 0.05 |  | 0.03 | 27.10 | OK |
| sinai_lowsun_fp | classic | 4.21 | 4.07 |  |  |  |  |  |  | 116.40 | OK |
| sinai_lowsun_fp | remastered | 5.33 | 3.90 | 0.07 | 1.31 | 0.93 | 0.05 | 0.24 | 0.08 | 77.60 | OK |
| sinai_lowsun_fp | delta | +1.12 | -0.17 |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp_noshafts | classic | missing |  |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp_noshafts | remastered | 6.53 | 5.18 | 0.09 | 1.23 | 1.14 | 0.06 |  | 0.03 | 78.90 | OK |
