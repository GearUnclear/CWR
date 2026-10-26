# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | sky | skytable | post | resolve | lindepth | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| coastline_sunset_r1 | off | 4.18 | 1.57 | 0.00 |  |  | 2.34 | 0.42 |  | 0.27 | 0.17 | 1.25 | 0.23 | 229.70 | 175 | 0 | OK:30 | OK |
| coastline_sunset_r1 | on | 5.40 | 2.04 | 0.00 | 0.28 |  | 2.66 | 0.46 |  | 0.28 | 0.18 | 1.33 | 0.40 | 190.80 | 175 | 0 | OK:30 | OK |
| coastline_sunset_r1 | delta | +1.22 | +0.47 | +0.00 |  |  | +0.32 | +0.04 |  | +0.01 | +0.01 | +0.08 | +0.17 |  | +0.0% |  |  |  |
| sunrise_ridge_r1 | off | 28.67 | 27.43 | 0.08 |  |  | 1.08 | 0.53 |  | 0.13 | 0.06 | 0.27 | 0.09 | 30.60 | 12,910 | 8,897 | OK:30 | OK |
| sunrise_ridge_r1 | on | 31.65 | 30.23 | 0.08 | 0.10 |  | 1.20 | 0.66 |  | 0.13 | 0.06 | 0.27 | 0.08 | 28.90 | 12,910 | 8,897 | OK:30 | OK |
| sunrise_ridge_r1 | delta | +2.98 | +2.80 | +0.00 |  |  | +0.12 | +0.13 |  | +0.00 | +0.00 | +0.00 | -0.01 |  | +0.0% | +0.0% |  |  |
| noon_fields_r1 | off | 10.12 | 9.46 | 0.06 |  |  | 0.52 | 0.30 |  | 0.11 | 0.06 |  | 0.04 | 58.80 | 6,347 | 4,506 | OK:30 | OK |
| noon_fields_r1 | on | 10.78 | 10.03 | 0.06 | 0.08 |  | 0.55 | 0.33 |  | 0.11 | 0.06 |  | 0.04 | 57.30 | 6,347 | 4,506 | OK:30 | OK |
| noon_fields_r1 | delta | +0.66 | +0.57 | +0.00 |  |  | +0.03 | +0.03 |  | +0.00 | +0.00 |  | +0.00 |  | +0.0% | +0.0% |  |  |
| coastline_sunset_r2 | off | 3.86 | 1.30 | 0.00 |  |  | 2.32 | 0.52 |  | 0.22 | 0.14 | 0.98 | 0.46 | 248.30 | 175 | 0 | OK:30 | OK |
| coastline_sunset_r2 | on | 5.19 | 1.95 | 0.00 | 0.27 |  | 2.57 | 0.48 |  | 0.30 | 0.17 | 1.35 | 0.26 | 195.50 | 175 | 0 | OK:30 | OK |
| coastline_sunset_r2 | delta | +1.33 | +0.65 | +0.00 |  |  | +0.25 | -0.04 |  | +0.08 | +0.03 | +0.37 | -0.20 |  | +0.0% |  |  |  |
| sunrise_ridge_r2 | off | 28.58 | 27.20 | 0.08 |  |  | 1.22 | 0.64 |  | 0.13 | 0.06 | 0.27 | 0.12 | 30.90 | 12,910 | 8,897 | OK:30 | OK |
| sunrise_ridge_r2 | on | 30.97 | 29.45 | 0.08 | 0.10 |  | 1.29 | 0.71 |  | 0.13 | 0.06 | 0.27 | 0.12 | 29.40 | 12,910 | 8,897 | OK:30 | OK |
| sunrise_ridge_r2 | delta | +2.39 | +2.25 | +0.00 |  |  | +0.07 | +0.07 |  | +0.00 | +0.00 | +0.00 | +0.00 |  | +0.0% | +0.0% |  |  |
| noon_fields_r2 | off | 10.25 | 9.58 | 0.06 |  |  | 0.53 | 0.31 |  | 0.11 | 0.06 |  | 0.04 | 58.20 | 6,347 | 4,506 | OK:30 | OK |
| noon_fields_r2 | on | 10.45 | 9.72 | 0.06 | 0.08 |  | 0.53 | 0.31 |  | 0.11 | 0.06 |  | 0.04 | 55.50 | 6,347 | 4,506 | OK:30 | OK |
| noon_fields_r2 | delta | +0.20 | +0.14 | +0.00 |  |  | +0.00 | +0.00 |  | +0.00 | +0.00 |  | +0.00 |  | +0.0% | +0.0% |  |  |
| coastline_sunset_r3 | off | 3.98 | 1.55 | 0.00 |  |  | 2.18 | 0.44 |  | 0.22 | 0.15 | 1.03 | 0.34 | 242.50 | 175 | 0 | OK:30 | OK |
| coastline_sunset_r3 | on | 5.25 | 2.05 | 0.00 | 0.27 |  | 2.62 | 0.44 |  | 0.28 | 0.21 | 1.34 | 0.34 | 188.00 | 175 | 0 | OK:30 | OK |
| coastline_sunset_r3 | delta | +1.27 | +0.50 | +0.00 |  |  | +0.44 | +0.00 |  | +0.06 | +0.06 | +0.31 | +0.00 |  | +0.0% |  |  |  |
| sunrise_ridge_r3 | off | 28.18 | 26.85 | 0.09 |  |  | 1.17 | 0.63 |  | 0.13 | 0.06 | 0.27 | 0.09 | 31.60 | 12,910 | 8,897 | OK:30 | OK |
| sunrise_ridge_r3 | on | 28.77 | 27.46 | 0.08 | 0.10 |  | 1.09 | 0.56 |  | 0.13 | 0.06 | 0.27 | 0.07 | 31.60 | 12,910 | 8,897 | OK:30 | OK |
| sunrise_ridge_r3 | delta | +0.59 | +0.61 | -0.01 |  |  | -0.08 | -0.07 |  | +0.00 | +0.00 | +0.00 | -0.02 |  | +0.0% | +0.0% |  |  |
| noon_fields_r3 | off | 10.13 | 9.44 | 0.06 |  |  | 0.55 | 0.33 |  | 0.11 | 0.06 |  | 0.04 | 58.70 | 6,347 | 4,506 | OK:30 | OK |
| noon_fields_r3 | on | 11.05 | 10.29 | 0.06 | 0.08 |  | 0.57 | 0.34 |  | 0.11 | 0.06 |  | 0.05 | 57.20 | 6,347 | 4,506 | OK:30 | OK |
| noon_fields_r3 | delta | +0.92 | +0.85 | +0.00 |  |  | +0.02 | +0.01 |  | +0.00 | +0.00 |  | +0.01 |  | +0.0% | +0.0% |  |  |
