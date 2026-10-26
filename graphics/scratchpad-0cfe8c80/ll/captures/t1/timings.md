# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | off | 7.02 | 5.63 | 0.17 | 1.02 | 2.67 | 1.93 | 0.10 |  |  | 1.17 | 0.29 |  | 0.14 | 0.07 | 0.26 | 0.19 | 6.37 | 75.70 | 1,212 | 1,316 | OK:30 | OK |
| lowsun_fp | on | 18.10 | 15.56 | 0.73 | 2.16 | 7.28 | 6.12 | 0.09 |  |  | 2.31 | 0.17 |  | 0.14 | 0.07 | 0.26 | 1.45 | 7.24 | 50.80 | 1,212 | 1,316 | OK:30 | OK |
| lowsun_fp | delta | +11.08 | +9.93 | +0.56 | +1.14 | +4.61 | +4.19 | -0.01 |  |  | +1.14 | -0.12 |  | +0.00 | +0.00 | +0.00 | +1.26 | +0.87 |  | +0.0% | +0.0% |  |  |
| noon_fields | off | 16.98 | 15.41 | 0.22 | 2.51 | 11.47 | 1.43 | 0.10 |  |  | 1.35 | 0.38 |  | 0.12 | 0.07 |  | 0.04 | 7.55 | 48.00 | 1,566 | 4,508 | OK:30 | OK |
| noon_fields | on | 14.48 | 12.93 | 0.19 | 1.85 | 9.39 | 1.69 | 0.10 |  |  | 1.33 | 0.15 |  | 0.12 | 0.07 |  | 0.77 | 7.76 | 52.40 | 1,566 | 4,508 | OK:30 | OK |
| noon_fields | delta | -2.50 | -2.48 | -0.03 | -0.66 | -2.08 | +0.26 | +0.00 |  |  | -0.02 | -0.23 |  | +0.00 | +0.00 |  | +0.73 | +0.21 |  | +0.0% | +0.0% |  |  |
| sunrise_ridge | off | 6.07 | 4.68 | 0.10 | 1.85 | 2.19 | 0.63 | 0.11 |  |  | 1.13 | 0.39 |  | 0.13 | 0.07 | 0.20 | 0.10 | 4.58 | 50.50 | 1,912 | 8,897 | OK:30 | OK |
| sunrise_ridge | on | 12.28 | 9.36 | 0.08 | 2.81 | 5.40 | 1.15 | 0.11 |  |  | 2.65 | 0.14 |  | 0.13 | 0.07 | 0.20 | 1.87 | 5.48 | 38.60 | 1,912 | 8,897 | OK:30 | OK |
| sunrise_ridge | delta | +6.21 | +4.68 | -0.02 | +0.96 | +3.21 | +0.52 | +0.00 |  |  | +1.52 | -0.25 |  | +0.00 | +0.00 | +0.00 | +1.77 | +0.90 |  | +0.0% | +0.0% |  |  |
| coastline_sunset | off | 7.12 | 5.10 | 0.00 | 3.06 | 0.00 | 2.04 | 0.09 |  |  | 1.82 | 0.18 |  | 0.10 | 0.07 | 0.21 | 0.99 | 2.91 | 149.50 | 177 | 0 | OK:30 | OK |
| coastline_sunset | on | 7.83 | 4.81 | 0.00 | 2.76 | 0.00 | 2.04 | 0.09 |  |  | 2.82 | 0.15 |  | 0.10 | 0.07 | 0.21 | 2.06 | 2.69 | 116.40 | 177 | 0 | OK:30 | OK |
| coastline_sunset | delta | +0.71 | -0.29 | +0.00 | -0.30 | +0.00 | +0.00 | +0.00 |  |  | +1.00 | -0.03 |  | +0.00 | +0.00 | +0.00 | +1.07 | -0.22 |  | +0.0% |  |  |  |
| town_night | off | 22.64 | 20.89 |  | 2.50 | 6.90 | 11.49 | 0.08 |  |  | 1.56 | 0.22 |  | 0.16 | 0.07 |  | 0.04 | 7.01 | 45.50 | 1,142 | 1,055 | OK:178 | OK |
| town_night | on | 18.46 | 16.81 |  | 2.09 | 5.36 | 9.36 | 0.07 |  |  | 1.39 | 0.18 |  | 0.16 | 0.07 |  | 0.73 | 9.29 | 46.20 | 1,185 | 1,078 | OK:62 | OK |
| town_night | delta | -4.18 | -4.08 |  | -0.41 | -1.54 | -2.13 | -0.01 |  |  | -0.17 | -0.04 |  | +0.00 | +0.00 |  | +0.69 | +2.28 |  | +3.6% | +2.1% |  |  |
