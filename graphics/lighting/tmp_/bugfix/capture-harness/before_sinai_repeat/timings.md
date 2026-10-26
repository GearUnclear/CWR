# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| sinai_lowsun_fp | classic | 2.18 | 2.00 |  |  |  |  |  |  | 178.10 |  |  |  | OK |
| sinai_lowsun_fp | remastered | 6.67 | 4.78 | 0.17 | 1.61 | 0.81 | 0.14 | 0.54 | 0.12 | 118.30 |  |  |  | OK |
| sinai_lowsun_fp | delta | +4.49 | +2.78 |  |  |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 6.28 | 6.11 |  |  |  |  |  |  | 97.00 |  |  |  | OK |
| sinai_desert_noon | remastered | 7.58 | 6.63 | 0.05 | 0.82 | 0.72 | 0.06 |  | 0.03 | 80.50 |  |  |  | OK |
| sinai_desert_noon | delta | +1.30 | +0.52 |  |  |  |  |  |  |  |  |  |  |  |
