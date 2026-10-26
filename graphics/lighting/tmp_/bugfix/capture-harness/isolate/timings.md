# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| sinai_lowsun_fp | classic | 2.37 | 2.21 |  |  |  |  |  |  | 189.70 |  |  |  | OK |
| sinai_lowsun_fp | remastered | 3.77 | 2.67 | 0.08 | 0.95 | 0.55 | 0.06 | 0.28 | 0.06 | 137.20 |  |  |  | OK |
| sinai_lowsun_fp | delta | +1.40 | +0.46 |  |  |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 7.03 | 6.92 |  |  |  |  |  |  | 89.30 |  |  |  | OK |
| sinai_desert_noon | remastered | 6.66 | 5.78 | 0.04 | 0.76 | 0.69 | 0.05 |  | 0.03 | 86.90 |  |  |  | OK |
| sinai_desert_noon | delta | -0.37 | -1.14 |  |  |  |  |  |  |  |  |  |  |  |
