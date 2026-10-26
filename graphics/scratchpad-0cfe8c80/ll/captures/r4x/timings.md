# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| town_night | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night | on | 15.11 | 12.99 | 0.27 | 2.45 | 4.84 | 5.70 | 0.08 |  |  | 1.79 | 0.30 | 0.01 | 0.15 | 0.07 |  | 0.96 | 11.97 | 44.60 | 1,202 | 1,097 | OK:285 | OK |
| overcast_broken | off | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| overcast_broken | on | 9.67 | 7.86 | 0.23 | 2.09 | 4.95 | 0.82 | 0.10 |  |  | 1.55 | 0.24 | 0.01 | 0.12 | 0.07 |  | 0.67 | 8.71 | 55.20 | 2,214 | 5,692 | OK:30 | OK |
