# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| smoke_1 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 104.10 | 1,291 | 740 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 4.36 | 3.34 | 0.05 | 0.16 | 1.34 | 1.84 | 0.98 | 0.27 |  | 0.09 | 0.05 | 0.23 | 0.34 | 1.83 | 96.70 | 763 | 740 |  | OK |
| smoke_3 | classic | 3.10 | 2.95 | 0.05 | 0.08 | 1.71 | 1.16 | 0.91 | 0.26 |  | 0.09 | 0.05 | 0.22 | 0.30 | 1.83 | 92.30 | 1,291 | 740 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 5.77 | 4.81 | 0.06 | 0.15 | 2.03 | 2.62 | 0.92 | 0.24 |  | 0.09 | 0.05 | 0.22 | 0.32 | 1.64 | 89.20 | 1,440 | 1,177 |  | OK |
