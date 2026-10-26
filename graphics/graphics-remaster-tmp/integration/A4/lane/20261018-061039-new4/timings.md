# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| perf_sunrise_ridge | off | 19.32 | 18.16 | 0.08 | 0.31 | 15.69 | 2.16 | 1.00 | 0.48 |  | 0.13 | 0.06 | 0.27 | 0.06 | 18.71 | 34.40 | 12,910 | 8,897 | OK:30 | OK |
| perf_sunrise_ridge | on | 3.85 | 2.52 | 0.12 | 0.41 | 1.78 | 0.33 | 1.15 | 0.38 |  | 0.19 | 0.10 | 0.39 | 0.10 | 3.18 | 71.80 | 1,912 | 8,897 | OK:30 | OK |
| perf_sunrise_ridge | delta | -15.47 | -15.64 | +0.04 | +0.10 | -13.91 | -1.83 | +0.15 | -0.10 |  | +0.06 | +0.04 | +0.12 | +0.04 | -15.53 |  | -85.2% | +0.0% |  |  |
| a4hill_sunrise_ridge | off | 19.34 | 18.18 | 0.08 | 0.28 | 15.73 | 2.17 | 0.99 | 0.47 |  | 0.13 | 0.06 | 0.27 | 0.07 | 18.78 | 34.50 | 12,910 | 8,897 | OK:30 | OK |
| a4hill_sunrise_ridge | on | 5.31 | 3.99 | 0.12 | 0.43 | 3.20 | 0.37 | 1.12 | 0.37 |  | 0.19 | 0.09 | 0.40 | 0.08 | 4.34 | 65.30 | 3,025 | 8,897 | OK:30 | OK |
| a4hill_sunrise_ridge | delta | -14.03 | -14.19 | +0.04 | +0.15 | -12.53 | -1.80 | +0.13 | -0.10 |  | +0.06 | +0.03 | +0.13 | +0.01 | -14.44 |  | -76.6% | +0.0% |  |  |
| a4group_sunrise_ridge | off | 19.43 | 18.24 | 0.08 | 0.28 | 15.82 | 2.14 | 1.03 | 0.50 |  | 0.13 | 0.06 | 0.27 | 0.07 | 18.75 | 34.20 | 12,910 | 8,897 | OK:30 | OK |
| a4group_sunrise_ridge | on | 16.03 | 14.89 | 0.08 | 0.23 | 13.62 | 1.04 | 1.00 | 0.46 |  | 0.13 | 0.06 | 0.27 | 0.08 | 15.31 | 39.60 | 7,551 | 8,897 | OK:30 | OK |
| a4group_sunrise_ridge | delta | -3.40 | -3.35 | +0.00 | -0.05 | -2.20 | -1.10 | -0.03 | -0.04 |  | +0.00 | +0.00 | +0.00 | +0.01 | -3.44 |  | -41.5% | +0.0% |  |  |
| a4sunvis_sunrise_ridge | off | 17.52 | 16.37 | 0.08 | 0.28 | 15.00 | 1.09 | 0.99 | 0.48 |  | 0.13 | 0.06 | 0.27 | 0.06 | 17.02 | 37.30 | 12,910 | 8,897 | OK:30 | OK |
| a4sunvis_sunrise_ridge | on | 3.96 | 2.62 | 0.13 | 0.37 | 1.85 | 0.40 | 1.14 | 0.36 |  | 0.19 | 0.10 | 0.41 | 0.08 | 3.22 | 68.60 | 1,912 | 8,897 | OK:30 | OK |
| a4sunvis_sunrise_ridge | delta | -13.56 | -13.75 | +0.05 | +0.09 | -13.15 | -0.69 | +0.15 | -0.12 |  | +0.06 | +0.04 | +0.14 | +0.02 | -13.80 |  | -85.2% | +0.0% |  |  |
| a4walk_sunrise_end | off | 21.66 | 20.46 | 0.07 | 0.28 | 17.92 | 2.25 | 1.03 | 0.51 |  | 0.13 | 0.06 | 0.27 | 0.06 | 20.76 | 31.10 | 13,712 | 9,656 | OK:30 | OK |
| a4walk_sunrise_end | on | 3.46 | 2.01 | 0.12 | 0.36 | 1.21 | 0.44 | 1.24 | 0.42 |  | 0.19 | 0.10 | 0.41 | 0.12 | 2.10 | 61.00 | 1,714 | 9,656 | OK:30 | OK |
| a4walk_sunrise_end | delta | -18.20 | -18.45 | +0.05 | +0.08 | -16.71 | -1.81 | +0.21 | -0.09 |  | +0.06 | +0.04 | +0.14 | +0.06 | -18.66 |  | -87.5% | +0.0% |  |  |
| a4valley_0515 | off | 31.10 | 29.71 | 0.04 | 7.07 | 20.91 | 1.73 | 1.26 | 0.73 |  | 0.06 | 0.07 | 0.31 | 0.09 | 29.97 | 30.40 | 11,406 | 7,966 | OK:30 | OK |
| a4valley_0515 | on | 16.49 | 15.28 | 0.04 | 0.50 | 13.49 | 1.29 | 1.09 | 0.57 |  | 0.06 | 0.07 | 0.31 | 0.08 | 15.68 | 40.50 | 5,242 | 7,966 | OK:30 | OK |
| a4valley_0515 | delta | -14.61 | -14.43 | +0.00 | -6.57 | -7.42 | -0.44 | -0.17 | -0.16 |  | +0.00 | +0.00 | +0.00 | -0.01 | -14.29 |  | -54.0% | +0.0% |  |  |
| smoke_1 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 190.60 | 1,291 | 740 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 4.69 | 3.44 | 0.10 | 0.40 | 1.34 | 1.70 | 1.14 | 0.28 |  | 0.17 | 0.11 | 0.48 | 0.11 | 2.05 | 163.50 | 763 | 740 |  | OK |
| smoke_3 | classic | 2.86 | 2.60 | 0.10 | 0.20 | 1.58 | 0.82 | 1.14 | 0.28 |  | 0.17 | 0.11 | 0.48 | 0.09 | 2.05 | 148.10 | 1,291 | 740 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 5.57 | 3.94 | 0.12 | 0.51 | 1.56 | 1.86 | 1.51 | 0.29 |  | 0.20 | 0.12 | 0.57 | 0.32 | 2.05 | 140.10 | 763 | 740 |  | OK |
