# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| d_same_off | off | 19.36 | 18.16 | 0.08 | 0.27 | 15.78 | 2.11 | 1.04 | 0.53 |  | 0.13 | 0.06 | 0.27 | 0.05 | 18.86 | 34.50 | 12,910 | 8,897 | OK:30 | OK |
| d_same_off | on | 17.56 | 16.40 | 0.08 | 0.29 | 15.02 | 1.09 | 0.99 | 0.44 |  | 0.13 | 0.06 | 0.27 | 0.09 | 17.08 | 37.70 | 12,910 | 8,897 | OK:30 | OK |
| d_same_off | delta | -1.80 | -1.76 | +0.00 | +0.02 | -0.76 | -1.02 | -0.05 | -0.09 |  | +0.00 | +0.00 | +0.00 | +0.04 | -1.78 |  | +0.0% | +0.0% |  |  |
| d_same_on | off | 3.50 | 2.21 | 0.12 | 0.36 | 1.53 | 0.33 | 1.09 | 0.36 |  | 0.17 | 0.08 | 0.36 | 0.11 | 2.34 | 70.70 | 1,895 | 8,897 | OK:30 | OK |
| d_same_on | on | 4.88 | 3.12 | 0.18 | 0.50 | 2.05 | 0.58 | 1.51 | 0.41 |  | 0.22 | 0.12 | 0.51 | 0.25 | 3.93 | 65.80 | 1,895 | 8,897 | OK:30 | OK |
| d_same_on | delta | +1.38 | +0.91 | +0.06 | +0.14 | +0.52 | +0.25 | +0.42 | +0.05 |  | +0.05 | +0.04 | +0.15 | +0.14 | +1.59 |  | +0.0% | +0.0% |  |  |
| d_hill | off | 19.49 | 18.30 | 0.09 | 0.29 | 15.80 | 2.21 | 1.02 | 0.52 |  | 0.13 | 0.06 | 0.27 | 0.05 | 18.74 | 34.60 | 12,910 | 8,897 | OK:30 | OK |
| d_hill | on | 5.09 | 3.73 | 0.12 | 0.38 | 2.97 | 0.37 | 1.16 | 0.36 |  | 0.19 | 0.09 | 0.39 | 0.13 | 4.26 | 69.70 | 3,008 | 8,897 | OK:30 | OK |
| d_hill | delta | -14.40 | -14.57 | +0.03 | +0.09 | -12.83 | -1.84 | +0.14 | -0.16 |  | +0.06 | +0.03 | +0.12 | +0.08 | -14.48 |  | -76.7% | +0.0% |  |  |
| d_group | off | 19.36 | 18.21 | 0.08 | 0.25 | 15.78 | 2.18 | 0.99 | 0.48 |  | 0.13 | 0.06 | 0.27 | 0.06 | 18.74 | 34.60 | 12,910 | 8,897 | OK:30 | OK |
| d_group | on | 16.08 | 14.99 | 0.08 | 0.23 | 13.73 | 1.03 | 0.94 | 0.44 |  | 0.13 | 0.06 | 0.27 | 0.05 | 15.40 | 40.00 | 7,551 | 8,897 | OK:30 | OK |
| d_group | delta | -3.28 | -3.22 | +0.00 | -0.02 | -2.05 | -1.15 | -0.05 | -0.04 |  | +0.00 | +0.00 | +0.00 | -0.01 | -3.34 |  | -41.5% | +0.0% |  |  |
| d_both | off | 17.57 | 16.45 | 0.08 | 0.27 | 15.15 | 1.03 | 0.95 | 0.43 |  | 0.13 | 0.06 | 0.27 | 0.07 | 17.03 | 37.60 | 12,910 | 8,897 | OK:30 | OK |
| d_both | on | 3.70 | 2.35 | 0.14 | 0.38 | 1.63 | 0.34 | 1.16 | 0.37 |  | 0.19 | 0.10 | 0.40 | 0.10 | 2.33 | 70.30 | 1,895 | 8,897 | OK:30 | OK |
| d_both | delta | -13.87 | -14.10 | +0.06 | +0.11 | -13.52 | -0.69 | +0.21 | -0.06 |  | +0.06 | +0.04 | +0.13 | +0.03 | -14.70 |  | -85.3% | +0.0% |  |  |
