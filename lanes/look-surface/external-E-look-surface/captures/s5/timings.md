# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 5.77 | 5.66 |  | 0.08 | 2.42 | 3.16 |  |  |  |  |  |  |  |  |  |  | 2.97 | 115.60 | 1,779 | 1,179 | OK:30 | OK |
| lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_fields | classic | 14.76 | 14.66 |  | 0.08 | 11.03 | 3.54 |  |  |  |  |  |  |  |  |  |  | 7.52 | 50.50 | 5,877 | 3,920 | OK:30 | OK |
| noon_fields | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 6.41 | 6.28 |  | 0.07 | 4.27 | 1.94 |  |  |  |  |  |  |  |  |  |  | 3.90 | 179.70 | 2,189 | 1,316 | OK:30 | OK |
| forest_edge_golden | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night | classic | 7.69 | 7.54 |  | 0.06 | 1.47 | 6.01 |  |  |  |  |  |  |  |  |  |  | 4.75 | 92.80 | 1,215 | 1,057 | OK:260 | OK |
| town_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| jeep_close_noon | classic | 3.60 | 3.52 |  | 0.08 | 2.72 | 0.71 |  |  |  |  |  |  |  |  |  |  | 3.14 | 130.30 | 2,489 | 1,625 | OK:30 | OK |
| jeep_close_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_noon | classic | 2.01 | 1.89 |  | 0.06 | 0.48 | 1.35 |  |  |  |  |  |  |  |  |  |  | 1.37 | 192.20 | 880 | 473 | OK:30 | OK |
| town_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_morning | classic | 6.12 | 6.00 |  | 0.07 | 3.16 | 2.78 |  |  |  |  |  |  |  |  |  |  | 3.47 | 183.00 | 2,162 | 1,309 | OK:30 | OK |
| forest_morning | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| tank_gunner_interior | classic | 9.23 | 8.60 |  | 0.08 | 3.63 | 4.88 |  |  |  |  |  |  |  |  |  |  | 4.28 | 85.20 | 2,520 | 1,208 | OK:30 | OK |
| tank_gunner_interior | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 7.24 | 7.13 |  | 0.07 | 2.18 | 4.87 |  |  |  |  |  |  |  |  |  |  | 4.41 | 103.70 | 603 | 314 | OK:30 | OK |
| sinai_lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 8.49 | 8.38 |  | 0.11 | 3.75 | 4.52 |  |  |  |  |  |  |  |  |  |  | 4.83 | 72.20 | 2,243 | 1,100 | OK:30 | OK |
| sinai_desert_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
