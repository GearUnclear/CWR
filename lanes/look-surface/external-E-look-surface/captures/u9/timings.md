# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 3.84 | 3.70 |  | 0.11 | 1.68 | 1.91 |  |  |  |  |  |  |  |  |  |  | 3.16 | 148.40 | 1,779 | 1,179 | OK:30 | OK |
| lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_fields | classic | 7.23 | 7.13 |  | 0.08 | 5.35 | 1.70 |  |  |  |  |  |  |  |  |  |  | 6.92 | 80.20 | 5,877 | 3,920 | OK:30 | OK |
| noon_fields | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 5.30 | 5.15 |  | 0.08 | 2.98 | 2.09 |  |  |  |  |  |  |  |  |  |  | 4.50 | 182.10 | 2,189 | 1,316 | OK:30 | OK |
| forest_edge_golden | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night | classic | 5.81 | 5.60 |  | 0.08 | 1.35 | 4.17 |  |  |  |  |  |  |  |  |  |  | 5.12 | 130.30 | 1,170 | 1,003 | OK:239 | OK |
| town_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| jeep_close_noon | classic | 4.26 | 4.17 |  | 0.09 | 3.24 | 0.83 |  |  |  |  |  |  |  |  |  |  | 3.41 | 148.70 | 2,489 | 1,625 | OK:30 | OK |
| jeep_close_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_noon | classic | 2.13 | 1.96 |  | 0.09 | 0.60 | 1.27 |  |  |  |  |  |  |  |  |  |  | 1.54 | 227.60 | 880 | 473 | OK:30 | OK |
| town_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_morning | classic | 4.86 | 4.70 |  | 0.09 | 2.94 | 1.67 |  |  |  |  |  |  |  |  |  |  | 3.75 | 179.80 | 2,162 | 1,309 | OK:30 | OK |
| forest_morning | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| tank_gunner_interior | classic | 5.15 | 4.84 |  | 0.09 | 2.02 | 2.72 |  |  |  |  |  |  |  |  |  |  | 4.29 | 128.10 | 2,519 | 1,208 | OK:30 | OK |
| tank_gunner_interior | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 2.74 | 2.56 |  | 0.14 | 1.08 | 1.34 |  |  |  |  |  |  |  |  |  |  | 2.04 | 204.60 | 603 | 314 | OK:30 | OK |
| sinai_lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 5.62 | 5.49 |  | 0.14 | 2.89 | 2.46 |  |  |  |  |  |  |  |  |  |  | 4.73 | 93.00 | 2,243 | 1,100 | OK:30 | OK |
| sinai_desert_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
