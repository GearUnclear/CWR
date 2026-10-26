# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 4.27 | 4.11 |  | 0.13 | 1.69 | 2.29 |  |  |  |  |  |  |  |  |  |  | 3.40 | 144.00 | 2,091 | 1,316 | OK:30 | OK |
| lowsun_fp | remastered | 8.56 | 6.76 | 0.14 | 1.27 | 3.38 | 2.11 | 0.12 |  |  | 1.51 | 0.29 | 0.01 | 0.18 | 0.08 |  | 0.11 | 8.13 | 94.50 | 1,090 | 1,179 | OK:30 | OK |
| lowsun_fp | delta | +4.29 | +2.65 |  | +1.14 | +1.69 | -0.18 |  |  |  |  |  |  |  |  |  |  | +4.73 |  | -47.9% | -10.4% |  |  |
| sunrise_ridge | classic | 26.23 | 26.08 |  | 7.83 | 15.36 | 2.89 |  |  |  |  |  |  |  |  |  |  | 25.59 | 34.40 | 13,076 | 8,897 | OK:30 | OK |
| sunrise_ridge | remastered | 9.08 | 7.43 | 0.10 | 5.03 | 2.10 | 0.30 | 0.16 |  |  | 1.23 | 0.26 | 0.01 | 0.20 | 0.06 |  | 0.11 | 4.91 | 69.70 | 1,659 | 7,680 | OK:30 | OK |
| sunrise_ridge | delta | -17.15 | -18.65 |  | -2.80 | -13.26 | -2.59 |  |  |  |  |  |  |  |  |  |  | -20.68 |  | -87.3% | -13.7% |  |  |
| noon_fields | classic | 8.86 | 8.73 |  | 0.11 | 6.41 | 2.21 |  |  |  |  |  |  |  |  |  |  | 8.72 | 68.40 | 6,767 | 4,508 | OK:30 | OK |
| noon_fields | remastered | 9.46 | 8.09 | 0.10 | 1.57 | 6.00 | 0.52 | 0.11 |  |  | 1.11 | 0.28 | 0.01 | 0.16 | 0.05 |  | 0.11 | 9.28 | 70.70 | 1,447 | 3,920 | OK:30 | OK |
| noon_fields | delta | +0.60 | -0.64 |  | +1.46 | -0.41 | -1.69 |  |  |  |  |  |  |  |  |  |  | +0.56 |  | -78.6% | -13.0% |  |  |
| forest_edge_golden | classic | 5.07 | 4.89 |  | 0.10 | 2.75 | 2.05 |  |  |  |  |  |  |  |  |  |  | 4.46 | 124.50 | 2,502 | 1,459 | OK:30 | OK |
| forest_edge_golden | remastered | 6.47 | 4.83 | 0.19 | 1.35 | 2.66 | 0.82 | 0.14 |  |  | 1.28 | 0.29 | 0.01 | 0.22 | 0.05 |  | 0.12 | 6.12 | 104.90 | 757 | 1,316 | OK:30 | OK |
| forest_edge_golden | delta | +1.40 | -0.06 |  | +1.25 | -0.09 | -1.23 |  |  |  |  |  |  |  |  |  |  | +1.66 |  | -69.7% | -9.8% |  |  |
| coastline_sunset | classic | 1.31 | 1.09 |  | 0.58 | 0.00 | 0.51 |  |  |  |  |  |  |  |  |  |  | 0.81 | 358.10 | 177 | 0 | OK:30 | OK |
| coastline_sunset | remastered | 3.32 | 1.60 | 0.00 | 1.37 | 0.00 | 0.23 | 0.13 |  |  | 1.45 | 0.11 | 0.01 | 0.17 | 0.06 |  | 0.11 | 3.10 | 251.90 | 154 | 0 | OK:30 | OK |
| coastline_sunset | delta | +2.01 | +0.51 |  | +0.79 | +0.00 | -0.28 |  |  |  |  |  |  |  |  |  |  | +2.29 |  | -13.0% |  |  |  |
| town_night | classic | 6.70 | 6.43 |  | 0.14 | 1.29 | 5.01 |  |  |  |  |  |  |  |  |  |  | 5.86 | 118.70 | 1,236 | 1,068 | OK:254 | OK |
| town_night | remastered | 10.48 | 8.94 | 0.19 | 1.10 | 3.43 | 4.42 | 0.05 |  |  | 1.20 | 0.26 | 0.01 | 0.15 | 0.05 |  | 0.10 | 10.25 | 75.40 | 1,117 | 1,018 | OK:68 | OK |
| town_night | delta | +3.78 | +2.51 |  | +0.96 | +2.14 | -0.59 |  |  |  |  |  |  |  |  |  |  | +4.39 |  | -9.6% | -4.7% |  |  |
| firefight_1 | classic | 4.14 | 3.74 |  | 0.25 | 0.59 | 2.90 |  |  |  |  |  |  |  |  |  |  | 0.82 | 197.20 | 446 | 411 | OK:46 | OK |
| firefight_1 | remastered | 6.43 | 4.57 |  | 0.98 | 1.77 | 1.82 | 0.19 |  |  | 1.28 | 0.28 | 0.01 | 0.20 | 0.07 |  | 0.19 | 5.53 | 129.60 | 503 | 528 | UNSETTLED:300 | OK |
| firefight_1 | delta | +2.29 | +0.83 |  | +0.73 | +1.18 | -1.08 |  |  |  |  |  |  |  |  |  |  | +4.71 |  | +11.3% | +22.2% |  |  |
| firefight_2 | classic | 4.14 | 3.74 |  | 0.25 | 0.59 | 2.90 |  |  |  |  |  |  |  |  |  |  | 0.82 | 197.20 | 550 | 518 | OK:46 | OK |
| firefight_2 | remastered | 6.43 | 4.57 |  | 0.98 | 1.77 | 1.82 | 0.19 |  |  | 1.28 | 0.28 | 0.01 | 0.20 | 0.07 |  | 0.19 | 5.53 | 129.60 | 463 | 481 | UNSETTLED:300 | OK |
| firefight_2 | delta | +2.29 | +0.83 |  | +0.73 | +1.18 | -1.08 |  |  |  |  |  |  |  |  |  |  | +4.71 |  | -15.8% | -7.1% |  |  |
| firefight_3 | classic | 4.14 | 3.74 |  | 0.25 | 0.59 | 2.90 |  |  |  |  |  |  |  |  |  |  | 0.82 | 197.20 | 479 | 394 | OK:46 | OK |
| firefight_3 | remastered | 6.43 | 4.57 |  | 0.98 | 1.77 | 1.82 | 0.19 |  |  | 1.28 | 0.28 | 0.01 | 0.20 | 0.07 |  | 0.19 | 5.53 | 129.60 | 549 | 521 | UNSETTLED:300 | OK |
| firefight_3 | delta | +2.29 | +0.83 |  | +0.73 | +1.18 | -1.08 |  |  |  |  |  |  |  |  |  |  | +4.71 |  | +12.8% | +24.4% |  |  |
| tank_gunner_interior | classic | 5.40 | 5.12 |  | 0.09 | 2.08 | 2.95 |  |  |  |  |  |  |  |  |  |  | 4.33 | 125.90 | 2,528 | 1,218 | OK:30 | OK |
| tank_gunner_interior | remastered | 8.29 | 7.37 | 0.12 | 0.89 | 2.82 | 3.66 | 0.09 |  |  | 0.60 | 0.14 |  | 0.28 | 0.05 |  | 0.10 | 8.11 | 77.60 | 1,535 | 1,233 | OK:30 | OK |
| tank_gunner_interior | delta | +2.89 | +2.25 |  | +0.80 | +0.74 | +0.71 |  |  |  |  |  |  |  |  |  |  | +3.78 |  | -39.3% | +1.2% |  |  |
| tank_gunner_optics | classic | 3.72 | 3.45 |  | 0.12 | 1.67 | 1.66 |  |  |  |  |  |  |  |  |  |  | 2.87 | 162.60 | 1,732 | 746 | OK:30 | OK |
| tank_gunner_optics | remastered | 7.53 | 6.76 | 0.06 | 0.93 | 3.44 | 2.39 | 0.11 |  |  | 0.51 | 0.09 |  | 0.16 |  |  | 0.22 | 7.04 | 109.50 | 1,148 | 746 | OK:30 | OK |
| tank_gunner_optics | delta | +3.81 | +3.31 |  | +0.81 | +1.77 | +0.73 |  |  |  |  |  |  |  |  |  |  | +4.17 |  | -33.7% | +0.0% |  |  |
| overcast_broken | classic | 11.70 | 11.57 |  | 0.15 | 8.65 | 2.77 |  |  |  |  |  |  |  |  |  |  | 11.20 | 51.50 | 8,243 | 5,692 | OK:30 | OK |
| overcast_broken | remastered | 8.94 | 7.42 | 0.15 | 2.27 | 4.70 | 0.46 | 0.10 |  |  | 1.24 | 0.26 | 0.01 | 0.15 | 0.06 |  | 0.18 | 8.67 | 65.30 | 1,863 | 5,083 | OK:30 | OK |
| overcast_broken | delta | -2.76 | -4.15 |  | +2.12 | -3.95 | -2.31 |  |  |  |  |  |  |  |  |  |  | -2.53 |  | -77.4% | -10.7% |  |  |
| overcast_heavy | classic | 11.73 | 11.59 |  | 0.13 | 8.71 | 2.75 |  |  |  |  |  |  |  |  |  |  | 10.94 | 52.10 | 7,957 | 5,692 | OK:30 | OK |
| overcast_heavy | remastered | 8.71 | 6.93 | 0.15 | 1.84 | 4.64 | 0.44 | 0.10 |  |  | 1.50 | 0.27 | 0.01 | 0.15 | 0.06 |  | 0.11 | 8.55 | 68.40 | 1,619 | 5,083 | OK:30 | OK |
| overcast_heavy | delta | -3.02 | -4.66 |  | +1.71 | -4.07 | -2.31 |  |  |  |  |  |  |  |  |  |  | -2.39 |  | -79.7% | -10.7% |  |  |
| optics | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| optics | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| woods_pine2_evening | classic | 5.75 | 5.60 |  | 0.13 | 3.07 | 2.40 |  |  |  |  |  |  |  |  |  |  | 5.00 | 112.00 | 3,250 | 1,811 | OK:30 | OK |
| woods_pine2_evening | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| blue_hour_coast | classic | 1.34 | 1.06 |  | 0.62 | 0.00 | 0.45 |  |  |  |  |  |  |  |  |  |  | 0.76 | 337.00 | 182 | 0 | OK:30 | OK |
| blue_hour_coast | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| dawn_valley | classic | 21.00 | 20.86 |  | 3.26 | 15.03 | 2.57 |  |  |  |  |  |  |  |  |  |  | 17.36 | 37.70 | 11,553 | 7,966 | OK:30 | OK |
| dawn_valley | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| pine_rays | classic | 5.48 | 5.28 |  | 0.08 | 2.23 | 2.97 |  |  |  |  |  |  |  |  |  |  | 4.38 | 122.00 | 3,163 | 1,678 | OK:30 | OK |
| pine_rays | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_fog_night | classic | 5.79 | 5.61 |  | 0.08 | 1.60 | 3.93 |  |  |  |  |  |  |  |  |  |  | 4.82 | 134.90 | 1,201 | 932 | OK:39 | OK |
| town_fog_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night_rain | classic | 4.25 | 3.96 |  | 0.17 | 0.67 | 3.12 |  |  |  |  |  |  |  |  |  |  | 3.71 | 156.10 | 843 | 571 | OK:174 | OK |
| town_night_rain | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| jeep_close_noon | classic | 4.96 | 4.85 |  | 0.12 | 3.43 | 1.30 |  |  |  |  |  |  |  |  |  |  | 4.09 | 126.50 | 2,723 | 1,774 | OK:30 | OK |
| jeep_close_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_noon | classic | 2.40 | 2.19 |  | 0.12 | 0.89 | 1.18 |  |  |  |  |  |  |  |  |  |  | 1.70 | 220.10 | 985 | 541 | OK:30 | OK |
| town_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_morning | classic | 4.81 | 4.65 |  | 0.10 | 2.82 | 1.72 |  |  |  |  |  |  |  |  |  |  | 4.15 | 162.60 | 2,435 | 1,448 | OK:30 | OK |
| forest_morning | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| rain_day | classic | 4.20 | 4.00 |  | 0.12 | 2.01 | 1.87 |  |  |  |  |  |  |  |  |  |  | 2.75 | 156.40 | 1,765 | 796 | OK:30 | OK |
| rain_day | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_vista | classic | 20.44 | 20.31 |  | 6.75 | 11.42 | 2.14 |  |  |  |  |  |  |  |  |  |  | 19.35 | 43.00 | 10,841 | 7,519 | OK:30 | OK |
| noon_vista | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 2.72 | 2.54 |  | 0.13 | 1.12 | 1.29 |  |  |  |  |  |  |  |  |  |  | 1.71 | 188.10 | 603 | 314 | OK:30 | OK |
| sinai_lowsun_fp | remastered | 6.47 | 4.81 | 0.24 | 1.18 | 2.64 | 0.99 | 0.13 |  |  | 1.33 | 0.29 | 0.01 | 0.22 | 0.06 |  | 0.12 | 6.05 | 111.70 | 417 | 314 | UNSETTLED:300 | OK |
| sinai_lowsun_fp | delta | +3.75 | +2.27 |  | +1.05 | +1.52 | -0.30 |  |  |  |  |  |  |  |  |  |  | +4.34 |  | -30.8% | +0.0% |  |  |
| sinai_desert_noon | classic | 5.73 | 5.60 |  | 0.15 | 2.94 | 2.51 |  |  |  |  |  |  |  |  |  |  | 4.59 | 95.20 | 2,243 | 1,100 | OK:30 | OK |
| sinai_desert_noon | remastered | 7.96 | 6.34 | 0.10 | 1.15 | 3.12 | 2.07 | 0.12 |  |  | 1.32 | 0.32 | 0.01 | 0.17 | 0.05 |  | 0.25 | 7.32 | 79.60 | 1,181 | 1,100 | OK:30 | OK |
| sinai_desert_noon | delta | +2.23 | +0.74 |  | +1.00 | +0.18 | -0.44 |  |  |  |  |  |  |  |  |  |  | +2.73 |  | -47.3% | +0.0% |  |  |
