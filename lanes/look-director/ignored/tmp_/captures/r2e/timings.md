# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 4.35 | 4.19 |  | 0.13 | 1.72 | 2.34 |  |  |  |  |  |  |  |  |  |  | 3.31 | 146.40 | 2,091 | 1,316 | OK:30 | OK |
| lowsun_fp | remastered | 8.61 | 6.80 | 0.14 | 1.28 | 3.36 | 2.16 | 0.12 |  |  | 1.53 | 0.29 | 0.01 | 0.18 | 0.09 |  | 0.11 | 8.18 | 93.80 | 1,087 | 1,179 | OK:30 | OK |
| lowsun_fp | delta | +4.26 | +2.61 |  | +1.15 | +1.64 | -0.18 |  |  |  |  |  |  |  |  |  |  | +4.87 |  | -48.0% | -10.4% |  |  |
| sunrise_ridge | classic | 26.30 | 26.15 |  | 7.82 | 15.35 | 2.99 |  |  |  |  |  |  |  |  |  |  | 25.60 | 34.50 | 13,076 | 8,897 | OK:30 | OK |
| sunrise_ridge | remastered | 9.25 | 7.63 | 0.10 | 5.30 | 1.99 | 0.34 | 0.16 |  |  | 1.24 | 0.27 | 0.01 | 0.19 | 0.06 |  | 0.12 | 4.93 | 70.30 | 1,659 | 7,680 | OK:30 | OK |
| sunrise_ridge | delta | -17.05 | -18.52 |  | -2.52 | -13.36 | -2.65 |  |  |  |  |  |  |  |  |  |  | -20.67 |  | -87.3% | -13.7% |  |  |
| noon_fields | classic | 8.96 | 8.83 |  | 0.11 | 6.51 | 2.21 |  |  |  |  |  |  |  |  |  |  | 8.75 | 67.90 | 6,767 | 4,508 | OK:30 | OK |
| noon_fields | remastered | 9.59 | 8.22 | 0.10 | 1.58 | 6.18 | 0.46 | 0.11 |  |  | 1.11 | 0.26 | 0.01 | 0.16 | 0.05 |  | 0.11 | 9.38 | 70.20 | 1,447 | 3,920 | OK:30 | OK |
| noon_fields | delta | +0.63 | -0.61 |  | +1.47 | -0.33 | -1.75 |  |  |  |  |  |  |  |  |  |  | +0.63 |  | -78.6% | -13.0% |  |  |
| forest_edge_golden | classic | 5.40 | 5.22 |  | 0.10 | 2.94 | 2.17 |  |  |  |  |  |  |  |  |  |  | 4.51 | 122.60 | 2,502 | 1,459 | OK:30 | OK |
| forest_edge_golden | remastered | 6.46 | 4.83 | 0.19 | 1.34 | 2.67 | 0.82 | 0.14 |  |  | 1.26 | 0.28 | 0.01 | 0.22 | 0.05 |  | 0.12 | 6.11 | 105.10 | 757 | 1,316 | OK:30 | OK |
| forest_edge_golden | delta | +1.06 | -0.39 |  | +1.24 | -0.27 | -1.35 |  |  |  |  |  |  |  |  |  |  | +1.60 |  | -69.7% | -9.8% |  |  |
| coastline_sunset | classic | 1.24 | 1.02 |  | 0.54 | 0.00 | 0.47 |  |  |  |  |  |  |  |  |  |  | 0.84 | 374.50 | 177 | 0 | OK:30 | OK |
| coastline_sunset | remastered | 3.29 | 1.63 | 0.00 | 1.43 | 0.00 | 0.20 | 0.13 |  |  | 1.39 | 0.11 | 0.01 | 0.17 | 0.06 |  | 0.11 | 3.11 | 244.80 | 154 | 0 | OK:30 | OK |
| coastline_sunset | delta | +2.05 | +0.61 |  | +0.89 | +0.00 | -0.27 |  |  |  |  |  |  |  |  |  |  | +2.27 |  | -13.0% |  |  |  |
| town_night | classic | 6.20 | 5.96 |  | 0.11 | 1.34 | 4.52 |  |  |  |  |  |  |  |  |  |  | 5.22 | 126.30 | 1,200 | 1,032 | OK:144 | OK |
| town_night | remastered | 10.89 | 9.27 | 0.16 | 1.12 | 3.54 | 4.61 | 0.05 |  |  | 1.28 | 0.27 | 0.01 | 0.15 | 0.05 |  | 0.10 | 10.45 | 73.70 | 1,170 | 1,073 | OK:31 | OK |
| town_night | delta | +4.69 | +3.31 |  | +1.01 | +2.20 | +0.09 |  |  |  |  |  |  |  |  |  |  | +5.23 |  | -2.5% | +3.8% |  |  |
| firefight_1 | classic | 4.55 | 3.98 |  | 0.31 | 0.61 | 3.06 |  |  |  |  |  |  |  |  |  |  | 0.83 | 197.30 | 438 | 385 | OK:47 | OK |
| firefight_1 | remastered | 6.55 | 4.58 |  | 0.92 | 1.67 | 1.99 | 0.19 |  |  | 1.38 | 0.29 | 0.01 | 0.20 | 0.07 |  | 0.24 | 5.45 | 128.90 | 419 | 387 | UNSETTLED:300 | OK |
| firefight_1 | delta | +2.00 | +0.60 |  | +0.61 | +1.06 | -1.07 |  |  |  |  |  |  |  |  |  |  | +4.62 |  | -4.3% | +0.5% |  |  |
| firefight_2 | classic | 4.55 | 3.98 |  | 0.31 | 0.61 | 3.06 |  |  |  |  |  |  |  |  |  |  | 0.83 | 197.30 | 507 | 487 | OK:47 | OK |
| firefight_2 | remastered | 6.55 | 4.58 |  | 0.92 | 1.67 | 1.99 | 0.19 |  |  | 1.38 | 0.29 | 0.01 | 0.20 | 0.07 |  | 0.24 | 5.45 | 128.90 | 566 | 599 | UNSETTLED:300 | OK |
| firefight_2 | delta | +2.00 | +0.60 |  | +0.61 | +1.06 | -1.07 |  |  |  |  |  |  |  |  |  |  | +4.62 |  | +10.4% | +18.7% |  |  |
| firefight_3 | classic | 4.55 | 3.98 |  | 0.31 | 0.61 | 3.06 |  |  |  |  |  |  |  |  |  |  | 0.83 | 197.30 | 485 | 420 | OK:47 | OK |
| firefight_3 | remastered | 6.55 | 4.58 |  | 0.92 | 1.67 | 1.99 | 0.19 |  |  | 1.38 | 0.29 | 0.01 | 0.20 | 0.07 |  | 0.24 | 5.45 | 128.90 | 521 | 548 | UNSETTLED:300 | OK |
| firefight_3 | delta | +2.00 | +0.60 |  | +0.61 | +1.06 | -1.07 |  |  |  |  |  |  |  |  |  |  | +4.62 |  | +6.9% | +23.4% |  |  |
| tank_gunner_interior | classic | 5.38 | 5.07 |  | 0.09 | 2.16 | 2.82 |  |  |  |  |  |  |  |  |  |  | 4.34 | 126.60 | 2,529 | 1,216 | OK:30 | OK |
| tank_gunner_interior | remastered | 8.25 | 7.31 | 0.06 | 0.94 | 2.89 | 3.49 | 0.09 |  |  | 0.61 | 0.13 |  | 0.28 | 0.05 |  | 0.12 | 8.09 | 79.40 | 1,531 | 1,228 | OK:30 | OK |
| tank_gunner_interior | delta | +2.87 | +2.24 |  | +0.85 | +0.73 | +0.67 |  |  |  |  |  |  |  |  |  |  | +3.75 |  | -39.5% | +1.0% |  |  |
| tank_gunner_optics | classic | 3.97 | 3.70 |  | 0.12 | 1.86 | 1.72 |  |  |  |  |  |  |  |  |  |  | 3.29 | 159.40 | 1,727 | 741 | OK:30 | OK |
| tank_gunner_optics | remastered | 7.54 | 6.77 | 0.07 | 0.96 | 3.53 | 2.28 | 0.11 |  |  | 0.50 | 0.10 |  | 0.16 |  |  | 0.21 | 7.09 | 108.60 | 1,145 | 741 | OK:30 | OK |
| tank_gunner_optics | delta | +3.57 | +3.07 |  | +0.84 | +1.67 | +0.56 |  |  |  |  |  |  |  |  |  |  | +3.80 |  | -33.7% | +0.0% |  |  |
| overcast_broken | classic | 11.71 | 11.58 |  | 0.15 | 8.73 | 2.70 |  |  |  |  |  |  |  |  |  |  | 11.19 | 51.30 | 8,243 | 5,692 | OK:30 | OK |
| overcast_broken | remastered | 8.99 | 7.47 | 0.14 | 2.43 | 4.59 | 0.45 | 0.11 |  |  | 1.24 | 0.26 | 0.01 | 0.15 | 0.06 |  | 0.20 | 8.54 | 65.30 | 1,863 | 5,083 | OK:30 | OK |
| overcast_broken | delta | -2.72 | -4.11 |  | +2.28 | -4.14 | -2.25 |  |  |  |  |  |  |  |  |  |  | -2.65 |  | -77.4% | -10.7% |  |  |
| overcast_heavy | classic | 11.56 | 11.41 |  | 0.12 | 8.63 | 2.66 |  |  |  |  |  |  |  |  |  |  | 11.08 | 52.70 | 7,957 | 5,692 | OK:30 | OK |
| overcast_heavy | remastered | 8.73 | 6.87 | 0.19 | 1.90 | 4.46 | 0.50 | 0.11 |  |  | 1.58 | 0.27 | 0.01 | 0.15 | 0.09 |  | 0.11 | 8.59 | 68.00 | 1,619 | 5,083 | OK:30 | OK |
| overcast_heavy | delta | -2.83 | -4.54 |  | +1.78 | -4.17 | -2.16 |  |  |  |  |  |  |  |  |  |  | -2.49 |  | -79.7% | -10.7% |  |  |
| woods_pine2_evening | classic | 5.99 | 5.84 |  | 0.13 | 3.31 | 2.40 |  |  |  |  |  |  |  |  |  |  | 5.01 | 110.80 | 3,250 | 1,811 | OK:30 | OK |
| woods_pine2_evening | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| blue_hour_coast | classic | 1.30 | 1.02 |  | 0.60 | 0.00 | 0.42 |  |  |  |  |  |  |  |  |  |  | 0.79 | 333.00 | 182 | 0 | OK:30 | OK |
| blue_hour_coast | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| dawn_valley | classic | 21.74 | 21.61 |  | 3.81 | 15.18 | 2.61 |  |  |  |  |  |  |  |  |  |  | 17.64 | 37.30 | 11,553 | 7,966 | OK:30 | OK |
| dawn_valley | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| pine_rays | classic | 5.81 | 5.61 |  | 0.08 | 2.39 | 3.15 |  |  |  |  |  |  |  |  |  |  | 4.38 | 117.40 | 3,163 | 1,678 | OK:30 | OK |
| pine_rays | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_fog_night | classic | 5.56 | 5.38 |  | 0.09 | 1.55 | 3.74 |  |  |  |  |  |  |  |  |  |  | 4.46 | 145.60 | 1,223 | 956 | OK:35 | OK |
| town_fog_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night_rain | classic | 4.56 | 4.37 |  | 0.08 | 0.93 | 3.36 |  |  |  |  |  |  |  |  |  |  | 3.47 | 162.60 | 932 | 689 | OK:30 | OK |
| town_night_rain | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| jeep_close_noon | classic | 4.85 | 4.74 |  | 0.12 | 3.35 | 1.27 |  |  |  |  |  |  |  |  |  |  | 4.02 | 129.90 | 2,723 | 1,774 | OK:30 | OK |
| jeep_close_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_noon | classic | 2.36 | 2.12 |  | 0.13 | 0.77 | 1.22 |  |  |  |  |  |  |  |  |  |  | 1.69 | 219.80 | 985 | 541 | OK:30 | OK |
| town_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_morning | classic | 5.09 | 4.92 |  | 0.11 | 2.96 | 1.85 |  |  |  |  |  |  |  |  |  |  | 4.19 | 163.80 | 2,435 | 1,448 | OK:30 | OK |
| forest_morning | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| rain_day | classic | 3.97 | 3.78 |  | 0.11 | 1.90 | 1.77 |  |  |  |  |  |  |  |  |  |  | 2.73 | 158.30 | 1,761 | 793 | OK:30 | OK |
| rain_day | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_vista | classic | 20.18 | 20.05 |  | 6.66 | 11.29 | 2.10 |  |  |  |  |  |  |  |  |  |  | 19.32 | 43.10 | 10,841 | 7,519 | OK:30 | OK |
| noon_vista | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 2.71 | 2.53 |  | 0.13 | 1.14 | 1.26 |  |  |  |  |  |  |  |  |  |  | 1.71 | 190.10 | 603 | 314 | OK:30 | OK |
| sinai_lowsun_fp | remastered | 6.48 | 4.82 | 0.22 | 1.14 | 2.66 | 1.02 | 0.12 |  |  | 1.33 | 0.26 | 0.01 | 0.22 | 0.06 |  | 0.12 | 5.94 | 108.90 | 417 | 314 | UNSETTLED:300 | OK |
| sinai_lowsun_fp | delta | +3.77 | +2.29 |  | +1.01 | +1.52 | -0.24 |  |  |  |  |  |  |  |  |  |  | +4.23 |  | -30.8% | +0.0% |  |  |
| sinai_desert_noon | classic | 5.82 | 5.69 |  | 0.14 | 3.00 | 2.55 |  |  |  |  |  |  |  |  |  |  | 4.57 | 94.30 | 2,243 | 1,100 | OK:30 | OK |
| sinai_desert_noon | remastered | 7.90 | 6.24 | 0.10 | 1.19 | 3.09 | 1.97 | 0.12 |  |  | 1.37 | 0.33 | 0.01 | 0.17 | 0.06 |  | 0.28 | 7.48 | 79.80 | 1,181 | 1,100 | OK:30 | OK |
| sinai_desert_noon | delta | +2.08 | +0.55 |  | +1.05 | +0.09 | -0.58 |  |  |  |  |  |  |  |  |  |  | +2.91 |  | -47.3% | +0.0% |  |  |
