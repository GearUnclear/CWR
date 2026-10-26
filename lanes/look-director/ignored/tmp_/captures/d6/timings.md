# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| noon_fields | classic | 8.40 | 8.28 |  | 0.11 | 6.08 | 2.09 |  |  |  |  |  |  |  |  |  |  | 8.34 | 71.70 | 6,767 | 4,508 | OK:30 | OK |
| noon_fields | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| lowsun_fp | classic | 4.41 | 4.22 |  | 0.16 | 1.76 | 2.30 |  |  |  |  |  |  |  |  |  |  | 3.27 | 152.30 | 2,091 | 1,316 | OK:30 | OK |
| lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| blue_hour_coast | classic | 1.33 | 1.03 |  | 0.63 | 0.00 | 0.40 |  |  |  |  |  |  |  |  |  |  | 0.91 | 360.90 | 182 | 0 | OK:30 | OK |
| blue_hour_coast | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night | classic | 6.27 | 5.98 |  | 0.13 | 1.27 | 4.59 |  |  |  |  |  |  |  |  |  |  | 5.80 | 123.80 | 1,213 | 1,047 | OK:171 | OK |
| town_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| jeep_close_noon | classic | 5.01 | 4.90 |  | 0.13 | 3.49 | 1.29 |  |  |  |  |  |  |  |  |  |  | 4.03 | 135.60 | 2,723 | 1,774 | OK:30 | OK |
| jeep_close_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 7.49 | 7.32 |  | 0.18 | 3.52 | 3.62 |  |  |  |  |  |  |  |  |  |  | 5.58 | 79.70 | 2,689 | 1,363 | OK:30 | OK |
| sinai_desert_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
