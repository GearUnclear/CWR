# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | resolve | lindepth | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| a5_sinai_lowsun_fp | off | 4.76 | 3.28 | 0.11 | 1.33 | 0.37 |  | 0.15 | 0.06 | 0.37 | 0.37 | 127.50 | 764 | 371 | OK:30 | OK |
| a5_sinai_lowsun_fp | on | 7.51 | 5.52 | 0.23 | 1.81 | 0.45 |  | 0.15 | 0.07 | 0.37 | 0.53 | 95.40 | 764 | 371 | OK:30 | OK |
| a5_sinai_lowsun_fp | delta | +2.75 | +2.24 | +0.12 | +0.48 | +0.08 |  | +0.00 | +0.01 | +0.00 | +0.16 |  | +0.0% | +0.0% |  |  |
