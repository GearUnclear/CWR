# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| d_same_off | off | 19.36 | 18.17 | 0.08 | 0.29 | 15.71 | 2.17 | 1.03 | 0.52 |  | 0.13 | 0.06 | 0.27 | 0.05 | 18.71 | 34.60 | 12,910 | 8,897 | OK:30 | OK |
| d_same_off | on | 17.44 | 16.30 | 0.11 | 0.28 | 14.93 | 1.09 | 0.97 | 0.43 |  | 0.13 | 0.06 | 0.27 | 0.08 | 16.90 | 37.70 | 12,910 | 8,897 | OK:30 | OK |
| d_same_off | delta | -1.92 | -1.87 | +0.03 | -0.01 | -0.78 | -1.08 | -0.06 | -0.09 |  | +0.00 | +0.00 | +0.00 | +0.03 | -1.81 |  | +0.0% | +0.0% |  |  |
| d_same_on | off | 3.62 | 2.31 | 0.13 | 0.41 | 1.57 | 0.33 | 1.14 | 0.36 |  | 0.18 | 0.10 | 0.38 | 0.11 | 2.33 | 71.10 | 1,895 | 8,897 | OK:30 | OK |
| d_same_on | on | 4.67 | 2.95 | 0.18 | 0.53 | 1.93 | 0.49 | 1.48 | 0.41 |  | 0.22 | 0.12 | 0.51 | 0.22 | 3.89 | 65.20 | 1,895 | 8,897 | OK:30 | OK |
| d_same_on | delta | +1.05 | +0.64 | +0.05 | +0.12 | +0.36 | +0.16 | +0.34 | +0.05 |  | +0.04 | +0.02 | +0.13 | +0.11 | +1.56 |  | +0.0% | +0.0% |  |  |
| d_hill | off | 19.23 | 18.08 | 0.08 | 0.28 | 15.64 | 2.16 | 0.99 | 0.48 |  | 0.13 | 0.06 | 0.27 | 0.05 | 18.70 | 34.70 | 12,910 | 8,897 | OK:30 | OK |
| d_hill | on | 5.13 | 3.84 | 0.12 | 0.40 | 2.98 | 0.46 | 1.09 | 0.33 |  | 0.18 | 0.09 | 0.38 | 0.10 | 4.28 | 71.00 | 3,008 | 8,897 | OK:30 | OK |
| d_hill | delta | -14.10 | -14.24 | +0.04 | +0.12 | -12.66 | -1.70 | +0.10 | -0.15 |  | +0.05 | +0.03 | +0.11 | +0.05 | -14.42 |  | -76.7% | +0.0% |  |  |
| d_group | off | 19.35 | 18.17 | 0.08 | 0.27 | 15.76 | 2.13 | 1.02 | 0.50 |  | 0.13 | 0.06 | 0.27 | 0.07 | 18.67 | 34.60 | 12,910 | 8,897 | OK:30 | OK |
| d_group | on | 16.06 | 14.81 | 0.08 | 0.24 | 13.51 | 1.06 | 1.11 | 0.50 |  | 0.13 | 0.06 | 0.27 | 0.15 | 15.25 | 40.00 | 7,551 | 8,897 | OK:30 | OK |
| d_group | delta | -3.29 | -3.36 | +0.00 | -0.03 | -2.25 | -1.07 | +0.09 | +0.00 |  | +0.00 | +0.00 | +0.00 | +0.08 | -3.42 |  | -41.5% | +0.0% |  |  |
| d_both | off | 17.63 | 16.43 | 0.10 | 0.27 | 15.09 | 1.07 | 1.03 | 0.47 |  | 0.13 | 0.06 | 0.27 | 0.10 | 17.00 | 37.60 | 12,910 | 8,897 | OK:30 | OK |
| d_both | on | 3.75 | 2.30 | 0.14 | 0.38 | 1.58 | 0.34 | 1.28 | 0.42 |  | 0.19 | 0.09 | 0.40 | 0.17 | 2.33 | 70.80 | 1,895 | 8,897 | OK:30 | OK |
| d_both | delta | -13.88 | -14.13 | +0.04 | +0.11 | -13.51 | -0.73 | +0.25 | -0.05 |  | +0.06 | +0.03 | +0.13 | +0.07 | -14.67 |  | -85.3% | +0.0% |  |  |
