# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| perf_sunrise_ridge | off | 17.95 | 16.73 | 0.08 | 0.25 | 15.08 | 1.40 | 1.06 | 0.50 |  | 0.13 | 0.06 | 0.27 | 0.10 | 17.41 | 36.60 | 12,910 | 8,897 | OK:30 | OK |
| perf_sunrise_ridge | on | 5.39 | 4.13 | 0.12 | 0.34 | 3.01 | 0.77 | 1.09 | 0.37 |  | 0.18 | 0.09 | 0.38 | 0.08 | 4.12 | 66.20 | 2,273 | 8,897 | OK:30 | OK |
| perf_sunrise_ridge | delta | -12.56 | -12.60 | +0.04 | +0.09 | -12.07 | -0.63 | +0.03 | -0.13 |  | +0.05 | +0.03 | +0.11 | -0.02 | -13.29 |  | -82.4% | +0.0% |  |  |
| a4hill_sunrise_ridge | off | 17.61 | 16.41 | 0.08 | 0.28 | 15.07 | 1.07 | 1.04 | 0.51 |  | 0.13 | 0.06 | 0.27 | 0.07 | 16.86 | 37.20 | 12,910 | 8,897 | OK:30 | OK |
| a4hill_sunrise_ridge | on | 5.92 | 4.81 | 0.09 | 0.26 | 3.87 | 0.68 | 0.94 | 0.37 |  | 0.14 | 0.07 | 0.30 | 0.06 | 5.31 | 63.50 | 3,386 | 8,897 | OK:30 | OK |
| a4hill_sunrise_ridge | delta | -11.69 | -11.60 | +0.01 | -0.02 | -11.20 | -0.39 | -0.10 | -0.14 |  | +0.01 | +0.01 | +0.03 | -0.01 | -11.55 |  | -73.8% | +0.0% |  |  |
| a4group_sunrise_ridge | off | 17.48 | 16.34 | 0.08 | 0.27 | 14.98 | 1.09 | 0.97 | 0.45 |  | 0.13 | 0.06 | 0.27 | 0.06 | 16.95 | 37.30 | 12,910 | 8,897 | OK:30 | OK |
| a4group_sunrise_ridge | on | 16.06 | 15.00 | 0.08 | 0.23 | 13.62 | 1.15 | 0.93 | 0.42 |  | 0.13 | 0.06 | 0.27 | 0.05 | 15.34 | 40.00 | 7,551 | 8,897 | OK:30 | OK |
| a4group_sunrise_ridge | delta | -1.42 | -1.34 | +0.00 | -0.04 | -1.36 | +0.06 | -0.04 | -0.03 |  | +0.00 | +0.00 | +0.00 | -0.01 | -1.61 |  | -41.5% | +0.0% |  |  |
| a4sunvis_sunrise_ridge | off | 17.69 | 16.51 | 0.10 | 0.27 | 15.18 | 1.07 | 1.02 | 0.46 |  | 0.13 | 0.06 | 0.26 | 0.10 | 17.04 | 37.70 | 12,910 | 8,897 | OK:30 | OK |
| a4sunvis_sunrise_ridge | on | 5.64 | 4.35 | 0.12 | 0.34 | 3.20 | 0.81 | 1.12 | 0.39 |  | 0.18 | 0.09 | 0.39 | 0.07 | 4.33 | 65.30 | 2,273 | 8,897 | OK:30 | OK |
| a4sunvis_sunrise_ridge | delta | -12.05 | -12.16 | +0.02 | +0.07 | -11.98 | -0.26 | +0.10 | -0.07 |  | +0.05 | +0.03 | +0.13 | -0.03 | -12.71 |  | -82.4% | +0.0% |  |  |
| a4walk_sunrise_end | off | 19.61 | 18.39 | 0.07 | 0.24 | 16.99 | 1.16 | 1.06 | 0.51 |  | 0.13 | 0.06 | 0.27 | 0.08 | 18.92 | 33.60 | 13,712 | 9,656 | OK:30 | OK |
| a4walk_sunrise_end | on | 6.04 | 4.60 | 0.12 | 0.39 | 3.26 | 0.95 | 1.23 | 0.37 |  | 0.20 | 0.12 | 0.41 | 0.14 | 3.34 | 60.80 | 2,093 | 9,656 | OK:30 | OK |
| a4walk_sunrise_end | delta | -13.57 | -13.79 | +0.05 | +0.15 | -13.73 | -0.21 | +0.17 | -0.14 |  | +0.07 | +0.06 | +0.14 | +0.06 | -15.58 |  | -84.7% | +0.0% |  |  |
| a4valley_0515 | off | 31.11 | 29.70 | 0.04 | 6.96 | 20.97 | 1.77 | 1.27 | 0.73 |  | 0.06 | 0.07 | 0.31 | 0.10 | 29.90 | 30.40 | 11,406 | 7,966 | OK:30 | OK |
| a4valley_0515 | on | 16.64 | 15.40 | 0.04 | 0.58 | 13.51 | 1.31 | 1.12 | 0.55 |  | 0.06 | 0.07 | 0.31 | 0.14 | 15.79 | 40.30 | 5,242 | 7,966 | OK:30 | OK |
| a4valley_0515 | delta | -14.47 | -14.30 | +0.00 | -6.38 | -7.46 | -0.46 | -0.15 | -0.18 |  | +0.00 | +0.00 | +0.00 | +0.04 | -14.11 |  | -54.0% | +0.0% |  |  |
| smoke_1 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 179.90 | 1,291 | 740 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 4.60 | 3.39 | 0.10 | 0.44 | 1.36 | 1.58 | 1.10 | 0.25 |  | 0.17 | 0.11 | 0.48 | 0.09 | 2.05 | 159.30 | 763 | 740 |  | OK |
| smoke_3 | classic | 3.40 | 3.04 | 0.10 | 0.32 | 1.81 | 0.92 | 1.26 | 0.29 |  | 0.17 | 0.11 | 0.49 | 0.19 | 2.05 | 145.60 | 1,291 | 740 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 6.34 | 4.46 | 0.16 | 0.74 | 1.65 | 2.07 | 1.55 | 0.64 |  | 0.18 | 0.14 | 0.45 | 0.14 | 2.05 | 136.20 | 763 | 740 |  | OK |
