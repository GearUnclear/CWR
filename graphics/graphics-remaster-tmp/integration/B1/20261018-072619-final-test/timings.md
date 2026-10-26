# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| shadows_woods_pine2_evening | off | 6.37 | 4.77 | 0.35 | 0.83 | 3.31 | 0.64 | 1.46 | 0.31 |  | 0.20 | 0.07 | 0.50 | 0.12 | 6.05 | 106.90 | 1,410 | 1,810 | OK:30 | OK |
| shadows_woods_pine2_evening | on | 6.86 | 5.26 | 0.43 | 0.86 | 3.57 | 0.83 | 1.46 | 0.33 |  | 0.20 | 0.07 | 0.50 | 0.10 | 6.33 | 103.20 | 1,410 | 1,810 | OK:30 | OK |
| shadows_woods_pine2_evening | delta | +0.49 | +0.49 | +0.08 | +0.03 | +0.26 | +0.19 | +0.00 | +0.02 |  | +0.00 | +0.00 | +0.00 | -0.02 | +0.28 |  | +0.0% | +0.0% |  |  |
| b1_forest_edge_golden | off | 4.80 | 3.75 | 0.26 | 0.76 | 1.89 | 1.10 | 0.89 | 0.29 |  | 0.14 | 0.08 |  | 0.03 | 4.16 | 119.90 | 866 | 1,459 | OK:30 | OK |
| b1_forest_edge_golden | on | 6.50 | 5.44 | 0.26 | 1.06 | 2.84 | 1.54 | 0.89 | 0.21 |  | 0.13 | 0.07 |  | 0.03 | 4.08 | 107.30 | 866 | 1,459 | OK:30 | OK |
| b1_forest_edge_golden | delta | +1.70 | +1.69 | +0.00 | +0.30 | +0.95 | +0.44 | +0.00 | -0.08 |  | -0.01 | -0.01 |  | +0.00 | -0.08 |  | +0.0% | +0.0% |  |  |
| b1_town_golden | off | 4.95 | 3.44 | 0.39 | 0.66 | 1.85 | 0.92 | 1.36 | 0.33 |  | 0.16 | 0.07 | 0.49 | 0.05 | 4.61 | 112.70 | 984 | 1,401 | OK:30 | OK |
| b1_town_golden | on | 6.98 | 5.16 | 0.60 | 0.95 | 2.56 | 1.65 | 1.67 | 0.32 |  | 0.16 | 0.07 | 0.49 | 0.39 | 4.88 | 101.20 | 984 | 1,401 | OK:30 | OK |
| b1_town_golden | delta | +2.03 | +1.72 | +0.21 | +0.29 | +0.71 | +0.73 | +0.31 | -0.01 |  | +0.00 | +0.00 | +0.00 | +0.34 | +0.27 |  | +0.0% | +0.0% |  |  |
| b1_lowsun_fp | off | 5.90 | 4.39 | 0.17 | 0.65 | 2.04 | 1.70 | 1.38 | 0.46 |  | 0.14 | 0.07 | 0.37 | 0.12 | 5.50 | 92.70 | 1,212 | 1,316 | OK:30 | OK |
| b1_lowsun_fp | on | 8.98 | 7.54 | 0.32 | 1.04 | 4.22 | 2.29 | 1.30 | 0.43 |  | 0.14 | 0.07 | 0.37 | 0.08 | 5.47 | 82.90 | 1,212 | 1,316 | OK:30 | OK |
| b1_lowsun_fp | delta | +3.08 | +3.15 | +0.15 | +0.39 | +2.18 | +0.59 | -0.08 | -0.03 |  | +0.00 | +0.00 | +0.00 | -0.04 | -0.03 |  | +0.0% | +0.0% |  |  |
| b1_woods_mixed_noon | off | 3.27 | 2.37 | 0.28 | 0.61 | 1.45 | 0.32 | 0.77 | 0.13 |  | 0.22 | 0.07 |  | 0.03 | 3.07 | 340.60 | 300 | 592 | OK:30 | OK |
| b1_woods_mixed_noon | on | 3.98 | 2.94 | 0.34 | 0.73 | 1.83 | 0.38 | 0.91 | 0.21 |  | 0.23 | 0.07 |  | 0.03 | 3.38 | 315.10 | 300 | 592 | OK:30 | OK |
| b1_woods_mixed_noon | delta | +0.71 | +0.57 | +0.06 | +0.12 | +0.38 | +0.06 | +0.14 | +0.08 |  | +0.01 | +0.00 |  | +0.00 | +0.31 |  | +0.0% | +0.0% |  |  |
| b1_sinai_lowsun_fp | off | 6.00 | 3.81 | 0.42 | 0.63 | 1.93 | 1.25 | 2.00 | 0.65 |  | 0.24 | 0.10 | 0.57 | 0.13 | 5.38 | 112.90 | 497 | 371 | OK:30 | OK |
| b1_sinai_lowsun_fp | on | 6.23 | 4.09 | 0.31 | 0.67 | 2.31 | 1.11 | 1.95 | 0.64 |  | 0.23 | 0.10 | 0.55 | 0.13 | 5.52 | 104.90 | 497 | 371 | OK:30 | OK |
| b1_sinai_lowsun_fp | delta | +0.23 | +0.28 | -0.11 | +0.04 | +0.38 | -0.14 | -0.05 | -0.01 |  | -0.01 | +0.00 | -0.02 | +0.00 | +0.14 |  | +0.0% | +0.0% |  |  |
| smoke_1 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 154.60 | 1,291 | 740 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 6.18 | 4.81 | 0.19 | 0.99 | 2.13 | 1.69 | 1.28 | 0.41 |  | 0.14 | 0.09 | 0.34 | 0.07 | 2.22 | 133.20 | 763 | 740 |  | OK |
| smoke_3 | classic | 2.52 | 2.30 | 0.19 | 0.17 | 1.34 | 0.78 | 1.34 | 0.42 |  | 0.14 | 0.08 | 0.34 | 0.14 | 2.13 | 123.30 | 1,291 | 740 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 6.12 | 4.81 | 0.21 | 0.91 | 2.21 | 1.69 | 1.23 | 0.36 |  | 0.14 | 0.08 | 0.34 | 0.10 | 2.13 | 118.90 | 763 | 740 |  | OK |
