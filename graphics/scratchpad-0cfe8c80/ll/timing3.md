# photoLight off and on, interleaved three times per view



GPU (ms; `triGpuTimes`, smoothed over the 150 frames before each shot): frame, post pass, the air pass, the reflections and the composite (where the camera's meter and composite run). CPU (ms; `triPerfStats` over the same frames): the render thread's whole frame, `drw:post` (the post pass's CPU side: the camera's readback collection and meter queue are in it) and the camera's own timers (`cam cpu=collect/queue`, smoothed). Medians of three, the GPU frame also as the minimum.

| view | mode | GPU frame min | GPU frame med | post med | air med | ssr med | composite med | CPU frame med | FPS med | drw:post med | cam collect | cam queue |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| blue_hour_coast | off | 4.61 | 5.26 | 1.83 | 0.06 | 0.43 | 0.06 | 5.46 | 183.30 | 0.44 | - | - |
| blue_hour_coast | on | 5.31 | 5.52 | 1.90 | 0.09 | 0.54 | 0.16 | 6.95 | 143.90 | 0.49 | 0.06 | 0.04 |
| coastline_sunset | off | 4.28 | 6.74 | 3.30 | 1.20 | 0.89 | 0.04 | 5.15 | 194.30 | 0.33 | - | - |
| coastline_sunset | on | 5.95 | 7.10 | 3.56 | 1.20 | 0.90 | 0.09 | 5.65 | 177.00 | 0.39 | 0.05 | 0.03 |
| forest_edge_golden | off | 6.66 | 11.89 | 1.60 | 0.41 | - | 0.04 | 15.65 | 63.90 | 0.27 | - | - |
| forest_edge_golden | on | 8.72 | 12.02 | 1.71 | 0.40 | - | 0.10 | 14.85 | 67.30 | 0.37 | 0.04 | 0.04 |
| lowsun_fp | off | 10.66 | 13.39 | 2.45 | 1.14 | - | 0.10 | 14.85 | 67.30 | 0.27 | - | - |
| lowsun_fp | on | 13.82 | 17.13 | 2.22 | 1.02 | - | 0.60 | 15.68 | 63.80 | 0.37 | 0.05 | 0.04 |
| noon_fields | off | 20.71 | 22.64 | 1.56 | 0.06 | - | 0.12 | 29.43 | 34.00 | 0.26 | - | - |
| noon_fields | on | 17.18 | 21.56 | 1.59 | 0.07 | - | 0.78 | 23.13 | 43.20 | 0.41 | 0.05 | 0.09 |
| sunrise_ridge | off | 6.07 | 9.89 | 2.28 | 1.01 | - | 0.05 | 18.81 | 53.20 | 0.23 | - | - |
| sunrise_ridge | on | 7.36 | 12.03 | 2.65 | 1.30 | - | 0.76 | 21.65 | 46.20 | 0.37 | 0.05 | 0.06 |
| town_night | off | 11.94 | 14.19 | 1.22 | 0.04 | - | 0.12 | 15.93 | 62.80 | 0.26 | - | - |
| town_night | on | 10.36 | 12.03 | 0.93 | 0.05 | - | 0.16 | 16.45 | 60.80 | 0.42 | 0.06 | 0.05 |
