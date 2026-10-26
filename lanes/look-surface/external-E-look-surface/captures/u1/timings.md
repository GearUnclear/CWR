# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 4.41 | 4.30 |  | 0.07 | 2.05 | 2.18 |  |  |  |  |  |  |  |  |  |  | 3.03 | 124.10 | 1,779 | 1,179 | OK:30 | OK |
| lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_fields | classic | 10.55 | 10.44 |  | 0.08 | 8.69 | 1.68 |  |  |  |  |  |  |  |  |  |  | 7.05 | 62.80 | 5,877 | 3,920 | OK:30 | OK |
| noon_fields | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 5.70 | 5.53 |  | 0.10 | 3.20 | 2.23 |  |  |  |  |  |  |  |  |  |  | 4.94 | 131.10 | 2,189 | 1,316 | OK:30 | OK |
| forest_edge_golden | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night | classic | 7.38 | 7.00 |  | 0.18 | 1.11 | 5.70 |  |  |  |  |  |  |  |  |  |  | 6.05 | 117.00 | 1,230 | 1,071 | OK:227 | OK |
| town_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| jeep_close_noon | classic | 4.50 | 4.40 |  | 0.10 | 3.40 | 0.89 |  |  |  |  |  |  |  |  |  |  | 3.98 | 136.70 | 2,489 | 1,625 | OK:30 | OK |
| jeep_close_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_noon | classic | 2.69 | 2.57 |  | 0.06 | 0.72 | 1.80 |  |  |  |  |  |  |  |  |  |  | 1.33 | 223.30 | 880 | 473 | OK:30 | OK |
| town_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_morning | classic | 5.77 | 5.65 |  | 0.07 | 2.96 | 2.63 |  |  |  |  |  |  |  |  |  |  | 3.46 | 188.20 | 2,162 | 1,309 | OK:30 | OK |
| forest_morning | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| tank_gunner_interior | classic | 4.91 | 4.48 |  | 0.08 | 1.71 | 2.68 |  |  |  |  |  |  |  |  |  |  | 4.09 | 129.10 | 2,519 | 1,208 | OK:30 | OK |
| tank_gunner_interior | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 3.04 | 2.92 |  | 0.08 | 1.33 | 1.51 |  |  |  |  |  |  |  |  |  |  | 1.88 | 191.10 | 603 | 314 | OK:30 | OK |
| sinai_lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 8.19 | 8.07 |  | 0.11 | 3.70 | 4.26 |  |  |  |  |  |  |  |  |  |  | 4.63 | 86.30 | 2,243 | 1,100 | OK:30 | OK |
| sinai_desert_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
