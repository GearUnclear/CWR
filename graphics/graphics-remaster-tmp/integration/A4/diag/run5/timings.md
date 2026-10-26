# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| d_same_off | off | 19.39 | 18.24 | 0.08 | 0.33 | 15.72 | 2.20 | 0.98 | 0.48 |  | 0.13 | 0.06 | 0.27 | 0.05 | 18.74 | 34.60 | 12,910 | 8,897 | OK:30 | OK |
| d_same_off | on | 17.55 | 16.42 | 0.08 | 0.27 | 15.06 | 1.09 | 0.96 | 0.43 |  | 0.13 | 0.06 | 0.27 | 0.08 | 16.99 | 37.70 | 12,910 | 8,897 | OK:30 | OK |
| d_same_off | delta | -1.84 | -1.82 | +0.00 | -0.06 | -0.66 | -1.11 | -0.02 | -0.05 |  | +0.00 | +0.00 | +0.00 | +0.03 | -1.75 |  | +0.0% | +0.0% |  |  |
| d_same_on | off | 3.20 | 2.01 | 0.12 | 0.31 | 1.39 | 0.32 | 1.01 | 0.35 |  | 0.17 | 0.08 | 0.34 | 0.07 | 2.35 | 69.00 | 1,903 | 8,897 | OK:30 | OK |
| d_same_on | on | 4.96 | 3.38 | 0.18 | 0.59 | 2.34 | 0.46 | 1.34 | 0.39 |  | 0.22 | 0.12 | 0.51 | 0.11 | 3.93 | 63.10 | 1,903 | 8,897 | OK:30 | OK |
| d_same_on | delta | +1.76 | +1.37 | +0.06 | +0.28 | +0.95 | +0.14 | +0.33 | +0.04 |  | +0.05 | +0.04 | +0.17 | +0.04 | +1.58 |  | +0.0% | +0.0% |  |  |
| d_hill | off | 19.23 | 18.07 | 0.08 | 0.25 | 15.75 | 2.08 | 1.00 | 0.48 |  | 0.13 | 0.06 | 0.27 | 0.06 | 18.75 | 34.70 | 12,910 | 8,897 | OK:30 | OK |
| d_hill | on | 5.06 | 3.67 | 0.12 | 0.41 | 2.93 | 0.33 | 1.17 | 0.38 |  | 0.19 | 0.09 | 0.39 | 0.11 | 4.30 | 68.10 | 3,016 | 8,897 | OK:30 | OK |
| d_hill | delta | -14.17 | -14.40 | +0.04 | +0.16 | -12.82 | -1.75 | +0.17 | -0.10 |  | +0.06 | +0.03 | +0.12 | +0.05 | -14.45 |  | -76.6% | +0.0% |  |  |
| d_group | off | 19.31 | 18.17 | 0.08 | 0.29 | 15.73 | 2.16 | 0.98 | 0.47 |  | 0.13 | 0.06 | 0.27 | 0.05 | 18.74 | 34.30 | 12,910 | 8,897 | OK:30 | OK |
| d_group | on | 15.98 | 14.76 | 0.08 | 0.23 | 13.49 | 1.04 | 1.09 | 0.47 |  | 0.13 | 0.06 | 0.27 | 0.17 | 15.25 | 40.20 | 7,551 | 8,897 | OK:30 | OK |
| d_group | delta | -3.33 | -3.41 | +0.00 | -0.06 | -2.24 | -1.12 | +0.11 | +0.00 |  | +0.00 | +0.00 | +0.00 | +0.12 | -3.49 |  | -41.5% | +0.0% |  |  |
| d_both | off | 17.57 | 16.39 | 0.08 | 0.25 | 15.07 | 1.07 | 1.01 | 0.46 |  | 0.13 | 0.06 | 0.27 | 0.10 | 17.02 | 37.70 | 12,910 | 8,897 | OK:30 | OK |
| d_both | on | 3.88 | 2.51 | 0.14 | 0.43 | 1.70 | 0.39 | 1.15 | 0.38 |  | 0.19 | 0.10 | 0.40 | 0.08 | 2.33 | 68.60 | 1,903 | 8,897 | OK:30 | OK |
| d_both | delta | -13.69 | -13.88 | +0.06 | +0.18 | -13.37 | -0.68 | +0.14 | -0.08 |  | +0.06 | +0.04 | +0.13 | -0.02 | -14.69 |  | -85.3% | +0.0% |  |  |
