# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| overcast_broken_r1 | classic | 11.58 | 11.45 |  | 0.15 | 8.58 | 2.71 |  |  |  |  |  |  |  |  |  |  | 11.10 | 51.50 | 8,243 | 5,692 | OK:30 | OK |
| overcast_broken_r1 | remastered | 8.91 | 7.52 | 0.23 | 1.99 | 4.95 | 0.58 | 0.10 |  |  | 1.13 | 0.43 | 0.01 | 0.12 | 0.06 |  | 0.04 | 8.43 | 63.70 | 2,214 | 5,692 | OK:30 | OK |
| overcast_broken_r1 | delta | -2.67 | -3.93 |  | +1.84 | -3.63 | -2.13 |  |  |  |  |  |  |  |  |  |  | -2.67 |  | -73.1% | +0.0% |  |  |
| broken_high_r1 | classic | 20.01 | 19.90 |  | 6.34 | 11.96 | 1.59 |  |  |  |  |  |  |  |  |  |  | 18.82 | 43.40 | 10,789 | 7,549 | OK:30 | OK |
| broken_high_r1 | remastered | 29.09 | 27.10 | 0.04 | 7.61 | 17.96 | 1.52 | 0.07 |  |  | 1.78 | 0.18 | 0.01 | 0.06 | 0.06 |  | 0.03 | 28.21 | 31.30 | 5,479 | 7,549 | OK:30 | OK |
| broken_high_r1 | delta | +9.08 | +7.20 |  | +1.27 | +6.00 | -0.07 |  |  |  |  |  |  |  |  |  |  | +9.39 |  | -49.2% | +0.0% |  |  |
| overcast_broken_r2 | classic | 10.71 | 10.57 |  | 0.15 | 8.19 | 2.23 |  |  |  |  |  |  |  |  |  |  | 9.92 | 57.70 | 8,243 | 5,692 | OK:30 | OK |
| overcast_broken_r2 | remastered | 8.92 | 7.43 | 0.23 | 2.01 | 4.82 | 0.60 | 0.10 |  |  | 1.22 | 0.50 | 0.01 | 0.11 | 0.06 |  | 0.04 | 8.43 | 63.50 | 2,214 | 5,692 | OK:30 | OK |
| overcast_broken_r2 | delta | -1.79 | -3.14 |  | +1.86 | -3.37 | -1.63 |  |  |  |  |  |  |  |  |  |  | -1.49 |  | -73.1% | +0.0% |  |  |
| broken_high_r2 | classic | 20.45 | 20.34 |  | 6.59 | 11.85 | 1.91 |  |  |  |  |  |  |  |  |  |  | 18.80 | 43.60 | 10,789 | 7,549 | OK:30 | OK |
| broken_high_r2 | remastered | 29.20 | 27.18 | 0.04 | 7.57 | 18.07 | 1.54 | 0.07 |  |  | 1.81 | 0.18 | 0.01 | 0.06 | 0.06 |  | 0.03 | 28.23 | 31.30 | 5,479 | 7,549 | OK:30 | OK |
| broken_high_r2 | delta | +8.75 | +6.84 |  | +0.98 | +6.22 | -0.37 |  |  |  |  |  |  |  |  |  |  | +9.43 |  | -49.2% | +0.0% |  |  |
| overcast_broken_r3 | classic | 10.56 | 10.43 |  | 0.15 | 8.23 | 2.05 |  |  |  |  |  |  |  |  |  |  | 9.74 | 58.60 | 8,243 | 5,692 | OK:30 | OK |
| overcast_broken_r3 | remastered | 8.94 | 7.56 | 0.23 | 1.95 | 5.05 | 0.55 | 0.10 |  |  | 1.12 | 0.37 | 0.01 | 0.11 | 0.06 |  | 0.04 | 8.44 | 63.70 | 2,214 | 5,692 | OK:30 | OK |
| overcast_broken_r3 | delta | -1.62 | -2.87 |  | +1.80 | -3.18 | -1.50 |  |  |  |  |  |  |  |  |  |  | -1.30 |  | -73.1% | +0.0% |  |  |
| broken_high_r3 | classic | 19.95 | 19.85 |  | 6.44 | 11.73 | 1.68 |  |  |  |  |  |  |  |  |  |  | 18.70 | 43.70 | 10,789 | 7,549 | OK:30 | OK |
| broken_high_r3 | remastered | 29.20 | 27.22 | 0.05 | 7.59 | 18.08 | 1.55 | 0.07 |  |  | 1.77 | 0.18 | 0.01 | 0.06 | 0.06 |  | 0.03 | 28.14 | 31.30 | 5,479 | 7,549 | OK:30 | OK |
| broken_high_r3 | delta | +9.25 | +7.37 |  | +1.15 | +6.35 | -0.13 |  |  |  |  |  |  |  |  |  |  | +9.44 |  | -49.2% | +0.0% |  |  |
