# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| tank_gunner_interior | classic | 6.05 | 5.69 |  | 0.11 | 2.57 | 3.01 |  |  |  |  |  |  |  |  |  |  | 5.25 | 115.20 | 2,520 | 1,208 | OK:30 | OK |
| tank_gunner_interior | remastered | 8.18 | 7.23 | 0.06 | 0.97 | 2.88 | 3.38 | 0.09 |  |  | 0.63 | 0.13 |  | 0.28 | 0.05 |  | 0.11 | 7.98 | 80.80 | 1,525 | 1,224 | OK:30 | OK |
| tank_gunner_interior | delta | +2.13 | +1.54 |  | +0.86 | +0.31 | +0.37 |  |  |  |  |  |  |  |  |  |  | +2.73 |  | -39.5% | +1.3% |  |  |
| tank_gunner_optics | classic | 8.18 | 7.23 | 0.06 | 0.97 | 2.88 | 3.38 | 0.09 |  |  | 0.63 | 0.13 |  | 0.28 | 0.05 |  | 0.11 | 7.98 | 80.80 | 1,713 | 745 | OK:30 | OK |
| tank_gunner_optics | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
