# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 4.72 | 4.58 |  | 0.10 | 1.68 | 2.80 |  |  |  |  |  |  |  |  |  |  | 3.38 | 128.70 | 2,091 | 1,316 | OK:30 | OK |
| lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_fields | classic | 13.74 | 13.61 |  | 0.11 | 9.91 | 3.59 |  |  |  |  |  |  |  |  |  |  | 8.83 | 43.90 | 6,767 | 4,508 | OK:30 | OK |
| noon_fields | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 5.33 | 5.17 |  | 0.09 | 2.59 | 2.49 |  |  |  |  |  |  |  |  |  |  | 4.53 | 96.00 | 2,502 | 1,459 | OK:30 | OK |
| forest_edge_golden | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sunrise_ridge | classic | 23.82 | 23.67 |  | 0.12 | 20.24 | 3.31 |  |  |  |  |  |  |  |  |  |  | 18.33 | 27.70 | 13,076 | 8,897 | OK:30 | OK |
| sunrise_ridge | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| coastline_sunset | classic | 1.39 | 1.16 |  | 0.63 | 0.00 | 0.53 |  |  |  |  |  |  |  |  |  |  | 0.88 | 350.60 | 177 | 0 | OK:30 | OK |
| coastline_sunset | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night | classic | 6.23 | 5.99 |  | 0.11 | 1.32 | 4.55 |  |  |  |  |  |  |  |  |  |  | 5.84 | 119.10 | 1,238 | 1,060 | OK:204 | OK |
| town_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| overcast_broken | classic | 18.03 | 17.90 |  | 0.15 | 13.81 | 3.94 |  |  |  |  |  |  |  |  |  |  | 12.44 | 38.90 | 8,243 | 5,692 | OK:30 | OK |
| overcast_broken | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 16.38 | 16.23 |  | 0.15 | 6.78 | 9.30 |  |  |  |  |  |  |  |  |  |  | 8.95 | 41.10 | 2,689 | 1,363 | OK:30 | OK |
| sinai_desert_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
