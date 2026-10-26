# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| b1_sinai_lowsun_fp | off | 6.32 | 3.87 | 0.36 | 0.66 | 2.03 | 1.19 | 2.22 | 0.70 |  | 0.24 | 0.11 | 0.57 | 0.22 | 5.34 | 103.50 | 497 | 371 | OK:30 | OK |
| b1_sinai_lowsun_fp | on | 6.85 | 4.61 | 0.35 | 0.73 | 2.52 | 1.37 | 2.05 | 0.74 |  | 0.23 | 0.10 | 0.55 | 0.13 | 5.71 | 89.10 | 497 | 371 | OK:30 | OK |
| b1_sinai_lowsun_fp | delta | +0.53 | +0.74 | -0.01 | +0.07 | +0.49 | +0.18 | -0.17 | +0.04 |  | -0.01 | -0.01 | -0.02 | -0.09 | +0.37 |  | +0.0% | +0.0% |  |  |
| smoke_1 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 174.60 | 1,291 | 740 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 4.23 | 3.29 | 0.13 | 0.62 | 1.53 | 1.14 | 0.88 | 0.31 |  | 0.10 | 0.05 | 0.23 | 0.04 | 1.61 | 152.90 | 763 | 740 |  | OK |
| smoke_3 | classic | 1.92 | 1.77 | 0.13 | 0.11 | 1.06 | 0.60 | 0.93 | 0.30 |  | 0.09 | 0.05 | 0.23 | 0.09 | 1.52 | 139.90 | 1,291 | 740 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 4.16 | 3.25 | 0.16 | 0.60 | 1.48 | 1.17 | 0.85 | 0.28 |  | 0.09 | 0.05 | 0.23 | 0.03 | 1.52 | 137.00 | 763 | 740 |  | OK |
