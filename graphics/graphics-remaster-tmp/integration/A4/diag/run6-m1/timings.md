# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| d_same_off | off | 19.20 | 18.05 | 0.08 | 0.26 | 15.67 | 2.11 | 0.99 | 0.45 |  | 0.13 | 0.06 | 0.27 | 0.08 | 18.72 | 34.70 | 12,910 | 8,897 | OK:30 | OK |
| d_same_off | on | 17.51 | 16.34 | 0.10 | 0.30 | 14.93 | 1.11 | 1.01 | 0.45 |  | 0.13 | 0.06 | 0.27 | 0.10 | 17.03 | 37.70 | 12,910 | 8,897 | OK:30 | OK |
| d_same_off | delta | -1.69 | -1.71 | +0.02 | +0.04 | -0.74 | -1.00 | +0.02 | +0.00 |  | +0.00 | +0.00 | +0.00 | +0.02 | -1.69 |  | +0.0% | +0.0% |  |  |
| d_same_on | off | 3.21 | 2.01 | 0.11 | 0.31 | 1.39 | 0.32 | 1.02 | 0.33 |  | 0.16 | 0.08 | 0.34 | 0.11 | 2.35 | 59.20 | 1,903 | 8,897 | OK:30 | OK |
| d_same_on | on | 5.11 | 3.33 | 0.21 | 0.66 | 2.19 | 0.48 | 1.53 | 0.38 |  | 0.23 | 0.13 | 0.55 | 0.24 | 4.07 | 53.30 | 1,903 | 8,897 | OK:30 | OK |
| d_same_on | delta | +1.90 | +1.32 | +0.10 | +0.35 | +0.80 | +0.16 | +0.51 | +0.05 |  | +0.07 | +0.05 | +0.21 | +0.13 | +1.72 |  | +0.0% | +0.0% |  |  |
| d_hill | off | 19.22 | 18.06 | 0.08 | 0.28 | 15.68 | 2.09 | 1.00 | 0.46 |  | 0.13 | 0.06 | 0.27 | 0.08 | 18.69 | 34.70 | 12,910 | 8,897 | OK:30 | OK |
| d_hill | on | 5.20 | 3.79 | 0.12 | 0.41 | 3.04 | 0.35 | 1.19 | 0.40 |  | 0.19 | 0.09 | 0.41 | 0.10 | 4.39 | 57.10 | 3,016 | 8,897 | OK:30 | OK |
| d_hill | delta | -14.02 | -14.27 | +0.04 | +0.13 | -12.64 | -1.74 | +0.19 | -0.06 |  | +0.06 | +0.03 | +0.14 | +0.02 | -14.30 |  | -76.6% | +0.0% |  |  |
| d_group | off | 19.35 | 18.17 | 0.08 | 0.33 | 15.66 | 2.18 | 1.02 | 0.50 |  | 0.13 | 0.06 | 0.27 | 0.06 | 18.67 | 34.60 | 12,910 | 8,897 | OK:30 | OK |
| d_group | on | 15.93 | 14.73 | 0.08 | 0.23 | 13.46 | 1.04 | 1.06 | 0.49 |  | 0.13 | 0.06 | 0.27 | 0.12 | 15.20 | 40.10 | 7,551 | 8,897 | OK:30 | OK |
| d_group | delta | -3.42 | -3.44 | +0.00 | -0.10 | -2.20 | -1.14 | +0.04 | -0.01 |  | +0.00 | +0.00 | +0.00 | +0.06 | -3.47 |  | -41.5% | +0.0% |  |  |
| d_both | off | 17.54 | 16.39 | 0.11 | 0.28 | 15.02 | 1.09 | 0.98 | 0.44 |  | 0.13 | 0.06 | 0.27 | 0.09 | 17.03 | 37.60 | 12,910 | 8,897 | OK:30 | OK |
| d_both | on | 3.78 | 2.43 | 0.15 | 0.41 | 1.62 | 0.40 | 1.16 | 0.40 |  | 0.19 | 0.09 | 0.40 | 0.09 | 2.34 | 58.30 | 1,903 | 8,897 | OK:30 | OK |
| d_both | delta | -13.76 | -13.96 | +0.04 | +0.13 | -13.40 | -0.69 | +0.18 | -0.04 |  | +0.06 | +0.03 | +0.13 | +0.00 | -14.69 |  | -85.3% | +0.0% |  |  |
