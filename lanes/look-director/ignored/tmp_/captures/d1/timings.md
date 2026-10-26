# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| noon_fields | classic | 18.02 | 17.89 |  | 0.11 | 12.87 | 4.90 |  |  |  |  |  |  |  |  |  |  | 10.40 | 40.90 | 6,767 | 4,508 | OK:30 | OK |
| noon_fields | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| lowsun_fp | classic | 5.88 | 5.75 |  | 0.10 | 1.79 | 3.85 |  |  |  |  |  |  |  |  |  |  | 3.37 | 115.40 | 2,091 | 1,316 | OK:30 | OK |
| lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 5.33 | 5.17 |  | 0.09 | 2.91 | 2.17 |  |  |  |  |  |  |  |  |  |  | 4.20 | 122.60 | 2,502 | 1,459 | OK:30 | OK |
| forest_edge_golden | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sunrise_ridge | classic | 27.28 | 27.14 |  | 8.01 | 16.06 | 3.06 |  |  |  |  |  |  |  |  |  |  | 25.81 | 26.10 | 13,076 | 8,897 | OK:30 | OK |
| sunrise_ridge | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| coastline_sunset | classic | 1.25 | 1.04 |  | 0.58 | 0.00 | 0.45 |  |  |  |  |  |  |  |  |  |  | 0.76 | 354.30 | 177 | 0 | OK:30 | OK |
| coastline_sunset | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| woods_pine2_evening | classic | 5.83 | 5.68 |  | 0.13 | 2.98 | 2.57 |  |  |  |  |  |  |  |  |  |  | 4.96 | 109.10 | 3,250 | 1,811 | OK:30 | OK |
| woods_pine2_evening | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| blue_hour_coast | classic | 1.54 | 1.25 |  | 0.78 | 0.00 | 0.47 |  |  |  |  |  |  |  |  |  |  | 0.87 | 336.50 | 182 | 0 | OK:30 | OK |
| blue_hour_coast | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| dawn_valley | classic | 21.02 | 20.88 |  | 3.57 | 14.84 | 2.47 |  |  |  |  |  |  |  |  |  |  | 17.26 | 37.60 | 11,553 | 7,966 | OK:30 | OK |
| dawn_valley | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| pine_rays | classic | 5.45 | 5.26 |  | 0.08 | 2.30 | 2.89 |  |  |  |  |  |  |  |  |  |  | 4.37 | 117.30 | 3,163 | 1,678 | OK:30 | OK |
| pine_rays | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night | classic | 6.90 | 6.67 |  | 0.11 | 1.46 | 5.11 |  |  |  |  |  |  |  |  |  |  | 6.15 | 113.00 | 1,304 | 1,125 | OK:222 | OK |
| town_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_fog_night | classic | 6.58 | 6.36 |  | 0.09 | 1.62 | 4.65 |  |  |  |  |  |  |  |  |  |  | 5.16 | 119.90 | 1,279 | 1,023 | OK:99 | OK |
| town_fog_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night_rain | classic | 5.14 | 4.88 |  | 0.12 | 0.90 | 3.86 |  |  |  |  |  |  |  |  |  |  | 4.36 | 142.50 | 928 | 704 | OK:226 | OK |
| town_night_rain | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| jeep_close_noon | classic | 4.89 | 4.78 |  | 0.12 | 3.41 | 1.24 |  |  |  |  |  |  |  |  |  |  | 4.01 | 128.60 | 2,723 | 1,774 | OK:30 | OK |
| jeep_close_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_noon | classic | 2.48 | 2.24 |  | 0.16 | 0.77 | 1.31 |  |  |  |  |  |  |  |  |  |  | 1.68 | 220.20 | 985 | 541 | OK:30 | OK |
| town_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_morning | classic | 4.96 | 4.79 |  | 0.11 | 2.97 | 1.71 |  |  |  |  |  |  |  |  |  |  | 4.17 | 158.70 | 2,435 | 1,448 | OK:30 | OK |
| forest_morning | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| overcast_broken | classic | 11.15 | 11.02 |  | 0.15 | 8.33 | 2.53 |  |  |  |  |  |  |  |  |  |  | 10.86 | 53.20 | 8,243 | 5,692 | OK:30 | OK |
| overcast_broken | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| overcast_heavy | classic | 11.48 | 11.33 |  | 0.13 | 8.54 | 2.66 |  |  |  |  |  |  |  |  |  |  | 11.11 | 53.00 | 7,957 | 5,692 | OK:30 | OK |
| overcast_heavy | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| rain_day | classic | 3.98 | 3.77 |  | 0.12 | 1.86 | 1.79 |  |  |  |  |  |  |  |  |  |  | 2.61 | 164.20 | 1,763 | 795 | OK:30 | OK |
| rain_day | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_vista | classic | 20.14 | 20.01 |  | 6.76 | 11.24 | 2.01 |  |  |  |  |  |  |  |  |  |  | 19.30 | 43.00 | 10,841 | 7,519 | OK:30 | OK |
| noon_vista | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 3.49 | 3.27 |  | 0.17 | 1.62 | 1.48 |  |  |  |  |  |  |  |  |  |  | 2.97 | 137.30 | 789 | 371 | OK:30 | OK |
| sinai_lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 7.41 | 7.25 |  | 0.18 | 3.37 | 3.70 |  |  |  |  |  |  |  |  |  |  | 6.28 | 76.50 | 2,689 | 1,363 | OK:30 | OK |
| sinai_desert_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
