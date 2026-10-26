# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| sinai_lowsun_fp | classic | 3.37 | 3.26 |  |  |  |  |  |  | 161.10 | 603 | 314 | OK:30 | OK |
| sinai_lowsun_fp | remastered | 3.77 | 2.67 | 0.21 | 0.97 | 0.62 | 0.05 | 0.22 | 0.08 | 163.60 | 583 | 314 | OK:30 | OK |
| sinai_lowsun_fp | delta | +0.40 | -0.59 |  |  |  |  |  |  |  | -3.3% | +0.0% |  |  |
| sinai_desert_noon | classic | 7.88 | 7.76 |  |  |  |  |  |  | 82.20 | 2,242 | 1,100 | OK:30 | OK |
| sinai_desert_noon | remastered | 7.89 | 6.79 | 0.19 | 0.96 | 0.89 | 0.05 |  | 0.03 | 64.20 | 2,231 | 1,100 | OK:30 | OK |
| sinai_desert_noon | delta | +0.01 | -0.97 |  |  |  |  |  |  |  | -0.5% | +0.0% |  |  |
