# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| clouds_coast_oc_up | off | 3.67 | 2.06 | 0.00 | 0.96 | 0.00 | 1.10 | 0.14 |  | 1.29 | 0.18 |  | 0.06 | 0.06 | 0.35 | 0.43 | 1.91 | 287.20 | 186 | 0 | OK:30 | OK |
| clouds_coast_oc_up | on | 2.60 | 0.87 | 0.00 | 0.56 | 0.00 | 0.30 | 0.22 |  | 1.09 | 0.31 |  | 0.06 | 0.06 | 0.34 | 0.12 | 2.04 | 300.00 | 186 | 0 | OK:30 | OK |
| clouds_coast_oc_up | delta | -1.07 | -1.19 | +0.00 | -0.40 | +0.00 | -0.80 | +0.08 |  | -0.20 | +0.13 |  | +0.00 | +0.00 | -0.01 | -0.31 | +0.13 |  | +0.0% |  |  |  |
| clouds_sunrise_oc | off | 5.12 | 3.73 | 0.10 | 1.28 | 2.04 | 0.42 | 0.10 |  | 1.13 | 0.39 |  | 0.13 | 0.06 | 0.27 | 0.05 | 3.98 | 60.80 | 1,913 | 8,897 | OK:30 | OK |
| clouds_sunrise_oc | on | 5.90 | 3.96 | 0.08 | 1.57 | 2.03 | 0.35 | 0.16 |  | 1.15 | 0.42 |  | 0.13 | 0.06 | 0.27 | 0.04 | 4.21 | 64.20 | 1,913 | 8,897 | OK:30 | OK |
| clouds_sunrise_oc | delta | +0.78 | +0.23 | -0.02 | +0.29 | -0.01 | -0.07 | +0.06 |  | +0.02 | +0.03 |  | +0.00 | +0.00 | +0.00 | -0.01 | +0.23 |  | +0.0% | +0.0% |  |  |
| clouds_coast_oc | off | 2.98 | 1.39 | 0.00 | 0.63 | 0.00 | 0.77 | 0.09 |  | 1.38 | 0.22 |  | 0.10 | 0.06 | 0.29 | 0.49 | 1.79 | 334.90 | 177 | 0 | OK:30 | OK |
| clouds_coast_oc | on | 2.25 | 0.86 | 0.00 | 0.65 | 0.00 | 0.22 | 0.12 |  | 1.09 | 0.37 |  | 0.10 | 0.06 | 0.29 | 0.04 | 1.91 | 318.30 | 177 | 0 | OK:30 | OK |
| clouds_coast_oc | delta | -0.73 | -0.53 | +0.00 | +0.02 | +0.00 | -0.55 | +0.03 |  | -0.29 | +0.15 |  | +0.00 | +0.00 | +0.00 | -0.45 | +0.12 |  | +0.0% |  |  |  |
| clouds_coastline_clear | off | 3.00 | 1.28 | 0.00 | 0.79 | 0.00 | 0.49 | 0.09 |  | 1.51 | 0.38 |  | 0.11 | 0.06 | 0.44 | 0.28 | 1.94 | 289.60 | 175 | 0 | OK:30 | OK |
| clouds_coastline_clear | on | 3.38 | 1.36 | 0.00 | 0.83 | 0.00 | 0.53 | 0.10 |  | 1.78 | 0.39 |  | 0.12 | 0.07 | 0.49 | 0.46 | 2.13 | 273.90 | 175 | 0 | OK:30 | OK |
| clouds_coastline_clear | delta | +0.38 | +0.08 | +0.00 | +0.04 | +0.00 | +0.04 | +0.01 |  | +0.27 | +0.01 |  | +0.01 | +0.01 | +0.05 | +0.18 | +0.19 |  | +0.0% |  |  |  |
| clouds_noon_fields_oc | off | 11.58 | 10.55 | 0.07 | 1.45 | 8.20 | 0.89 | 0.09 |  | 0.81 | 0.30 |  | 0.12 | 0.07 |  | 0.03 | 6.24 | 67.00 | 1,515 | 4,506 | OK:30 | OK |
| clouds_noon_fields_oc | on | 6.92 | 5.87 | 0.06 | 1.04 | 4.44 | 0.40 | 0.12 |  | 0.71 | 0.28 |  | 0.11 | 0.06 |  | 0.03 | 6.40 | 77.40 | 1,515 | 4,506 | OK:30 | OK |
| clouds_noon_fields_oc | delta | -4.66 | -4.68 | -0.01 | -0.41 | -3.76 | -0.49 | +0.03 |  | -0.10 | -0.02 |  | -0.01 | -0.01 |  | +0.00 | +0.16 |  | +0.0% | +0.0% |  |  |
