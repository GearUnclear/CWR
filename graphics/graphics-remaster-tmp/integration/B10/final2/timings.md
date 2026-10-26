# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| clouds_coast_oc_up | off | 2.88 | 1.36 | 0.00 | 0.78 | 0.00 | 0.58 | 0.16 |  | 1.18 | 0.30 |  | 0.06 | 0.07 | 0.39 | 0.13 | 1.96 | 268.20 | 186 | 0 | OK:30 | OK |
| clouds_coast_oc_up | on | 2.71 | 0.93 | 0.00 | 0.61 | 0.00 | 0.31 | 0.24 |  | 1.13 | 0.31 |  | 0.06 | 0.07 | 0.38 | 0.10 | 2.24 | 258.20 | 186 | 0 | OK:30 | OK |
| clouds_coast_oc_up | delta | -0.17 | -0.43 | +0.00 | -0.17 | +0.00 | -0.27 | +0.08 |  | -0.05 | +0.01 |  | +0.00 | +0.00 | -0.01 | -0.03 | +0.28 |  | +0.0% |  |  |  |
| clouds_sunrise_oc | off | 4.86 | 3.40 | 0.09 | 1.07 | 1.86 | 0.47 | 0.11 |  | 1.18 | 0.41 |  | 0.14 | 0.07 | 0.28 | 0.06 | 3.99 | 67.10 | 1,913 | 8,897 | OK:30 | OK |
| clouds_sunrise_oc | on | 5.02 | 3.33 | 0.08 | 1.03 | 1.89 | 0.42 | 0.16 |  | 1.24 | 0.43 |  | 0.13 | 0.06 | 0.27 | 0.12 | 4.21 | 61.70 | 1,913 | 8,897 | OK:30 | OK |
| clouds_sunrise_oc | delta | +0.16 | -0.07 | -0.01 | -0.04 | +0.03 | -0.05 | +0.05 |  | +0.06 | +0.02 |  | -0.01 | -0.01 | -0.01 | +0.06 | +0.22 |  | +0.0% | +0.0% |  |  |
| clouds_coast_oc | off | 5.75 | 3.66 | 0.00 | 1.64 | 0.00 | 2.02 | 0.16 |  | 1.72 | 0.33 |  | 0.17 | 0.21 | 0.47 | 0.16 | 2.94 | 196.90 | 177 | 0 | OK:30 | OK |
| clouds_coast_oc | on | 4.53 | 2.11 | 0.00 | 1.55 | 0.00 | 0.56 | 0.23 |  | 1.59 | 0.36 |  | 0.17 | 0.13 | 0.47 | 0.08 | 2.82 | 236.00 | 177 | 0 | OK:30 | OK |
| clouds_coast_oc | delta | -1.22 | -1.55 | +0.00 | -0.09 | +0.00 | -1.46 | +0.07 |  | -0.13 | +0.03 |  | +0.00 | -0.08 | +0.00 | -0.08 | -0.12 |  | +0.0% |  |  |  |
| clouds_coastline_clear | off | 3.39 | 1.21 | 0.00 | 0.92 | 0.00 | 0.29 | 0.14 |  | 1.84 | 0.37 |  | 0.16 | 0.13 | 0.67 | 0.10 | 2.99 | 215.10 | 175 | 0 | OK:30 | OK |
| clouds_coastline_clear | on | 4.46 | 1.99 | 0.00 | 1.56 | 0.00 | 0.42 | 0.17 |  | 2.03 | 0.33 |  | 0.21 | 0.19 | 0.75 | 0.16 | 3.29 | 226.20 | 175 | 0 | OK:30 | OK |
| clouds_coastline_clear | delta | +1.07 | +0.78 | +0.00 | +0.64 | +0.00 | +0.13 | +0.03 |  | +0.19 | -0.04 |  | +0.05 | +0.06 | +0.08 | +0.06 | +0.30 |  | +0.0% |  |  |  |
| clouds_noon_fields_oc | off | 6.99 | 5.94 | 0.08 | 0.94 | 4.58 | 0.42 | 0.09 |  | 0.83 | 0.36 |  | 0.12 | 0.06 |  | 0.03 | 6.20 | 81.10 | 1,515 | 4,506 | OK:30 | OK |
| clouds_noon_fields_oc | on | 7.01 | 5.99 | 0.06 | 1.03 | 4.58 | 0.38 | 0.12 |  | 0.68 | 0.25 |  | 0.11 | 0.06 |  | 0.03 | 6.45 | 78.50 | 1,515 | 4,506 | OK:30 | OK |
| clouds_noon_fields_oc | delta | +0.02 | +0.05 | -0.02 | +0.09 | +0.00 | -0.04 | +0.03 |  | -0.15 | -0.11 |  | -0.01 | +0.00 |  | +0.00 | +0.25 |  | +0.0% | +0.0% |  |  |
| clouds_broken_high | off | 25.74 | 24.48 | 0.04 | 7.56 | 15.42 | 1.50 | 0.06 |  | 1.08 | 0.67 |  | 0.06 | 0.07 |  | 0.03 | 24.70 | 34.60 | 5,479 | 7,549 | OK:30 | OK |
| clouds_broken_high | on | 27.41 | 26.03 | 0.04 | 8.16 | 16.33 | 1.54 | 0.08 |  | 1.14 | 0.73 |  | 0.06 | 0.07 |  | 0.03 | 25.45 | 33.80 | 5,479 | 7,549 | OK:30 | OK |
| clouds_broken_high | delta | +1.67 | +1.55 | +0.00 | +0.60 | +0.91 | +0.04 | +0.02 |  | +0.06 | +0.06 |  | +0.00 | +0.00 |  | +0.00 | +0.75 |  | +0.0% | +0.0% |  |  |
| clouds_overcast_broken | off | 7.95 | 6.83 | 0.23 | 1.52 | 4.71 | 0.61 | 0.08 |  | 0.90 | 0.44 |  | 0.12 | 0.07 |  | 0.03 | 7.13 | 64.30 | 2,214 | 5,692 | OK:30 | OK |
| clouds_overcast_broken | on | 7.99 | 6.77 | 0.23 | 1.74 | 4.49 | 0.54 | 0.12 |  | 0.89 | 0.44 |  | 0.12 | 0.07 |  | 0.03 | 7.26 | 67.40 | 2,214 | 5,692 | OK:30 | OK |
| clouds_overcast_broken | delta | +0.04 | -0.06 | +0.00 | +0.22 | -0.22 | -0.07 | +0.04 |  | -0.01 | +0.00 |  | +0.00 | +0.00 |  | +0.00 | +0.13 |  | +0.0% | +0.0% |  |  |
| clouds_broken_up | off | 6.12 | 4.44 | 0.31 | 1.61 | 2.23 | 0.59 | 0.19 |  | 1.25 | 0.42 |  | 0.06 | 0.07 | 0.42 | 0.05 | 5.47 | 90.30 | 1,767 | 3,474 | OK:30 | OK |
| clouds_broken_up | on | 6.34 | 4.36 | 0.33 | 1.51 | 2.24 | 0.62 | 0.30 |  | 1.31 | 0.50 |  | 0.05 | 0.07 | 0.41 | 0.05 | 5.74 | 85.00 | 1,767 | 3,474 | OK:30 | OK |
| clouds_broken_up | delta | +0.22 | -0.08 | +0.02 | -0.10 | +0.01 | +0.03 | +0.11 |  | +0.06 | +0.08 |  | -0.01 | +0.00 | -0.01 | +0.00 | +0.27 |  | +0.0% | +0.0% |  |  |
| clouds_overcast_heavy | off | 6.48 | 5.33 | 0.22 | 1.19 | 3.66 | 0.49 | 0.09 |  | 0.91 | 0.45 |  | 0.12 | 0.07 |  | 0.03 | 5.89 | 73.90 | 1,888 | 5,691 | OK:30 | OK |
| clouds_overcast_heavy | on | 6.84 | 5.53 | 0.24 | 1.25 | 3.77 | 0.51 | 0.15 |  | 0.92 | 0.46 |  | 0.12 | 0.07 |  | 0.03 | 6.21 | 73.50 | 1,888 | 5,691 | OK:30 | OK |
| clouds_overcast_heavy | delta | +0.36 | +0.20 | +0.02 | +0.06 | +0.11 | +0.02 | +0.06 |  | +0.01 | +0.01 |  | +0.00 | +0.00 |  | +0.00 | +0.32 |  | +0.0% | +0.0% |  |  |
| clouds_sun_up | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| clouds_sun_up | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| clouds_ground_sweep | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| clouds_ground_sweep | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| clouds_noon_fields_clear | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| clouds_noon_fields_clear | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_1 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 145.40 | 1,291 | 740 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 6.79 | 5.32 | 0.21 | 1.00 | 2.29 | 2.04 | 0.11 |  | 1.26 | 0.37 |  | 0.14 | 0.08 | 0.35 | 0.08 | 2.33 | 127.10 | 763 | 740 |  | OK |
| smoke_3 | classic | 2.65 | 2.44 | 0.19 | 0.16 | 1.39 | 0.89 | 0.10 |  | 1.25 | 0.38 |  | 0.14 | 0.08 | 0.34 | 0.09 | 2.06 | 120.80 | 1,291 | 740 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 6.00 | 4.62 | 0.18 | 0.89 | 2.06 | 1.67 | 0.09 |  | 1.20 | 0.38 |  | 0.13 | 0.08 | 0.33 | 0.08 | 2.06 | 114.20 | 763 | 740 |  | OK |
