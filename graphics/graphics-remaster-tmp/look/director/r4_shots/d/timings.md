# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| overcast_broken | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| overcast_broken | remastered | 42.01 | 37.26 | 0.14 | 10.82 | 23.53 | 2.90 | 0.11 |  |  | 4.46 | 0.13 | 0.01 | 0.15 | 0.06 |  | 0.40 | 30.33 | 22.30 | 1,863 | 5,083 | OK:30 | OK |
| overcast_heavy | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| overcast_heavy | remastered | 40.61 | 34.70 | 0.14 | 9.30 | 23.32 | 2.07 | 0.11 |  |  | 5.41 | 0.13 | 0.01 | 0.15 | 0.59 |  | 0.12 | 37.54 | 23.30 | 1,619 | 5,083 | OK:30 | OK |
| rain_day | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| rain_day | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_vista | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_vista | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | remastered | 28.78 | 22.56 | 0.76 | 3.81 | 12.04 | 6.72 | 0.12 |  |  | 4.39 | 0.13 | 0.01 | 0.22 | 0.14 |  | 0.36 | 19.53 | 35.40 | 582 | 314 | UNSETTLED:300 | OK |
| sinai_desert_noon | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | remastered | 27.40 | 23.64 | 0.89 | 3.59 | 11.45 | 8.60 | 0.12 |  |  | 3.41 | 0.13 | 0.01 | 0.17 | 0.06 |  | 0.18 | 24.48 | 33.70 | 1,181 | 1,100 | OK:30 | OK |
| optics | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| optics | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
