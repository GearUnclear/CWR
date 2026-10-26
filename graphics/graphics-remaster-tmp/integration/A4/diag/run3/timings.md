# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| d_same_off | off | 19.31 | 18.16 | 0.08 | 0.29 | 15.71 | 2.15 | 0.99 | 0.49 |  | 0.13 | 0.06 | 0.26 | 0.05 | 18.75 | 34.80 | 12,910 | 8,897 | OK:30 | OK |
| d_same_off | on | 17.57 | 16.41 | 0.09 | 0.25 | 15.08 | 1.08 | 1.00 | 0.46 |  | 0.13 | 0.06 | 0.26 | 0.09 | 16.93 | 38.00 | 12,910 | 8,897 | OK:30 | OK |
| d_same_off | delta | -1.74 | -1.75 | +0.01 | -0.04 | -0.63 | -1.07 | +0.01 | -0.03 |  | +0.00 | +0.00 | +0.00 | +0.04 | -1.82 |  | +0.0% | +0.0% |  |  |
| d_same_on | off | 3.09 | 1.87 | 0.11 | 0.29 | 1.29 | 0.29 | 1.04 | 0.38 |  | 0.16 | 0.07 | 0.32 | 0.11 | 2.33 | 69.90 | 1,903 | 8,897 | OK:30 | OK |
| d_same_on | on | 4.96 | 3.23 | 0.18 | 0.54 | 2.14 | 0.55 | 1.48 | 0.48 |  | 0.25 | 0.12 | 0.50 | 0.14 | 3.90 | 63.40 | 1,903 | 8,897 | OK:30 | OK |
| d_same_on | delta | +1.87 | +1.36 | +0.07 | +0.25 | +0.85 | +0.26 | +0.44 | +0.10 |  | +0.09 | +0.05 | +0.18 | +0.03 | +1.57 |  | +0.0% | +0.0% |  |  |
| d_hill | off | 19.28 | 18.11 | 0.08 | 0.29 | 15.64 | 2.18 | 1.00 | 0.47 |  | 0.13 | 0.06 | 0.26 | 0.08 | 18.65 | 34.70 | 12,910 | 8,897 | OK:30 | OK |
| d_hill | on | 5.12 | 3.79 | 0.12 | 0.53 | 2.90 | 0.36 | 1.13 | 0.36 |  | 0.19 | 0.09 | 0.38 | 0.11 | 4.36 | 67.30 | 3,016 | 8,897 | OK:30 | OK |
| d_hill | delta | -14.16 | -14.32 | +0.04 | +0.24 | -12.74 | -1.82 | +0.13 | -0.11 |  | +0.06 | +0.03 | +0.12 | +0.03 | -14.29 |  | -76.6% | +0.0% |  |  |
| d_group | off | 19.45 | 18.27 | 0.08 | 0.33 | 15.76 | 2.19 | 1.02 | 0.52 |  | 0.13 | 0.06 | 0.27 | 0.05 | 18.71 | 34.50 | 12,910 | 8,897 | OK:30 | OK |
| d_group | on | 15.95 | 14.85 | 0.08 | 0.23 | 13.58 | 1.04 | 0.96 | 0.44 |  | 0.13 | 0.06 | 0.27 | 0.07 | 15.44 | 40.10 | 7,551 | 8,897 | OK:30 | OK |
| d_group | delta | -3.50 | -3.42 | +0.00 | -0.10 | -2.18 | -1.15 | -0.06 | -0.08 |  | +0.00 | +0.00 | +0.00 | +0.02 | -3.27 |  | -41.5% | +0.0% |  |  |
| d_both | off | 17.63 | 16.50 | 0.08 | 0.28 | 15.16 | 1.06 | 0.97 | 0.45 |  | 0.13 | 0.06 | 0.27 | 0.07 | 17.09 | 37.50 | 12,910 | 8,897 | OK:30 | OK |
| d_both | on | 3.73 | 2.41 | 0.14 | 0.41 | 1.62 | 0.37 | 1.11 | 0.33 |  | 0.20 | 0.10 | 0.40 | 0.08 | 2.33 | 68.30 | 1,903 | 8,897 | OK:30 | OK |
| d_both | delta | -13.90 | -14.09 | +0.06 | +0.13 | -13.54 | -0.69 | +0.14 | -0.12 |  | +0.07 | +0.04 | +0.13 | +0.01 | -14.76 |  | -85.3% | +0.0% |  |  |
