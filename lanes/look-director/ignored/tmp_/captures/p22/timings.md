# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| tank_gunner_interior | classic | 5.97 | 5.68 |  | 0.11 | 2.46 | 3.11 |  |  |  |  |  |  |  |  |  |  | 4.86 | 115.40 | 2,519 | 1,208 | OK:30 | OK |
| tank_gunner_interior | remastered | 8.32 | 7.38 | 0.06 | 0.93 | 2.99 | 3.45 | 0.09 |  |  | 0.61 | 0.12 |  | 0.28 | 0.08 |  | 0.09 | 8.09 | 79.50 | 1,525 | 1,224 | OK:30 | OK |
| tank_gunner_interior | delta | +2.35 | +1.70 |  | +0.82 | +0.53 | +0.34 |  |  |  |  |  |  |  |  |  |  | +3.23 |  | -39.5% | +1.3% |  |  |
| tank_gunner_optics | classic | 8.32 | 7.38 | 0.06 | 0.93 | 2.99 | 3.45 | 0.09 |  |  | 0.61 | 0.12 |  | 0.28 | 0.08 |  | 0.09 | 8.09 | 79.50 | 1,714 | 739 | OK:30 | OK |
| tank_gunner_optics | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
