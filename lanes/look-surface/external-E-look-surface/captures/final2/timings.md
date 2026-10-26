# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 8.96 | 8.85 |  | 0.07 | 3.94 | 4.83 |  |  |  |  |  |  |  |  |  |  | 3.13 | 96.90 | 1,779 | 1,179 | OK:30 | OK |
| lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_fields | classic | 17.55 | 17.44 |  | 0.08 | 13.41 | 3.96 |  |  |  |  |  |  |  |  |  |  | 16.02 | 41.80 | 5,877 | 3,920 | OK:30 | OK |
| noon_fields | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 6.41 | 6.28 |  | 0.06 | 4.10 | 2.12 |  |  |  |  |  |  |  |  |  |  | 3.86 | 180.90 | 2,189 | 1,316 | OK:30 | OK |
| forest_edge_golden | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night | classic | 11.60 | 11.44 |  | 0.06 | 1.90 | 9.48 |  |  |  |  |  |  |  |  |  |  | 4.59 | 73.20 | 1,126 | 966 | OK:224 | OK |
| town_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| jeep_close_noon | classic | 6.71 | 6.63 |  | 0.08 | 4.74 | 1.81 |  |  |  |  |  |  |  |  |  |  | 3.44 | 73.10 | 2,489 | 1,625 | OK:30 | OK |
| jeep_close_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_noon | classic | 2.42 | 2.30 |  | 0.06 | 0.55 | 1.69 |  |  |  |  |  |  |  |  |  |  | 1.49 | 143.90 | 880 | 473 | OK:30 | OK |
| town_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_morning | classic | 7.41 | 7.29 |  | 0.07 | 4.36 | 2.87 |  |  |  |  |  |  |  |  |  |  | 4.09 | 116.10 | 2,162 | 1,309 | OK:30 | OK |
| forest_morning | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| tank_gunner_interior | classic | 5.17 | 4.87 |  | 0.09 | 2.07 | 2.70 |  |  |  |  |  |  |  |  |  |  | 4.37 | 123.80 | 2,520 | 1,208 | OK:30 | OK |
| tank_gunner_interior | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 2.45 | 2.33 |  | 0.08 | 1.31 | 0.94 |  |  |  |  |  |  |  |  |  |  | 1.69 | 199.50 | 603 | 314 | OK:30 | OK |
| sinai_lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 7.58 | 7.47 |  | 0.11 | 3.46 | 3.90 |  |  |  |  |  |  |  |  |  |  | 5.70 | 63.20 | 2,243 | 1,100 | OK:30 | OK |
| sinai_desert_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
