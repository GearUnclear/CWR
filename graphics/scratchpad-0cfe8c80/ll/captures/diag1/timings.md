# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | off | 31.22 | 25.76 | 1.52 | 4.02 | 12.90 | 8.84 | 0.13 |  |  | 4.10 | 0.14 | 0.01 | 0.14 | 0.06 |  | 0.17 | 23.29 | 29.10 | 1,212 | 1,316 | OK:34 | OK |
| lowsun_fp | on | 17.08 | 14.45 | 0.17 | 2.85 | 7.16 | 4.44 | 0.10 |  |  | 2.38 | 0.19 | 0.01 | 0.13 | 0.06 |  | 0.61 | 7.54 | 43.70 | 1,212 | 1,316 | OK:36 | OK |
| lowsun_fp | delta | -14.14 | -11.31 | -1.35 | -1.17 | -5.74 | -4.40 | -0.03 |  |  | -1.72 | +0.05 | +0.00 | -0.01 | +0.00 |  | +0.44 | -15.75 |  | +0.0% | +0.0% |  |  |
| sunrise_ridge | off | 8.03 | 5.98 | 0.08 | 2.43 | 3.02 | 0.53 | 0.11 |  |  | 1.74 | 0.41 | 0.01 | 0.13 | 0.06 |  | 0.05 | 5.01 | 57.60 | 1,913 | 8,897 | OK:37 | OK |
| sunrise_ridge | on | 9.92 | 7.45 | 0.08 | 2.44 | 3.82 | 1.19 | 0.11 |  |  | 2.17 | 0.15 | 0.01 | 0.13 | 0.07 |  | 0.39 | 5.12 | 58.80 | 1,913 | 8,897 | OK:35 | OK |
| sunrise_ridge | delta | +1.89 | +1.47 | +0.00 | +0.01 | +0.80 | +0.66 | +0.00 |  |  | +0.43 | -0.26 | +0.00 | +0.00 | +0.01 |  | +0.34 | +0.11 |  | +0.0% | +0.0% |  |  |
| coastline_sunset | off | 5.54 | 2.62 | 0.00 | 2.28 | 0.00 | 0.34 | 0.16 |  |  | 2.56 | 0.23 | 0.02 | 0.16 | 0.11 |  | 0.06 | 4.92 | 190.80 | 177 | 0 | OK:37 | OK |
| coastline_sunset | on | 5.83 | 2.91 | 0.00 | 2.40 | 0.00 | 0.51 | 0.16 |  |  | 2.55 | 0.23 | 0.02 | 0.17 | 0.13 |  | 0.13 | 5.46 | 180.50 | 177 | 0 | OK:36 | OK |
| coastline_sunset | delta | +0.29 | +0.29 | +0.00 | +0.12 | +0.00 | +0.17 | +0.00 |  |  | -0.01 | +0.00 | +0.00 | +0.01 | +0.02 |  | +0.07 | +0.54 |  | +0.0% |  |  |  |
| town_night | off | 9.51 | 8.29 | 0.16 | 1.05 | 3.14 | 4.10 | 0.08 |  |  | 1.01 | 0.40 | 0.01 | 0.15 | 0.06 |  | 0.04 | 9.20 | 78.90 | 1,121 | 1,018 | OK:214 | OK |
| town_night | on | 11.83 | 10.11 | 0.20 | 1.35 | 3.57 | 5.20 | 0.08 |  |  | 1.39 | 0.16 | 0.01 | 0.16 | 0.12 |  | 0.54 | 9.47 | 59.40 | 1,124 | 1,018 | OK:196 | OK |
| town_night | delta | +2.32 | +1.82 | +0.04 | +0.30 | +0.43 | +1.10 | +0.00 |  |  | +0.38 | -0.24 | +0.00 | +0.01 | +0.06 |  | +0.50 | +0.27 |  | +0.3% | +0.0% |  |  |
