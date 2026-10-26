# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 4.38 | 4.19 |  | 0.15 | 1.72 | 2.32 |  |  |  |  |  |  |  |  |  |  | 3.24 | 149.90 | 2,091 | 1,316 | OK:30 | OK |
| lowsun_fp | remastered | 8.16 | 6.45 | 0.14 | 1.24 | 3.25 | 1.97 | 0.12 |  |  | 1.43 | 0.26 | 0.01 | 0.18 | 0.05 |  | 0.11 | 8.06 | 98.70 | 1,090 | 1,179 | OK:30 | OK |
| lowsun_fp | delta | +3.78 | +2.26 |  | +1.09 | +1.53 | -0.35 |  |  |  |  |  |  |  |  |  |  | +4.82 |  | -47.9% | -10.4% |  |  |
| sunrise_ridge | classic | 25.02 | 24.88 |  | 7.50 | 14.63 | 2.75 |  |  |  |  |  |  |  |  |  |  | 24.42 | 35.90 | 13,076 | 8,897 | OK:30 | OK |
| sunrise_ridge | remastered | 8.59 | 6.96 | 0.10 | 4.58 | 2.07 | 0.31 | 0.17 |  |  | 1.23 | 0.25 | 0.01 | 0.20 | 0.06 |  | 0.12 | 4.89 | 72.40 | 1,659 | 7,680 | OK:30 | OK |
| sunrise_ridge | delta | -16.43 | -17.92 |  | -2.92 | -12.56 | -2.44 |  |  |  |  |  |  |  |  |  |  | -19.53 |  | -87.3% | -13.7% |  |  |
| noon_fields | classic | 8.49 | 8.36 |  | 0.11 | 6.14 | 2.12 |  |  |  |  |  |  |  |  |  |  | 8.37 | 71.50 | 6,767 | 4,508 | OK:30 | OK |
| noon_fields | remastered | 9.13 | 7.78 | 0.10 | 1.52 | 5.82 | 0.44 | 0.11 |  |  | 1.08 | 0.27 | 0.01 | 0.16 | 0.05 |  | 0.11 | 9.00 | 73.80 | 1,447 | 3,920 | OK:30 | OK |
| noon_fields | delta | +0.64 | -0.58 |  | +1.41 | -0.32 | -1.68 |  |  |  |  |  |  |  |  |  |  | +0.63 |  | -78.6% | -13.0% |  |  |
| forest_edge_golden | classic | 5.47 | 5.28 |  | 0.11 | 2.93 | 2.23 |  |  |  |  |  |  |  |  |  |  | 4.46 | 129.20 | 2,502 | 1,459 | OK:30 | OK |
| forest_edge_golden | remastered | 6.80 | 5.12 | 0.20 | 1.45 | 2.84 | 0.83 | 0.16 |  |  | 1.27 | 0.25 | 0.01 | 0.24 | 0.06 |  | 0.12 | 6.67 | 104.30 | 757 | 1,316 | OK:30 | OK |
| forest_edge_golden | delta | +1.33 | -0.16 |  | +1.34 | -0.09 | -1.40 |  |  |  |  |  |  |  |  |  |  | +2.21 |  | -69.7% | -9.8% |  |  |
| coastline_sunset | classic | 1.31 | 1.07 |  | 0.63 | 0.00 | 0.44 |  |  |  |  |  |  |  |  |  |  | 0.89 | 383.60 | 177 | 0 | OK:30 | OK |
| coastline_sunset | remastered | 3.45 | 1.67 | 0.00 | 1.50 | 0.00 | 0.17 | 0.14 |  |  | 1.49 | 0.11 | 0.01 | 0.18 | 0.06 |  | 0.12 | 3.35 | 254.00 | 154 | 0 | OK:30 | OK |
| coastline_sunset | delta | +2.14 | +0.60 |  | +0.87 | +0.00 | -0.27 |  |  |  |  |  |  |  |  |  |  | +2.46 |  | -13.0% |  |  |  |
| town_night | classic | 6.39 | 6.13 |  | 0.10 | 1.43 | 4.60 |  |  |  |  |  |  |  |  |  |  | 5.97 | 118.30 | 1,299 | 1,131 | OK:239 | OK |
| town_night | remastered | 10.63 | 8.83 | 0.14 | 1.06 | 3.33 | 4.45 | 0.05 |  |  | 1.38 | 0.32 | 0.01 | 0.24 | 0.05 |  | 0.10 | 10.30 | 77.90 | 1,177 | 1,067 | OK:30 | OK |
| town_night | delta | +4.24 | +2.70 |  | +0.96 | +1.90 | -0.15 |  |  |  |  |  |  |  |  |  |  | +4.33 |  | -9.4% | -5.7% |  |  |
| firefight_1 | classic | 4.43 | 4.02 |  | 0.24 | 0.63 | 3.15 |  |  |  |  |  |  |  |  |  |  | 0.82 | 202.90 | 458 | 445 | OK:45 | OK |
| firefight_1 | remastered | 6.79 | 4.61 |  | 1.09 | 1.74 | 1.78 | 0.21 |  |  | 1.55 | 0.29 | 0.01 | 0.22 | 0.07 |  | 0.16 | 6.24 | 129.80 | 509 | 517 | UNSETTLED:300 | OK |
| firefight_1 | delta | +2.36 | +0.59 |  | +0.85 | +1.11 | -1.37 |  |  |  |  |  |  |  |  |  |  | +5.42 |  | +10.0% | +13.9% |  |  |
| firefight_2 | classic | 4.43 | 4.02 |  | 0.24 | 0.63 | 3.15 |  |  |  |  |  |  |  |  |  |  | 0.82 | 202.90 | 542 | 525 | OK:45 | OK |
| firefight_2 | remastered | 6.79 | 4.61 |  | 1.09 | 1.74 | 1.78 | 0.21 |  |  | 1.55 | 0.29 | 0.01 | 0.22 | 0.07 |  | 0.16 | 6.24 | 129.80 | 470 | 457 | UNSETTLED:300 | OK |
| firefight_2 | delta | +2.36 | +0.59 |  | +0.85 | +1.11 | -1.37 |  |  |  |  |  |  |  |  |  |  | +5.42 |  | -13.3% | -13.0% |  |  |
| firefight_3 | classic | 4.43 | 4.02 |  | 0.24 | 0.63 | 3.15 |  |  |  |  |  |  |  |  |  |  | 0.82 | 202.90 | 486 | 371 | OK:45 | OK |
| firefight_3 | remastered | 6.79 | 4.61 |  | 1.09 | 1.74 | 1.78 | 0.21 |  |  | 1.55 | 0.29 | 0.01 | 0.22 | 0.07 |  | 0.16 | 6.24 | 129.80 | 443 | 417 | UNSETTLED:300 | OK |
| firefight_3 | delta | +2.36 | +0.59 |  | +0.85 | +1.11 | -1.37 |  |  |  |  |  |  |  |  |  |  | +5.42 |  | -8.8% | +11.0% |  |  |
| tank_gunner_interior | classic | 5.22 | 4.95 |  | 0.10 | 2.08 | 2.76 |  |  |  |  |  |  |  |  |  |  | 4.12 | 134.20 | 2,528 | 1,215 | OK:30 | OK |
| tank_gunner_interior | remastered | 7.84 | 6.95 | 0.06 | 0.86 | 2.77 | 3.31 | 0.09 |  |  | 0.59 | 0.14 |  | 0.28 | 0.05 |  | 0.09 | 7.73 | 83.00 | 1,534 | 1,231 | OK:30 | OK |
| tank_gunner_interior | delta | +2.62 | +2.00 |  | +0.76 | +0.69 | +0.55 |  |  |  |  |  |  |  |  |  |  | +3.61 |  | -39.3% | +1.3% |  |  |
| tank_gunner_optics | classic | 4.92 | 4.53 |  | 0.34 | 2.06 | 2.13 |  |  |  |  |  |  |  |  |  |  | 3.42 | 157.00 | 1,730 | 744 | OK:30 | OK |
| tank_gunner_optics | remastered | 7.35 | 6.63 | 0.07 | 1.00 | 3.36 | 2.27 | 0.11 |  |  | 0.45 | 0.10 |  | 0.17 |  |  | 0.14 | 7.12 | 114.70 | 1,142 | 744 | OK:30 | OK |
| tank_gunner_optics | delta | +2.43 | +2.10 |  | +0.66 | +1.30 | +0.14 |  |  |  |  |  |  |  |  |  |  | +3.70 |  | -34.0% | +0.0% |  |  |
| overcast_broken | classic | 11.59 | 11.46 |  | 0.15 | 8.65 | 2.66 |  |  |  |  |  |  |  |  |  |  | 11.13 | 51.20 | 8,243 | 5,692 | OK:30 | OK |
| overcast_broken | remastered | 8.52 | 7.05 | 0.13 | 2.19 | 4.42 | 0.44 | 0.10 |  |  | 1.20 | 0.28 | 0.01 | 0.15 | 0.05 |  | 0.17 | 8.33 | 67.80 | 1,863 | 5,083 | OK:30 | OK |
| overcast_broken | delta | -3.07 | -4.41 |  | +2.04 | -4.23 | -2.22 |  |  |  |  |  |  |  |  |  |  | -2.80 |  | -77.4% | -10.7% |  |  |
| overcast_heavy | classic | 11.76 | 11.62 |  | 0.12 | 8.72 | 2.78 |  |  |  |  |  |  |  |  |  |  | 11.11 | 51.60 | 7,957 | 5,692 | OK:30 | OK |
| overcast_heavy | remastered | 8.28 | 6.54 | 0.13 | 1.83 | 4.29 | 0.42 | 0.10 |  |  | 1.46 | 0.25 | 0.01 | 0.15 | 0.05 |  | 0.11 | 8.17 | 70.40 | 1,619 | 5,083 | OK:30 | OK |
| overcast_heavy | delta | -3.48 | -5.08 |  | +1.71 | -4.43 | -2.36 |  |  |  |  |  |  |  |  |  |  | -2.94 |  | -79.7% | -10.7% |  |  |
| woods_pine2_evening | classic | 5.57 | 5.41 |  | 0.13 | 3.00 | 2.29 |  |  |  |  |  |  |  |  |  |  | 4.96 | 117.20 | 3,250 | 1,811 | OK:30 | OK |
| woods_pine2_evening | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| blue_hour_coast | classic | 1.32 | 1.03 |  | 0.63 | 0.00 | 0.39 |  |  |  |  |  |  |  |  |  |  | 0.84 | 367.80 | 182 | 0 | OK:30 | OK |
| blue_hour_coast | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| dawn_valley | classic | 19.95 | 19.82 |  | 3.12 | 14.31 | 2.39 |  |  |  |  |  |  |  |  |  |  | 16.45 | 39.30 | 11,553 | 7,966 | OK:30 | OK |
| dawn_valley | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| pine_rays | classic | 5.59 | 5.39 |  | 0.08 | 2.36 | 2.95 |  |  |  |  |  |  |  |  |  |  | 4.33 | 127.10 | 3,163 | 1,678 | OK:30 | OK |
| pine_rays | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_fog_night | classic | 6.12 | 5.95 |  | 0.08 | 1.69 | 4.18 |  |  |  |  |  |  |  |  |  |  | 5.21 | 129.10 | 1,290 | 1,028 | OK:30 | OK |
| town_fog_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night_rain | classic | 7.25 | 7.05 |  | 0.10 | 1.96 | 4.99 |  |  |  |  |  |  |  |  |  |  | 5.65 | 122.50 | 1,106 | 887 | OK:124 | OK |
| town_night_rain | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| jeep_close_noon | classic | 4.32 | 4.22 |  | 0.12 | 2.98 | 1.12 |  |  |  |  |  |  |  |  |  |  | 3.93 | 133.00 | 2,723 | 1,774 | OK:30 | OK |
| jeep_close_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_noon | classic | 2.26 | 2.04 |  | 0.12 | 0.79 | 1.13 |  |  |  |  |  |  |  |  |  |  | 1.73 | 214.60 | 985 | 541 | OK:30 | OK |
| town_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_morning | classic | 4.70 | 4.54 |  | 0.10 | 2.70 | 1.73 |  |  |  |  |  |  |  |  |  |  | 4.15 | 157.20 | 2,435 | 1,448 | OK:30 | OK |
| forest_morning | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| rain_day | classic | 4.29 | 4.11 |  | 0.11 | 2.15 | 1.85 |  |  |  |  |  |  |  |  |  |  | 2.84 | 150.20 | 1,746 | 778 | OK:30 | OK |
| rain_day | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_vista | classic | 20.29 | 20.16 |  | 6.79 | 11.36 | 2.01 |  |  |  |  |  |  |  |  |  |  | 19.16 | 43.10 | 10,841 | 7,519 | OK:30 | OK |
| noon_vista | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 2.63 | 2.44 |  | 0.14 | 1.06 | 1.25 |  |  |  |  |  |  |  |  |  |  | 1.56 | 197.10 | 603 | 314 | OK:30 | OK |
| sinai_lowsun_fp | remastered | 6.48 | 4.92 | 0.22 | 1.20 | 2.78 | 0.94 | 0.12 |  |  | 1.22 | 0.27 | 0.01 | 0.22 | 0.06 |  | 0.11 | 5.93 | 112.10 | 417 | 314 | UNSETTLED:300 | OK |
| sinai_lowsun_fp | delta | +3.85 | +2.48 |  | +1.06 | +1.72 | -0.31 |  |  |  |  |  |  |  |  |  |  | +4.37 |  | -30.8% | +0.0% |  |  |
| sinai_desert_noon | classic | 5.79 | 5.66 |  | 0.14 | 2.97 | 2.54 |  |  |  |  |  |  |  |  |  |  | 4.59 | 94.10 | 2,243 | 1,100 | OK:30 | OK |
| sinai_desert_noon | remastered | 8.09 | 6.35 | 0.12 | 1.19 | 3.15 | 2.01 | 0.11 |  |  | 1.45 | 0.36 | 0.01 | 0.17 | 0.05 |  | 0.29 | 7.34 | 79.00 | 1,181 | 1,100 | OK:30 | OK |
| sinai_desert_noon | delta | +2.30 | +0.69 |  | +1.05 | +0.18 | -0.53 |  |  |  |  |  |  |  |  |  |  | +2.75 |  | -47.3% | +0.0% |  |  |
