# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| perf_sunrise_ridge | off | 19.34 | 18.16 | 0.08 | 0.29 | 15.75 | 2.12 | 1.02 | 0.50 |  | 0.13 | 0.06 | 0.27 | 0.06 | 18.81 | 34.30 | 12,910 | 8,897 | OK:30 | OK |
| perf_sunrise_ridge | on | 3.91 | 2.60 | 0.14 | 0.53 | 1.68 | 0.39 | 1.12 | 0.33 |  | 0.19 | 0.10 | 0.40 | 0.10 | 2.36 | 67.10 | 1,903 | 8,897 | OK:30 | OK |
| perf_sunrise_ridge | delta | -15.43 | -15.56 | +0.06 | +0.24 | -14.07 | -1.73 | +0.10 | -0.17 |  | +0.06 | +0.04 | +0.13 | +0.04 | -16.45 |  | -85.3% | +0.0% |  |  |
| a4hill_sunrise_ridge | off | 19.36 | 18.18 | 0.08 | 0.26 | 15.81 | 2.11 | 1.02 | 0.48 |  | 0.13 | 0.06 | 0.27 | 0.08 | 18.70 | 34.80 | 12,910 | 8,897 | OK:30 | OK |
| a4hill_sunrise_ridge | on | 5.15 | 3.74 | 0.12 | 0.39 | 3.00 | 0.35 | 1.20 | 0.37 |  | 0.19 | 0.09 | 0.40 | 0.15 | 4.28 | 68.20 | 3,016 | 8,897 | OK:30 | OK |
| a4hill_sunrise_ridge | delta | -14.21 | -14.44 | +0.04 | +0.13 | -12.81 | -1.76 | +0.18 | -0.11 |  | +0.06 | +0.03 | +0.13 | +0.07 | -14.42 |  | -76.6% | +0.0% |  |  |
| a4group_sunrise_ridge | off | 19.43 | 18.24 | 0.08 | 0.28 | 15.77 | 2.19 | 1.04 | 0.53 |  | 0.13 | 0.06 | 0.27 | 0.05 | 18.71 | 34.60 | 12,910 | 8,897 | OK:30 | OK |
| a4group_sunrise_ridge | on | 16.03 | 14.94 | 0.08 | 0.23 | 13.67 | 1.04 | 0.94 | 0.44 |  | 0.13 | 0.06 | 0.27 | 0.05 | 15.48 | 40.20 | 7,551 | 8,897 | OK:30 | OK |
| a4group_sunrise_ridge | delta | -3.40 | -3.30 | +0.00 | -0.05 | -2.10 | -1.15 | -0.10 | -0.09 |  | +0.00 | +0.00 | +0.00 | +0.00 | -3.23 |  | -41.5% | +0.0% |  |  |
| a4sunvis_sunrise_ridge | off | 17.50 | 16.37 | 0.09 | 0.26 | 14.99 | 1.13 | 0.97 | 0.44 |  | 0.13 | 0.06 | 0.27 | 0.07 | 16.88 | 37.70 | 12,910 | 8,897 | OK:30 | OK |
| a4sunvis_sunrise_ridge | on | 3.63 | 2.33 | 0.14 | 0.38 | 1.60 | 0.36 | 1.12 | 0.34 |  | 0.19 | 0.09 | 0.40 | 0.09 | 2.32 | 67.80 | 1,903 | 8,897 | OK:30 | OK |
| a4sunvis_sunrise_ridge | delta | -13.87 | -14.04 | +0.05 | +0.12 | -13.39 | -0.77 | +0.15 | -0.10 |  | +0.06 | +0.03 | +0.13 | +0.02 | -14.56 |  | -85.3% | +0.0% |  |  |
| a4walk_sunrise_end | off | 21.41 | 20.21 | 0.07 | 0.23 | 17.73 | 2.25 | 1.04 | 0.51 |  | 0.13 | 0.06 | 0.27 | 0.06 | 20.68 | 31.30 | 13,712 | 9,656 | OK:30 | OK |
| a4walk_sunrise_end | on | 3.52 | 2.00 | 0.12 | 0.37 | 1.26 | 0.37 | 1.31 | 0.46 |  | 0.19 | 0.10 | 0.41 | 0.14 | 2.09 | 61.10 | 1,705 | 9,656 | OK:30 | OK |
| a4walk_sunrise_end | delta | -17.89 | -18.21 | +0.05 | +0.14 | -16.47 | -1.88 | +0.27 | -0.05 |  | +0.06 | +0.04 | +0.14 | +0.08 | -18.59 |  | -87.6% | +0.0% |  |  |
| a4valley_0515 | off | 31.09 | 29.76 | 0.04 | 7.02 | 21.02 | 1.71 | 1.19 | 0.69 |  | 0.06 | 0.07 | 0.31 | 0.07 | 29.72 | 30.50 | 11,406 | 7,966 | OK:30 | OK |
| a4valley_0515 | on | 16.48 | 15.34 | 0.04 | 0.47 | 13.58 | 1.29 | 1.02 | 0.53 |  | 0.06 | 0.07 | 0.31 | 0.06 | 15.45 | 40.40 | 5,242 | 7,966 | OK:30 | OK |
| a4valley_0515 | delta | -14.61 | -14.42 | +0.00 | -6.55 | -7.44 | -0.42 | -0.17 | -0.16 |  | +0.00 | +0.00 | +0.00 | -0.01 | -14.27 |  | -54.0% | +0.0% |  |  |
| smoke_1 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 185.90 | 1,291 | 740 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 4.58 | 3.32 | 0.10 | 0.41 | 1.37 | 1.54 | 1.15 | 0.29 |  | 0.17 | 0.11 | 0.48 | 0.10 | 2.53 | 161.40 | 763 | 740 |  | OK |
| smoke_3 | classic | 3.67 | 3.28 | 0.10 | 0.38 | 1.92 | 0.99 | 1.14 | 0.28 |  | 0.17 | 0.11 | 0.49 | 0.09 | 2.53 | 145.40 | 1,291 | 740 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 6.40 | 4.51 | 0.17 | 0.70 | 1.83 | 1.98 | 1.62 | 0.69 |  | 0.17 | 0.13 | 0.41 | 0.22 | 2.53 | 134.00 | 763 | 740 |  | OK |
