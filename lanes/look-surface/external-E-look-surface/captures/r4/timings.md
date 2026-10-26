# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 9.01 | 8.90 |  | 0.08 | 3.89 | 4.94 |  |  |  |  |  |  |  |  |  |  | 2.79 | 87.50 | 1,779 | 1,179 | OK:30 | OK |
| lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_fields | classic | 9.41 | 9.31 |  | 0.08 | 7.22 | 2.01 |  |  |  |  |  |  |  |  |  |  | 7.01 | 63.40 | 5,877 | 3,920 | OK:30 | OK |
| noon_fields | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 5.49 | 5.37 |  | 0.06 | 3.25 | 2.05 |  |  |  |  |  |  |  |  |  |  | 3.88 | 128.00 | 2,189 | 1,316 | OK:30 | OK |
| forest_edge_golden | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night | classic | 8.13 | 7.98 |  | 0.06 | 1.56 | 6.36 |  |  |  |  |  |  |  |  |  |  | 4.55 | 92.10 | 1,257 | 1,080 | OK:265 | OK |
| town_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| jeep_close_noon | classic | 15.00 | 14.91 |  | 0.08 | 11.18 | 3.65 |  |  |  |  |  |  |  |  |  |  | 12.28 | 54.10 | 2,489 | 1,625 | OK:30 | OK |
| jeep_close_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_noon | classic | 2.31 | 2.19 |  | 0.06 | 0.70 | 1.43 |  |  |  |  |  |  |  |  |  |  | 1.37 | 196.20 | 880 | 473 | OK:30 | OK |
| town_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_morning | classic | 3.83 | 3.71 |  | 0.07 | 2.22 | 1.42 |  |  |  |  |  |  |  |  |  |  | 3.47 | 182.50 | 2,162 | 1,309 | OK:30 | OK |
| forest_morning | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| tank_gunner_interior | classic | 8.51 | 8.15 |  | 0.08 | 3.39 | 4.68 |  |  |  |  |  |  |  |  |  |  | 4.33 | 101.00 | 2,519 | 1,208 | OK:30 | OK |
| tank_gunner_interior | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 2.61 | 2.48 |  | 0.08 | 1.39 | 1.01 |  |  |  |  |  |  |  |  |  |  | 1.36 | 193.00 | 603 | 314 | OK:30 | OK |
| sinai_lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 5.73 | 5.61 |  | 0.11 | 2.98 | 2.52 |  |  |  |  |  |  |  |  |  |  | 4.76 | 86.90 | 2,243 | 1,100 | OK:30 | OK |
| sinai_desert_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
