# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| shadows_woods_pine2_evening | off | 10.48 | 8.58 | 0.65 | 1.44 | 5.98 | 1.16 | 1.76 | 0.29 |  | 0.20 | 0.13 | 0.55 | 0.32 | 6.17 | 81.70 | 1,410 | 1,810 | OK:30 | OK |
| shadows_woods_pine2_evening | on | 6.60 | 4.82 | 0.38 | 0.84 | 3.25 | 0.73 | 1.63 | 0.28 |  | 0.20 | 0.07 | 0.54 | 0.18 | 5.89 | 103.60 | 1,410 | 1,810 | OK:30 | OK |
| shadows_woods_pine2_evening | delta | -3.88 | -3.76 | -0.27 | -0.60 | -2.73 | -0.43 | -0.13 | -0.01 |  | +0.00 | -0.06 | -0.01 | -0.14 | -0.28 |  | +0.0% | +0.0% |  |  |
| b1_forest_edge_golden | off | 4.44 | 3.23 | 0.31 | 0.67 | 1.59 | 0.97 | 0.89 | 0.29 |  | 0.13 | 0.07 |  | 0.03 | 3.78 | 118.40 | 866 | 1,459 | OK:30 | OK |
| b1_forest_edge_golden | on | 4.62 | 3.62 | 0.25 | 0.86 | 1.82 | 0.94 | 0.83 | 0.27 |  | 0.13 | 0.07 |  | 0.03 | 3.79 | 108.30 | 866 | 1,459 | OK:30 | OK |
| b1_forest_edge_golden | delta | +0.18 | +0.39 | -0.06 | +0.19 | +0.23 | -0.03 | -0.06 | -0.02 |  | +0.00 | +0.00 |  | +0.00 | +0.01 |  | +0.0% | +0.0% |  |  |
| b1_town_golden | off | 6.24 | 4.75 | 0.48 | 0.94 | 2.66 | 1.15 | 1.34 | 0.32 |  | 0.16 | 0.07 | 0.48 | 0.06 | 4.50 | 110.90 | 984 | 1,401 | OK:30 | OK |
| b1_town_golden | on | 9.07 | 7.18 | 0.92 | 1.02 | 3.69 | 2.47 | 1.74 | 0.16 |  | 0.16 | 0.07 | 0.48 | 0.61 | 4.49 | 95.80 | 984 | 1,401 | OK:30 | OK |
| b1_town_golden | delta | +2.83 | +2.43 | +0.44 | +0.08 | +1.03 | +1.32 | +0.40 | -0.16 |  | +0.00 | +0.00 | +0.00 | +0.55 | -0.01 |  | +0.0% | +0.0% |  |  |
| b1_lowsun_fp | off | 5.83 | 4.46 | 0.18 | 0.66 | 2.30 | 1.50 | 1.24 | 0.37 |  | 0.14 | 0.07 | 0.36 | 0.08 | 5.17 | 116.40 | 1,212 | 1,316 | OK:30 | OK |
| b1_lowsun_fp | on | 6.33 | 4.82 | 0.18 | 0.70 | 2.48 | 1.64 | 1.37 | 0.35 |  | 0.15 | 0.07 | 0.41 | 0.11 | 5.53 | 113.60 | 1,212 | 1,316 | OK:30 | OK |
| b1_lowsun_fp | delta | +0.50 | +0.36 | +0.00 | +0.04 | +0.18 | +0.14 | +0.13 | -0.02 |  | +0.01 | +0.00 | +0.05 | +0.03 | +0.36 |  | +0.0% | +0.0% |  |  |
| b1_woods_mixed_noon | off | 5.39 | 4.03 | 0.43 | 1.08 | 2.46 | 0.49 | 1.17 | 0.22 |  | 0.32 | 0.13 |  | 0.05 | 4.93 | 400.80 | 300 | 592 | OK:30 | OK |
| b1_woods_mixed_noon | on | 5.71 | 4.26 | 0.46 | 1.10 | 2.64 | 0.52 | 1.26 | 0.24 |  | 0.33 | 0.12 |  | 0.06 | 5.24 | 400.80 | 300 | 592 | OK:30 | OK |
| b1_woods_mixed_noon | delta | +0.32 | +0.23 | +0.03 | +0.02 | +0.18 | +0.03 | +0.09 | +0.02 |  | +0.01 | -0.01 |  | +0.01 | +0.31 |  | +0.0% | +0.0% |  |  |
