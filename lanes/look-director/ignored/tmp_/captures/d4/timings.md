# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| sunrise_ridge | classic | 40.93 | 40.78 |  | 7.84 | 26.82 | 6.13 |  |  |  |  |  |  |  |  |  |  | 36.14 | 22.40 | 13,076 | 8,897 | OK:30 | OK |
| sunrise_ridge | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| woods_pine2_evening | classic | 10.18 | 10.03 |  | 0.12 | 5.80 | 4.11 |  |  |  |  |  |  |  |  |  |  | 5.01 | 73.20 | 3,250 | 1,811 | OK:30 | OK |
| woods_pine2_evening | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| blue_hour_coast | classic | 1.30 | 1.14 |  | 0.22 | 0.00 | 0.92 |  |  |  |  |  |  |  |  |  |  | 0.57 | 330.80 | 182 | 0 | OK:30 | OK |
| blue_hour_coast | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night | classic | 8.29 | 8.11 |  | 0.07 | 1.59 | 6.45 |  |  |  |  |  |  |  |  |  |  | 4.98 | 101.10 | 1,288 | 1,115 | OK:271 | OK |
| town_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_fog_night | classic | 8.27 | 8.10 |  | 0.08 | 1.57 | 6.45 |  |  |  |  |  |  |  |  |  |  | 4.98 | 104.80 | 1,227 | 980 | OK:134 | OK |
| town_fog_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night_rain | classic | 5.55 | 5.38 |  | 0.08 | 0.98 | 4.32 |  |  |  |  |  |  |  |  |  |  | 2.94 | 145.00 | 919 | 693 | OK:30 | OK |
| town_night_rain | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| rain_day | classic | 3.38 | 3.24 |  | 0.09 | 1.69 | 1.47 |  |  |  |  |  |  |  |  |  |  | 2.78 | 133.60 | 1,746 | 778 | OK:30 | OK |
| rain_day | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
