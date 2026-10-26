# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 4.35 | 4.19 |  | 0.12 | 1.87 | 2.19 |  |  |  |  |  |  |  |  |  |  | 3.33 | 145.00 | 1,779 | 1,179 | OK:30 | OK |
| lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 5.45 | 5.30 |  | 0.08 | 3.16 | 2.06 |  |  |  |  |  |  |  |  |  |  | 4.60 | 177.70 | 2,189 | 1,316 | OK:30 | OK |
| forest_edge_golden | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| jeep_close_noon | classic | 4.49 | 4.39 |  | 0.10 | 3.36 | 0.93 |  |  |  |  |  |  |  |  |  |  | 3.50 | 139.90 | 2,489 | 1,625 | OK:30 | OK |
| jeep_close_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_noon | classic | 2.20 | 2.03 |  | 0.10 | 0.60 | 1.33 |  |  |  |  |  |  |  |  |  |  | 1.59 | 218.20 | 880 | 473 | OK:30 | OK |
| town_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_fields | classic | 7.05 | 6.95 |  | 0.08 | 5.27 | 1.60 |  |  |  |  |  |  |  |  |  |  | 6.71 | 81.60 | 5,877 | 3,920 | OK:30 | OK |
| noon_fields | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night | classic | 5.76 | 5.58 |  | 0.07 | 1.38 | 4.14 |  |  |  |  |  |  |  |  |  |  | 5.34 | 128.80 | 1,244 | 1,073 | OK:225 | OK |
| town_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_morning | classic | 4.83 | 4.68 |  | 0.09 | 2.89 | 1.70 |  |  |  |  |  |  |  |  |  |  | 4.11 | 181.90 | 2,162 | 1,309 | OK:30 | OK |
| forest_morning | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| tank_gunner_interior | classic | 5.25 | 4.99 |  | 0.09 | 2.15 | 2.75 |  |  |  |  |  |  |  |  |  |  | 4.16 | 128.00 | 2,517 | 1,208 | OK:30 | OK |
| tank_gunner_interior | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 2.72 | 2.53 |  | 0.14 | 1.06 | 1.33 |  |  |  |  |  |  |  |  |  |  | 1.94 | 195.00 | 603 | 314 | OK:30 | OK |
| sinai_lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 5.96 | 5.83 |  | 0.15 | 2.93 | 2.75 |  |  |  |  |  |  |  |  |  |  | 4.73 | 93.50 | 2,243 | 1,100 | OK:30 | OK |
| sinai_desert_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
