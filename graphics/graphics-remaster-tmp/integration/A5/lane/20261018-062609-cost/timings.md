# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | resolve | lindepth | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| a5cost_forest | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| a5cost_forest | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| a5cost_lowsun | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| a5cost_lowsun | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| a5cost_town | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| a5cost_town | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
