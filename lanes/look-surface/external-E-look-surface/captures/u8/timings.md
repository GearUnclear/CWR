# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 3.78 | 3.67 |  | 0.07 | 1.37 | 2.23 |  |  |  |  |  |  |  |  |  |  | 2.85 | 142.70 | 1,779 | 1,179 | OK:30 | OK |
| lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_fields | classic | 11.78 | 11.68 |  | 0.08 | 8.01 | 3.59 |  |  |  |  |  |  |  |  |  |  | 7.74 | 57.80 | 5,877 | 3,920 | OK:30 | OK |
| noon_fields | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 7.38 | 7.26 |  | 0.07 | 4.03 | 3.16 |  |  |  |  |  |  |  |  |  |  | 3.83 | 181.50 | 2,189 | 1,316 | OK:30 | OK |
| forest_edge_golden | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night | classic | 5.52 | 5.37 |  | 0.06 | 1.51 | 3.80 |  |  |  |  |  |  |  |  |  |  | 4.88 | 109.20 | 1,213 | 1,051 | OK:243 | OK |
| town_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| jeep_close_noon | classic | 5.37 | 5.28 |  | 0.08 | 3.66 | 1.53 |  |  |  |  |  |  |  |  |  |  | 3.12 | 113.30 | 2,489 | 1,625 | OK:30 | OK |
| jeep_close_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_noon | classic | 1.43 | 1.32 |  | 0.06 | 0.38 | 0.88 |  |  |  |  |  |  |  |  |  |  | 1.34 | 232.10 | 880 | 473 | OK:30 | OK |
| town_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_morning | classic | 5.09 | 4.94 |  | 0.09 | 3.01 | 1.83 |  |  |  |  |  |  |  |  |  |  | 3.85 | 183.20 | 2,162 | 1,309 | OK:30 | OK |
| forest_morning | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| tank_gunner_interior | classic | 5.38 | 5.10 |  | 0.09 | 2.21 | 2.79 |  |  |  |  |  |  |  |  |  |  | 4.23 | 128.50 | 2,519 | 1,208 | OK:30 | OK |
| tank_gunner_interior | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 3.31 | 3.19 |  | 0.08 | 1.25 | 1.86 |  |  |  |  |  |  |  |  |  |  | 1.90 | 165.30 | 603 | 314 | OK:30 | OK |
| sinai_lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 6.06 | 5.94 |  | 0.11 | 3.19 | 2.65 |  |  |  |  |  |  |  |  |  |  | 4.95 | 74.60 | 2,243 | 1,100 | OK:30 | OK |
| sinai_desert_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
