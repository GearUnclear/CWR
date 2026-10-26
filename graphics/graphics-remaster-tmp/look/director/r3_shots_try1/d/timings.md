# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| overcast_broken | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| overcast_broken | remastered | 9.20 | 7.64 | 0.14 | 2.45 | 4.71 | 0.48 | 0.11 |  |  | 1.27 | 0.26 | 0.01 | 0.15 | 0.06 |  | 0.19 | 8.95 | 63.50 | 1,863 | 5,083 | OK:30 | OK |
| overcast_heavy | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| overcast_heavy | remastered | 9.00 | 7.15 | 0.14 | 1.97 | 4.71 | 0.47 | 0.11 |  |  | 1.56 | 0.29 | 0.01 | 0.15 | 0.06 |  | 0.12 | 8.74 | 66.70 | 1,619 | 5,083 | OK:30 | OK |
| rain_day | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| rain_day | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_vista | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_vista | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | remastered | 6.49 | 4.92 | 0.21 | 1.09 | 2.68 | 1.16 | 0.12 |  |  | 1.25 | 0.29 | 0.01 | 0.21 | 0.06 |  | 0.11 | 5.66 | 97.30 | 417 | 314 | UNSETTLED:300 | OK |
| sinai_desert_noon | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | remastered | 8.20 | 6.52 | 0.10 | 1.24 | 3.25 | 2.03 | 0.12 |  |  | 1.39 | 0.28 | 0.01 | 0.17 | 0.06 |  | 0.35 | 7.51 | 80.30 | 1,181 | 1,100 | OK:30 | OK |
| optics | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| optics | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
