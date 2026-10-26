# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| overcast_broken | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| overcast_broken | remastered | 9.14 | 7.62 | 0.14 | 2.22 | 4.94 | 0.47 | 0.11 |  |  | 1.24 | 0.27 | 0.01 | 0.15 | 0.06 |  | 0.19 | 8.90 | 64.40 | 1,863 | 5,083 | OK:30 | OK |
| overcast_heavy | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| overcast_heavy | remastered | 8.98 | 7.13 | 0.14 | 1.87 | 4.76 | 0.50 | 0.11 |  |  | 1.56 | 0.30 | 0.01 | 0.15 | 0.06 |  | 0.11 | 8.69 | 66.50 | 1,619 | 5,083 | OK:30 | OK |
| rain_day | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| rain_day | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_vista | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_vista | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | remastered | 6.64 | 5.01 | 0.21 | 1.13 | 2.72 | 1.16 | 0.12 |  |  | 1.30 | 0.29 | 0.01 | 0.21 | 0.06 |  | 0.12 | 5.67 | 101.60 | 417 | 314 | UNSETTLED:300 | OK |
| sinai_desert_noon | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | remastered | 8.08 | 6.35 | 0.10 | 1.23 | 3.18 | 1.95 | 0.12 |  |  | 1.43 | 0.30 | 0.01 | 0.17 | 0.06 |  | 0.33 | 7.54 | 80.60 | 1,181 | 1,100 | OK:30 | OK |
| optics | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| optics | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
