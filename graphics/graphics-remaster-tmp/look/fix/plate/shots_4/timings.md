# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| overcast_broken | classic | 26.87 | 26.77 |  | 0.11 | 20.34 | 6.32 |  |  |  |  |  |  |  |  |  |  | 10.59 | 30.10 | 7,001 | 5,083 | OK:30 | OK |
| overcast_broken | remastered | 21.74 | 19.15 | 0.14 | 5.91 | 12.16 | 1.08 | 0.11 |  |  | 2.30 | 0.18 | 0.01 | 0.15 | 0.06 |  | 0.12 | 19.83 | 37.10 | 1,863 | 5,083 | OK:30 | OK |
| overcast_broken | delta | -5.13 | -7.62 |  | +5.80 | -8.18 | -5.24 |  |  |  |  |  |  |  |  |  |  | +9.24 |  | -73.4% | +0.0% |  |  |
| overcast_heavy | classic | 18.76 | 18.65 |  | 0.09 | 15.89 | 2.67 |  |  |  |  |  |  |  |  |  |  | 17.50 | 40.10 | 6,757 | 5,083 | OK:30 | OK |
| overcast_heavy | remastered | 14.40 | 11.90 | 0.17 | 3.19 | 7.67 | 1.04 | 0.11 |  |  | 2.22 | 0.22 | 0.01 | 0.15 | 0.06 |  | 0.11 | 8.35 | 44.60 | 1,619 | 5,083 | OK:30 | OK |
| overcast_heavy | delta | -4.36 | -6.75 |  | +3.10 | -8.22 | -1.63 |  |  |  |  |  |  |  |  |  |  | -9.15 |  | -76.0% | +0.0% |  |  |
