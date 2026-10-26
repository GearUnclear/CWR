# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| clouds_sun_up | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| clouds_sun_up | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| clouds_ground_sweep | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| clouds_ground_sweep | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_1 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 143.90 | 1,291 | 740 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 11.15 | 9.50 | 0.13 | 1.05 | 4.42 | 4.03 | 0.06 |  | 1.56 | 0.17 |  | 0.09 | 0.05 | 0.22 | 0.78 | 2.19 | 115.30 | 763 | 740 |  | OK |
| smoke_3 | classic | 5.67 | 5.50 | 0.13 | 0.13 | 3.02 | 2.35 | 0.06 |  | 1.35 | 0.21 |  | 0.09 | 0.05 | 0.22 | 0.54 | 1.51 | 103.00 | 1,291 | 740 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 8.22 | 6.94 | 0.13 | 1.08 | 3.41 | 2.45 | 0.06 |  | 1.18 | 0.14 |  | 0.09 | 0.05 | 0.22 | 0.51 | 1.51 | 93.00 | 763 | 740 |  | OK |
