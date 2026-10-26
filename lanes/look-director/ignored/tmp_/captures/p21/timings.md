# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| tank_gunner_interior | classic | 6.08 | 5.78 |  | 0.11 | 2.58 | 3.09 |  |  |  |  |  |  |  |  |  |  | 4.91 | 113.80 | 2,520 | 1,208 | OK:30 | OK |
| tank_gunner_interior | remastered | 8.23 | 7.32 | 0.06 | 0.95 | 2.96 | 3.41 | 0.09 |  |  | 0.61 | 0.14 |  | 0.28 | 0.05 |  | 0.11 | 7.96 | 81.70 | 1,526 | 1,224 | OK:30 | OK |
| tank_gunner_interior | delta | +2.15 | +1.54 |  | +0.84 | +0.38 | +0.32 |  |  |  |  |  |  |  |  |  |  | +3.05 |  | -39.4% | +1.3% |  |  |
| tank_gunner_optics | classic | 8.23 | 7.32 | 0.06 | 0.95 | 2.96 | 3.41 | 0.09 |  |  | 0.61 | 0.14 |  | 0.28 | 0.05 |  | 0.11 | 7.96 | 81.70 | 1,706 | 739 | OK:30 | OK |
| tank_gunner_optics | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
