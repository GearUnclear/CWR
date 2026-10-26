# GPU time, photoLight off and on (interleaved three times per view)

RTX 4060 Ti, 1920x1080, scale 1.0, 4x MSAA, TAA on; `triGpuTimes` smoothed over 150 frames per shot. One to three other agents' PoseidonGame processes ran on the same GPU during the run (gpu_timing.csv), so the frame and world numbers swing; the minimum of the three rounds is the least disturbed. The camera's own work is in the post sections (composite: the meter pass, its readback and the camera's composite).

| view | mode | frame min | frame med | world min | post min | post med | composite min | composite med |
|---|---|---|---|---|---|---|---|---|
| forest_edge_golden | off | 6.24 | 6.94 | 5.05 | 0.82 | 0.87 | 0.04 | 0.04 |
| forest_edge_golden | on | 5.82 | 6.07 | 4.58 | 0.95 | 1.04 | 0.31 | 0.40 |
| lowsun_fp | off | 7.00 | 7.17 | 5.69 | 1.09 | 1.10 | 0.07 | 0.10 |
| lowsun_fp | on | 7.47 | 8.84 | 5.86 | 1.15 | 1.39 | 0.31 | 0.55 |
| noon_fields | off | 7.68 | 8.60 | 6.75 | 0.72 | 0.73 | 0.04 | 0.04 |
| noon_fields | on | 8.12 | 8.96 | 7.06 | 0.84 | 1.05 | 0.29 | 0.43 |
| sunrise_ridge | off | 5.30 | 8.75 | 3.83 | 1.22 | 1.44 | 0.17 | 0.37 |
| sunrise_ridge | on | 6.36 | 6.42 | 4.58 | 1.40 | 1.52 | 0.61 | 0.73 |
| town_night | off | 14.51 | 14.97 | 12.93 | 1.24 | 1.26 | 0.04 | 0.04 |
| town_night | on | 10.05 | 16.05 | 8.69 | 1.10 | 1.20 | 0.48 | 0.59 |
