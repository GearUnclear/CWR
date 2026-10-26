# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 4.13 | 3.96 |  | 0.12 | 1.82 | 2.02 |  |  |  |  |  |  |  |  |  |  | 3.40 | 144.30 | 1,779 | 1,179 | OK:30 | OK |
| lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_fields | classic | 7.17 | 7.07 |  | 0.08 | 5.35 | 1.64 |  |  |  |  |  |  |  |  |  |  | 6.85 | 80.50 | 5,877 | 3,920 | OK:30 | OK |
| noon_fields | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| jeep_close_noon | classic | 4.14 | 4.04 |  | 0.09 | 3.10 | 0.85 |  |  |  |  |  |  |  |  |  |  | 3.78 | 139.20 | 2,489 | 1,625 | OK:30 | OK |
| jeep_close_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 5.42 | 5.27 |  | 0.08 | 3.13 | 2.06 |  |  |  |  |  |  |  |  |  |  | 4.68 | 178.50 | 2,189 | 1,316 | OK:30 | OK |
| forest_edge_golden | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night | classic | 5.63 | 5.42 |  | 0.07 | 1.34 | 4.01 |  |  |  |  |  |  |  |  |  |  | 5.09 | 132.30 | 1,182 | 1,019 | OK:221 | OK |
| town_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_noon | classic | 2.23 | 2.04 |  | 0.10 | 0.62 | 1.32 |  |  |  |  |  |  |  |  |  |  | 1.44 | 222.40 | 880 | 473 | OK:30 | OK |
| town_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_morning | classic | 5.11 | 4.96 |  | 0.09 | 3.08 | 1.80 |  |  |  |  |  |  |  |  |  |  | 4.18 | 182.60 | 2,162 | 1,309 | OK:30 | OK |
| forest_morning | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| tank_gunner_interior | classic | 5.35 | 5.08 |  | 0.09 | 2.16 | 2.82 |  |  |  |  |  |  |  |  |  |  | 4.21 | 127.30 | 2,516 | 1,208 | OK:30 | OK |
| tank_gunner_interior | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 3.61 | 3.49 |  | 0.08 | 1.37 | 2.05 |  |  |  |  |  |  |  |  |  |  | 1.68 | 170.80 | 603 | 314 | OK:30 | OK |
| sinai_lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 6.71 | 6.60 |  | 0.12 | 3.55 | 2.93 |  |  |  |  |  |  |  |  |  |  | 4.78 | 83.30 | 2,243 | 1,100 | OK:30 | OK |
| sinai_desert_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
