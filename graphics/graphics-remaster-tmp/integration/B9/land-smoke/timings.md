# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| smoke_1 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 186.60 | 1,291 | 740 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 4.84 | 3.89 | 0.13 | 0.83 | 1.75 | 1.31 | 0.07 |  |  | 0.85 | 0.31 |  | 0.09 | 0.05 | 0.22 | 0.03 | 1.91 | 158.50 | 763 | 740 |  | OK |
| smoke_3 | classic | 3.24 | 3.09 | 0.13 | 0.12 | 1.80 | 1.17 | 0.07 |  |  | 0.84 | 0.30 |  | 0.09 | 0.05 | 0.22 | 0.03 | 1.48 | 142.20 | 1,291 | 740 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 4.90 | 3.93 | 0.13 | 0.80 | 1.85 | 1.28 | 0.06 |  |  | 0.85 | 0.31 |  | 0.09 | 0.05 | 0.22 | 0.03 | 1.48 | 131.00 | 763 | 740 |  | OK |
