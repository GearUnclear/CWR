# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 4.37 | 4.19 |  | 0.13 | 1.78 | 2.29 |  |  |  |  |  |  |  |  |  |  | 3.56 | 126.90 | 1,779 | 1,179 | OK:30 | OK |
| lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_fields | classic | 11.23 | 11.13 |  | 0.08 | 7.82 | 3.23 |  |  |  |  |  |  |  |  |  |  | 7.15 | 59.60 | 5,877 | 3,920 | OK:30 | OK |
| noon_fields | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 5.89 | 5.70 |  | 0.10 | 3.31 | 2.29 |  |  |  |  |  |  |  |  |  |  | 5.34 | 165.70 | 2,189 | 1,316 | OK:30 | OK |
| forest_edge_golden | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night | classic | 5.36 | 5.18 |  | 0.08 | 1.24 | 3.86 |  |  |  |  |  |  |  |  |  |  | 4.53 | 124.80 | 1,125 | 968 | OK:158 | OK |
| town_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| jeep_close_noon | classic | 4.04 | 3.95 |  | 0.08 | 3.06 | 0.80 |  |  |  |  |  |  |  |  |  |  | 3.32 | 136.50 | 2,489 | 1,625 | OK:30 | OK |
| jeep_close_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_noon | classic | 2.16 | 2.05 |  | 0.06 | 0.73 | 1.26 |  |  |  |  |  |  |  |  |  |  | 1.36 | 221.90 | 880 | 473 | OK:30 | OK |
| town_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_morning | classic | 7.74 | 7.62 |  | 0.07 | 4.85 | 2.70 |  |  |  |  |  |  |  |  |  |  | 3.51 | 184.30 | 2,162 | 1,309 | OK:30 | OK |
| forest_morning | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| tank_gunner_interior | classic | 9.24 | 8.49 |  | 0.08 | 2.99 | 5.43 |  |  |  |  |  |  |  |  |  |  | 4.41 | 91.00 | 2,529 | 1,211 | OK:30 | OK |
| tank_gunner_interior | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 3.40 | 3.29 |  | 0.07 | 1.41 | 1.81 |  |  |  |  |  |  |  |  |  |  | 1.40 | 159.80 | 602 | 314 | OK:30 | OK |
| sinai_lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 11.41 | 11.30 |  | 0.12 | 5.36 | 5.82 |  |  |  |  |  |  |  |  |  |  | 5.37 | 66.80 | 2,242 | 1,100 | OK:30 | OK |
| sinai_desert_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
