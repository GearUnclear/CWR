# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 9.61 | 9.50 |  | 0.07 | 4.19 | 5.23 |  |  |  |  |  |  |  |  |  |  | 7.41 | 81.60 | 1,779 | 1,179 | OK:30 | OK |
| lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_fields | classic | 21.78 | 21.67 |  | 0.08 | 16.36 | 5.23 |  |  |  |  |  |  |  |  |  |  | 12.23 | 35.00 | 5,877 | 3,920 | OK:30 | OK |
| noon_fields | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 13.92 | 13.76 |  | 0.07 | 7.86 | 5.83 |  |  |  |  |  |  |  |  |  |  | 12.26 | 179.60 | 2,189 | 1,316 | OK:30 | OK |
| forest_edge_golden | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night | classic | 7.43 | 7.28 |  | 0.06 | 1.61 | 5.61 |  |  |  |  |  |  |  |  |  |  | 4.99 | 97.90 | 1,208 | 1,048 | OK:197 | OK |
| town_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| jeep_close_noon | classic | 6.85 | 6.77 |  | 0.08 | 4.90 | 1.79 |  |  |  |  |  |  |  |  |  |  | 3.50 | 96.50 | 2,489 | 1,625 | OK:30 | OK |
| jeep_close_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_noon | classic | 4.25 | 4.13 |  | 0.06 | 1.16 | 2.91 |  |  |  |  |  |  |  |  |  |  | 1.38 | 150.10 | 880 | 473 | OK:30 | OK |
| town_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_morning | classic | 5.83 | 5.71 |  | 0.07 | 3.17 | 2.47 |  |  |  |  |  |  |  |  |  |  | 3.66 | 181.80 | 2,162 | 1,309 | OK:30 | OK |
| forest_morning | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| tank_gunner_interior | classic | 13.74 | 12.54 |  | 0.08 | 5.11 | 7.35 |  |  |  |  |  |  |  |  |  |  | 4.87 | 66.90 | 2,526 | 1,211 | OK:30 | OK |
| tank_gunner_interior | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 3.16 | 2.96 |  | 0.15 | 1.46 | 1.36 |  |  |  |  |  |  |  |  |  |  | 2.32 | 170.90 | 602 | 314 | OK:30 | OK |
| sinai_lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 8.38 | 8.26 |  | 0.11 | 3.43 | 4.72 |  |  |  |  |  |  |  |  |  |  | 4.80 | 77.90 | 2,243 | 1,100 | OK:30 | OK |
| sinai_desert_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
