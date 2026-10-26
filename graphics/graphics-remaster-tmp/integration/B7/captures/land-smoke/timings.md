# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| smoke_1 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 161.90 | 1,291 | 740 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 5.64 | 4.41 | 0.13 | 1.25 | 1.81 | 1.34 | 0.07 |  |  | 1.11 | 0.32 | 0.01 | 0.09 | 0.05 |  | 0.02 | 3.16 | 139.70 | 763 | 740 |  | OK |
| smoke_3 | classic | 1.76 | 1.60 | 0.13 | 0.13 | 0.92 | 0.55 | 0.07 |  |  | 1.07 | 0.31 | 0.01 | 0.09 | 0.05 |  | 0.02 | 1.47 | 128.10 | 1,291 | 740 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 5.02 | 3.74 | 0.13 | 0.82 | 1.65 | 1.27 | 0.07 |  |  | 1.14 | 0.38 | 0.01 | 0.09 | 0.05 |  | 0.02 | 1.47 | 118.80 | 763 | 740 |  | OK |
