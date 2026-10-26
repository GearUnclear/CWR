# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| perf_sunrise_ridge | off | 18.05 | 16.87 | 0.08 | 0.25 | 15.19 | 1.43 | 1.02 | 0.51 |  | 0.13 | 0.06 | 0.27 | 0.06 | 17.38 | 36.80 | 12,910 | 8,897 | OK:30 | OK |
| perf_sunrise_ridge | on | 5.89 | 4.64 | 0.15 | 0.33 | 3.42 | 0.89 | 1.09 | 0.42 |  | 0.17 | 0.08 | 0.34 | 0.07 | 4.92 | 65.90 | 2,280 | 8,897 | OK:30 | OK |
| perf_sunrise_ridge | delta | -12.16 | -12.23 | +0.07 | +0.08 | -11.77 | -0.54 | +0.07 | -0.09 |  | +0.04 | +0.02 | +0.07 | +0.01 | -12.46 |  | -82.3% | +0.0% |  |  |
| a4hill_sunrise_ridge | off | 17.67 | 16.46 | 0.08 | 0.28 | 15.08 | 1.10 | 1.05 | 0.48 |  | 0.13 | 0.06 | 0.27 | 0.12 | 16.79 | 37.80 | 12,910 | 8,897 | OK:30 | OK |
| a4hill_sunrise_ridge | on | 6.12 | 4.90 | 0.10 | 0.34 | 3.03 | 1.53 | 1.03 | 0.35 |  | 0.16 | 0.08 | 0.34 | 0.10 | 5.32 | 63.50 | 3,393 | 8,897 | OK:30 | OK |
| a4hill_sunrise_ridge | delta | -11.55 | -11.56 | +0.02 | +0.06 | -12.05 | +0.43 | -0.02 | -0.13 |  | +0.03 | +0.02 | +0.07 | -0.02 | -11.47 |  | -73.7% | +0.0% |  |  |
| a4group_sunrise_ridge | off | 17.54 | 16.35 | 0.10 | 0.25 | 15.01 | 1.09 | 1.02 | 0.46 |  | 0.13 | 0.06 | 0.27 | 0.10 | 16.99 | 37.70 | 12,910 | 8,897 | OK:30 | OK |
| a4group_sunrise_ridge | on | 15.98 | 14.90 | 0.08 | 0.23 | 13.62 | 1.04 | 0.94 | 0.44 |  | 0.13 | 0.06 | 0.27 | 0.05 | 15.32 | 40.20 | 7,551 | 8,897 | OK:30 | OK |
| a4group_sunrise_ridge | delta | -1.56 | -1.45 | -0.02 | -0.02 | -1.39 | -0.05 | -0.08 | -0.02 |  | +0.00 | +0.00 | +0.00 | -0.05 | -1.67 |  | -41.5% | +0.0% |  |  |
| a4sunvis_sunrise_ridge | off | 17.43 | 16.31 | 0.08 | 0.28 | 14.95 | 1.08 | 0.96 | 0.42 |  | 0.13 | 0.06 | 0.26 | 0.09 | 16.96 | 37.70 | 12,910 | 8,897 | OK:30 | OK |
| a4sunvis_sunrise_ridge | on | 6.26 | 4.98 | 0.11 | 0.32 | 3.60 | 1.05 | 1.12 | 0.36 |  | 0.17 | 0.08 | 0.40 | 0.11 | 4.99 | 64.60 | 2,280 | 8,897 | OK:30 | OK |
| a4sunvis_sunrise_ridge | delta | -11.17 | -11.33 | +0.03 | +0.04 | -11.35 | -0.03 | +0.16 | -0.06 |  | +0.04 | +0.02 | +0.14 | +0.02 | -11.97 |  | -82.3% | +0.0% |  |  |
| a4walk_sunrise_end | off | 19.62 | 18.38 | 0.07 | 0.23 | 17.01 | 1.14 | 1.07 | 0.51 |  | 0.13 | 0.06 | 0.27 | 0.10 | 18.88 | 34.10 | 13,712 | 9,656 | OK:30 | OK |
| a4walk_sunrise_end | on | 6.05 | 4.88 | 0.10 | 0.35 | 3.64 | 0.89 | 1.01 | 0.33 |  | 0.17 | 0.08 | 0.36 | 0.06 | 5.36 | 60.50 | 2,100 | 9,656 | OK:30 | OK |
| a4walk_sunrise_end | delta | -13.57 | -13.50 | +0.03 | +0.12 | -13.37 | -0.25 | -0.06 | -0.18 |  | +0.04 | +0.02 | +0.09 | -0.04 | -13.52 |  | -84.7% | +0.0% |  |  |
| a4valley_0515 | off | 24.06 | 22.73 | 0.04 | 0.47 | 20.71 | 1.55 | 1.18 | 0.67 |  | 0.06 | 0.07 | 0.31 | 0.07 | 22.94 | 31.30 | 11,406 | 7,966 | OK:30 | OK |
| a4valley_0515 | on | 16.84 | 15.56 | 0.04 | 0.52 | 13.71 | 1.33 | 1.15 | 0.61 |  | 0.06 | 0.07 | 0.31 | 0.11 | 15.84 | 40.10 | 5,323 | 7,966 | OK:30 | OK |
| a4valley_0515 | delta | -7.22 | -7.17 | +0.00 | +0.05 | -7.00 | -0.22 | -0.03 | -0.06 |  | +0.00 | +0.00 | +0.00 | +0.04 | -7.10 |  | -53.3% | +0.0% |  |  |
| smoke_1 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 180.10 | 1,291 | 740 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 4.70 | 3.46 | 0.10 | 0.41 | 1.36 | 1.69 | 1.16 | 0.25 |  | 0.17 | 0.14 | 0.47 | 0.13 | 1.91 | 159.60 | 763 | 740 |  | OK |
| smoke_3 | classic | 2.98 | 2.72 | 0.10 | 0.19 | 1.59 | 0.93 | 1.21 | 0.24 |  | 0.17 | 0.12 | 0.47 | 0.20 | 1.91 | 148.10 | 1,291 | 740 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 5.29 | 3.79 | 0.13 | 0.53 | 1.53 | 1.74 | 1.38 | 0.28 |  | 0.20 | 0.12 | 0.56 | 0.22 | 1.91 | 142.30 | 763 | 740 |  | OK |
