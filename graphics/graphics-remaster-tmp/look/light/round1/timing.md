# GPU time, photoLight off and on (interleaved three times per view)

RTX 4060 Ti, 1920x1080, scale 1.0, 4x MSAA, TAA; `triGpuTimes` over 150 frames per shot, three shots per mode and view, off and on interleaved. Another lane's PoseidonGame (look-fog) was on the same GPU during this round's runs, so frame and world times swing; the minimum of three is the least disturbed. The camera's own work is in the post pass (composite section: the meter pass, its readback and the camera's composite).

| view | mode | frame min | frame med | world min | sky min | post min | post med | composite min | composite med |
|---|---|---|---|---|---|---|---|---|---|
| forest_edge_golden | off | 9.24 | 11.68 | 7.37 | 0.12 | 1.53 | 1.53 | 0.04 | 0.04 |
| forest_edge_golden | on | 9.34 | 12.60 | 7.01 | 0.11 | 2.00 | 2.08 | 0.66 | 0.66 |
| lowsun_fp | off | 10.85 | 13.24 | 8.60 | 0.10 | 1.98 | 1.99 | 0.04 | 0.08 |
| lowsun_fp | on | 8.20 | 8.62 | 6.29 | 0.09 | 1.65 | 1.83 | 0.35 | 0.46 |
| noon_fields | off | 11.32 | 15.98 | 10.23 | 0.10 | 0.84 | 1.12 | 0.04 | 0.08 |
| noon_fields | on | 11.02 | 15.99 | 9.82 | 0.10 | 0.95 | 1.36 | 0.30 | 0.68 |
| sunrise_ridge | off | 5.69 | 6.13 | 3.64 | 0.11 | 1.59 | 1.74 | 0.04 | 0.04 |
| sunrise_ridge | on | 6.12 | 6.77 | 3.97 | 0.11 | 1.84 | 1.90 | 0.52 | 0.57 |
| town_night | off | 10.57 | 17.16 | 9.00 | 0.08 | 1.28 | 1.35 | 0.08 | 0.10 |
| town_night | on | 9.99 | 13.51 | 8.51 | 0.08 | 1.10 | 1.16 | 0.42 | 0.44 |
