# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| perf_sunrise_ridge | off | 34.76 | 33.13 | 0.42 |  |  |  | 1.47 | 0.61 |  | 0.13 | 0.06 | 0.27 | 0.40 |  | 25.40 | 12,910 | 8,897 | OK:30 | OK |
| perf_sunrise_ridge | on | 33.47 | 31.93 | 0.35 |  |  |  | 1.38 | 0.56 |  | 0.13 | 0.06 | 0.27 | 0.36 |  | 24.60 | 12,910 | 8,897 | OK:30 | OK |
| perf_sunrise_ridge | delta | -1.29 | -1.20 | -0.07 |  |  |  | -0.09 | -0.05 |  | +0.00 | +0.00 | +0.00 | -0.04 |  |  | +0.0% | +0.0% |  |  |
| a4hill_sunrise_ridge | off | 29.78 | 28.03 | 0.35 |  |  |  | 1.58 | 0.43 |  | 0.13 | 0.06 | 0.27 | 0.69 |  | 26.10 | 12,910 | 8,897 | OK:30 | OK |
| a4hill_sunrise_ridge | on | 29.51 | 27.70 | 0.43 |  |  |  | 1.64 | 0.49 |  | 0.13 | 0.06 | 0.27 | 0.69 |  | 24.60 | 12,910 | 8,897 | OK:30 | OK |
| a4hill_sunrise_ridge | delta | -0.27 | -0.33 | +0.08 |  |  |  | +0.06 | +0.06 |  | +0.00 | +0.00 | +0.00 | +0.00 |  |  | +0.0% | +0.0% |  |  |
| a4group_sunrise_ridge | off | 29.93 | 28.05 | 0.10 |  |  |  | 1.72 | 1.10 |  | 0.13 | 0.06 | 0.27 | 0.16 |  | 23.50 | 12,910 | 8,897 | OK:30 | OK |
| a4group_sunrise_ridge | on | 19.82 | 18.62 | 0.09 |  |  |  | 1.03 | 0.43 |  | 0.13 | 0.06 | 0.27 | 0.14 |  | 32.80 | 12,910 | 8,897 | OK:30 | OK |
| a4group_sunrise_ridge | delta | -10.11 | -9.43 | -0.01 |  |  |  | -0.69 | -0.67 |  | +0.00 | +0.00 | +0.00 | -0.02 |  |  | +0.0% | +0.0% |  |  |
| a4sunvis_sunrise_ridge | off | 17.96 | 16.77 | 0.08 |  |  |  | 1.03 | 0.50 |  | 0.13 | 0.06 | 0.27 | 0.08 |  | 37.40 | 12,910 | 8,897 | OK:30 | OK |
| a4sunvis_sunrise_ridge | on | 17.69 | 16.54 | 0.09 |  |  |  | 0.99 | 0.44 |  | 0.13 | 0.06 | 0.27 | 0.10 |  | 37.40 | 12,910 | 8,897 | OK:30 | OK |
| a4sunvis_sunrise_ridge | delta | -0.27 | -0.23 | +0.01 |  |  |  | -0.04 | -0.06 |  | +0.00 | +0.00 | +0.00 | +0.02 |  |  | +0.0% | +0.0% |  |  |
| a4walk_sunrise_end | off | 19.63 | 18.38 | 0.09 |  |  |  | 1.09 | 0.51 |  | 0.13 | 0.06 | 0.27 | 0.11 |  | 33.90 | 13,712 | 9,656 | OK:30 | OK |
| a4walk_sunrise_end | on | 19.54 | 18.33 | 0.07 |  |  |  | 1.05 | 0.47 |  | 0.13 | 0.06 | 0.27 | 0.11 |  | 34.20 | 13,712 | 9,656 | OK:30 | OK |
| a4walk_sunrise_end | delta | -0.09 | -0.05 | -0.02 |  |  |  | -0.04 | -0.04 |  | +0.00 | +0.00 | +0.00 | +0.00 |  |  | +0.0% | +0.0% |  |  |
| a4valley_0515 | off | 23.99 | 22.63 | 0.04 |  |  |  | 1.20 | 0.68 |  | 0.06 | 0.07 | 0.31 | 0.09 |  | 31.40 | 11,406 | 7,966 | OK:30 | OK |
| a4valley_0515 | on | 24.02 | 22.73 | 0.04 |  |  |  | 1.15 | 0.62 |  | 0.06 | 0.07 | 0.31 | 0.09 |  | 31.30 | 11,406 | 7,966 | OK:30 | OK |
| a4valley_0515 | delta | +0.03 | +0.10 | +0.00 |  |  |  | -0.05 | -0.06 |  | +0.00 | +0.00 | +0.00 | +0.00 |  |  | +0.0% | +0.0% |  |  |
