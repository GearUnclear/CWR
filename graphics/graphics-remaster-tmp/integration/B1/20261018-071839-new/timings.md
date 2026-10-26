# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| shadows_woods_pine2_evening | off | 8.12 | 5.93 | 0.62 | 0.85 | 3.76 | 1.31 | 2.04 | 0.27 |  | 0.20 | 0.24 | 0.58 | 0.38 | 6.11 | 93.40 | 1,410 | 1,810 | OK:30 | OK |
| shadows_woods_pine2_evening | on | 8.14 | 5.93 | 0.46 | 0.92 | 3.78 | 1.22 | 2.06 | 0.25 |  | 0.20 | 0.24 | 0.56 | 0.47 | 6.44 | 88.60 | 1,410 | 1,810 | OK:30 | OK |
| shadows_woods_pine2_evening | delta | +0.02 | +0.00 | -0.16 | +0.07 | +0.02 | -0.09 | +0.02 | -0.02 |  | +0.00 | +0.00 | -0.02 | +0.09 | +0.33 |  | +0.0% | +0.0% |  |  |
| b1_forest_edge_golden | off | 4.43 | 3.37 | 0.26 | 0.71 | 1.79 | 0.88 | 0.90 | 0.29 |  | 0.13 | 0.07 |  | 0.03 | 3.90 | 118.00 | 866 | 1,459 | OK:30 | OK |
| b1_forest_edge_golden | on | 4.67 | 3.71 | 0.32 | 0.76 | 1.95 | 1.01 | 0.80 | 0.28 |  | 0.13 | 0.07 |  | 0.03 | 4.09 | 115.20 | 866 | 1,459 | OK:30 | OK |
| b1_forest_edge_golden | delta | +0.24 | +0.34 | +0.06 | +0.05 | +0.16 | +0.13 | -0.10 | -0.01 |  | +0.00 | +0.00 |  | +0.00 | +0.19 |  | +0.0% | +0.0% |  |  |
| b1_town_golden | off | 5.70 | 4.18 | 0.41 | 1.20 | 2.00 | 0.97 | 1.37 | 0.34 |  | 0.16 | 0.07 | 0.49 | 0.07 | 4.61 | 115.00 | 984 | 1,401 | OK:30 | OK |
| b1_town_golden | on | 8.07 | 6.30 | 0.62 | 1.36 | 3.26 | 1.69 | 1.62 | 0.38 |  | 0.16 | 0.07 | 0.48 | 0.23 | 4.85 | 103.80 | 984 | 1,401 | OK:30 | OK |
| b1_town_golden | delta | +2.37 | +2.12 | +0.21 | +0.16 | +1.26 | +0.72 | +0.25 | +0.04 |  | +0.00 | +0.00 | -0.01 | +0.16 | +0.24 |  | +0.0% | +0.0% |  |  |
| b1_lowsun_fp | off | 7.97 | 6.64 | 0.17 | 1.09 | 3.62 | 1.93 | 1.20 | 0.35 |  | 0.14 | 0.07 | 0.37 | 0.05 | 5.17 | 103.20 | 1,212 | 1,316 | OK:30 | OK |
| b1_lowsun_fp | on | 6.09 | 4.72 | 0.21 | 0.70 | 2.21 | 1.81 | 1.23 | 0.32 |  | 0.14 | 0.07 | 0.37 | 0.12 | 5.35 | 92.80 | 1,212 | 1,316 | OK:30 | OK |
| b1_lowsun_fp | delta | -1.88 | -1.92 | +0.04 | -0.39 | -1.41 | -0.12 | +0.03 | -0.03 |  | +0.00 | +0.00 | +0.00 | +0.07 | +0.18 |  | +0.0% | +0.0% |  |  |
| b1_woods_mixed_noon | off | 4.64 | 3.52 | 0.37 | 0.94 | 2.17 | 0.41 | 0.97 | 0.14 |  | 0.34 | 0.09 |  | 0.05 | 3.89 | 401.10 | 300 | 592 | OK:30 | OK |
| b1_woods_mixed_noon | on | 5.64 | 4.48 | 0.43 | 0.92 | 3.09 | 0.47 | 1.01 | 0.15 |  | 0.34 | 0.10 |  | 0.05 | 5.21 | 373.50 | 300 | 592 | OK:30 | OK |
| b1_woods_mixed_noon | delta | +1.00 | +0.96 | +0.06 | -0.02 | +0.92 | +0.06 | +0.04 | +0.01 |  | +0.00 | +0.01 |  | +0.00 | +1.32 |  | +0.0% | +0.0% |  |  |
