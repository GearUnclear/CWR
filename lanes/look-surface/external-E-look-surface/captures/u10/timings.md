# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 3.30 | 3.19 |  | 0.07 | 1.32 | 1.80 |  |  |  |  |  |  |  |  |  |  | 2.85 | 164.60 | 1,779 | 1,179 | OK:30 | OK |
| lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_fields | classic | 11.42 | 11.32 |  | 0.08 | 9.58 | 1.66 |  |  |  |  |  |  |  |  |  |  | 7.05 | 59.20 | 5,877 | 3,920 | OK:30 | OK |
| noon_fields | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 8.54 | 8.41 |  | 0.07 | 4.81 | 3.54 |  |  |  |  |  |  |  |  |  |  | 3.91 | 182.10 | 2,189 | 1,316 | OK:30 | OK |
| forest_edge_golden | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night | classic | 9.04 | 8.89 |  | 0.06 | 1.46 | 7.37 |  |  |  |  |  |  |  |  |  |  | 4.97 | 93.10 | 1,194 | 1,027 | OK:215 | OK |
| town_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| jeep_close_noon | classic | 3.72 | 3.63 |  | 0.08 | 2.61 | 0.94 |  |  |  |  |  |  |  |  |  |  | 3.13 | 148.70 | 2,489 | 1,625 | OK:30 | OK |
| jeep_close_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_noon | classic | 1.88 | 1.74 |  | 0.07 | 0.60 | 1.07 |  |  |  |  |  |  |  |  |  |  | 1.36 | 231.70 | 880 | 473 | OK:30 | OK |
| town_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_morning | classic | 4.12 | 4.00 |  | 0.07 | 2.55 | 1.39 |  |  |  |  |  |  |  |  |  |  | 3.49 | 186.50 | 2,162 | 1,309 | OK:30 | OK |
| forest_morning | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| tank_gunner_interior | classic | 6.92 | 6.30 |  | 0.08 | 2.14 | 4.08 |  |  |  |  |  |  |  |  |  |  | 4.21 | 97.40 | 2,524 | 1,209 | OK:30 | OK |
| tank_gunner_interior | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 3.01 | 2.90 |  | 0.08 | 1.16 | 1.66 |  |  |  |  |  |  |  |  |  |  | 1.92 | 174.00 | 603 | 314 | OK:30 | OK |
| sinai_lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 5.70 | 5.59 |  | 0.11 | 3.01 | 2.47 |  |  |  |  |  |  |  |  |  |  | 4.71 | 87.50 | 2,243 | 1,100 | OK:30 | OK |
| sinai_desert_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
