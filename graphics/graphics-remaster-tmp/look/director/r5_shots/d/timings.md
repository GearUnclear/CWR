# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| overcast_broken | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| overcast_broken | remastered | 32.70 | 28.78 | 0.14 | 8.49 | 18.26 | 2.03 | 0.11 |  |  | 3.63 | 0.20 | 0.01 | 0.15 | 0.23 |  | 0.55 | 23.15 | 27.60 | 1,863 | 5,083 | OK:30 | OK |
| overcast_heavy | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| overcast_heavy | remastered | 20.12 | 17.06 | 0.14 | 4.36 | 11.50 | 1.19 | 0.11 |  |  | 2.77 | 0.12 | 0.01 | 0.15 | 0.06 |  | 0.12 | 18.17 | 40.20 | 1,619 | 5,083 | OK:30 | OK |
| rain_day | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| rain_day | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_vista | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_vista | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | remastered | 8.80 | 6.80 | 0.33 | 1.42 | 3.66 | 1.73 | 0.13 |  |  | 1.66 | 0.24 | 0.01 | 0.21 | 0.11 |  | 0.16 | 6.16 | 70.00 | 417 | 314 | UNSETTLED:300 | OK |
| sinai_desert_noon | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | remastered | 7.99 | 6.25 | 0.10 | 1.23 | 3.05 | 1.97 | 0.12 |  |  | 1.43 | 0.29 | 0.01 | 0.17 | 0.06 |  | 0.28 | 7.47 | 81.30 | 1,181 | 1,100 | OK:30 | OK |
| optics | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| optics | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
