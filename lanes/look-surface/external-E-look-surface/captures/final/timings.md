# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 3.17 | 3.06 |  | 0.07 | 1.19 | 1.79 |  |  |  |  |  |  |  |  |  |  | 2.86 | 163.20 | 1,779 | 1,179 | OK:30 | OK |
| lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_fields | classic | 9.56 | 9.46 |  | 0.08 | 7.60 | 1.78 |  |  |  |  |  |  |  |  |  |  | 7.14 | 63.40 | 5,877 | 3,920 | OK:30 | OK |
| noon_fields | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 6.25 | 6.12 |  | 0.06 | 3.03 | 3.03 |  |  |  |  |  |  |  |  |  |  | 3.85 | 179.70 | 2,189 | 1,316 | OK:30 | OK |
| forest_edge_golden | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night | classic | 11.44 | 11.25 |  | 0.06 | 2.06 | 9.13 |  |  |  |  |  |  |  |  |  |  | 5.15 | 73.30 | 1,221 | 1,055 | OK:270 | OK |
| town_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| jeep_close_noon | classic | 15.59 | 15.50 |  | 0.08 | 11.20 | 4.22 |  |  |  |  |  |  |  |  |  |  | 14.07 | 52.10 | 2,489 | 1,625 | OK:30 | OK |
| jeep_close_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_noon | classic | 4.40 | 4.28 |  | 0.06 | 1.22 | 3.00 |  |  |  |  |  |  |  |  |  |  | 1.37 | 147.60 | 880 | 473 | OK:30 | OK |
| town_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_morning | classic | 10.52 | 10.41 |  | 0.07 | 6.58 | 3.76 |  |  |  |  |  |  |  |  |  |  | 3.84 | 176.70 | 2,162 | 1,309 | OK:30 | OK |
| forest_morning | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| tank_gunner_interior | classic | 7.21 | 6.03 |  | 0.08 | 1.75 | 4.20 |  |  |  |  |  |  |  |  |  |  | 4.26 | 121.00 | 2,520 | 1,208 | OK:30 | OK |
| tank_gunner_interior | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 5.08 | 4.97 |  | 0.07 | 1.53 | 3.37 |  |  |  |  |  |  |  |  |  |  | 2.32 | 126.90 | 603 | 314 | OK:30 | OK |
| sinai_lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 7.26 | 7.15 |  | 0.11 | 3.55 | 3.48 |  |  |  |  |  |  |  |  |  |  | 4.90 | 84.80 | 2,243 | 1,100 | OK:30 | OK |
| sinai_desert_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
