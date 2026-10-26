# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 4.28 | 4.10 |  | 0.15 | 1.68 | 2.27 |  |  |  |  |  |  |  |  |  |  | 3.35 | 148.80 | 2,091 | 1,316 | OK:30 | OK |
| lowsun_fp | remastered | 15.73 | 13.19 | 0.18 | 2.16 | 6.80 | 4.24 | 0.12 |  |  | 2.26 | 0.20 | 0.01 | 0.18 | 0.12 |  | 0.11 | 10.48 | 59.10 | 1,087 | 1,179 | OK:30 | OK |
| lowsun_fp | delta | +11.45 | +9.09 |  | +2.01 | +5.12 | +1.97 |  |  |  |  |  |  |  |  |  |  | +7.13 |  | -48.0% | -10.4% |  |  |
| sunrise_ridge | classic | 25.07 | 24.93 |  | 7.63 | 14.57 | 2.73 |  |  |  |  |  |  |  |  |  |  | 24.48 | 35.90 | 13,076 | 8,897 | OK:30 | OK |
| sunrise_ridge | remastered | 12.30 | 10.01 | 0.11 | 5.74 | 3.59 | 0.68 | 0.14 |  |  | 1.95 | 0.18 | 0.01 | 0.17 | 0.05 |  | 0.10 | 4.89 | 60.70 | 1,659 | 7,680 | OK:30 | OK |
| sunrise_ridge | delta | -12.77 | -14.92 |  | -1.89 | -10.98 | -2.05 |  |  |  |  |  |  |  |  |  |  | -19.59 |  | -87.3% | -13.7% |  |  |
| noon_fields | classic | 8.49 | 8.37 |  | 0.11 | 6.14 | 2.12 |  |  |  |  |  |  |  |  |  |  | 8.37 | 71.20 | 6,767 | 4,508 | OK:30 | OK |
| noon_fields | remastered | 16.83 | 14.85 | 0.13 | 2.61 | 11.13 | 1.11 | 0.11 |  |  | 1.73 | 0.17 | 0.01 | 0.16 | 0.05 |  | 0.11 | 9.07 | 58.60 | 1,447 | 3,920 | OK:30 | OK |
| noon_fields | delta | +8.34 | +6.48 |  | +2.50 | +4.99 | -1.01 |  |  |  |  |  |  |  |  |  |  | +0.70 |  | -78.6% | -13.0% |  |  |
| forest_edge_golden | classic | 5.48 | 5.29 |  | 0.11 | 2.93 | 2.25 |  |  |  |  |  |  |  |  |  |  | 4.46 | 125.50 | 2,502 | 1,459 | OK:30 | OK |
| forest_edge_golden | remastered | 6.57 | 4.84 | 0.19 | 1.30 | 2.60 | 0.94 | 0.14 |  |  | 1.36 | 0.28 | 0.01 | 0.23 | 0.06 |  | 0.13 | 6.09 | 102.90 | 757 | 1,316 | OK:30 | OK |
| forest_edge_golden | delta | +1.09 | -0.45 |  | +1.19 | -0.33 | -1.31 |  |  |  |  |  |  |  |  |  |  | +1.63 |  | -69.7% | -9.8% |  |  |
| coastline_sunset | classic | 1.34 | 1.10 |  | 0.66 | 0.00 | 0.43 |  |  |  |  |  |  |  |  |  |  | 0.88 | 405.30 | 177 | 0 | OK:30 | OK |
| coastline_sunset | remastered | 4.71 | 2.53 | 0.00 | 2.04 | 0.00 | 0.49 | 0.11 |  |  | 1.94 | 0.11 | 0.01 | 0.16 | 0.08 |  | 0.45 | 2.89 | 212.80 | 154 | 0 | OK:30 | OK |
| coastline_sunset | delta | +3.37 | +1.43 |  | +1.38 | +0.00 | +0.06 |  |  |  |  |  |  |  |  |  |  | +2.01 |  | -13.0% |  |  |  |
| town_night | classic | 6.03 | 5.69 |  | 0.13 | 1.32 | 4.24 |  |  |  |  |  |  |  |  |  |  | 5.49 | 122.70 | 1,245 | 1,073 | OK:184 | OK |
| town_night | remastered | 19.54 | 17.23 | 0.17 | 2.22 | 6.26 | 8.75 | 0.05 |  |  | 1.98 | 0.16 | 0.01 | 0.24 | 0.44 |  | 0.10 | 10.31 | 47.40 | 1,190 | 1,066 | OK:102 | OK |
| town_night | delta | +13.51 | +11.54 |  | +2.09 | +4.94 | +4.51 |  |  |  |  |  |  |  |  |  |  | +4.82 |  | -4.4% | -0.7% |  |  |
| firefight_1 | classic | 2.47 | 2.34 |  | 0.06 | 0.21 | 2.08 |  |  |  |  |  |  |  |  |  |  | 0.84 | 242.30 | 463 | 397 | OK:47 | OK |
| firefight_1 | remastered | 6.37 | 4.27 |  | 1.03 | 1.72 | 1.52 | 0.18 |  |  | 1.56 | 0.32 | 0.01 | 0.21 | 0.07 |  | 0.15 | 3.85 | 154.20 | 480 | 485 | UNSETTLED:300 | OK |
| firefight_1 | delta | +3.90 | +1.93 |  | +0.97 | +1.51 | -0.56 |  |  |  |  |  |  |  |  |  |  | +3.01 |  | +3.5% | +18.1% |  |  |
| firefight_2 | classic | 2.47 | 2.34 |  | 0.06 | 0.21 | 2.08 |  |  |  |  |  |  |  |  |  |  | 0.84 | 242.30 | 517 | 515 | OK:47 | OK |
| firefight_2 | remastered | 6.37 | 4.27 |  | 1.03 | 1.72 | 1.52 | 0.18 |  |  | 1.56 | 0.32 | 0.01 | 0.21 | 0.07 |  | 0.15 | 3.85 | 154.20 | 557 | 592 | UNSETTLED:300 | OK |
| firefight_2 | delta | +3.90 | +1.93 |  | +0.97 | +1.51 | -0.56 |  |  |  |  |  |  |  |  |  |  | +3.01 |  | +7.2% | +13.0% |  |  |
| firefight_3 | classic | 2.47 | 2.34 |  | 0.06 | 0.21 | 2.08 |  |  |  |  |  |  |  |  |  |  | 0.84 | 242.30 | 515 | 458 | OK:47 | OK |
| firefight_3 | remastered | 6.37 | 4.27 |  | 1.03 | 1.72 | 1.52 | 0.18 |  |  | 1.56 | 0.32 | 0.01 | 0.21 | 0.07 |  | 0.15 | 3.85 | 154.20 | 460 | 479 | UNSETTLED:300 | OK |
| firefight_3 | delta | +3.90 | +1.93 |  | +0.97 | +1.51 | -0.56 |  |  |  |  |  |  |  |  |  |  | +3.01 |  | -10.7% | +4.4% |  |  |
| tank_gunner_interior | classic | 5.39 | 5.10 |  | 0.10 | 2.16 | 2.84 |  |  |  |  |  |  |  |  |  |  | 4.18 | 131.00 | 2,531 | 1,218 | OK:30 | OK |
| tank_gunner_interior | remastered | 7.79 | 6.89 | 0.06 | 0.87 | 2.77 | 3.26 | 0.09 |  |  | 0.59 | 0.13 |  | 0.28 | 0.05 |  | 0.09 | 7.70 | 83.40 | 1,533 | 1,231 | OK:30 | OK |
| tank_gunner_interior | delta | +2.40 | +1.79 |  | +0.77 | +0.61 | +0.42 |  |  |  |  |  |  |  |  |  |  | +3.52 |  | -39.4% | +1.1% |  |  |
| tank_gunner_optics | classic | 4.93 | 4.54 |  | 0.31 | 2.06 | 2.17 |  |  |  |  |  |  |  |  |  |  | 2.82 | 153.30 | 1,729 | 743 | OK:30 | OK |
| tank_gunner_optics | remastered | 7.77 | 7.01 | 0.07 | 1.07 | 3.55 | 2.39 | 0.12 |  |  | 0.47 | 0.09 |  | 0.18 |  |  | 0.16 | 7.52 | 107.10 | 1,151 | 743 | OK:30 | OK |
| tank_gunner_optics | delta | +2.84 | +2.47 |  | +0.76 | +1.49 | +0.22 |  |  |  |  |  |  |  |  |  |  | +4.70 |  | -33.4% | +0.0% |  |  |
| overcast_broken | classic | 11.08 | 10.95 |  | 0.15 | 8.25 | 2.55 |  |  |  |  |  |  |  |  |  |  | 10.82 | 53.70 | 8,243 | 5,692 | OK:30 | OK |
| overcast_broken | remastered | 8.46 | 7.01 | 0.14 | 2.20 | 4.39 | 0.43 | 0.10 |  |  | 1.17 | 0.26 | 0.01 | 0.15 | 0.05 |  | 0.17 | 8.30 | 69.00 | 1,863 | 5,083 | OK:30 | OK |
| overcast_broken | delta | -2.62 | -3.94 |  | +2.05 | -3.86 | -2.12 |  |  |  |  |  |  |  |  |  |  | -2.52 |  | -77.4% | -10.7% |  |  |
| overcast_heavy | classic | 10.99 | 10.85 |  | 0.12 | 8.20 | 2.52 |  |  |  |  |  |  |  |  |  |  | 10.76 | 54.90 | 7,957 | 5,692 | OK:30 | OK |
| overcast_heavy | remastered | 8.31 | 6.56 | 0.13 | 1.84 | 4.30 | 0.42 | 0.10 |  |  | 1.47 | 0.25 | 0.01 | 0.15 | 0.05 |  | 0.11 | 8.21 | 72.10 | 1,619 | 5,083 | OK:30 | OK |
| overcast_heavy | delta | -2.68 | -4.29 |  | +1.72 | -3.90 | -2.10 |  |  |  |  |  |  |  |  |  |  | -2.55 |  | -79.7% | -10.7% |  |  |
| woods_pine2_evening | classic | 5.80 | 5.64 |  | 0.13 | 3.13 | 2.38 |  |  |  |  |  |  |  |  |  |  | 4.97 | 114.70 | 3,250 | 1,811 | OK:30 | OK |
| woods_pine2_evening | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| blue_hour_coast | classic | 1.34 | 1.05 |  | 0.62 | 0.00 | 0.43 |  |  |  |  |  |  |  |  |  |  | 0.86 | 344.70 | 182 | 0 | OK:30 | OK |
| blue_hour_coast | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| dawn_valley | classic | 20.02 | 19.89 |  | 3.13 | 14.37 | 2.40 |  |  |  |  |  |  |  |  |  |  | 16.71 | 39.10 | 11,553 | 7,966 | OK:30 | OK |
| dawn_valley | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| pine_rays | classic | 5.60 | 5.39 |  | 0.08 | 2.34 | 2.97 |  |  |  |  |  |  |  |  |  |  | 4.31 | 126.70 | 3,163 | 1,678 | OK:30 | OK |
| pine_rays | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_fog_night | classic | 6.17 | 5.97 |  | 0.10 | 1.54 | 4.33 |  |  |  |  |  |  |  |  |  |  | 4.76 | 138.60 | 1,261 | 1,003 | OK:57 | OK |
| town_fog_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night_rain | classic | 6.98 | 6.14 |  | 0.36 | 0.95 | 4.83 |  |  |  |  |  |  |  |  |  |  | 4.44 | 127.70 | 881 | 620 | OK:261 | OK |
| town_night_rain | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| jeep_close_noon | classic | 4.97 | 4.86 |  | 0.13 | 3.48 | 1.25 |  |  |  |  |  |  |  |  |  |  | 4.05 | 137.60 | 2,723 | 1,774 | OK:30 | OK |
| jeep_close_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_noon | classic | 2.39 | 2.15 |  | 0.14 | 0.87 | 1.14 |  |  |  |  |  |  |  |  |  |  | 1.59 | 224.60 | 985 | 541 | OK:30 | OK |
| town_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_morning | classic | 5.23 | 5.05 |  | 0.12 | 3.04 | 1.90 |  |  |  |  |  |  |  |  |  |  | 4.15 | 172.80 | 2,435 | 1,448 | OK:30 | OK |
| forest_morning | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| rain_day | classic | 4.25 | 4.04 |  | 0.13 | 2.16 | 1.75 |  |  |  |  |  |  |  |  |  |  | 2.71 | 164.10 | 1,753 | 785 | OK:30 | OK |
| rain_day | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_vista | classic | 19.98 | 19.86 |  | 6.71 | 11.18 | 1.97 |  |  |  |  |  |  |  |  |  |  | 18.91 | 43.70 | 10,841 | 7,519 | OK:30 | OK |
| noon_vista | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 3.02 | 2.79 |  | 0.33 | 1.04 | 1.42 |  |  |  |  |  |  |  |  |  |  | 1.83 | 201.90 | 603 | 314 | OK:30 | OK |
| sinai_lowsun_fp | remastered | 6.83 | 5.15 | 0.24 | 1.27 | 2.88 | 1.00 | 0.14 |  |  | 1.31 | 0.28 | 0.01 | 0.25 | 0.06 |  | 0.13 | 6.34 | 111.90 | 417 | 314 | UNSETTLED:300 | OK |
| sinai_lowsun_fp | delta | +3.81 | +2.36 |  | +0.94 | +1.84 | -0.42 |  |  |  |  |  |  |  |  |  |  | +4.51 |  | -30.8% | +0.0% |  |  |
| sinai_desert_noon | classic | 5.91 | 5.77 |  | 0.16 | 3.06 | 2.55 |  |  |  |  |  |  |  |  |  |  | 4.51 | 97.40 | 2,242 | 1,100 | OK:30 | OK |
| sinai_desert_noon | remastered | 7.50 | 5.90 | 0.10 | 1.16 | 2.91 | 1.83 | 0.11 |  |  | 1.31 | 0.29 | 0.01 | 0.17 | 0.05 |  | 0.28 | 7.19 | 84.50 | 1,180 | 1,100 | OK:30 | OK |
| sinai_desert_noon | delta | +1.59 | +0.13 |  | +1.00 | -0.15 | -0.72 |  |  |  |  |  |  |  |  |  |  | +2.68 |  | -47.4% | +0.0% |  |  |
