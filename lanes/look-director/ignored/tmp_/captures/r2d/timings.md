# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 4.00 | 3.84 |  | 0.13 | 1.55 | 2.17 |  |  |  |  |  |  |  |  |  |  | 3.40 | 146.00 | 2,091 | 1,316 | OK:30 | OK |
| lowsun_fp | remastered | 8.29 | 6.55 | 0.14 | 1.24 | 3.31 | 2.00 | 0.12 |  |  | 1.46 | 0.27 | 0.01 | 0.18 | 0.06 |  | 0.12 | 8.11 | 95.60 | 1,090 | 1,179 | OK:30 | OK |
| lowsun_fp | delta | +4.29 | +2.71 |  | +1.11 | +1.76 | -0.17 |  |  |  |  |  |  |  |  |  |  | +4.71 |  | -47.9% | -10.4% |  |  |
| sunrise_ridge | classic | 25.24 | 25.10 |  | 7.60 | 14.72 | 2.78 |  |  |  |  |  |  |  |  |  |  | 24.57 | 35.60 | 13,076 | 8,897 | OK:30 | OK |
| sunrise_ridge | remastered | 9.09 | 7.41 | 0.10 | 5.04 | 2.06 | 0.31 | 0.17 |  |  | 1.26 | 0.25 | 0.01 | 0.20 | 0.06 |  | 0.13 | 4.89 | 70.40 | 1,659 | 7,680 | OK:30 | OK |
| sunrise_ridge | delta | -16.15 | -17.69 |  | -2.56 | -12.66 | -2.47 |  |  |  |  |  |  |  |  |  |  | -19.68 |  | -87.3% | -13.7% |  |  |
| noon_fields | classic | 8.69 | 8.57 |  | 0.11 | 6.28 | 2.18 |  |  |  |  |  |  |  |  |  |  | 8.44 | 69.70 | 6,767 | 4,508 | OK:30 | OK |
| noon_fields | remastered | 9.55 | 8.13 | 0.11 | 1.63 | 6.02 | 0.49 | 0.11 |  |  | 1.16 | 0.29 | 0.01 | 0.16 | 0.05 |  | 0.11 | 9.11 | 69.80 | 1,447 | 3,920 | OK:30 | OK |
| noon_fields | delta | +0.86 | -0.44 |  | +1.52 | -0.26 | -1.69 |  |  |  |  |  |  |  |  |  |  | +0.67 |  | -78.6% | -13.0% |  |  |
| forest_edge_golden | classic | 5.42 | 5.23 |  | 0.11 | 2.88 | 2.25 |  |  |  |  |  |  |  |  |  |  | 4.49 | 124.40 | 2,502 | 1,459 | OK:30 | OK |
| forest_edge_golden | remastered | 6.58 | 4.98 | 0.19 | 1.45 | 2.70 | 0.83 | 0.14 |  |  | 1.23 | 0.28 | 0.01 | 0.22 | 0.05 |  | 0.12 | 6.16 | 103.50 | 757 | 1,316 | OK:30 | OK |
| forest_edge_golden | delta | +1.16 | -0.25 |  | +1.34 | -0.18 | -1.42 |  |  |  |  |  |  |  |  |  |  | +1.67 |  | -69.7% | -9.8% |  |  |
| coastline_sunset | classic | 1.27 | 1.04 |  | 0.59 | 0.00 | 0.46 |  |  |  |  |  |  |  |  |  |  | 0.88 | 358.40 | 177 | 0 | OK:30 | OK |
| coastline_sunset | remastered | 6.01 | 3.01 | 0.00 | 2.76 | 0.00 | 0.25 | 0.24 |  |  | 2.46 | 0.19 | 0.01 | 0.28 | 0.14 |  | 0.21 | 5.52 | 166.30 | 154 | 0 | OK:30 | OK |
| coastline_sunset | delta | +4.74 | +1.97 |  | +2.17 | +0.00 | -0.21 |  |  |  |  |  |  |  |  |  |  | +4.64 |  | -13.0% |  |  |  |
| town_night | classic | 6.74 | 6.50 |  | 0.11 | 1.48 | 4.92 |  |  |  |  |  |  |  |  |  |  | 6.11 | 114.50 | 1,280 | 1,122 | OK:217 | OK |
| town_night | remastered | 10.87 | 9.31 | 0.16 | 1.12 | 3.44 | 4.75 | 0.06 |  |  | 1.21 | 0.28 | 0.01 | 0.15 | 0.05 |  | 0.10 | 10.51 | 71.90 | 1,170 | 1,068 | OK:111 | OK |
| town_night | delta | +4.13 | +2.81 |  | +1.01 | +1.96 | -0.17 |  |  |  |  |  |  |  |  |  |  | +4.40 |  | -8.6% | -4.8% |  |  |
| firefight_1 | classic | 4.65 | 3.99 |  | 0.29 | 0.73 | 2.97 |  |  |  |  |  |  |  |  |  |  | 0.89 | 190.30 | 475 | 400 | OK:46 | OK |
| firefight_1 | remastered | 6.92 | 4.93 |  | 1.09 | 1.81 | 2.04 | 0.22 |  |  | 1.33 | 0.26 | 0.01 | 0.23 | 0.08 |  | 0.18 | 5.87 | 124.90 | 520 | 547 | UNSETTLED:300 | OK |
| firefight_1 | delta | +2.27 | +0.94 |  | +0.80 | +1.08 | -0.93 |  |  |  |  |  |  |  |  |  |  | +4.98 |  | +8.7% | +26.9% |  |  |
| firefight_2 | classic | 4.65 | 3.99 |  | 0.29 | 0.73 | 2.97 |  |  |  |  |  |  |  |  |  |  | 0.89 | 190.30 | 520 | 563 | OK:46 | OK |
| firefight_2 | remastered | 6.92 | 4.93 |  | 1.09 | 1.81 | 2.04 | 0.22 |  |  | 1.33 | 0.26 | 0.01 | 0.23 | 0.08 |  | 0.18 | 5.87 | 124.90 | 480 | 460 | UNSETTLED:300 | OK |
| firefight_2 | delta | +2.27 | +0.94 |  | +0.80 | +1.08 | -0.93 |  |  |  |  |  |  |  |  |  |  | +4.98 |  | -7.7% | -18.3% |  |  |
| firefight_3 | classic | 4.65 | 3.99 |  | 0.29 | 0.73 | 2.97 |  |  |  |  |  |  |  |  |  |  | 0.89 | 190.30 | 478 | 371 | OK:46 | OK |
| firefight_3 | remastered | 6.92 | 4.93 |  | 1.09 | 1.81 | 2.04 | 0.22 |  |  | 1.33 | 0.26 | 0.01 | 0.23 | 0.08 |  | 0.18 | 5.87 | 124.90 | 580 | 573 | UNSETTLED:300 | OK |
| firefight_3 | delta | +2.27 | +0.94 |  | +0.80 | +1.08 | -0.93 |  |  |  |  |  |  |  |  |  |  | +4.98 |  | +17.6% | +35.3% |  |  |
| tank_gunner_interior | classic | 5.28 | 5.03 |  | 0.09 | 1.98 | 2.95 |  |  |  |  |  |  |  |  |  |  | 4.44 | 123.90 | 2,529 | 1,216 | OK:30 | OK |
| tank_gunner_interior | remastered | 7.87 | 6.99 | 0.06 | 0.87 | 2.79 | 3.33 | 0.09 |  |  | 0.58 | 0.12 |  | 0.28 | 0.05 |  | 0.09 | 7.80 | 80.00 | 1,532 | 1,231 | OK:30 | OK |
| tank_gunner_interior | delta | +2.59 | +1.96 |  | +0.78 | +0.81 | +0.38 |  |  |  |  |  |  |  |  |  |  | +3.36 |  | -39.4% | +1.2% |  |  |
| tank_gunner_optics | classic | 3.67 | 3.39 |  | 0.12 | 1.63 | 1.63 |  |  |  |  |  |  |  |  |  |  | 2.89 | 165.90 | 1,724 | 743 | OK:30 | OK |
| tank_gunner_optics | remastered | 7.01 | 6.30 | 0.06 | 0.88 | 3.41 | 2.01 | 0.10 |  |  | 0.47 | 0.08 |  | 0.15 |  |  | 0.20 | 6.86 | 113.40 | 1,143 | 742 | OK:30 | OK |
| tank_gunner_optics | delta | +3.34 | +2.91 |  | +0.76 | +1.78 | +0.38 |  |  |  |  |  |  |  |  |  |  | +3.97 |  | -33.7% | -0.1% |  |  |
| overcast_broken | classic | 11.63 | 11.50 |  | 0.15 | 8.67 | 2.67 |  |  |  |  |  |  |  |  |  |  | 11.10 | 51.40 | 8,243 | 5,692 | OK:30 | OK |
| overcast_broken | remastered | 8.72 | 7.21 | 0.14 | 2.21 | 4.54 | 0.47 | 0.10 |  |  | 1.23 | 0.28 | 0.01 | 0.15 | 0.06 |  | 0.17 | 8.39 | 67.00 | 1,863 | 5,083 | OK:30 | OK |
| overcast_broken | delta | -2.91 | -4.29 |  | +2.06 | -4.13 | -2.20 |  |  |  |  |  |  |  |  |  |  | -2.71 |  | -77.4% | -10.7% |  |  |
| overcast_heavy | classic | 11.56 | 11.42 |  | 0.13 | 8.56 | 2.73 |  |  |  |  |  |  |  |  |  |  | 11.04 | 52.10 | 7,957 | 5,692 | OK:30 | OK |
| overcast_heavy | remastered | 8.53 | 6.74 | 0.14 | 1.87 | 4.41 | 0.46 | 0.11 |  |  | 1.50 | 0.27 | 0.01 | 0.15 | 0.06 |  | 0.11 | 8.32 | 67.80 | 1,619 | 5,083 | OK:30 | OK |
| overcast_heavy | delta | -3.03 | -4.68 |  | +1.74 | -4.15 | -2.27 |  |  |  |  |  |  |  |  |  |  | -2.72 |  | -79.7% | -10.7% |  |  |
| woods_pine2_evening | classic | 6.33 | 6.18 |  | 0.13 | 3.13 | 2.92 |  |  |  |  |  |  |  |  |  |  | 5.04 | 108.90 | 3,250 | 1,811 | OK:30 | OK |
| woods_pine2_evening | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| blue_hour_coast | classic | 1.31 | 1.02 |  | 0.59 | 0.00 | 0.42 |  |  |  |  |  |  |  |  |  |  | 0.88 | 321.80 | 182 | 0 | OK:30 | OK |
| blue_hour_coast | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| dawn_valley | classic | 21.74 | 21.60 |  | 3.34 | 15.59 | 2.67 |  |  |  |  |  |  |  |  |  |  | 17.96 | 36.60 | 11,553 | 7,966 | OK:30 | OK |
| dawn_valley | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| pine_rays | classic | 5.29 | 5.10 |  | 0.07 | 2.24 | 2.79 |  |  |  |  |  |  |  |  |  |  | 4.42 | 119.60 | 3,163 | 1,678 | OK:30 | OK |
| pine_rays | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_fog_night | classic | 6.08 | 5.89 |  | 0.09 | 1.57 | 4.23 |  |  |  |  |  |  |  |  |  |  | 4.90 | 138.00 | 1,282 | 1,022 | OK:50 | OK |
| town_fog_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night_rain | classic | 6.15 | 5.96 |  | 0.11 | 1.30 | 4.55 |  |  |  |  |  |  |  |  |  |  | 4.88 | 134.80 | 1,122 | 885 | OK:79 | OK |
| town_night_rain | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| jeep_close_noon | classic | 4.75 | 4.65 |  | 0.12 | 3.26 | 1.27 |  |  |  |  |  |  |  |  |  |  | 4.13 | 125.90 | 2,723 | 1,774 | OK:30 | OK |
| jeep_close_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_noon | classic | 2.39 | 2.15 |  | 0.13 | 0.81 | 1.20 |  |  |  |  |  |  |  |  |  |  | 1.74 | 215.80 | 985 | 541 | OK:30 | OK |
| town_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_morning | classic | 4.98 | 4.82 |  | 0.11 | 2.88 | 1.83 |  |  |  |  |  |  |  |  |  |  | 4.22 | 157.80 | 2,435 | 1,448 | OK:30 | OK |
| forest_morning | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| rain_day | classic | 4.38 | 4.17 |  | 0.12 | 2.20 | 1.85 |  |  |  |  |  |  |  |  |  |  | 2.76 | 148.40 | 1,765 | 796 | OK:30 | OK |
| rain_day | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_vista | classic | 20.71 | 20.58 |  | 6.83 | 11.64 | 2.11 |  |  |  |  |  |  |  |  |  |  | 19.58 | 42.50 | 10,841 | 7,519 | OK:30 | OK |
| noon_vista | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 2.76 | 2.58 |  | 0.14 | 1.17 | 1.28 |  |  |  |  |  |  |  |  |  |  | 1.71 | 190.50 | 603 | 314 | OK:30 | OK |
| sinai_lowsun_fp | remastered | 6.33 | 4.78 | 0.23 | 1.13 | 2.65 | 0.99 | 0.12 |  |  | 1.23 | 0.27 | 0.01 | 0.22 | 0.06 |  | 0.13 | 5.53 | 111.20 | 417 | 314 | UNSETTLED:300 | OK |
| sinai_lowsun_fp | delta | +3.57 | +2.20 |  | +0.99 | +1.48 | -0.29 |  |  |  |  |  |  |  |  |  |  | +3.82 |  | -30.8% | +0.0% |  |  |
| sinai_desert_noon | classic | 5.79 | 5.66 |  | 0.14 | 3.03 | 2.48 |  |  |  |  |  |  |  |  |  |  | 4.69 | 92.50 | 2,243 | 1,100 | OK:30 | OK |
| sinai_desert_noon | remastered | 8.01 | 6.27 | 0.10 | 1.19 | 3.04 | 2.04 | 0.12 |  |  | 1.44 | 0.36 | 0.01 | 0.17 | 0.06 |  | 0.32 | 7.34 | 78.30 | 1,181 | 1,100 | OK:30 | OK |
| sinai_desert_noon | delta | +2.22 | +0.61 |  | +1.05 | +0.01 | -0.44 |  |  |  |  |  |  |  |  |  |  | +2.65 |  | -47.3% | +0.0% |  |  |
