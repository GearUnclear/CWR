# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | resolve | lindepth | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| a5sweep_forest | off | 5.41 | 4.69 | 0.25 | 0.52 | 0.25 |  | 0.16 | 0.08 |  | 0.04 | 126.10 | 2,142 | 1,459 | OK:30 | OK |
| a5sweep_forest | on | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| a5sweep_lowsun | off | 4.99 | 3.80 | 0.08 | 1.03 | 0.28 |  | 0.16 | 0.07 | 0.44 | 0.08 | 136.90 | 1,890 | 1,316 | OK:30 | OK |
| a5sweep_lowsun | on | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
