# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| tank_gunner_interior | classic | 5.83 | 5.57 |  | 0.09 | 2.26 | 3.21 |  |  |  |  |  |  |  |  |  |  | 4.63 | 117.10 | 2,520 | 1,208 | OK:30 | OK |
| tank_gunner_interior | remastered | 8.09 | 7.19 | 0.06 | 0.96 | 2.89 | 3.34 | 0.09 |  |  | 0.59 | 0.12 |  | 0.28 | 0.05 |  | 0.09 | 7.83 | 80.40 | 1,525 | 1,224 | OK:30 | OK |
| tank_gunner_interior | delta | +2.26 | +1.62 |  | +0.87 | +0.63 | +0.13 |  |  |  |  |  |  |  |  |  |  | +3.20 |  | -39.5% | +1.3% |  |  |
| tank_gunner_optics | classic | 3.56 | 3.29 |  | 0.12 | 1.57 | 1.61 |  |  |  |  |  |  |  |  |  |  | 2.71 | 167.90 | 1,719 | 739 | OK:30 | OK |
| tank_gunner_optics | remastered | 8.04 | 6.79 | 0.06 | 0.98 | 3.61 | 2.20 | 0.10 |  |  | 0.94 | 0.23 | 0.01 | 0.15 |  |  | 0.10 | 5.75 | 101.80 | 1,141 | 739 | OK:30 | OK |
| tank_gunner_optics | delta | +4.48 | +3.50 |  | +0.86 | +2.04 | +0.59 |  |  |  |  |  |  |  |  |  |  | +3.04 |  | -33.6% | +0.0% |  |  |
