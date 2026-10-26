# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| sinai_lowsun_fp_r1 | classic | 3.78 | 3.54 |  | 0.21 | 1.51 | 1.82 |  |  |  |  |  |  |  |  |  |  | 2.95 | 143.60 | 789 | 371 | OK:30 | OK |
| sinai_lowsun_fp_r1 | remastered | 5.82 | 3.83 | 0.24 | 1.05 | 1.92 | 0.86 | 0.11 |  |  | 1.70 | 0.47 | 0.01 | 0.16 | 0.07 |  | 0.04 | 5.49 | 115.60 | 497 | 371 | OK:30 | OK |
| sinai_lowsun_fp_r1 | delta | +2.04 | +0.29 |  | +0.84 | +0.41 | -0.96 |  |  |  |  |  |  |  |  |  |  | +2.54 |  | -37.0% | +0.0% |  |  |
| sinai_desert_noon_r1 | classic | 7.25 | 7.09 |  | 0.18 | 3.37 | 3.54 |  |  |  |  |  |  |  |  |  |  | 6.20 | 78.60 | 2,688 | 1,363 | OK:30 | OK |
| sinai_desert_noon_r1 | remastered | 6.94 | 5.35 | 0.10 | 1.17 | 2.58 | 1.60 | 0.11 |  |  | 1.31 | 0.64 | 0.01 | 0.12 | 0.06 |  | 0.03 | 6.29 | 84.10 | 1,265 | 1,363 | OK:30 | OK |
| sinai_desert_noon_r1 | delta | -0.31 | -1.74 |  | +0.99 | -0.79 | -1.94 |  |  |  |  |  |  |  |  |  |  | +0.09 |  | -52.9% | +0.0% |  |  |
| sinai_lowsun_fp_r2 | classic | 3.10 | 2.86 |  | 0.29 | 1.47 | 1.11 |  |  |  |  |  |  |  |  |  |  | 2.22 | 144.60 | 789 | 371 | OK:30 | OK |
| sinai_lowsun_fp_r2 | remastered | 6.43 | 4.30 | 0.27 | 1.23 | 2.11 | 0.97 | 0.12 |  |  | 1.79 | 0.44 | 0.01 | 0.18 | 0.08 |  | 0.04 | 6.06 | 104.30 | 497 | 371 | OK:30 | OK |
| sinai_lowsun_fp_r2 | delta | +3.33 | +1.44 |  | +0.94 | +0.64 | -0.14 |  |  |  |  |  |  |  |  |  |  | +3.84 |  | -37.0% | +0.0% |  |  |
| sinai_desert_noon_r2 | classic | 7.05 | 6.89 |  | 0.18 | 3.25 | 3.46 |  |  |  |  |  |  |  |  |  |  | 6.25 | 82.80 | 2,689 | 1,363 | OK:30 | OK |
| sinai_desert_noon_r2 | remastered | 6.96 | 5.38 | 0.10 | 1.08 | 2.66 | 1.64 | 0.11 |  |  | 1.30 | 0.63 | 0.01 | 0.12 | 0.06 |  | 0.03 | 6.30 | 84.30 | 1,266 | 1,363 | OK:30 | OK |
| sinai_desert_noon_r2 | delta | -0.09 | -1.51 |  | +0.90 | -0.59 | -1.82 |  |  |  |  |  |  |  |  |  |  | +0.05 |  | -52.9% | +0.0% |  |  |
| sinai_lowsun_fp_r3 | classic | 3.24 | 3.01 |  | 0.20 | 1.63 | 1.18 |  |  |  |  |  |  |  |  |  |  | 2.15 | 144.20 | 788 | 371 | OK:30 | OK |
| sinai_lowsun_fp_r3 | remastered | 6.35 | 4.25 | 0.27 | 1.16 | 2.10 | 0.99 | 0.12 |  |  | 1.77 | 0.39 | 0.01 | 0.18 | 0.10 |  | 0.04 | 6.00 | 103.60 | 496 | 371 | OK:30 | OK |
| sinai_lowsun_fp_r3 | delta | +3.11 | +1.24 |  | +0.96 | +0.47 | -0.19 |  |  |  |  |  |  |  |  |  |  | +3.85 |  | -37.1% | +0.0% |  |  |
| sinai_desert_noon_r3 | classic | 7.25 | 7.09 |  | 0.18 | 3.23 | 3.68 |  |  |  |  |  |  |  |  |  |  | 6.08 | 85.30 | 2,689 | 1,363 | OK:30 | OK |
| sinai_desert_noon_r3 | remastered | 6.97 | 5.34 | 0.10 | 1.03 | 2.66 | 1.64 | 0.11 |  |  | 1.35 | 0.55 | 0.01 | 0.12 | 0.06 |  | 0.03 | 6.27 | 83.80 | 1,266 | 1,363 | OK:30 | OK |
| sinai_desert_noon_r3 | delta | -0.28 | -1.75 |  | +0.85 | -0.57 | -2.04 |  |  |  |  |  |  |  |  |  |  | +0.19 |  | -52.9% | +0.0% |  |  |
