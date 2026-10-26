# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| overcast_broken | classic | 11.96 | 11.83 |  | 0.15 | 8.91 | 2.77 |  |  |  |  |  |  |  |  |  |  | 11.28 | 51.00 | 8,243 | 5,692 | OK:30 | OK |
| overcast_broken | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| overcast_heavy | classic | 11.60 | 11.46 |  | 0.12 | 8.65 | 2.69 |  |  |  |  |  |  |  |  |  |  | 11.13 | 52.80 | 7,957 | 5,692 | OK:30 | OK |
| overcast_heavy | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| rain_day | classic | 4.15 | 3.95 |  | 0.12 | 2.00 | 1.83 |  |  |  |  |  |  |  |  |  |  | 2.70 | 158.90 | 1,746 | 778 | OK:30 | OK |
| rain_day | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_vista | classic | 20.76 | 20.64 |  | 6.85 | 11.65 | 2.14 |  |  |  |  |  |  |  |  |  |  | 19.60 | 41.80 | 10,841 | 7,519 | OK:30 | OK |
| noon_vista | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
