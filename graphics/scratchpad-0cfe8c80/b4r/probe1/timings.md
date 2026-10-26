# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| combat_eye_k00 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 801 | 679 |  | OK |
| combat_eye_k00 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_eye_k05 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 810 | 708 |  | OK |
| combat_eye_k05 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_eye_k06 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 849 | 749 |  | OK |
| combat_eye_k06 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_eye_k08 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 842 | 750 |  | OK |
| combat_eye_k08 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_eye_k11 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 832 | 748 |  | OK |
| combat_eye_k11 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
