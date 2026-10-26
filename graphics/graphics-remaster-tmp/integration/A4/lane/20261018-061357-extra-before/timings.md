# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| town_night | off | 6.48 | 5.59 |  |  |  |  | 0.68 | 0.39 |  | 0.17 | 0.07 |  | 0.04 |  | 118.80 | 1,299 | 1,121 | OK:220 | OK |
| town_night | on | 6.35 | 5.51 |  |  |  |  | 0.64 | 0.35 |  | 0.17 | 0.07 |  | 0.05 |  | 115.10 | 1,257 | 1,097 | OK:56 | OK |
| town_night | delta | -0.13 | -0.08 |  |  |  |  | -0.04 | -0.04 |  | +0.00 | +0.00 |  | +0.01 |  |  | -3.2% | -2.1% |  |  |
| noon_fields | off | 9.10 | 8.50 | 0.07 |  |  |  | 0.46 | 0.25 |  | 0.11 | 0.06 |  | 0.04 |  | 70.90 | 6,398 | 4,508 | OK:30 | OK |
| noon_fields | on | 9.08 | 8.50 | 0.07 |  |  |  | 0.44 | 0.23 |  | 0.11 | 0.06 |  | 0.04 |  | 70.30 | 6,398 | 4,508 | OK:30 | OK |
| noon_fields | delta | -0.02 | +0.00 | +0.00 |  |  |  | -0.02 | -0.02 |  | +0.00 | +0.00 |  | +0.00 |  |  | +0.0% | +0.0% |  |  |
| noon_walk | off | 9.03 | 8.40 | 0.08 |  |  |  | 0.48 | 0.27 |  | 0.11 | 0.06 |  | 0.04 |  | 69.90 | 6,310 | 4,396 | OK:30 | OK |
| noon_walk | on | 8.55 | 7.98 | 0.08 |  |  |  | 0.43 | 0.21 |  | 0.11 | 0.06 |  | 0.04 |  | 72.40 | 6,310 | 4,396 | OK:30 | OK |
| noon_walk | delta | -0.48 | -0.42 | +0.00 |  |  |  | -0.05 | -0.06 |  | +0.00 | +0.00 |  | +0.00 |  |  | +0.0% | +0.0% |  |  |
| sunrise_walk | off | 20.92 | 19.74 | 0.12 |  |  |  | 1.01 | 0.47 |  | 0.12 | 0.06 | 0.27 | 0.10 |  | 34.20 | 12,939 | 8,936 | OK:30 | OK |
| sunrise_walk | on | 20.81 | 19.66 | 0.10 |  |  |  | 0.99 | 0.47 |  | 0.12 | 0.06 | 0.27 | 0.07 |  | 37.40 | 12,939 | 8,936 | OK:30 | OK |
| sunrise_walk | delta | -0.11 | -0.08 | -0.02 |  |  |  | -0.02 | +0.00 |  | +0.00 | +0.00 | +0.00 | -0.03 |  |  | +0.0% | +0.0% |  |  |
| forest_edge_golden | off | 5.51 | 4.74 | 0.26 |  |  |  | 0.57 | 0.26 |  | 0.17 | 0.08 |  | 0.05 |  | 122.50 | 2,142 | 1,459 | OK:30 | OK |
| forest_edge_golden | on | 5.99 | 5.24 | 0.33 |  |  |  | 0.54 | 0.21 |  | 0.18 | 0.09 |  | 0.06 |  | 116.10 | 2,142 | 1,459 | OK:30 | OK |
| forest_edge_golden | delta | +0.48 | +0.50 | +0.07 |  |  |  | -0.03 | -0.05 |  | +0.01 | +0.01 |  | +0.01 |  |  | +0.0% | +0.0% |  |  |
| lowsun_fp | off | 6.14 | 4.75 | 0.09 |  |  |  | 1.21 | 0.26 |  | 0.19 | 0.09 | 0.51 | 0.15 |  | 121.40 | 1,890 | 1,316 | OK:30 | OK |
| lowsun_fp | on | 6.10 | 4.73 | 0.09 |  |  |  | 1.19 | 0.30 |  | 0.19 | 0.09 | 0.51 | 0.10 |  | 118.20 | 1,890 | 1,316 | OK:30 | OK |
| lowsun_fp | delta | -0.04 | -0.02 | +0.00 |  |  |  | -0.02 | +0.04 |  | +0.00 | +0.00 | +0.00 | -0.05 |  |  | +0.0% | +0.0% |  |  |
