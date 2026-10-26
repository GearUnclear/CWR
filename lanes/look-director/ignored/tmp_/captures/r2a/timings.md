# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 48.79 | 48.65 |  | 0.11 | 2.98 | 45.57 |  |  |  |  |  |  |  |  |  |  | 8.30 | 19.60 | 2,091 | 1,316 | OK:30 | OK |
| lowsun_fp | remastered | 16.85 | 13.90 | 0.24 | 2.18 | 6.99 | 4.73 | 0.12 |  |  | 2.67 | 0.47 | 0.01 | 0.19 | 0.34 |  | 0.12 | 8.66 | 57.60 | 1,090 | 1,179 | OK:30 | OK |
| lowsun_fp | delta | -31.94 | -34.75 |  | +2.07 | +4.01 | -40.84 |  |  |  |  |  |  |  |  |  |  | +0.36 |  | -47.9% | -10.4% |  |  |
| sunrise_ridge | classic | 43.35 | 43.20 |  | 9.17 | 28.69 | 5.33 |  |  |  |  |  |  |  |  |  |  | 25.99 | 20.30 | 13,076 | 8,897 | OK:30 | OK |
| sunrise_ridge | remastered | 17.56 | 13.94 | 0.19 | 6.15 | 6.20 | 1.59 | 0.14 |  |  | 2.96 | 0.13 | 0.01 | 0.18 | 0.05 |  | 0.09 | 5.27 | 46.00 | 1,659 | 7,680 | OK:30 | OK |
| sunrise_ridge | delta | -25.79 | -29.26 |  | -3.02 | -22.49 | -3.74 |  |  |  |  |  |  |  |  |  |  | -20.72 |  | -87.3% | -13.7% |  |  |
| noon_fields | classic | 29.33 | 29.20 |  | 0.11 | 21.12 | 7.98 |  |  |  |  |  |  |  |  |  |  | 8.94 | 29.00 | 6,767 | 4,508 | OK:30 | OK |
| noon_fields | remastered | 20.20 | 17.75 | 0.22 | 3.16 | 13.10 | 1.49 | 0.11 |  |  | 2.18 | 0.15 | 0.01 | 0.16 | 0.05 |  | 0.11 | 9.48 | 35.40 | 1,447 | 3,920 | OK:30 | OK |
| noon_fields | delta | -9.13 | -11.45 |  | +3.05 | -8.02 | -6.49 |  |  |  |  |  |  |  |  |  |  | +0.54 |  | -78.6% | -13.0% |  |  |
| forest_edge_golden | classic | 18.55 | 18.39 |  | 0.09 | 9.47 | 8.83 |  |  |  |  |  |  |  |  |  |  | 4.69 | 28.90 | 2,502 | 1,459 | OK:30 | OK |
| forest_edge_golden | remastered | 17.81 | 14.32 | 0.20 | 3.94 | 7.37 | 3.01 | 0.14 |  |  | 2.80 | 0.17 | 0.01 | 0.23 | 0.06 |  | 0.28 | 6.31 | 54.40 | 757 | 1,316 | OK:30 | OK |
| forest_edge_golden | delta | -0.74 | -4.07 |  | +3.85 | -2.10 | -5.82 |  |  |  |  |  |  |  |  |  |  | +1.62 |  | -69.7% | -9.8% |  |  |
| coastline_sunset | classic | 1.26 | 1.14 |  | 0.22 | 0.00 | 0.92 |  |  |  |  |  |  |  |  |  |  | 0.57 | 316.40 | 177 | 0 | OK:30 | OK |
| coastline_sunset | remastered | 7.96 | 4.54 | 0.00 | 3.45 | 0.00 | 1.09 | 0.11 |  |  | 3.18 | 0.13 | 0.01 | 0.16 | 0.46 |  | 0.63 | 2.93 | 155.00 | 154 | 0 | OK:30 | OK |
| coastline_sunset | delta | +6.70 | +3.40 |  | +3.23 | +0.00 | +0.17 |  |  |  |  |  |  |  |  |  |  | +2.36 |  | -13.0% |  |  |  |
| town_night | classic | 14.14 | 13.96 |  | 0.08 | 2.05 | 11.84 |  |  |  |  |  |  |  |  |  |  | 4.86 | 71.20 | 1,305 | 1,124 | OK:138 | OK |
| town_night | remastered | 25.05 | 21.56 | 0.27 | 2.56 | 7.20 | 11.81 | 0.07 |  |  | 2.88 | 0.27 | 0.01 | 0.15 | 0.06 |  | 0.32 | 10.62 | 34.90 | 1,158 | 1,045 | OK:63 | OK |
| town_night | delta | +10.91 | +7.60 |  | +2.48 | +5.15 | -0.03 |  |  |  |  |  |  |  |  |  |  | +5.76 |  | -11.3% | -7.0% |  |  |
| firefight_1 | classic | 2.74 | 2.61 |  | 0.06 | 0.28 | 2.27 |  |  |  |  |  |  |  |  |  |  | 0.90 | 188.50 | 454 | 457 | OK:41 | OK |
| firefight_1 | remastered | 15.39 | 10.92 |  | 1.62 | 4.11 | 5.18 | 0.14 |  |  | 2.99 | 0.12 | 0.01 | 0.16 | 0.05 |  | 0.18 | 4.38 | 75.50 | 533 | 511 | UNSETTLED:300 | OK |
| firefight_1 | delta | +12.65 | +8.31 |  | +1.56 | +3.83 | +2.91 |  |  |  |  |  |  |  |  |  |  | +3.48 |  | +14.8% | +10.6% |  |  |
| firefight_2 | classic | 2.74 | 2.61 |  | 0.06 | 0.28 | 2.27 |  |  |  |  |  |  |  |  |  |  | 0.90 | 188.50 | 570 | 581 | OK:41 | OK |
| firefight_2 | remastered | 15.39 | 10.92 |  | 1.62 | 4.11 | 5.18 | 0.14 |  |  | 2.99 | 0.12 | 0.01 | 0.16 | 0.05 |  | 0.18 | 4.38 | 75.50 | 473 | 478 | UNSETTLED:300 | OK |
| firefight_2 | delta | +12.65 | +8.31 |  | +1.56 | +3.83 | +2.91 |  |  |  |  |  |  |  |  |  |  | +3.48 |  | -17.0% | -17.7% |  |  |
| firefight_3 | classic | 2.74 | 2.61 |  | 0.06 | 0.28 | 2.27 |  |  |  |  |  |  |  |  |  |  | 0.90 | 188.50 | 491 | 453 | OK:41 | OK |
| firefight_3 | remastered | 15.39 | 10.92 |  | 1.62 | 4.11 | 5.18 | 0.14 |  |  | 2.99 | 0.12 | 0.01 | 0.16 | 0.05 |  | 0.18 | 4.38 | 75.50 | 528 | 551 | UNSETTLED:300 | OK |
| firefight_3 | delta | +12.65 | +8.31 |  | +1.56 | +3.83 | +2.91 |  |  |  |  |  |  |  |  |  |  | +3.48 |  | +7.0% | +17.8% |  |  |
| tank_gunner_interior | classic | 11.57 | 10.81 |  | 0.08 | 4.25 | 6.49 |  |  |  |  |  |  |  |  |  |  | 4.61 | 61.10 | 2,542 | 1,223 | OK:30 | OK |
| tank_gunner_interior | remastered | 20.58 | 18.11 | 0.08 | 1.48 | 6.98 | 9.65 | 0.09 |  |  | 1.46 | 0.14 |  | 0.29 | 0.86 |  | 0.13 | 8.35 | 37.80 | 1,549 | 1,223 | OK:30 | OK |
| tank_gunner_interior | delta | +9.01 | +7.30 |  | +1.40 | +2.73 | +3.16 |  |  |  |  |  |  |  |  |  |  | +3.74 |  | -39.1% | +0.0% |  |  |
| tank_gunner_optics | classic | 9.37 | 8.31 |  | 0.09 | 4.09 | 4.13 |  |  |  |  |  |  |  |  |  |  | 2.84 | 91.00 | 1,725 | 742 | OK:30 | OK |
| tank_gunner_optics | remastered | 18.46 | 16.11 | 0.06 | 1.82 | 6.42 | 7.87 | 0.10 |  |  | 0.51 | 0.10 |  | 0.15 |  |  | 0.13 | 6.97 | 56.40 | 1,151 | 738 | OK:30 | OK |
| tank_gunner_optics | delta | +9.09 | +7.80 |  | +1.73 | +2.33 | +3.74 |  |  |  |  |  |  |  |  |  |  | +4.13 |  | -33.3% | -0.5% |  |  |
| overcast_broken | classic | 11.81 | 11.68 |  | 0.15 | 8.77 | 2.75 |  |  |  |  |  |  |  |  |  |  | 11.24 | 48.50 | 8,243 | 5,692 | OK:30 | OK |
| overcast_broken | remastered | 15.20 | 13.08 | 0.16 | 3.99 | 8.21 | 0.87 | 0.10 |  |  | 1.84 | 0.25 | 0.01 | 0.15 | 0.06 |  | 0.21 | 8.84 | 37.60 | 1,863 | 5,083 | OK:30 | OK |
| overcast_broken | delta | +3.39 | +1.40 |  | +3.84 | -0.56 | -1.88 |  |  |  |  |  |  |  |  |  |  | -2.40 |  | -77.4% | -10.7% |  |  |
| overcast_heavy | classic | 22.00 | 21.86 |  | 0.12 | 16.34 | 5.39 |  |  |  |  |  |  |  |  |  |  | 11.25 | 30.20 | 7,957 | 5,692 | OK:30 | OK |
| overcast_heavy | remastered | 19.07 | 16.00 | 0.24 | 4.28 | 10.27 | 1.44 | 0.10 |  |  | 2.79 | 0.19 | 0.01 | 0.15 | 0.07 |  | 0.11 | 8.63 | 42.40 | 1,619 | 5,083 | OK:30 | OK |
| overcast_heavy | delta | -2.93 | -5.86 |  | +4.16 | -6.07 | -3.95 |  |  |  |  |  |  |  |  |  |  | -2.62 |  | -79.7% | -10.7% |  |  |
| woods_pine2_evening | classic | 11.31 | 11.17 |  | 0.11 | 6.57 | 4.49 |  |  |  |  |  |  |  |  |  |  | 5.60 | 46.90 | 3,250 | 1,811 | OK:30 | OK |
| woods_pine2_evening | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| blue_hour_coast | classic | 2.39 | 2.23 |  | 0.23 | 0.00 | 2.00 |  |  |  |  |  |  |  |  |  |  | 0.59 | 250.10 | 182 | 0 | OK:30 | OK |
| blue_hour_coast | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| dawn_valley | classic | 39.94 | 39.80 |  | 3.75 | 30.16 | 5.89 |  |  |  |  |  |  |  |  |  |  | 18.42 | 20.50 | 11,553 | 7,966 | OK:30 | OK |
| dawn_valley | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| pine_rays | classic | 10.88 | 10.71 |  | 0.07 | 4.71 | 5.93 |  |  |  |  |  |  |  |  |  |  | 4.48 | 70.30 | 3,163 | 1,678 | OK:30 | OK |
| pine_rays | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_fog_night | classic | 15.90 | 15.74 |  | 0.07 | 2.42 | 13.25 |  |  |  |  |  |  |  |  |  |  | 5.95 | 66.70 | 1,294 | 1,043 | OK:145 | OK |
| town_fog_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night_rain | classic | 14.53 | 14.37 |  | 0.25 | 1.51 | 12.61 |  |  |  |  |  |  |  |  |  |  | 5.92 | 69.30 | 1,020 | 778 | UNSETTLED:300 | OK |
| town_night_rain | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| jeep_close_noon | classic | 13.93 | 13.83 |  | 0.11 | 9.65 | 4.07 |  |  |  |  |  |  |  |  |  |  | 4.02 | 68.90 | 2,723 | 1,774 | OK:30 | OK |
| jeep_close_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_noon | classic | 5.35 | 5.20 |  | 0.08 | 1.56 | 3.57 |  |  |  |  |  |  |  |  |  |  | 1.84 | 148.10 | 985 | 541 | OK:30 | OK |
| town_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_morning | classic | 7.79 | 7.57 |  | 0.11 | 4.25 | 3.21 |  |  |  |  |  |  |  |  |  |  | 4.24 | 141.20 | 2,435 | 1,448 | OK:30 | OK |
| forest_morning | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| rain_day | classic | 8.01 | 7.87 |  | 0.08 | 3.71 | 4.07 |  |  |  |  |  |  |  |  |  |  | 2.93 | 91.40 | 1,756 | 788 | OK:32 | OK |
| rain_day | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_vista | classic | 37.68 | 37.55 |  | 7.32 | 25.15 | 5.08 |  |  |  |  |  |  |  |  |  |  | 20.49 | 24.50 | 10,841 | 7,519 | OK:30 | OK |
| noon_vista | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 5.58 | 5.46 |  | 0.08 | 1.57 | 3.82 |  |  |  |  |  |  |  |  |  |  | 2.15 | 135.20 | 603 | 314 | OK:30 | OK |
| sinai_lowsun_fp | remastered | 17.19 | 13.74 | 0.62 | 2.33 | 7.61 | 3.81 | 0.12 |  |  | 2.64 | 0.17 | 0.01 | 0.26 | 0.11 |  | 0.18 | 5.82 | 60.80 | 417 | 314 | UNSETTLED:300 | OK |
| sinai_lowsun_fp | delta | +11.61 | +8.28 |  | +2.25 | +6.04 | -0.01 |  |  |  |  |  |  |  |  |  |  | +3.67 |  | -30.8% | +0.0% |  |  |
| sinai_desert_noon | classic | 6.46 | 6.35 |  | 0.12 | 3.28 | 2.95 |  |  |  |  |  |  |  |  |  |  | 4.87 | 68.30 | 2,243 | 1,100 | OK:30 | OK |
| sinai_desert_noon | remastered | 17.80 | 14.90 | 0.58 | 2.39 | 6.90 | 5.61 | 0.11 |  |  | 2.60 | 0.17 | 0.01 | 0.17 | 0.06 |  | 0.17 | 7.62 | 52.00 | 1,181 | 1,100 | OK:30 | OK |
| sinai_desert_noon | delta | +11.34 | +8.55 |  | +2.27 | +3.62 | +2.66 |  |  |  |  |  |  |  |  |  |  | +2.75 |  | -47.3% | +0.0% |  |  |
