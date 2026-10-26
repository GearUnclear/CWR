# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 4.64 | 4.53 |  | 0.07 | 1.52 | 2.94 |  |  |  |  |  |  |  |  |  |  | 3.17 | 118.00 | 1,779 | 1,179 | OK:30 | OK |
| lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_fields | classic | 14.45 | 14.34 |  | 0.08 | 10.46 | 3.80 |  |  |  |  |  |  |  |  |  |  | 7.86 | 45.60 | 5,877 | 3,920 | OK:30 | OK |
| noon_fields | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 6.41 | 6.28 |  | 0.06 | 3.12 | 3.09 |  |  |  |  |  |  |  |  |  |  | 3.89 | 137.10 | 2,189 | 1,316 | OK:30 | OK |
| forest_edge_golden | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night | classic | 7.39 | 7.20 |  | 0.08 | 1.84 | 5.28 |  |  |  |  |  |  |  |  |  |  | 6.18 | 99.90 | 1,234 | 1,066 | OK:268 | OK |
| town_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| jeep_close_noon | classic | 4.62 | 4.52 |  | 0.09 | 3.27 | 1.16 |  |  |  |  |  |  |  |  |  |  | 3.92 | 107.80 | 2,489 | 1,625 | OK:30 | OK |
| jeep_close_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_noon | classic | 2.47 | 2.29 |  | 0.10 | 0.62 | 1.57 |  |  |  |  |  |  |  |  |  |  | 1.64 | 175.80 | 880 | 473 | OK:30 | OK |
| town_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_morning | classic | 4.95 | 4.81 |  | 0.08 | 2.91 | 1.82 |  |  |  |  |  |  |  |  |  |  | 4.07 | 138.10 | 2,162 | 1,309 | OK:30 | OK |
| forest_morning | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| tank_gunner_interior | classic | 5.26 | 4.99 |  | 0.09 | 2.17 | 2.73 |  |  |  |  |  |  |  |  |  |  | 4.36 | 124.90 | 2,520 | 1,208 | OK:30 | OK |
| tank_gunner_interior | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 5.07 | 4.95 |  | 0.08 | 1.44 | 3.44 |  |  |  |  |  |  |  |  |  |  | 2.60 | 127.10 | 602 | 314 | OK:30 | OK |
| sinai_lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 15.94 | 15.82 |  | 0.12 | 7.49 | 8.21 |  |  |  |  |  |  |  |  |  |  | 5.20 | 51.30 | 2,243 | 1,100 | OK:30 | OK |
| sinai_desert_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
