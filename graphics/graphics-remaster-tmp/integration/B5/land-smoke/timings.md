# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| smoke_1 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 93.10 | 1,291 | 740 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 16.17 | 14.11 | 0.15 | 2.64 | 6.39 | 5.07 | 0.06 |  | 1.59 | 0.13 |  | 0.09 | 0.05 | 0.22 | 0.93 | 3.51 | 77.70 | 763 | 740 |  | OK |
| smoke_3 | classic | 5.40 | 5.18 | 0.14 | 0.24 | 2.76 | 2.18 | 0.06 |  | 1.74 | 0.11 |  | 0.09 | 0.05 | 0.22 | 1.12 | 2.53 | 72.40 | 1,291 | 740 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 16.53 | 14.04 | 0.13 | 2.42 | 6.22 | 5.39 | 0.06 |  | 1.85 | 0.10 |  | 0.09 | 0.05 | 0.22 | 1.25 | 2.53 | 64.80 | 763 | 740 |  | OK |
