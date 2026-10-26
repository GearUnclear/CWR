# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| noon_fields | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_fields | remastered | 22.75 | 21.66 | 0.55 | 3.67 | 15.80 | 2.19 | 0.07 |  |  | 0.89 | 0.24 | 0.01 | 0.08 | 0.05 |  | 0.02 | 12.29 | 32.80 | 1,447 | 3,920 | OK:30 | OK |
| lowsun_fp | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| lowsun_fp | remastered | 19.19 | 17.23 | 0.32 | 2.72 | 8.40 | 6.11 | 0.07 |  |  | 1.75 | 0.11 | 0.01 | 0.09 | 0.05 |  | 0.03 | 6.02 | 66.90 | 1,091 | 1,179 | OK:30 | OK |
| forest_edge_golden | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | remastered | 6.12 | 4.79 | 0.20 | 0.97 | 2.50 | 1.32 | 0.08 |  |  | 1.08 | 0.24 | 0.01 | 0.09 | 0.05 |  | 0.02 | 4.59 | 97.70 | 757 | 1,316 | OK:30 | OK |
| sunrise_ridge | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sunrise_ridge | remastered | 13.10 | 10.79 | 0.09 | 3.28 | 6.28 | 1.24 | 0.08 |  |  | 2.07 | 0.14 | 0.01 | 0.09 | 0.05 |  | 0.03 | 8.43 | 47.50 | 1,661 | 7,680 | OK:30 | OK |
| coastline_sunset | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| coastline_sunset | remastered | 3.56 | 1.53 | 0.00 | 1.27 | 0.00 | 0.26 | 0.09 |  |  | 1.81 | 0.10 | 0.01 | 0.09 | 0.11 |  | 0.03 | 2.61 | 240.10 | 154 | 0 | OK:30 | OK |
| woods_pine2_evening | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| woods_pine2_evening | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| blue_hour_coast | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| blue_hour_coast | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| dawn_valley | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| dawn_valley | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| pine_rays | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| pine_rays | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night | remastered | 13.99 | 12.89 | 0.15 | 1.93 | 5.64 | 5.32 | 0.05 |  |  | 0.79 | 0.40 | 0.01 | 0.10 | 0.05 |  | 0.03 | 11.23 | 59.90 | 1,177 | 1,074 | UNSETTLED:300 | OK |
| town_fog_night | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_fog_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night_rain | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night_rain | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| jeep_close_noon | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| jeep_close_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_noon | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_morning | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_morning | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| overcast_broken | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| overcast_broken | remastered | 6.94 | 5.75 | 0.14 | 1.71 | 3.66 | 0.39 | 0.07 |  |  | 0.98 | 0.36 | 0.01 | 0.08 | 0.05 |  | 0.03 | 6.39 | 73.60 | 1,863 | 5,083 | OK:30 | OK |
| overcast_heavy | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| overcast_heavy | remastered | 6.65 | 5.48 | 0.13 | 1.53 | 3.57 | 0.39 | 0.07 |  |  | 0.96 | 0.48 | 0.01 | 0.08 | 0.05 |  | 0.02 | 6.00 | 75.80 | 1,619 | 5,083 | OK:30 | OK |
| rain_day | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| rain_day | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_vista | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_vista | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | remastered | 5.21 | 3.68 | 0.25 | 0.75 | 1.96 | 0.96 | 0.08 |  |  | 1.31 | 0.44 | 0.01 | 0.11 | 0.06 |  | 0.03 | 4.45 | 129.10 | 418 | 314 | UNSETTLED:300 | OK |
| sinai_desert_noon | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | remastered | 9.30 | 7.85 | 0.10 | 1.47 | 3.85 | 2.53 | 0.08 |  |  | 1.23 | 0.58 | 0.01 | 0.08 | 0.05 |  | 0.02 | 5.39 | 82.70 | 1,181 | 1,100 | OK:30 | OK |
| firefight_1 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| firefight_1 | remastered | 4.39 | 2.78 |  | 0.50 | 1.10 | 1.18 | 0.08 |  |  | 1.27 | 0.66 | 0.01 | 0.09 | 0.05 |  | 0.05 | 3.96 | 164.40 | 471 | 473 | UNSETTLED:300 | OK |
| firefight_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| firefight_2 | remastered | 4.39 | 2.78 |  | 0.50 | 1.10 | 1.18 | 0.08 |  |  | 1.27 | 0.66 | 0.01 | 0.09 | 0.05 |  | 0.05 | 3.96 | 164.40 | 506 | 495 | UNSETTLED:300 | OK |
| firefight_3 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| firefight_3 | remastered | 4.39 | 2.78 |  | 0.50 | 1.10 | 1.18 | 0.08 |  |  | 1.27 | 0.66 | 0.01 | 0.09 | 0.05 |  | 0.05 | 3.96 | 164.40 | 578 | 574 | UNSETTLED:300 | OK |
| tank_gunner_interior | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| tank_gunner_interior | remastered | 9.81 | 8.93 | 0.06 | 0.69 | 3.63 | 4.61 | 0.06 |  |  | 0.27 | 0.17 |  |  | 0.05 |  | 0.02 | 5.43 | 79.30 | 1,538 | 1,208 | OK:30 | OK |
| tank_gunner_optics | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| tank_gunner_optics | remastered | 8.09 | 7.23 | 0.09 | 0.77 | 3.07 | 3.39 | 0.07 |  |  | 0.33 | 0.17 |  | 0.08 |  |  | 0.02 | 4.15 | 107.30 | 1,150 | 739 | OK:30 | OK |
| optics | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| optics | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
