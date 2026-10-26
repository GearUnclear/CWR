# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| firefight_1 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| firefight_1 | remastered | 16.78 | 12.23 |  | 1.80 | 5.67 | 4.76 | 0.15 |  |  | 2.97 | 0.10 | 0.01 | 0.16 | 0.05 |  | 0.12 | 10.65 | 64.40 | 467 | 504 | UNSETTLED:300 | OK |
| firefight_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| firefight_2 | remastered | 16.78 | 12.23 |  | 1.80 | 5.67 | 4.76 | 0.15 |  |  | 2.97 | 0.10 | 0.01 | 0.16 | 0.05 |  | 0.12 | 10.65 | 64.40 | 557 | 656 | UNSETTLED:300 | OK |
| firefight_3 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| firefight_3 | remastered | 16.78 | 12.23 |  | 1.80 | 5.67 | 4.76 | 0.15 |  |  | 2.97 | 0.10 | 0.01 | 0.16 | 0.05 |  | 0.12 | 10.65 | 64.40 | 534 | 574 | UNSETTLED:300 | OK |
| town_night | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| ffh | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| ffh | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| ffh_day | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| ffh_day | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
