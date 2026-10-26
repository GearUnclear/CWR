# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| town_night | off | 7.23 | 6.20 |  | 0.27 | 1.57 | 4.37 | 0.78 | 0.49 |  | 0.17 | 0.07 |  | 0.04 | 5.95 | 109.30 | 1,323 | 1,146 | OK:273 | OK |
| town_night | on | 5.89 | 4.95 |  | 0.28 | 1.19 | 3.47 | 0.74 | 0.42 |  | 0.19 | 0.08 |  | 0.05 | 4.94 | 95.50 | 1,220 | 1,124 | OK:45 | OK |
| town_night | delta | -1.34 | -1.25 |  | +0.01 | -0.38 | -0.90 | -0.04 | -0.07 |  | +0.02 | +0.01 |  | +0.01 | -1.01 |  | -7.8% | -1.9% |  |  |
| noon_fields | off | 9.54 | 8.87 | 0.07 | 0.23 | 8.15 | 0.49 | 0.53 | 0.31 |  | 0.12 | 0.06 |  | 0.04 | 9.12 | 61.00 | 6,398 | 4,508 | OK:30 | OK |
| noon_fields | on | 5.95 | 5.23 | 0.11 | 0.34 | 4.49 | 0.41 | 0.59 | 0.33 |  | 0.14 | 0.07 |  | 0.05 | 4.13 | 82.80 | 1,566 | 4,508 | OK:30 | OK |
| noon_fields | delta | -3.59 | -3.64 | +0.04 | +0.11 | -3.66 | -0.08 | +0.06 | +0.02 |  | +0.02 | +0.01 |  | +0.01 | -4.99 |  | -75.5% | +0.0% |  |  |
| noon_walk | off | 9.26 | 8.62 | 0.09 | 0.23 | 7.87 | 0.52 | 0.50 | 0.28 |  | 0.11 | 0.06 |  | 0.04 | 8.58 | 64.60 | 6,310 | 4,396 | OK:30 | OK |
| noon_walk | on | 6.35 | 5.56 | 0.10 | 0.38 | 4.77 | 0.42 | 0.64 | 0.33 |  | 0.17 | 0.09 |  | 0.06 | 4.06 | 79.70 | 1,482 | 4,396 | OK:30 | OK |
| noon_walk | delta | -2.91 | -3.06 | +0.01 | +0.15 | -3.10 | -0.10 | +0.14 | +0.05 |  | +0.06 | +0.03 |  | +0.02 | -4.52 |  | -76.5% | +0.0% |  |  |
| sunrise_walk | off | 21.04 | 19.87 | 0.10 | 3.57 | 15.18 | 1.12 | 1.00 | 0.46 |  | 0.12 | 0.06 | 0.27 | 0.09 | 17.14 | 31.00 | 12,939 | 8,936 | OK:30 | OK |
| sunrise_walk | on | 5.13 | 3.35 | 0.25 | 0.63 | 2.18 | 0.54 | 1.52 | 0.36 |  | 0.23 | 0.13 | 0.61 | 0.19 | 2.33 | 64.10 | 1,937 | 8,936 | OK:30 | OK |
| sunrise_walk | delta | -15.91 | -16.52 | +0.15 | -2.94 | -13.00 | -0.58 | +0.52 | -0.10 |  | +0.11 | +0.07 | +0.34 | +0.10 | -14.81 |  | -85.0% | +0.0% |  |  |
| forest_edge_golden | off | 5.17 | 4.47 | 0.24 | 0.25 | 3.52 | 0.70 | 0.51 | 0.26 |  | 0.14 | 0.07 |  | 0.04 | 4.87 | 111.30 | 2,142 | 1,459 | OK:30 | OK |
| forest_edge_golden | on | 3.96 | 3.30 | 0.29 | 0.33 | 1.76 | 1.21 | 0.51 | 0.25 |  | 0.14 | 0.08 |  | 0.04 | 2.60 | 120.10 | 866 | 1,459 | OK:30 | OK |
| forest_edge_golden | delta | -1.21 | -1.17 | +0.05 | +0.08 | -1.76 | +0.51 | +0.00 | -0.01 |  | +0.00 | +0.01 |  | +0.00 | -2.27 |  | -59.6% | +0.0% |  |  |
| lowsun_fp | off | 8.88 | 7.39 | 0.07 | 0.23 | 3.90 | 3.26 | 1.35 | 0.15 |  | 0.14 | 0.06 | 0.37 | 0.63 | 4.29 | 105.50 | 1,890 | 1,316 | OK:30 | OK |
| lowsun_fp | on | 4.29 | 3.22 | 0.07 | 0.22 | 1.54 | 1.46 | 0.95 | 0.25 |  | 0.13 | 0.06 | 0.36 | 0.13 | 3.51 | 102.90 | 1,212 | 1,316 | OK:30 | OK |
| lowsun_fp | delta | -4.59 | -4.17 | +0.00 | -0.01 | -2.36 | -1.80 | -0.40 | +0.10 |  | -0.01 | +0.00 | -0.01 | -0.50 | -0.78 |  | -35.9% | +0.0% |  |  |
