# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| d_hill | off | 19.37 | 18.18 | 0.08 | 0.29 | 15.73 | 2.15 | 1.02 | 0.51 |  | 0.13 | 0.06 | 0.27 | 0.05 | 18.75 | 34.60 | 12,910 | 8,897 | OK:30 | OK |
| d_hill | on | 5.07 | 3.76 | 0.12 | 0.40 | 3.01 | 0.35 | 1.11 | 0.36 |  | 0.18 | 0.09 | 0.38 | 0.10 | 4.33 | 67.50 | 3,016 | 8,897 | OK:30 | OK |
| d_hill | delta | -14.30 | -14.42 | +0.04 | +0.11 | -12.72 | -1.80 | +0.09 | -0.15 |  | +0.05 | +0.03 | +0.11 | +0.05 | -14.42 |  | -76.6% | +0.0% |  |  |
