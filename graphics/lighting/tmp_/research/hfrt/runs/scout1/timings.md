# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| sumN_0700 | classic | 1.71 | 1.63 |  |  |  |  |  |  | 119.20 | OK |
| sumN_0700 | remastered | 20.50 | 19.70 | 0.00 | 0.71 | 0.63 | 0.05 |  | 0.03 | 48.20 | OK |
| sumN_0700 | delta | +18.79 | +18.07 |  |  |  |  |  |  |  |  |
| sumS_0700 | classic | 1.14 | 1.07 |  |  |  |  |  |  | 165.50 | OK |
| sumS_0700 | remastered | 45.08 | 43.69 | 0.00 | 1.32 | 1.23 | 0.05 |  | 0.03 | 28.40 | OK |
| sumS_0700 | delta | +43.94 | +42.62 |  |  |  |  |  |  |  |  |
| shoulder_0700 | classic | 39.17 | 39.09 |  |  |  |  |  |  | 25.20 | OK |
| shoulder_0700 | remastered | 39.61 | 38.40 | 0.00 | 1.13 | 1.05 | 0.05 |  | 0.03 | 21.30 | OK |
| shoulder_0700 | delta | +0.44 | -0.69 |  |  |  |  |  |  |  |  |
| sumN_1730 | classic | 21.96 | 21.87 |  |  |  |  |  |  | 26.80 | OK |
| sumN_1730 | remastered | 50.24 | 48.87 | 0.00 | 1.27 | 1.19 | 0.05 |  | 0.03 | 24.50 | OK |
| sumN_1730 | delta | +28.28 | +27.00 |  |  |  |  |  |  |  |  |
| sumS_1730 | classic | 40.38 | 40.29 |  |  |  |  |  |  | 24.90 | OK |
| sumS_1730 | remastered | 28.87 | 27.93 | 0.00 | 0.86 | 0.78 | 0.05 |  | 0.03 | 24.10 | OK |
| sumS_1730 | delta | -11.51 | -12.36 |  |  |  |  |  |  |  |  |
| shoulder_1730 | classic | 34.84 | 34.75 |  |  |  |  |  |  | 42.40 | OK |
| shoulder_1730 | remastered | 27.31 | 26.28 | 0.00 | 0.93 | 0.85 | 0.05 |  | 0.03 | 26.20 | OK |
| shoulder_1730 | delta | -7.53 | -8.47 |  |  |  |  |  |  |  |  |
