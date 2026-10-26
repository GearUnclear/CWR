# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| d_same_off | off | 18.05 | 16.87 | 0.08 | 0.27 | 15.26 | 1.34 | 1.02 | 0.49 |  | 0.13 | 0.06 | 0.27 | 0.07 | 17.43 | 36.50 | 12,910 | 8,897 | OK:30 | OK |
| d_same_off | on | 17.59 | 16.47 | 0.10 | 0.29 | 15.07 | 1.11 | 0.96 | 0.42 |  | 0.13 | 0.06 | 0.27 | 0.09 | 16.95 | 37.00 | 12,910 | 8,897 | OK:30 | OK |
| d_same_off | delta | -0.46 | -0.40 | +0.02 | +0.02 | -0.19 | -0.23 | -0.06 | -0.07 |  | +0.00 | +0.00 | +0.00 | +0.02 | -0.48 |  | +0.0% | +0.0% |  |  |
| d_same_on | off | 16.03 | 14.85 | 0.08 | 0.23 | 13.54 | 1.09 | 1.05 | 0.49 |  | 0.13 | 0.06 | 0.27 | 0.10 | 15.23 | 38.70 | 7,551 | 8,897 | OK:30 | OK |
| d_same_on | on | 16.01 | 14.93 | 0.08 | 0.23 | 13.53 | 1.17 | 0.95 | 0.41 |  | 0.13 | 0.06 | 0.27 | 0.08 | 15.26 | 38.80 | 7,551 | 8,897 | OK:30 | OK |
| d_same_on | delta | -0.02 | +0.08 | +0.00 | +0.00 | -0.01 | +0.08 | -0.10 | -0.08 |  | +0.00 | +0.00 | +0.00 | -0.02 | +0.03 |  | +0.0% | +0.0% |  |  |
| d_hill | off | 17.70 | 16.55 | 0.08 | 0.26 | 15.18 | 1.10 | 0.99 | 0.46 |  | 0.13 | 0.06 | 0.27 | 0.08 | 16.97 | 37.60 | 12,910 | 8,897 | OK:30 | OK |
| d_hill | on | 17.56 | 16.44 | 0.09 | 0.28 | 15.04 | 1.12 | 0.95 | 0.43 |  | 0.13 | 0.06 | 0.27 | 0.06 | 16.93 | 36.60 | 12,910 | 8,897 | OK:30 | OK |
| d_hill | delta | -0.14 | -0.11 | +0.01 | +0.02 | -0.14 | +0.02 | -0.04 | -0.03 |  | +0.00 | +0.00 | +0.00 | -0.02 | -0.04 |  | +0.0% | +0.0% |  |  |
| d_group | off | 17.48 | 16.29 | 0.08 | 0.27 | 14.98 | 1.04 | 1.03 | 0.48 |  | 0.13 | 0.06 | 0.27 | 0.10 | 16.91 | 37.80 | 12,910 | 8,897 | OK:30 | OK |
| d_group | on | 16.02 | 14.95 | 0.08 | 0.23 | 13.69 | 1.03 | 0.93 | 0.43 |  | 0.13 | 0.06 | 0.27 | 0.05 | 15.33 | 40.20 | 7,551 | 8,897 | OK:30 | OK |
| d_group | delta | -1.46 | -1.34 | +0.00 | -0.04 | -1.29 | -0.01 | -0.10 | -0.05 |  | +0.00 | +0.00 | +0.00 | -0.05 | -1.58 |  | -41.5% | +0.0% |  |  |
| d_both | off | 17.56 | 16.37 | 0.09 | 0.27 | 15.04 | 1.05 | 1.03 | 0.49 |  | 0.13 | 0.06 | 0.27 | 0.08 | 16.93 | 38.00 | 12,910 | 8,897 | OK:30 | OK |
| d_both | on | 15.91 | 14.70 | 0.08 | 0.23 | 13.39 | 1.07 | 1.08 | 0.48 |  | 0.13 | 0.06 | 0.27 | 0.14 | 15.21 | 38.90 | 7,551 | 8,897 | OK:30 | OK |
| d_both | delta | -1.65 | -1.67 | -0.01 | -0.04 | -1.65 | +0.02 | +0.05 | -0.01 |  | +0.00 | +0.00 | +0.00 | +0.06 | -1.72 |  | -41.5% | +0.0% |  |  |
