# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 9.62 | 9.51 |  | 0.07 | 4.21 | 5.22 |  |  |  |  |  |  |  |  |  |  | 4.79 | 79.30 | 1,779 | 1,179 | OK:30 | OK |
| lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_fields | classic | 8.67 | 8.57 |  | 0.08 | 6.70 | 1.79 |  |  |  |  |  |  |  |  |  |  | 7.07 | 65.20 | 5,877 | 3,920 | OK:30 | OK |
| noon_fields | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 8.62 | 8.49 |  | 0.06 | 4.58 | 3.84 |  |  |  |  |  |  |  |  |  |  | 3.90 | 159.50 | 2,189 | 1,316 | OK:30 | OK |
| forest_edge_golden | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night | classic | 13.18 | 13.02 |  | 0.06 | 1.70 | 11.27 |  |  |  |  |  |  |  |  |  |  | 7.00 | 66.20 | 1,231 | 1,057 | OK:167 | OK |
| town_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| jeep_close_noon | classic | 6.94 | 6.85 |  | 0.08 | 4.77 | 2.00 |  |  |  |  |  |  |  |  |  |  | 3.34 | 84.60 | 2,489 | 1,625 | OK:30 | OK |
| jeep_close_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_noon | classic | 4.69 | 4.57 |  | 0.06 | 1.24 | 3.27 |  |  |  |  |  |  |  |  |  |  | 1.39 | 149.50 | 880 | 473 | OK:30 | OK |
| town_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_morning | classic | 7.73 | 7.61 |  | 0.07 | 4.53 | 3.01 |  |  |  |  |  |  |  |  |  |  | 3.52 | 167.40 | 2,162 | 1,309 | OK:30 | OK |
| forest_morning | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| tank_gunner_interior | classic | 6.73 | 6.25 |  | 0.08 | 2.67 | 3.50 |  |  |  |  |  |  |  |  |  |  | 4.55 | 84.60 | 2,523 | 1,210 | OK:30 | OK |
| tank_gunner_interior | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 4.51 | 4.40 |  | 0.08 | 1.91 | 2.41 |  |  |  |  |  |  |  |  |  |  | 2.63 | 127.50 | 603 | 314 | OK:30 | OK |
| sinai_lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 8.96 | 8.84 |  | 0.11 | 5.20 | 3.53 |  |  |  |  |  |  |  |  |  |  | 5.15 | 67.40 | 2,243 | 1,100 | OK:30 | OK |
| sinai_desert_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
