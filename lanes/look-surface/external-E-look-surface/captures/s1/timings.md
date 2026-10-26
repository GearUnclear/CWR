# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 4.62 | 4.47 |  | 0.11 | 1.77 | 2.60 |  |  |  |  |  |  |  |  |  |  | 3.26 | 122.10 | 1,779 | 1,179 | OK:30 | OK |
| lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_fields | classic | 7.24 | 7.14 |  | 0.08 | 5.46 | 1.60 |  |  |  |  |  |  |  |  |  |  | 6.74 | 80.10 | 5,877 | 3,920 | OK:30 | OK |
| noon_fields | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 5.55 | 5.40 |  | 0.08 | 3.11 | 2.21 |  |  |  |  |  |  |  |  |  |  | 4.67 | 177.30 | 2,189 | 1,316 | OK:30 | OK |
| forest_edge_golden | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night | classic | 5.83 | 5.65 |  | 0.06 | 1.43 | 4.16 |  |  |  |  |  |  |  |  |  |  | 5.19 | 129.20 | 1,234 | 1,068 | OK:283 | OK |
| town_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| jeep_close_noon | classic | 4.52 | 4.42 |  | 0.09 | 3.35 | 0.98 |  |  |  |  |  |  |  |  |  |  | 3.83 | 127.90 | 2,489 | 1,625 | OK:30 | OK |
| jeep_close_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_noon | classic | 2.38 | 2.19 |  | 0.10 | 0.72 | 1.36 |  |  |  |  |  |  |  |  |  |  | 1.45 | 213.50 | 880 | 473 | OK:30 | OK |
| town_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_morning | classic | 4.88 | 4.74 |  | 0.09 | 2.94 | 1.71 |  |  |  |  |  |  |  |  |  |  | 4.11 | 185.90 | 2,162 | 1,309 | OK:30 | OK |
| forest_morning | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| tank_gunner_interior | classic | 5.33 | 5.06 |  | 0.09 | 2.09 | 2.88 |  |  |  |  |  |  |  |  |  |  | 4.18 | 123.10 | 2,519 | 1,208 | OK:30 | OK |
| tank_gunner_interior | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 4.48 | 4.36 |  | 0.07 | 2.08 | 2.21 |  |  |  |  |  |  |  |  |  |  | 1.48 | 154.20 | 603 | 314 | OK:30 | OK |
| sinai_lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 10.97 | 10.85 |  | 0.11 | 6.35 | 4.39 |  |  |  |  |  |  |  |  |  |  | 5.47 | 60.70 | 2,243 | 1,100 | OK:30 | OK |
| sinai_desert_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
