# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | sky | skytable | post | resolve | lindepth | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| coastline_sunset_r1 | off | 4.18 | 1.60 | 0.00 |  |  | 2.32 | 0.33 |  | 0.25 | 0.16 | 1.23 | 0.34 | 265.40 | 175 | 0 | OK:30 | OK |
| coastline_sunset_r1 | on | 5.07 | 2.05 | 0.00 | 0.28 |  | 2.52 | 0.33 |  | 0.28 | 0.29 | 1.34 | 0.28 | 212.20 | 175 | 0 | OK:30 | OK |
| coastline_sunset_r1 | delta | +0.89 | +0.45 | +0.00 |  |  | +0.20 | +0.00 |  | +0.03 | +0.13 | +0.11 | -0.06 |  | +0.0% |  |  |  |
| sunrise_ridge_r1 | off | 24.91 | 23.70 | 0.08 |  |  | 1.05 | 0.50 |  | 0.13 | 0.06 | 0.27 | 0.10 | 37.20 | 12,910 | 8,897 | OK:30 | OK |
| sunrise_ridge_r1 | on | 24.81 | 23.62 | 0.08 | 0.10 |  | 0.99 | 0.47 |  | 0.13 | 0.06 | 0.27 | 0.06 | 36.90 | 12,910 | 8,897 | OK:30 | OK |
| sunrise_ridge_r1 | delta | -0.10 | -0.08 | +0.00 |  |  | -0.06 | -0.03 |  | +0.00 | +0.00 | +0.00 | -0.04 |  | +0.0% | +0.0% |  |  |
| noon_fields_r1 | off | 9.43 | 8.86 | 0.06 |  |  | 0.43 | 0.22 |  | 0.11 | 0.06 |  | 0.04 | 67.90 | 6,347 | 4,506 | OK:30 | OK |
| noon_fields_r1 | on | 10.07 | 9.43 | 0.06 | 0.08 |  | 0.46 | 0.24 |  | 0.11 | 0.06 |  | 0.04 | 65.80 | 6,347 | 4,506 | OK:30 | OK |
| noon_fields_r1 | delta | +0.64 | +0.57 | +0.00 |  |  | +0.03 | +0.02 |  | +0.00 | +0.00 |  | +0.00 |  | +0.0% | +0.0% |  |  |
| coastline_sunset_r2 | off | 3.53 | 1.39 | 0.00 |  |  | 1.90 | 0.31 |  | 0.23 | 0.15 | 1.06 | 0.16 | 283.40 | 175 | 0 | OK:30 | OK |
| coastline_sunset_r2 | on | 4.81 | 1.90 | 0.00 | 0.28 |  | 2.38 | 0.33 |  | 0.28 | 0.18 | 1.31 | 0.27 | 212.00 | 175 | 0 | OK:30 | OK |
| coastline_sunset_r2 | delta | +1.28 | +0.51 | +0.00 |  |  | +0.48 | +0.02 |  | +0.05 | +0.03 | +0.25 | +0.11 |  | +0.0% |  |  |  |
| sunrise_ridge_r2 | off | 24.37 | 23.23 | 0.08 |  |  | 0.98 | 0.44 |  | 0.13 | 0.06 | 0.27 | 0.09 | 37.30 | 12,910 | 8,897 | OK:30 | OK |
| sunrise_ridge_r2 | on | 24.86 | 23.66 | 0.08 | 0.10 |  | 1.00 | 0.44 |  | 0.13 | 0.06 | 0.27 | 0.11 | 36.80 | 12,910 | 8,897 | OK:30 | OK |
| sunrise_ridge_r2 | delta | +0.49 | +0.43 | +0.00 |  |  | +0.02 | +0.00 |  | +0.00 | +0.00 | +0.00 | +0.02 |  | +0.0% | +0.0% |  |  |
| noon_fields_r2 | off | 9.54 | 8.96 | 0.06 |  |  | 0.44 | 0.23 |  | 0.11 | 0.06 |  | 0.04 | 68.20 | 6,347 | 4,506 | OK:30 | OK |
| noon_fields_r2 | on | 9.98 | 9.39 | 0.06 | 0.09 |  | 0.42 | 0.20 |  | 0.11 | 0.06 |  | 0.04 | 66.00 | 6,347 | 4,506 | OK:30 | OK |
| noon_fields_r2 | delta | +0.44 | +0.43 | +0.00 |  |  | -0.02 | -0.03 |  | +0.00 | +0.00 |  | +0.00 |  | +0.0% | +0.0% |  |  |
| coastline_sunset_r3 | off | 3.49 | 1.27 | 0.00 |  |  | 1.98 | 0.33 |  | 0.22 | 0.15 | 1.05 | 0.22 | 282.00 | 175 | 0 | OK:30 | OK |
| coastline_sunset_r3 | on | 5.17 | 2.20 | 0.00 | 0.28 |  | 2.47 | 0.37 |  | 0.28 | 0.18 | 1.32 | 0.33 | 215.10 | 175 | 0 | OK:30 | OK |
| coastline_sunset_r3 | delta | +1.68 | +0.93 | +0.00 |  |  | +0.49 | +0.04 |  | +0.06 | +0.03 | +0.27 | +0.11 |  | +0.0% |  |  |  |
| sunrise_ridge_r3 | off | 24.33 | 23.19 | 0.08 |  |  | 0.98 | 0.44 |  | 0.13 | 0.06 | 0.27 | 0.08 | 37.10 | 12,910 | 8,897 | OK:30 | OK |
| sunrise_ridge_r3 | on | 24.94 | 23.71 | 0.08 | 0.10 |  | 1.03 | 0.49 |  | 0.13 | 0.06 | 0.27 | 0.08 | 37.20 | 12,910 | 8,897 | OK:30 | OK |
| sunrise_ridge_r3 | delta | +0.61 | +0.52 | +0.00 |  |  | +0.05 | +0.05 |  | +0.00 | +0.00 | +0.00 | +0.00 |  | +0.0% | +0.0% |  |  |
| noon_fields_r3 | off | 9.58 | 8.97 | 0.06 |  |  | 0.47 | 0.25 |  | 0.11 | 0.06 |  | 0.04 | 67.70 | 6,347 | 4,506 | OK:30 | OK |
| noon_fields_r3 | on | 10.14 | 9.50 | 0.06 | 0.08 |  | 0.46 | 0.24 |  | 0.11 | 0.06 |  | 0.04 | 64.70 | 6,347 | 4,506 | OK:30 | OK |
| noon_fields_r3 | delta | +0.56 | +0.53 | +0.00 |  |  | -0.01 | -0.01 |  | +0.00 | +0.00 |  | +0.00 |  | +0.0% | +0.0% |  |  |
