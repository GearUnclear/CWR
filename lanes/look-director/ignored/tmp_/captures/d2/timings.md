# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| noon_fields | classic | 16.89 | 16.76 |  | 0.11 | 12.06 | 4.59 |  |  |  |  |  |  |  |  |  |  | 8.77 | 45.70 | 6,767 | 4,508 | OK:30 | OK |
| noon_fields | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| lowsun_fp | classic | 6.51 | 6.37 |  | 0.11 | 2.49 | 3.77 |  |  |  |  |  |  |  |  |  |  | 3.55 | 108.30 | 2,091 | 1,316 | OK:30 | OK |
| lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sunrise_ridge | classic | 30.92 | 30.77 |  | 5.43 | 21.48 | 3.86 |  |  |  |  |  |  |  |  |  |  | 17.15 | 23.10 | 13,076 | 8,897 | OK:30 | OK |
| sunrise_ridge | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| woods_pine2_evening | classic | 10.13 | 9.99 |  | 0.12 | 5.63 | 4.24 |  |  |  |  |  |  |  |  |  |  | 5.08 | 70.70 | 3,250 | 1,811 | OK:30 | OK |
| woods_pine2_evening | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| blue_hour_coast | classic | 1.32 | 1.16 |  | 0.22 | 0.00 | 0.94 |  |  |  |  |  |  |  |  |  |  | 0.58 | 328.30 | 182 | 0 | OK:30 | OK |
| blue_hour_coast | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| dawn_valley | classic | 20.35 | 20.22 |  | 3.27 | 14.50 | 2.45 |  |  |  |  |  |  |  |  |  |  | 16.58 | 37.50 | 11,553 | 7,966 | OK:30 | OK |
| dawn_valley | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| pine_rays | classic | 5.65 | 5.45 |  | 0.08 | 2.36 | 3.01 |  |  |  |  |  |  |  |  |  |  | 4.37 | 121.30 | 3,163 | 1,678 | OK:30 | OK |
| pine_rays | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| jeep_close_noon | classic | 5.01 | 4.90 |  | 0.13 | 3.45 | 1.32 |  |  |  |  |  |  |  |  |  |  | 4.10 | 128.40 | 2,723 | 1,774 | OK:30 | OK |
| jeep_close_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| overcast_broken | classic | 11.10 | 10.98 |  | 0.15 | 8.29 | 2.53 |  |  |  |  |  |  |  |  |  |  | 10.85 | 53.70 | 8,243 | 5,692 | OK:30 | OK |
| overcast_broken | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| rain_day | classic | 4.03 | 3.81 |  | 0.13 | 1.94 | 1.74 |  |  |  |  |  |  |  |  |  |  | 2.67 | 163.40 | 1,751 | 783 | OK:30 | OK |
| rain_day | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_vista | classic | 20.86 | 20.73 |  | 6.88 | 11.80 | 2.05 |  |  |  |  |  |  |  |  |  |  | 19.36 | 41.70 | 10,841 | 7,519 | OK:30 | OK |
| noon_vista | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 7.68 | 7.51 |  | 0.19 | 3.63 | 3.69 |  |  |  |  |  |  |  |  |  |  | 5.67 | 75.70 | 2,689 | 1,363 | OK:30 | OK |
| sinai_desert_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
