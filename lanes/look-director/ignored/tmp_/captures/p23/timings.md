# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| tank_gunner_interior | classic | 5.83 | 5.52 |  | 0.10 | 2.36 | 3.06 |  |  |  |  |  |  |  |  |  |  | 5.13 | 115.40 | 2,519 | 1,208 | OK:30 | OK |
| tank_gunner_interior | remastered | 8.40 | 7.49 | 0.06 | 0.93 | 3.14 | 3.43 | 0.09 |  |  | 0.60 | 0.13 |  | 0.28 | 0.07 |  | 0.09 | 8.01 | 79.70 | 1,525 | 1,224 | OK:30 | OK |
| tank_gunner_interior | delta | +2.57 | +1.97 |  | +0.83 | +0.78 | +0.37 |  |  |  |  |  |  |  |  |  |  | +2.88 |  | -39.5% | +1.3% |  |  |
| tank_gunner_optics | classic | 8.40 | 7.49 | 0.06 | 0.93 | 3.14 | 3.43 | 0.09 |  |  | 0.60 | 0.13 |  | 0.28 | 0.07 |  | 0.09 | 8.01 | 79.70 | 1,712 | 739 | OK:30 | OK |
| tank_gunner_optics | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
