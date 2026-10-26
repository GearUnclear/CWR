# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 4.32 | 4.17 |  | 0.14 | 1.73 | 2.30 |  |  |  |  |  |  |  |  |  |  | 3.37 | 139.90 | 2,091 | 1,316 | OK:30 | OK |
| lowsun_fp | remastered | 8.51 | 6.77 | 0.14 | 1.25 | 3.47 | 2.05 | 0.12 |  |  | 1.46 | 0.28 | 0.01 | 0.18 | 0.06 |  | 0.11 | 8.21 | 93.30 | 1,090 | 1,179 | OK:30 | OK |
| lowsun_fp | delta | +4.19 | +2.60 |  | +1.11 | +1.74 | -0.25 |  |  |  |  |  |  |  |  |  |  | +4.84 |  | -47.9% | -10.4% |  |  |
| sunrise_ridge | classic | 25.58 | 25.43 |  | 7.74 | 14.83 | 2.86 |  |  |  |  |  |  |  |  |  |  | 24.62 | 35.00 | 13,076 | 8,897 | OK:30 | OK |
| sunrise_ridge | remastered | 8.99 | 7.36 | 0.10 | 5.01 | 2.06 | 0.30 | 0.16 |  |  | 1.23 | 0.24 | 0.01 | 0.20 | 0.06 |  | 0.12 | 4.92 | 70.50 | 1,659 | 7,680 | OK:30 | OK |
| sunrise_ridge | delta | -16.59 | -18.07 |  | -2.73 | -12.77 | -2.56 |  |  |  |  |  |  |  |  |  |  | -19.70 |  | -87.3% | -13.7% |  |  |
| noon_fields | classic | 8.57 | 8.44 |  | 0.11 | 6.17 | 2.17 |  |  |  |  |  |  |  |  |  |  | 8.35 | 68.40 | 6,767 | 4,508 | OK:30 | OK |
| noon_fields | remastered | 9.55 | 8.13 | 0.10 | 1.64 | 6.00 | 0.49 | 0.11 |  |  | 1.16 | 0.27 | 0.01 | 0.16 | 0.05 |  | 0.11 | 9.24 | 71.10 | 1,447 | 3,920 | OK:30 | OK |
| noon_fields | delta | +0.98 | -0.31 |  | +1.53 | -0.17 | -1.68 |  |  |  |  |  |  |  |  |  |  | +0.89 |  | -78.6% | -13.0% |  |  |
| forest_edge_golden | classic | 5.71 | 5.53 |  | 0.11 | 3.13 | 2.30 |  |  |  |  |  |  |  |  |  |  | 4.49 | 117.30 | 2,502 | 1,459 | OK:30 | OK |
| forest_edge_golden | remastered | 6.60 | 4.98 | 0.19 | 1.37 | 2.78 | 0.82 | 0.15 |  |  | 1.24 | 0.28 | 0.01 | 0.22 | 0.05 |  | 0.12 | 6.26 | 104.20 | 757 | 1,316 | OK:30 | OK |
| forest_edge_golden | delta | +0.89 | -0.55 |  | +1.26 | -0.35 | -1.48 |  |  |  |  |  |  |  |  |  |  | +1.77 |  | -69.7% | -9.8% |  |  |
| coastline_sunset | classic | 1.25 | 1.04 |  | 0.57 | 0.00 | 0.46 |  |  |  |  |  |  |  |  |  |  | 0.82 | 351.80 | 177 | 0 | OK:30 | OK |
| coastline_sunset | remastered | 3.30 | 1.60 | 0.00 | 1.41 | 0.00 | 0.18 | 0.12 |  |  | 1.44 | 0.12 | 0.01 | 0.17 | 0.06 |  | 0.11 | 3.08 | 234.00 | 154 | 0 | OK:30 | OK |
| coastline_sunset | delta | +2.05 | +0.56 |  | +0.84 | +0.00 | -0.28 |  |  |  |  |  |  |  |  |  |  | +2.26 |  | -13.0% |  |  |  |
| town_night | classic | 6.18 | 5.90 |  | 0.14 | 1.18 | 4.57 |  |  |  |  |  |  |  |  |  |  | 5.73 | 124.40 | 1,216 | 1,037 | OK:189 | OK |
| town_night | remastered | 10.50 | 8.93 | 0.16 | 1.21 | 3.43 | 4.29 | 0.05 |  |  | 1.24 | 0.31 | 0.01 | 0.15 | 0.05 |  | 0.10 | 10.17 | 76.20 | 1,122 | 1,016 | OK:85 | OK |
| town_night | delta | +4.32 | +3.03 |  | +1.07 | +2.25 | -0.28 |  |  |  |  |  |  |  |  |  |  | +4.44 |  | -7.7% | -2.0% |  |  |
| firefight_1 | classic | 4.33 | 3.90 |  | 0.23 | 0.60 | 3.06 |  |  |  |  |  |  |  |  |  |  | 0.83 | 198.50 | 446 | 398 | OK:46 | OK |
| firefight_1 | remastered | 6.55 | 4.56 |  | 0.97 | 1.78 | 1.81 | 0.20 |  |  | 1.37 | 0.31 | 0.01 | 0.21 | 0.07 |  | 0.20 | 5.51 | 129.30 | 440 | 393 | UNSETTLED:300 | OK |
| firefight_1 | delta | +2.22 | +0.66 |  | +0.74 | +1.18 | -1.25 |  |  |  |  |  |  |  |  |  |  | +4.68 |  | -1.3% | -1.3% |  |  |
| firefight_2 | classic | 4.33 | 3.90 |  | 0.23 | 0.60 | 3.06 |  |  |  |  |  |  |  |  |  |  | 0.83 | 198.50 | 539 | 506 | OK:46 | OK |
| firefight_2 | remastered | 6.55 | 4.56 |  | 0.97 | 1.78 | 1.81 | 0.20 |  |  | 1.37 | 0.31 | 0.01 | 0.21 | 0.07 |  | 0.20 | 5.51 | 129.30 | 472 | 472 | UNSETTLED:300 | OK |
| firefight_2 | delta | +2.22 | +0.66 |  | +0.74 | +1.18 | -1.25 |  |  |  |  |  |  |  |  |  |  | +4.68 |  | -12.4% | -6.7% |  |  |
| firefight_3 | classic | 4.33 | 3.90 |  | 0.23 | 0.60 | 3.06 |  |  |  |  |  |  |  |  |  |  | 0.83 | 198.50 | 456 | 375 | OK:46 | OK |
| firefight_3 | remastered | 6.55 | 4.56 |  | 0.97 | 1.78 | 1.81 | 0.20 |  |  | 1.37 | 0.31 | 0.01 | 0.21 | 0.07 |  | 0.20 | 5.51 | 129.30 | 491 | 468 | UNSETTLED:300 | OK |
| firefight_3 | delta | +2.22 | +0.66 |  | +0.74 | +1.18 | -1.25 |  |  |  |  |  |  |  |  |  |  | +4.68 |  | +7.1% | +19.9% |  |  |
| tank_gunner_interior | classic | 5.25 | 4.92 |  | 0.09 | 2.07 | 2.75 |  |  |  |  |  |  |  |  |  |  | 4.34 | 124.50 | 2,534 | 1,221 | OK:30 | OK |
| tank_gunner_interior | remastered | 8.26 | 7.35 | 0.06 | 0.88 | 3.01 | 3.46 | 0.09 |  |  | 0.59 | 0.13 |  | 0.28 | 0.05 |  | 0.10 | 8.05 | 79.20 | 1,539 | 1,236 | OK:30 | OK |
| tank_gunner_interior | delta | +3.01 | +2.43 |  | +0.79 | +0.94 | +0.71 |  |  |  |  |  |  |  |  |  |  | +3.71 |  | -39.3% | +1.2% |  |  |
| tank_gunner_optics | classic | 3.81 | 3.49 |  | 0.12 | 1.66 | 1.70 |  |  |  |  |  |  |  |  |  |  | 2.87 | 162.50 | 1,733 | 747 | OK:30 | OK |
| tank_gunner_optics | remastered | 7.60 | 6.79 | 0.11 | 0.96 | 3.49 | 2.35 | 0.11 |  |  | 0.53 | 0.09 |  | 0.16 |  |  | 0.23 | 7.18 | 108.90 | 1,145 | 747 | OK:30 | OK |
| tank_gunner_optics | delta | +3.79 | +3.30 |  | +0.84 | +1.83 | +0.65 |  |  |  |  |  |  |  |  |  |  | +4.31 |  | -33.9% | +0.0% |  |  |
| overcast_broken | classic | 11.02 | 10.89 |  | 0.15 | 8.23 | 2.51 |  |  |  |  |  |  |  |  |  |  | 10.80 | 53.60 | 8,243 | 5,692 | OK:30 | OK |
| overcast_broken | remastered | 8.90 | 7.33 | 0.15 | 2.31 | 4.57 | 0.45 | 0.11 |  |  | 1.30 | 0.26 | 0.01 | 0.15 | 0.06 |  | 0.19 | 8.62 | 65.70 | 1,863 | 5,083 | OK:30 | OK |
| overcast_broken | delta | -2.12 | -3.56 |  | +2.16 | -3.66 | -2.06 |  |  |  |  |  |  |  |  |  |  | -2.18 |  | -77.4% | -10.7% |  |  |
| overcast_heavy | classic | 11.61 | 11.47 |  | 0.12 | 8.62 | 2.73 |  |  |  |  |  |  |  |  |  |  | 11.09 | 52.60 | 7,957 | 5,692 | OK:30 | OK |
| overcast_heavy | remastered | 8.72 | 6.73 | 0.14 | 1.87 | 4.38 | 0.48 | 0.11 |  |  | 1.70 | 0.32 | 0.01 | 0.15 | 0.11 |  | 0.11 | 8.42 | 68.00 | 1,619 | 5,083 | OK:30 | OK |
| overcast_heavy | delta | -2.89 | -4.74 |  | +1.75 | -4.24 | -2.25 |  |  |  |  |  |  |  |  |  |  | -2.67 |  | -79.7% | -10.7% |  |  |
| woods_pine2_evening | classic | 5.80 | 5.65 |  | 0.13 | 3.10 | 2.42 |  |  |  |  |  |  |  |  |  |  | 5.00 | 113.70 | 3,250 | 1,811 | OK:30 | OK |
| woods_pine2_evening | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| blue_hour_coast | classic | 1.39 | 1.10 |  | 0.63 | 0.00 | 0.47 |  |  |  |  |  |  |  |  |  |  | 0.87 | 333.60 | 182 | 0 | OK:30 | OK |
| blue_hour_coast | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| dawn_valley | classic | 20.58 | 20.44 |  | 3.29 | 14.63 | 2.52 |  |  |  |  |  |  |  |  |  |  | 16.90 | 38.50 | 11,553 | 7,966 | OK:30 | OK |
| dawn_valley | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| pine_rays | classic | 5.40 | 5.20 |  | 0.08 | 2.27 | 2.85 |  |  |  |  |  |  |  |  |  |  | 4.34 | 123.70 | 3,163 | 1,678 | OK:30 | OK |
| pine_rays | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_fog_night | classic | 5.65 | 5.46 |  | 0.09 | 1.52 | 3.85 |  |  |  |  |  |  |  |  |  |  | 4.85 | 142.60 | 1,259 | 990 | OK:30 | OK |
| town_fog_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night_rain | classic | 6.27 | 6.06 |  | 0.09 | 1.24 | 4.74 |  |  |  |  |  |  |  |  |  |  | 5.35 | 133.30 | 1,101 | 861 | OK:121 | OK |
| town_night_rain | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| jeep_close_noon | classic | 4.92 | 4.81 |  | 0.12 | 3.37 | 1.32 |  |  |  |  |  |  |  |  |  |  | 4.07 | 130.40 | 2,723 | 1,774 | OK:30 | OK |
| jeep_close_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_noon | classic | 2.36 | 2.11 |  | 0.14 | 0.81 | 1.15 |  |  |  |  |  |  |  |  |  |  | 1.61 | 239.80 | 985 | 541 | OK:30 | OK |
| town_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_morning | classic | 5.15 | 4.97 |  | 0.12 | 2.99 | 1.87 |  |  |  |  |  |  |  |  |  |  | 4.15 | 171.00 | 2,435 | 1,448 | OK:30 | OK |
| forest_morning | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| rain_day | classic | 4.12 | 3.94 |  | 0.11 | 2.06 | 1.76 |  |  |  |  |  |  |  |  |  |  | 2.84 | 153.80 | 1,746 | 778 | OK:30 | OK |
| rain_day | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_vista | classic | 20.97 | 20.84 |  | 6.93 | 11.75 | 2.16 |  |  |  |  |  |  |  |  |  |  | 19.32 | 41.80 | 10,841 | 7,519 | OK:30 | OK |
| noon_vista | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 2.67 | 2.50 |  | 0.13 | 1.10 | 1.27 |  |  |  |  |  |  |  |  |  |  | 1.68 | 194.40 | 603 | 314 | OK:30 | OK |
| sinai_lowsun_fp | remastered | 6.28 | 4.64 | 0.21 | 1.14 | 2.58 | 0.92 | 0.12 |  |  | 1.32 | 0.28 | 0.01 | 0.21 | 0.05 |  | 0.11 | 5.82 | 110.40 | 417 | 314 | UNSETTLED:300 | OK |
| sinai_lowsun_fp | delta | +3.61 | +2.14 |  | +1.01 | +1.48 | -0.35 |  |  |  |  |  |  |  |  |  |  | +4.14 |  | -30.8% | +0.0% |  |  |
| sinai_desert_noon | classic | 6.00 | 5.87 |  | 0.14 | 3.12 | 2.60 |  |  |  |  |  |  |  |  |  |  | 4.78 | 92.60 | 2,243 | 1,100 | OK:30 | OK |
| sinai_desert_noon | remastered | 8.01 | 6.35 | 0.10 | 1.25 | 3.16 | 1.95 | 0.12 |  |  | 1.37 | 0.32 | 0.01 | 0.17 | 0.05 |  | 0.27 | 7.51 | 78.60 | 1,181 | 1,100 | OK:30 | OK |
| sinai_desert_noon | delta | +2.01 | +0.48 |  | +1.11 | +0.04 | -0.65 |  |  |  |  |  |  |  |  |  |  | +2.73 |  | -47.3% | +0.0% |  |  |
