# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 4.22 | 4.06 |  | 0.12 | 1.68 | 2.26 |  |  |  |  |  |  |  |  |  |  | 3.33 | 144.20 | 1,779 | 1,179 | OK:30 | OK |
| lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_fields | classic | 7.28 | 7.18 |  | 0.08 | 5.43 | 1.67 |  |  |  |  |  |  |  |  |  |  | 6.76 | 75.80 | 5,877 | 3,920 | OK:30 | OK |
| noon_fields | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 5.59 | 5.43 |  | 0.08 | 3.02 | 2.33 |  |  |  |  |  |  |  |  |  |  | 4.59 | 140.90 | 2,189 | 1,316 | OK:30 | OK |
| forest_edge_golden | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night | classic | 6.27 | 6.09 |  | 0.08 | 1.33 | 4.68 |  |  |  |  |  |  |  |  |  |  | 5.37 | 125.80 | 1,195 | 1,027 | OK:249 | OK |
| town_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| jeep_close_noon | classic | 4.30 | 4.20 |  | 0.10 | 3.14 | 0.97 |  |  |  |  |  |  |  |  |  |  | 3.50 | 131.10 | 2,489 | 1,625 | OK:30 | OK |
| jeep_close_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_noon | classic | 2.66 | 2.48 |  | 0.09 | 0.66 | 1.73 |  |  |  |  |  |  |  |  |  |  | 1.89 | 120.90 | 880 | 473 | OK:30 | OK |
| town_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_morning | classic | 5.92 | 5.77 |  | 0.08 | 3.02 | 2.67 |  |  |  |  |  |  |  |  |  |  | 4.28 | 105.80 | 2,162 | 1,309 | OK:30 | OK |
| forest_morning | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| tank_gunner_interior | classic | 7.47 | 7.19 |  | 0.09 | 2.11 | 5.00 |  |  |  |  |  |  |  |  |  |  | 4.99 | 85.80 | 2,527 | 1,210 | OK:30 | OK |
| tank_gunner_interior | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 2.63 | 2.44 |  | 0.14 | 1.08 | 1.22 |  |  |  |  |  |  |  |  |  |  | 2.01 | 193.40 | 603 | 314 | OK:30 | OK |
| sinai_lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 8.25 | 8.12 |  | 0.12 | 4.74 | 3.26 |  |  |  |  |  |  |  |  |  |  | 5.66 | 61.00 | 2,242 | 1,100 | OK:30 | OK |
| sinai_desert_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
