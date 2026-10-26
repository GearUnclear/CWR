# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 4.26 | 4.15 |  | 0.07 | 1.93 | 2.14 |  |  |  |  |  |  |  |  |  |  | 3.05 | 110.90 | 1,779 | 1,179 | OK:30 | OK |
| lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_fields | classic | 17.26 | 17.16 |  | 0.08 | 12.99 | 4.09 |  |  |  |  |  |  |  |  |  |  | 7.19 | 43.90 | 5,877 | 3,920 | OK:30 | OK |
| noon_fields | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 6.81 | 6.69 |  | 0.06 | 3.49 | 3.13 |  |  |  |  |  |  |  |  |  |  | 3.86 | 181.10 | 2,189 | 1,316 | OK:30 | OK |
| forest_edge_golden | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night | classic | 6.32 | 6.17 |  | 0.06 | 1.53 | 4.59 |  |  |  |  |  |  |  |  |  |  | 4.35 | 113.00 | 1,207 | 1,062 | OK:225 | OK |
| town_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| jeep_close_noon | classic | 4.31 | 4.23 |  | 0.08 | 3.15 | 1.00 |  |  |  |  |  |  |  |  |  |  | 3.15 | 117.70 | 2,489 | 1,625 | OK:30 | OK |
| jeep_close_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_noon | classic | 1.77 | 1.65 |  | 0.06 | 0.43 | 1.17 |  |  |  |  |  |  |  |  |  |  | 1.39 | 176.40 | 880 | 473 | OK:30 | OK |
| town_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_morning | classic | 6.06 | 5.94 |  | 0.07 | 3.32 | 2.55 |  |  |  |  |  |  |  |  |  |  | 3.59 | 132.80 | 2,162 | 1,309 | OK:30 | OK |
| forest_morning | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| tank_gunner_interior | classic | 5.54 | 5.25 |  | 0.10 | 2.10 | 3.06 |  |  |  |  |  |  |  |  |  |  | 4.44 | 112.60 | 2,520 | 1,208 | OK:30 | OK |
| tank_gunner_interior | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 3.53 | 3.42 |  | 0.07 | 1.36 | 1.98 |  |  |  |  |  |  |  |  |  |  | 1.36 | 163.80 | 603 | 314 | OK:30 | OK |
| sinai_lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
